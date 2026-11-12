#pragma once
#include "CounselorPracticeSettings.hpp"
#include "../RuntimeProbe/CounselorPracticeBindings.generated.hpp"
#include <string_view>
#include <array>
namespace cfgpractice {
constexpr bool KeepPamela(std::u16string_view line){
    return line==u"PamelaSweaterActivate"||line==u"PamelaSweaterActive"||line==u"PamelaJasonStunned";
}
constexpr bool MutePamela(std::u16string_view line){
    // Unknown/non-Pamela lines are never silently classified as taunts.
    return line.size()>=6&&line.substr(0,6)==u"Pamela"&&!KeepPamela(line);
}
enum class Write:unsigned {NotEntered,Returned,Uncertain};
template<class T>struct Override {
    T before{},desired{};bool attempted=false,owned=false,uncertain=false;
    template<class Host>bool Apply(Host& host,T wanted){
        if(attempted)return false;T value{};
        if(!host.Current()||!host.Read(value))return false;
        before=value;desired=wanted;attempted=true;
        if(value==wanted)return false; // do not own preexisting/native state
        const auto result=host.Set(wanted);
        if(result==Write::NotEntered){attempted=false;return false;}
        if(result==Write::Uncertain||!host.Current()||!host.Read(value)||value!=wanted){uncertain=true;return false;}
        owned=true;return true;
    }
    template<class Host>bool Restore(Host& host){
        if(!owned||uncertain)return false;
        T value{};if(!host.Current()||!host.Read(value))return false;
        if(value!=desired){owned=false;return false;} // external change, preserve it
        const auto result=host.Set(before);if(result==Write::NotEntered)return false;
        owned=false;
        if(result==Write::Uncertain||!host.Current()||!host.Read(value)||value!=before){uncertain=true;return false;}
        return true;
    }
};
struct Report {
    std::uint32_t options=0,voiceHookReady=0,voiceHookFailure=0,peakReads=0,hudStage=0,hudNodes=0;
    std::uint64_t voiceSeen=0,voiceMuted=0,voiceKept=0,voiceRefused=0,healthHidden=0,healthRefused=0,
        grabWrites=0,grabRestores=0,grabRefused=0,grabDeparted=0,hudWrites=0,hudRestores=0,hudRefused=0,hudDeparted=0,
        speedRestores=0,speedRestoreRefused=0,speedDeparted=0,
        grabDistantSeen=0,grabNearSkipped=0,axeGrants=0,macheteFallbackGrants=0,axeClassMissing=0;
};
// Observation only: fixed text records cannot authorize a widget mutation.
struct HudNodeDiagnostic {
    std::uintptr_t object=0;std::uint32_t node=UINT32_MAX,reason=0,visibility=255;
    std::array<char,64> name{},type{};
};
struct HudDiagnostics {
    std::uint32_t refusedCount=0,visibleCount=0,complete=0,visibleTruncated=0,refusedTruncated=0,treeCapacityHit=0;
    std::array<HudNodeDiagnostic,16> refused{};
    std::array<HudNodeDiagnostic,64> visible{};
    void Refused(const HudNodeDiagnostic& value){
        for(unsigned i=0;i<refusedCount;++i)if(refused[i].object==value.object&&refused[i].node==value.node&&refused[i].reason==value.reason)return;
        if(refusedCount==refused.size()){refusedTruncated=1;return;}refused[refusedCount++]=value;
    }
    void Visible(const HudNodeDiagnostic& value){
        if(complete||!(value.visibility==0||value.visibility==3||value.visibility==4))return;
        for(unsigned i=0;i<visibleCount;++i)if(visible[i].object==value.object)return;
        if(visibleCount==visible.size()){visibleTruncated=1;return;}visible[visibleCount++]=value;
    }
    bool Valid()const{
        if(refusedCount>refused.size()||visibleCount>visible.size()||complete>1||visibleTruncated>1||refusedTruncated>1||treeCapacityHit>1)return false;
        const auto valid=[](const HudNodeDiagnostic& n){return n.reason<=6&&(n.visibility<=4||n.visibility==255)&&
            (n.node<192||n.node==UINT32_MAX)&&n.name.back()==0&&n.type.back()==0;};
        for(unsigned i=0;i<refusedCount;++i)if(!valid(refused[i])||!refused[i].reason)return false;
        for(unsigned i=0;i<visibleCount;++i)if(!valid(visible[i])||visible[i].reason||!(visible[i].visibility==0||visible[i].visibility==3||visible[i].visibility==4))return false;
        return true;
    }
};
struct GrabEvent {
    std::uintptr_t victim=0;std::int32_t serial=0;unsigned range=0;
    void Observe(std::uintptr_t next,std::int32_t nextSerial,unsigned nextRange,Report& report){
        if(next&&nextRange&&(next!=victim||nextSerial!=serial||nextRange!=range)){
            if(nextRange==2)++report.grabDistantSeen;else if(nextRange==1)++report.grabNearSkipped;
        }
        victim=next;serial=nextSerial;range=nextRange;
    }
};
inline std::uintptr_t ConvergenceWeapon(std::uintptr_t axe,std::uintptr_t machete,Report& report){
    if(!axe)++report.axeClassMissing;return axe?axe:machete;
}
}
