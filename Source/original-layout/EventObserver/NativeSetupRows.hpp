#pragma once
#include "MenuReflectionParameters.hpp"
#include "MenuTextLifetime.hpp"
#include "../Core/SetupMenuModel.hpp"
#include <string>
namespace cfgmenu::rows {
constexpr unsigned Capacity=RowCount;
constexpr std::uint64_t PhaseGapMs=25;
struct Spec {
    RowId id=RowId::End;std::u16string_view title{};
    std::array<std::u16string_view,32> labels{};unsigned count=0,selected=0;
    bool reuseNativeCount=false,configureNativeCount=false,reuseNativeSkill=false,preserveNativeValue=false,informationOnly=false;
    // Challenges retains its authored first focus target instead of collapsing
    // it beneath a new first row. Only an explicitly scoped Off/On donor row.
    bool reuseChallengeDonor=false;
    bool Valid()const noexcept{
        if(id>=RowId::End||title.empty()||title.size()>=128||title.find(char16_t{})!=std::u16string_view::npos||
            !count||count>labels.size()||selected>=count||(reuseNativeCount&&id!=RowId::Count)||(configureNativeCount&&!reuseNativeCount)||
            (reuseNativeSkill&&(id!=RowId::Skill||reuseNativeCount||count!=3))||
            (informationOnly&&(id!=RowId::Weather||reuseNativeCount||count!=1||selected))||
            (reuseChallengeDonor&&(id!=RowId::Knives||reuseNativeSkill||reuseNativeCount||informationOnly||count!=2||selected||
                labels[0]!=u"Off"||labels[1]!=u"On")))return false;
        for(unsigned i=0;i<count;++i)if(labels[i].empty()||labels[i].size()>=128||labels[i].find(char16_t{})!=std::u16string_view::npos)return false;
        return true;
    }
};
struct Entry {
    Spec spec{};std::uintptr_t widget=0;bool attached=false;
    std::u16string titleStorage{};std::array<std::u16string,32> labelStorage{};
    void Copy(const Spec& source){
        spec=source;titleStorage.assign(source.title);spec.title=titleStorage;
        for(unsigned i=0;i<source.count;++i){labelStorage[i].assign(source.labels[i]);spec.labels[i]=labelStorage[i];}
    }
};
enum class State {Empty,Building,Ready,RollingBack,Retiring,Failed,Detached};
enum class Phase {Append,Options,ConvertTitle,Title,ConvertValue,Value};
// One shared owner-thread scheduler. Engine work only while setup is visible.
// Create+Attach is deliberately indivisible: an unowned zero-serial widget
// must never be saved across callbacks waiting for attachment/GC retention.
class Runtime {
    std::array<Entry,Capacity> entries_{};unsigned count_=0,index_=0;
    std::uint64_t generation_=0,last_=0;bool paced_=false;
    State state_=State::Empty;Phase phase_=Phase::Append;
    reflection::RetainedMenuText::Request title_{},value_{};
    void Fail()noexcept{state_=State::RollingBack;index_=count_;}
public:
    Runtime()=default;
    Runtime(const Runtime&)=delete;Runtime& operator=(const Runtime&)=delete;
    Runtime(Runtime&&)=delete;Runtime& operator=(Runtime&&)=delete;
    bool Begin(std::uint64_t generation,const Spec* specs,unsigned count){
        if(state_==State::Building||state_==State::Ready||state_==State::RollingBack||state_==State::Retiring||!generation||!specs||!count||count>Capacity)return false;
        for(unsigned i=0;i<count;++i){if(!specs[i].Valid())return false;
            for(unsigned j=0;j<i;++j)if(specs[j].id==specs[i].id)return false;}
        // Stage deep copies BEFORE retiring the previous view. A caller may
        // legitimately provide views into its previous labels when rebuilding;
        // comparing descriptor addresses alone does not catch that alias.
        std::array<Entry,Capacity> staged{};
        for(unsigned i=0;i<count;++i)staged[i].Copy(specs[i]);
        entries_={};for(unsigned i=0;i<count;++i)entries_[i].Copy(staged[i].spec);
        count_=count;index_=0;generation_=generation;phase_=Phase::Append;title_={};value_={};last_=0;paced_=false;state_=State::Building;return true;
    }
    template<class Host> bool Step(Host& host,std::uint64_t now){
        if(state_!=State::Building&&state_!=State::RollingBack&&state_!=State::Retiring)return false;
        if(paced_&&(now<last_||now-last_<PhaseGapMs))return false;
        paced_=true;last_=now;
        if(!host.Current(generation_)){
            // Travel/back/owner replacement: never dispatch into the old tree.
            // Its native tree owns attached rows; leave lifetime cleanup to it.
            state_=State::Detached;return false;
        }
        if(state_==State::RollingBack||state_==State::Retiring){
            while(index_&&!entries_[index_-1].attached)--index_;
            if(!index_){state_=state_==State::Retiring?State::Empty:State::Failed;return true;}
            auto& entry=entries_[--index_];
            if(!host.RemoveOwned(entry.widget)){state_=State::Failed;return false;}
            entry.attached=false;return true;
        }
        auto& entry=entries_[index_];bool okay=false;
        switch(phase_){
        case Phase::Append:{
            // Host reports attempted attachment even on an ambiguous return,
            // allowing rollback of OWN rows only. No native panel ClearChildren.
            okay=(entry.spec.reuseNativeCount||entry.spec.reuseNativeSkill||entry.spec.reuseChallengeDonor)?host.BorrowNativeRow(entry.widget,entry.attached,entry.spec):host.Append(entry.widget,entry.attached);break;}
        case Phase::Options:okay=host.CopyOptions(entry.widget,entry.spec);break;
        case Phase::ConvertTitle:okay=host.Text(entry.spec.title,title_);break;
        case Phase::Title:okay=host.SetText(entry.widget,u"TitleText",title_);break;
        case Phase::ConvertValue:okay=host.Text(entry.spec.labels[entry.spec.selected],value_);break;
        case Phase::Value:okay=host.SetText(entry.widget,u"ValueText",value_);break;
        }
        if(!okay){Fail();return false;}
        if(phase_==Phase::Value||(phase_==Phase::Append&&entry.spec.reuseNativeCount&&!entry.spec.configureNativeCount)||
           (phase_==Phase::Options&&entry.spec.configureNativeCount)||
           (phase_==Phase::Title&&entry.spec.preserveNativeValue)){
            title_={};value_={};phase_=Phase::Append;
            if(++index_==count_)state_=State::Ready;
        }else phase_=static_cast<Phase>(static_cast<unsigned>(phase_)+1);
        return true;
    }
    // Native Start reads ALL rows into temporary storage before committing any
    // settings. An invalid index cannot leave half of a configuration applied.
    template<class Host> bool Choices(Host& host,std::uint64_t generation,std::array<unsigned,Capacity>& out){
        out={};if(state_!=State::Ready||generation!=generation_||!host.Current(generation))return false;
        std::array<unsigned,Capacity> candidate{};
        for(unsigned i=0;i<count_;++i)if(!host.Index(entries_[i].widget,candidate[i])||candidate[i]>=entries_[i].spec.count)return false;
        if(!host.Current(generation))return false;out=candidate;return true;
    }
    State CurrentState()const noexcept{return state_;}
    // Request BEFORE forwarding Back/travel while this view still owns the
    // panel. One attached row is removed per paced callback. No ClearChildren.
    // Native route departure is allowed only after Empty; an ownership loss
    // remains Detached, not a falsely successful cleanup of a cached widget.
    bool Retire()noexcept{
        if(state_==State::Retiring)return true;
        if(state_!=State::Ready&&state_!=State::Building&&state_!=State::Empty)return false;
        state_=State::Retiring;index_=count_;title_={};value_={};return true;
    }
    // Called by the frontend route owner on Back/travel, including after Ready.
    // The departing native panel owns the rows: do not dispatch old removals.
    void Detached()noexcept{state_=State::Detached;title_={};value_={};}
    unsigned Count()const noexcept{return count_;}
    const Entry& At(unsigned i)const noexcept{return entries_[i];}
};
}
