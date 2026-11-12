#pragma once
#include "KnifeNotifyCompletion.hpp"
namespace cfgknife {
struct ReleaseDiagnostics {unsigned stage=0,failure=0,reads=0,calls=0,entryReads=0;};
// Stack-only adapter for ONE already validated native Release invocation.
// Scalar transaction checks supplement, never replace, the originating
// BundleAccess's current-world/identity/GC/reference proofs and read/call caps.
struct ReleaseFrame {
    NotifyWindow window{};
    const void* lease=nullptr;void* access=nullptr;
    bool (*current)(void*,const ReleaseFrame&)=nullptr;
    std::uint32_t thread=0;unsigned depth=0,entries=0;
    std::uintptr_t returnPc=0;
    bool inside=false,returned=false,ambiguous=false,faulted=false;
    bool Transaction(const void* actualLease,std::uint32_t actualThread,bool entered,unsigned actualDepth)const noexcept {
        return lease&&lease==actualLease&&thread&&thread==actualThread&&entered&&depth&&depth==actualDepth&&window.armed;
    }
    bool Enter(std::uintptr_t killer,const void* actualLease,std::uint32_t actualThread,
               bool entered,unsigned actualDepth,std::uintptr_t pc)noexcept {
        if(killer!=window.jason.object||!Transaction(actualLease,actualThread,entered,actualDepth))return false;
        if(entries++||inside||!returnPc||pc!=returnPc||window.stopped||!current||!current(access,*this)){
            ambiguous=true;return false;
        }
        inside=true;return true;
    }
    void Leave(bool originalReturned)noexcept {inside=false;returned=originalReturned;if(!originalReturned){ambiguous=true;faulted=true;}}
    bool Geometry(const void* actualLease,std::uint32_t actualThread,bool entered,unsigned actualDepth)const noexcept {
        return inside&&entries==1&&!ambiguous&&Transaction(actualLease,actualThread,entered,actualDepth);
    }
    bool Completed(const void* actualLease,std::uint32_t actualThread,bool entered)const noexcept {
        return lease&&lease==actualLease&&thread==actualThread&&!entered&&!inside&&entries==1&&returned&&!ambiguous&&window.armed;
    }
};
}
