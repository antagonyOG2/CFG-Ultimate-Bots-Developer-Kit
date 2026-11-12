#pragma once
#include "OwnedPointerCell.hpp"
#include "../RuntimeProbe/SandboxMenuContinuationObservation.hpp"
namespace cfgmenu::mapcontinuation {
template<class Reader> class NativeHost {
    Reader& r_;const Binding& b_;
public:
    NativeHost(Reader& reader,const Binding& binding):r_(reader),b_(binding){}
    bool Observe(continuation::Plan& out){
        out={};if(!Current(b_.plan))return false;
        for(unsigned i=0;i<b_.plan.count;++i){std::uintptr_t value=0;if(!r_.Read(b_.plan.cells[i].address,value)||value!=b_.plan.cells[i].previous)return false;}
        out=b_.plan;return true;
    }
    bool Current(const continuation::Plan& currentPlan){
        if(currentPlan.settingsClass!=b_.settings||currentPlan.count!=b_.plan.count)return false;
        for(unsigned i=0;i<currentPlan.count;++i)if(currentPlan.cells[i].address!=b_.plan.cells[i].address||currentPlan.cells[i].previous!=b_.plan.cells[i].previous)return false;
        return mapcontinuation::Current(r_,b_,GetCurrentThreadId());
    }
    bool Published(const continuation::Plan& currentPlan){
        if(!Current(currentPlan))return false;
        for(unsigned i=0;i<currentPlan.count;++i){std::uintptr_t value=0;
            if(!r_.Read(currentPlan.cells[i].address,value)||value!=b_.settings)return false;
        }
        return true;
    }
    continuation::Write Exchange(const continuation::Cell& cell,std::uintptr_t expected,std::uintptr_t replacement)noexcept{
        using W=continuation::Write;bool owned=false;
        if(GetCurrentThreadId()!=b_.context.thread)return W::Refused;
        for(unsigned i=0;i<b_.plan.count;++i){const auto& bound=b_.plan.cells[i];
            if(bound.address==cell.address&&bound.previous==cell.previous&&
               ((expected==cell.previous&&replacement==b_.settings)||(expected==b_.settings&&replacement==cell.previous)))owned=true;
        }
        if(!owned)return W::Refused;
        const auto result=ExchangeOwnedNullablePointer(cell.address,expected,replacement);
        if(result==sandboxstart::WriteResult::Refused)return W::Refused;
        if(result==sandboxstart::WriteResult::Indeterminate)return W::Indeterminate;
        std::uintptr_t value=0;
        return r_.Read(cell.address,value)&&value==replacement?W::Changed:W::Indeterminate;
    }
};
}
