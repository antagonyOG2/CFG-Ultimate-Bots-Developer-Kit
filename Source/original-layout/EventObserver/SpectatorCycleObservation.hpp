#pragma once
#include "../RuntimeProbe/NameObservation.hpp"
#include "SpawnAssetObservation.hpp"
#include "../RuntimeProbe/ReflectionLayout.generated.hpp"
#include "../RuntimeProbe/ObjectivePanelBindings.hpp"
namespace cfgspectator {
struct CycleHookRetry {
    static constexpr unsigned MaximumAttempts=8;
    static constexpr std::uint64_t RetryMilliseconds=250;
    unsigned attempts=0;
    std::uint64_t retryAt=0;
    bool finished=false,ready=false;
    bool Due(std::uint64_t now)const{return !finished&&now>=retryAt;}
    void Complete(bool succeeded,std::uint64_t now){
        if(finished)return;
        if(succeeded){ready=true;finished=true;return;}
        if(++attempts>=MaximumAttempts){finished=true;return;}
        retryAt=now+RetryMilliseconds;
    }
    void Reset(){*this={};}
};
// Read-only event hint. This cannot publish a camera or authorize a target;
// the production spectator snapshot still proves the complete native roster.
// A bounded alternate-event label helps identify a changed input transport.
// It is diagnostic ONLY; matching this text never admits a cycle or camera.
inline bool AlternateCycleDiagnostic(std::u16string_view name,std::array<char,96>& output){
    output={};
    if(name.empty()||name.size()>127||
       (name.find(u"Spectat")==name.npos&&name.find(u"View")==name.npos&&
        name.find(u"Next")==name.npos&&name.find(u"Prev")==name.npos))return false;
    constexpr char prefix[]="spectator-event-";unsigned at=0;
    for(const auto c:prefix)if(c)output[at++]=c;
    for(const auto c:name){if(at+1>=output.size())break;
        output[at++]=(c>=u'A'&&c<=u'Z')||(c>=u'a'&&c<=u'z')||(c>=u'0'&&c<=u'9')||c==u'_'?static_cast<char>(c):'_';}
    return true;
}
template<class R>bool CompletedCycleEvent(R& r,std::uintptr_t module,std::uintptr_t controller,
    std::uintptr_t function,cfgnames::Text* observedName=nullptr){
    if(observedName)*observedName={};
    cfgnames::Observation<R> names(r,module);cfgnames::Text label,kind;
    std::uintptr_t type=0,owner=0,controllerType=0,executor=0;
    std::uint16_t bytes=1,result=0;std::uint8_t params=1;std::uint32_t flags=0;
    if(!function||!names.Object(function,label))return false;
    if(observedName)*observedName=label; // same read, not another metadata walk
    if((label.View()!=u"ServerViewNextPlayer"&&label.View()!=u"ServerViewPrevPlayer")||
       !r.Read(function+cfgprobe::ObjectClass,type)||!names.Object(type,kind)||kind.View()!=u"Function"||
       !r.Read(function+0x20,owner)||!r.Read(controller+cfgprobe::ObjectClass,controllerType)||
       !cfgspawn::ClassContains(r,controllerType,owner)||
       !r.Read(function+cfgreflection::disk::FunctionFlags,flags)||!(flags&0x400)||
       !r.Read(function+cfgobjectivepanel::disk::FunctionNumParms,params))return false;
    // UE's native parameter layout is defined by the same metadata constants
    // used by objective/menu frame validation; no input/controller writes.
    if(!r.Read(function+cfgobjectivepanel::disk::FunctionParameterBytes,bytes)||
       !r.Read(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,result))return false;
    if(params||bytes||result!=0xffff||!r.Read(function+0xB0,executor)||
       executor<=module||executor>=module+0x3000000)return false;
    std::uintptr_t again=0;std::uint32_t flagsAgain=0;std::uint8_t paramsAgain=1;
    std::uint16_t bytesAgain=1,resultAgain=0;
    if(!r.Read(function+cfgprobe::ObjectClass,again)||again!=type||
       !r.Read(function+0x20,again)||again!=owner||
       !r.Read(controller+cfgprobe::ObjectClass,again)||again!=controllerType||
       !r.Read(function+0xB0,again)||again!=executor||
       !r.Read(function+cfgreflection::disk::FunctionFlags,flagsAgain)||flagsAgain!=flags||
       !r.Read(function+cfgobjectivepanel::disk::FunctionNumParms,paramsAgain)||paramsAgain!=params||
       !r.Read(function+cfgobjectivepanel::disk::FunctionParameterBytes,bytesAgain)||bytesAgain!=bytes||
       !r.Read(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,resultAgain)||resultAgain!=result)return false;
    return true;
}
}
