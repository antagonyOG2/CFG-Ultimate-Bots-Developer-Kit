#include "SpectatorCycleObservation.hpp"
#include <string>
#include "../RuntimeProbe/SpectatorRuntime.hpp"
#define main CfgExistingNameFixtureMain
#include "NameObservationTests.cpp"
#undef main
int main(){
    {std::array<char,96> label{};
     assert(!cfgspectator::AlternateCycleDiagnostic(u"ReceiveTick",label)&&label[0]==0);
     assert(cfgspectator::AlternateCycleDiagnostic(u"InpActEvt_NextSpectator_K2Node",label));
     assert(std::string_view(label.data())=="spectator-event-InpActEvt_NextSpectator_K2Node");
     assert(cfgspectator::AlternateCycleDiagnostic(u"Previous\nView",label));
     assert(std::string_view(label.data())=="spectator-event-Previous_View");
     std::u16string longName=u"Next";longName.append(123,u'x');
     assert(cfgspectator::AlternateCycleDiagnostic(longName,label)&&label.back()==0);
     longName.append(2,u'x');assert(!cfgspectator::AlternateCycleDiagnostic(longName,label));}
    {cfgspectator::CycleHookRetry retry;std::uint64_t now=1000;
     for(unsigned i=0;i<cfgspectator::CycleHookRetry::MaximumAttempts;++i){
         assert(retry.Due(now));retry.Complete(false,now);
         if(i+1<cfgspectator::CycleHookRetry::MaximumAttempts){assert(!retry.finished&&!retry.Due(now+249));now+=250;}
     }
     assert(retry.finished&&!retry.ready&&retry.attempts==cfgspectator::CycleHookRetry::MaximumAttempts&&!retry.Due(now+1000));
     retry.Reset();assert(retry.Due(now));retry.Complete(true,now);assert(retry.finished&&retry.ready&&retry.attempts==0);}
    {cfgspectator::Snapshot s{};s.jason={{0x1000,1,false},{0x1100,2,false},true};
     s.counselors[0]={{0x1200,3,false},{0x1300,4,false},true};s.count=1;
     assert(cfgspectator::ResolveAttachedSelection(s,0x1200)&&s.observedState.object==0x1300);
     assert(cfgspectator::ResolveAttachedSelection(s,0x1000)&&s.observedState.object==0x1100);
     assert(!cfgspectator::ResolveAttachedSelection(s,0x1400));}
    Fixture f;constexpr auto module=Fixture::Base;
    constexpr std::uintptr_t function=0x600000,type=0x610000,owner=0x620000,controller=0x630000;
    const auto name=[&](std::uintptr_t object,int index,const char* label){
        const auto entry=Fixture::Entry+static_cast<std::uintptr_t>(index)*0x1000;
        f.r.Set(Fixture::Chunk+8*index,entry);f.r.Set(entry,std::uint32_t{});
        std::array<unsigned char,128> text{};std::memcpy(text.data(),label,std::strlen(label));
        f.r.Set(entry+cfgnames::disk::EntryText,text);f.r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{index,0});
    };
    name(function,13,"ServerViewPrevPlayer");name(type,14,"Function");
    f.r.Set(function+cfgprobe::ObjectClass,type);f.r.Set(function+0x20,owner);f.r.Set(controller+cfgprobe::ObjectClass,owner);
    f.r.Set(function+cfgreflection::disk::FunctionFlags,std::uint32_t{0x400});
    f.r.Set(function+cfgobjectivepanel::disk::FunctionNumParms,std::uint8_t{});
    f.r.Set(function+cfgobjectivepanel::disk::FunctionParameterBytes,std::uint16_t{});
    f.r.Set(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,std::uint16_t{0xffff});
    f.r.Set(function+0xB0,module+0x10100);
    assert(cfgspectator::CompletedCycleEvent(f.r,module,controller,function));
    name(function,13,"ServerViewNextPlayer");assert(cfgspectator::CompletedCycleEvent(f.r,module,controller,function));
    for(unsigned n=0;n<6;++n){auto bad=f.r;
        switch(n){case 0:bad.Set(function+cfgobjectivepanel::disk::FunctionNumParms,std::uint8_t{1});break;
        case 1:bad.Set(function+cfgobjectivepanel::disk::FunctionParameterBytes,std::uint16_t{8});break;
        case 2:bad.Set(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,std::uint16_t{});break;
        case 3:bad.Set(function+cfgreflection::disk::FunctionFlags,std::uint32_t{});break;
        case 4:bad.Set(function+0xB0,module+0x4000000);break;
        default:bad.Set(controller+cfgprobe::ObjectClass,std::uintptr_t{});break;}
        assert(!cfgspectator::CompletedCycleEvent(bad,module,controller,function));
    }
    name(function,13,"ClientSetSpectatorCamera");cfgnames::Text observed;
    assert(!cfgspectator::CompletedCycleEvent(f.r,module,controller,function,&observed));
    assert(observed.View()==u"ClientSetSpectatorCamera"); // diagnostic output never admits a camera event
    observed.count=20;assert(!cfgspectator::CompletedCycleEvent(f.r,module,controller,0,&observed)&&observed.count==0);
    std::puts("CFG native spectator completion hints PASS; read-only fixture, no camera authority or game access.");
}
