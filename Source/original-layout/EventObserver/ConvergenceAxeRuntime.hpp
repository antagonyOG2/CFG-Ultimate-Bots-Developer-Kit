#pragma once
#include "LoadedItemClassSurvey.hpp"
#include "../Core/GameplayLimits.hpp"
namespace cfgconvergence {
template<class Host> auto AxeClassHint(Host& host,int)->decltype(host.LoadedAxeHint()){return host.LoadedAxeHint();}
template<class Host> std::uintptr_t AxeClassHint(Host&,long){return 0;}
template<class Host> auto AxeDeferred(Host& host,unsigned index,int)->decltype(host.Pause(index)){return host.Pause(index);}
template<class Host> bool AxeDeferred(Host&,unsigned,long){return false;}
// One finite loaded-class survey per owned world, then one native AddOrSwap
// transaction per helper. No asset loads, manual spawns, inventory edits or
// retries after an ambiguous native call. The class is re-proved by NativeAccess.
class AxeRuntime {
    cfgloadout::expanded::ClassSurvey survey_{};
    cfgloadout::expanded::Evidence before_{};
    cfgofflinesettings::Request owner_{};
    std::uintptr_t type_=0;std::uint64_t next_=0,deadline_=0;
    unsigned index_=0,preparations_=0;bool servicing_=false,executing_=false,surveyYield_=false,helperYield_=false;
    bool replacement_=false;unsigned completedPrefix_=0,verifiedMask_=0,uncertainMask_=0;
    std::array<unsigned,cfglimits::MaximumHelpers> replacementCounts_{};
    std::array<std::uint64_t,cfglimits::MaximumHelpers> replacementAt_{};
    void Advance(){if(replacement_){index_=completedPrefix_;replacement_=false;stage=5;}else{++index_;stage=1;}
        preparations_=0;before_={};owner_={};helperYield_=true;}
public:
    // 0 survey,1 next helper,2 prepare,3 grant,4 verify,5 complete,6 unavailable.
    unsigned stage=0,scanned=0,attempts=0,grants=0,refused=0,alreadyArmed=0,lastReads=0,lastFailure=0;
    bool ContinuationDue(std::uint64_t now)const{return stage>=1&&stage<=4&&!servicing_&&!executing_&&now>=next_;}
    // Caller has JUST observed exact native GetCurrentWeapon == null on this
    // living owned helper. Never queue from a failed ownership/read observation.
    // Replacements are finite, paced and never recover an ambiguous native call.
    bool RequestRearm(unsigned index,std::uint64_t now){
        if(servicing_||executing_||stage!=5||index>=index_||index>=cfglimits::MaximumHelpers||
            !(verifiedMask_&(1u<<index))||(uncertainMask_&(1u<<index))||replacementCounts_[index]>=4||
            now<replacementAt_[index]||now>UINT64_MAX-15000)return false;
        ++replacementCounts_[index];replacementAt_[index]=now+15000;
        completedPrefix_=index_;index_=index;replacement_=true;stage=1;preparations_=0;before_={};owner_={};next_=0;return true;
    }
    template<class Access> bool Service(Access& access){
        if(executing_)return false;
        struct Guard{bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(executing_);
        if(stage==2){
            if(!access.Observe(before_,true)){lastFailure=access.lastStage;if(++preparations_>=4){++refused;Advance();}return true;}
            // A native held Axe needs no replacement. Observe proved its owner.
            if((replacement_&&before_.equipped)||access.AlreadyAxe(before_)){++alreadyArmed;verifiedMask_|=1u<<index_;Advance();return true;}
            stage=3;return true;
        }
        if(stage==3){
            stage=4;++attempts; // consume BEFORE native dispatch/reentry
            if(!access.Grant(before_)){lastFailure=access.lastStage;++refused;uncertainMask_|=1u<<index_;Advance();}return true;
        }
        if(stage==4){if(access.Verify(before_)){++grants;verifiedMask_|=1u<<index_;}else{lastFailure=access.lastStage;++refused;uncertainMask_|=1u<<index_;}Advance();return true;}
        return false;
    }
    template<class Reader,class Host> bool Step(Reader& reader,Host& host,std::uintptr_t module,
        std::uintptr_t world,unsigned count,bool active,std::uint64_t now,bool rosterComplete=true){
        // The UI may request one additional owned helper later in this match.
        // Preserve the consumed prefix; service only the new roster suffix.
        if(stage==5&&count>index_&&index_<cfglimits::MaximumHelpers)stage=1;
        if(servicing_||stage>=5||now<next_)return false;
        struct Guard{bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(servicing_);
        next_=now+(stage?100:250);if(!deadline_)deadline_=now+90000;
        if(helperYield_){helperYield_=false;return false;} // publish/drive the newly armed helper promptly
        if(stage==0){
            // A missing loaded class cannot delay sweater detection or normal
            // Blackboard work: at most every other convergence opportunity.
            if(surveyYield_){surveyYield_=false;return false;}surveyYield_=true;
            if(now>deadline_){stage=6;++refused;return true;}
            // Reuse an exact-name hint already proved by local loadout/held
            // ownership discovery. NativeAccess still re-proves its class,
            // defaults, fields, native ABI and identities before any grant.
            if(const auto hint=AxeClassHint(host,0)){type_=hint;stage=1;return true;}
            survey_.Step(reader,module,world,cfgloadout::Slot::Weapon,3);
            scanned=survey_.visited;lastReads=survey_.lastReads;
            if(survey_.phase==2){type_=survey_.found;stage=1;}
            else if(survey_.phase==3){stage=6;++refused;}
            return true;
        }
        if(!active)return false;
        if(index_>=cfglimits::MaximumHelpers){stage=5;return true;}
        if(index_>=count){if(!rosterComplete)return false;stage=5;return true;}
        if(AxeDeferred(host,index_,0))return false; // temporary emote ownership is NOT a failed grant
        host.Select(index_);
        if(stage==1){if(!host.Owner(owner_)){++refused;Advance();}else stage=2;return true;}
        struct Access:cfgloadout::expanded::NativeAccess<Reader,Host>{
            Reader& reader;std::uintptr_t type;
            Access(Reader& r,Host& h,cfgofflinesettings::Request o,std::uintptr_t m,std::uintptr_t t):
                cfgloadout::expanded::NativeAccess<Reader,Host>(r,h,{o,cfgloadout::Slot::Weapon,3},m,t),reader(r),type(t){}
            bool AlreadyAxe(const cfgloadout::expanded::Evidence& e){std::uintptr_t heldType=0;
                return e.equipped&&reader.Read(e.equipped+cfgprobe::ObjectClass,heldType)&&heldType==type;}
        }access(reader,host,owner_,module,type_);
        const bool progressed=Service(access);lastReads=access.lastReads;return progressed;
    }
};
}
