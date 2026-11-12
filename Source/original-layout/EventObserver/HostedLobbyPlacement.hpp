#pragma once
#include "NativeSetupRows.hpp"
#include "NativeMenuRetention.hpp"
#include "../Core/HostedOptions.hpp"
#include "HostedLobbyReport.hpp"
namespace cfghostedlobby {
// This capability is supplied ONLY by a dedicated retained host-lobby provider.
// A generic/offline NativeRetentionScope or a sampled IsHost flag is insufficient.
struct RowLayout {std::uintptr_t world=0,menu=0,rowClass=0,panel=0,anchor=0;unsigned after=0;std::uint64_t generation=0;};
struct RowHooks {
    enum class Domain {None,HostLobby};Domain domain=Domain::None;void* context=nullptr;
    bool(*current)(void*,const RowLayout&)=nullptr;
    bool(*append)(void*,const RowLayout&,unsigned,std::uintptr_t&,bool&)=nullptr;
    bool(*options)(void*,std::uintptr_t,const cfgmenu::rows::Spec&)=nullptr;
    bool(*text)(void*,std::u16string_view,cfgmenu::reflection::RetainedMenuText::Request&)=nullptr;
    bool(*setText)(void*,std::uintptr_t,std::u16string_view,const cfgmenu::reflection::RetainedMenuText::Request&)=nullptr;
    bool(*index)(void*,std::uintptr_t,unsigned&)=nullptr;bool(*remove)(void*,std::uintptr_t)=nullptr;
    bool Complete()const{return domain==Domain::HostLobby&&context&&current&&append&&options&&text&&setText&&index&&remove;}
};
class Placement {
    cfgmenu::rows::Runtime rows_;RowLayout layout_{};unsigned appended_=0;
public:
    template<class Host>bool Service(Host& h,const RowLayout& layout,std::uint64_t now,cfghostedoptions::Store& store){
        using namespace cfgmenu;
        if(!layout.world||!layout.menu||!layout.rowClass||!layout.panel||!layout.anchor||layout.after>NodeLimit||!layout.generation)return false;
        if(!h.ProvedHostLobbyLease(layout))return false;
        if(layout_.world!=layout.world||layout_.menu!=layout.menu||layout_.generation!=layout.generation){
            rows_.Detached();layout_=layout;appended_=0;
            std::array<rows::Spec,3> specs{};
            const RowId ids[]{RowId::OnlineJason,RowId::OnlineDifficulty,RowId::OnlineFill};
            const std::u16string_view names[]{u"Jason Bot",u"Jason Bot Difficulty",u"Fill Empty Slots with Bots"};
            const unsigned selected[]{store.values.jason,store.values.difficulty?store.values.difficulty-1:1,store.values.fill};
            for(unsigned i=0;i<3;++i){auto& s=specs[i];s.id=ids[i];s.title=names[i];s.selected=selected[i];s.preserveNativeValue=true;
                if(i==1){s.count=3;s.labels[0]=u"Easy";s.labels[1]=u"Normal";s.labels[2]=u"Hard";}
                else{s.count=2;s.labels[0]=u"Off";s.labels[1]=u"On";}}
            if(!rows_.Begin(layout.generation,specs.data(),3))return false;
        }
        struct Adapter{Host& host;const RowLayout& layout;unsigned& appended;
            bool Current(std::uint64_t generation){return generation==layout.generation&&host.ProvedHostLobbyLease(layout);}
            bool Append(std::uintptr_t& widget,bool& attached){if(!host.AppendAfter(layout,layout.after+appended,widget,attached))return false;++appended;return true;}
            bool BorrowNativeRow(std::uintptr_t&,bool&,const rows::Spec&){return false;}
            bool CopyOptions(std::uintptr_t row,const rows::Spec& spec){return host.CopyOptions(row,spec);}
            bool Text(std::u16string_view text,reflection::RetainedMenuText::Request& out){return host.Text(text,out);}
            bool SetText(std::uintptr_t row,std::u16string_view field,const reflection::RetainedMenuText::Request& text){return host.SetText(row,field,text);}
            bool Index(std::uintptr_t row,unsigned& out){return host.Index(row,out);}
            bool RemoveOwned(std::uintptr_t row){return host.RemoveOwned(row);}
        }adapter{h,layout_,appended_};
        (void)rows_.Step(adapter,now);
        if(rows_.CurrentState()!=rows::State::Ready)return false;
        std::array<unsigned,rows::Capacity> choices{};if(!rows_.Choices(adapter,layout.generation,choices))return false;
        return store.Save({choices[0],choices[1]+1,choices[2],1});
    }
    void Detach(){rows_.Detached();layout_={};appended_=0;}
    unsigned State()const{return static_cast<unsigned>(rows_.CurrentState());}
    unsigned Placed()const{return rows_.CurrentState()==cfgmenu::rows::State::Ready?3u:0u;}
};
struct HookHost {
    RowHooks& hooks;std::uintptr_t module=0;
    bool ProvedHostLobbyLease(const RowLayout& l){return hooks.Complete()&&cfgmenu::rows::NativeRetentionScope::Current(module)&&hooks.current(hooks.context,l);}
    bool AppendAfter(const RowLayout& l,unsigned index,std::uintptr_t& row,bool& attached){return ProvedHostLobbyLease(l)&&hooks.append(hooks.context,l,index,row,attached);}
    bool CopyOptions(std::uintptr_t row,const cfgmenu::rows::Spec& spec){return hooks.options(hooks.context,row,spec);}
    bool Text(std::u16string_view text,cfgmenu::reflection::RetainedMenuText::Request& out){return hooks.text(hooks.context,text,out);}
    bool SetText(std::uintptr_t row,std::u16string_view field,const cfgmenu::reflection::RetainedMenuText::Request& text){return hooks.setText(hooks.context,row,field,text);}
    bool Index(std::uintptr_t row,unsigned& out){return hooks.index(hooks.context,row,out);}
    bool RemoveOwned(std::uintptr_t row){return hooks.remove(hooks.context,row);}
};
}
