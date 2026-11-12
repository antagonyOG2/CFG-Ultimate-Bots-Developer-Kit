#pragma once
#include "OwnedSandboxPrebirthGate.hpp"
#include "../RuntimeProbe/StartingPlayerBindings.generated.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"

namespace cfgmenu::starting {
constexpr unsigned MaximumReads=24000;
template<class Reader> class Budget {
    Reader& reader_;
public:
    unsigned reads=0;
    explicit Budget(Reader& r):reader_(r){}
    template<class T> bool Read(std::uintptr_t p,T& out){if(reads==MaximumReads)return false;++reads;return reader_.Read(p,out);}
};
struct Event {
    std::uintptr_t module=0,receiver=0,function=0,parameters=0;
    DWORD thread=0;bool exactImageVerified=false;
};
struct Observation {
    promotion::Context owner{};
    Event event{};
    std::uintptr_t parameterProperty=0;
    unsigned reads=0;
};
// ProcessEvent(receiver RCX, UFunction RDX, borrowed NewPlayer* R8).
// R8 is NOT an FFrame. No stepping, thunk execution or frame alteration.
template<class Reader,std::size_t N> bool Code(Reader& r,std::uintptr_t at,const std::array<unsigned char,N>& wanted){
    std::array<unsigned char,N> bytes{};return r.Read(at,bytes)&&bytes==wanted;
}
template<class Reader> bool Cache(Reader& r,std::uintptr_t module,std::uintptr_t a,std::uintptr_t b,std::uintptr_t& out){
    std::uintptr_t other=0;return r.Read(module+a,out)&&r.Read(module+b,other)&&cfgprobe::MatchingClassCache(out,other);
}
template<class Reader> bool Metadata(Reader& r,const Event& e,std::uintptr_t& parameter){
    using namespace cfgprobe;using namespace startingdisk;
    parameter=0;std::uintptr_t cached=0,base=0,controller=0;
    if(!ObjectPointer(e.function)||!r.Read(e.module+FunctionCache,cached)||cached!=e.function||
       !Cache(r,e.module,GameModeBaseClass,GameModeBaseConstructor,base)||
       !Cache(r,e.module,PlayerControllerClass,PlayerControllerConstructor,controller))return false;
    cfgreflection::Observation<Reader> reflection(r,e.module);cfgreflection::Field f,p;
    std::uintptr_t child=0,executor=0,cleanup=0,propertyType=0,vtable=0;
    std::uint32_t flags=0;std::uint16_t bytes=0;std::int32_t storage=0;
    unsigned char num=0;struct Script{std::uintptr_t data;std::int32_t count,capacity;}script{};
    if(!reflection.ReadField(e.function,base,f)||f.label.View()!=u"HandleStartingNewPlayer"||f.kind.View()!=u"Function"||
       !r.Read(e.function+0x88,flags)||flags!=FunctionFlags||!r.Read(e.function+0x8C,num)||num!=1||
       !r.Read(e.function+0x8E,bytes)||bytes!=ParameterBytes||!r.Read(e.function+0x40,storage)||storage!=8||
       !r.Read(e.function+0x48,script)||script.count!=0||script.capacity!=0||
       !r.Read(e.function+0xB0,executor)||executor!=e.module+Executor||
       !r.Read(e.function+0x68,cleanup)||cleanup!=0||!r.Read(e.function+0x38,child)||
       !reflection.ReadField(child,e.function,p)||p.label.View()!=u"NewPlayer"||p.kind.View()!=u"ObjectProperty"||p.next)return false;
    cfgreflection::Property property;
    if(!reflection.ReadProperty(p,property)||property.dimension!=1||property.elementBytes!=8||property.offset!=0||property.flags!=ParameterFlags||
       !r.Read(p.object,vtable)||vtable!=e.module+ObjectPropertyVtable||!r.Read(p.object+0x70,propertyType)||propertyType!=controller)return false;
    parameter=p.object;return true;
}
template<class Reader> bool ActorTypes(Reader& r,const promotion::Context& c){
    using namespace cfgprobe;using namespace birthdisk;
    std::uintptr_t pc=0,ps=0;
    if(!Cache(r,c.module,SCPlayerController_SandboxClass,SCPlayerController_SandboxConstructor,pc)||
       !Cache(r,c.module,SCPlayerStateClass,SCPlayerStateConstructor,ps))return false;
    Ancestry a,b;if(!CaptureAncestry(r,c.controller,a)||!a.Contains(pc)||!CaptureAncestry(r,c.playerState,b)||!b.Contains(ps))return false;
    std::uintptr_t defaults=0,type=0;
    for(unsigned i=0;i<2;++i){const auto object=i?c.playerState:c.controller;const auto klass=i?b.classes[0]:a.classes[0];
        if(!r.Read(klass+0xF8,defaults)||!ObjectPointer(defaults)||defaults==object||!r.Read(defaults+ObjectClass,type)||type!=klass)return false;}
    std::uint32_t stateFlags=0;unsigned char role=0;
    return r.Read(c.playerState+0x394,stateFlags)&&!(stateFlags&6)&&r.Read(c.playerState+0x110,role)&&role==3&&
        StableAncestry(r,c.controller,a)&&StableAncestry(r,c.playerState,b);
}
// Fresh owner-chain discovery only. Event addresses in output are borrowed for
// this callback ONLY. Never retains packet storage, allocates serials, scans
// GObjects, selects a pawn or installs a gate. thread must be the actual calling
// thread ID, not merely the expected owner ID copied from a saved context.
template<class Reader> bool Observe(Reader& reader,const Event& e,Observation& out,unsigned* stage=nullptr){
    using namespace cfgprobe;out={};Budget<Reader> r(reader);promotion::Context c{};
    if(stage)*stage=1;
    if(!e.exactImageVerified||!e.thread||!ObjectPointer(e.module)||!ObjectPointer(e.receiver)||!ObjectPointer(e.parameters))return false;
    if(stage)*stage=2;
    std::uintptr_t parameter=0;if(!Metadata(r,e,parameter))return false;
    if(stage)*stage=3;
    DWORD thread=0;unsigned char initialized=0;
    if(!r.Read(e.module+dispatch::ThreadIdRva,thread)||thread!=e.thread||!r.Read(e.module+dispatch::ThreadFlagRva,initialized)||initialized!=1||
       !Code(r,e.module+startingdisk::Wrapper,startingdisk::WrapperCode)||
       !Code(r,e.module+startingdisk::Executor,startingdisk::ExecutorCode)||
       !Code(r,e.module+startingdisk::Implementation,startingdisk::ImplementationCode))return false;
    c.module=e.module;c.thread=e.thread;c.exactImageVerified=true;c.mode=e.receiver;c.modeVtable=e.module+gatedisk::Table;
    if(stage)*stage=4;
    if(!r.Read(e.parameters,c.controller)||!ObjectPointer(c.controller)||
       !r.Read(c.module+EngineGlobalCandidate,c.engine)||!ObjectPointer(c.engine)||
       !r.Read(c.engine+EngineGameViewport,c.viewport)||!ObjectPointer(c.viewport)||
       !r.Read(c.viewport+ViewportWorld,c.world)||!ObjectPointer(c.world)||
       !r.Read(c.world+WorldGameInstance,c.gameInstance)||!ObjectPointer(c.gameInstance)||
       !r.Read(c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)||!ObjectPointer(c.localPlayers)||
       !r.Read(c.localPlayers,c.player)||!ObjectPointer(c.player)||
       !r.Read(c.controller+0x388,c.playerState)||!ObjectPointer(c.playerState))return false;
    const std::array<std::uintptr_t,4> actors{{c.world,c.mode,c.controller,c.playerState}};
    if(stage)*stage=5;
    if(!lifetime::ObserveBatch(r,c.module+lifetime::ObjectArrayRva,actors,c.registry,c.identities))return false;
    if(stage)*stage=6;
    if(!promotion::gate::OwnerCurrent(r,c,e.thread,c.modeVtable,true))return false;
    if(stage)*stage=7;
    if(!ActorTypes(r,c))return false;
    if(stage)*stage=8;
    std::uintptr_t implementation=0,dispatcher=0,controller=0,finalParameter=0;
    if(!r.Read(c.modeVtable+startingdisk::ImplementationSlot,implementation)||implementation!=c.module+startingdisk::Implementation||
       !r.Read(c.modeVtable+cfgprobe::dispatch::EventSlotBytes,dispatcher)||dispatcher!=c.module+cfgprobe::dispatch::NativeActorDispatcherRva||
       !r.Read(e.parameters,controller)||controller!=c.controller||!Metadata(r,e,finalParameter)||finalParameter!=parameter||
       !promotion::gate::OwnerCurrent(r,c,e.thread,c.modeVtable,true))return false;
    out.owner=c;out.event=e;out.parameterProperty=parameter;out.reads=r.reads;if(stage)*stage=0;return true;
}

template<class Reader> bool PickedMetadata(Reader& r,const promotion::Context& c,cfgreflection::Field& field){
    std::uintptr_t stateClass=0;if(!Cache(r,c.module,birthdisk::SCPlayerStateClass,birthdisk::SCPlayerStateConstructor,stateClass))return false;
    if(!startingdisk::PickedFlagsVerified)return false;
    cfgreflection::Observation<Reader> reflection(r,c.module);
    if(!reflection.Find(stateClass,u"PickedCounselorClass",field,false)||field.kind.View()!=u"SoftClassProperty")return false;
    std::int32_t dimension=0,bytes=0,offset=0;std::uint64_t flags=0;std::uintptr_t vtable=0;
    return r.Read(field.object,vtable)&&vtable==c.module+softdisk::PropertyVtable&&
        r.Read(field.object+0x30,dimension)&&dimension==1&&r.Read(field.object+0x34,bytes)&&bytes==40&&
        r.Read(field.object+0x44,offset)&&offset==static_cast<int>(birthdisk::PickedCounselorClass)&&
        r.Read(field.object+0x38,flags)&&flags==startingdisk::PickedFlags;
}
template<class Reader> bool CounselorClass(Reader& r,const promotion::Context& c,std::uintptr_t selected){
    using namespace cfgprobe;std::uintptr_t counselor=0,killer=0;
    if(!Cache(r,c.module,birthdisk::SCCounselorCharacterClass,birthdisk::SCCounselorCharacterConstructor,counselor)||
       !Cache(r,c.module,birthdisk::SCKillerCharacterClass,birthdisk::SCKillerCharacterConstructor,killer))return false;
    std::array<std::uintptr_t,MaximumAncestry> classes{},supers{};unsigned n=0;bool found=false;
    while(selected){if(!ObjectPointer(selected)||selected==killer||n==classes.size())return false;
        for(unsigned i=0;i<n;++i)if(classes[i]==selected)return false;
        classes[n]=selected;found|=selected==counselor;if(!r.Read(selected+ClassSuper,supers[n]))return false;selected=supers[n++];}
    if(!found)return false;
    for(unsigned i=0;i<n;++i){std::uintptr_t value=0;if(!r.Read(classes[i]+ClassSuper,value)||value!=supers[i])return false;}return true;
}
// Explicit optional selection step. Resolver is supplied by the host and must
// resolve ONLY an already-loaded class from this exact live soft value, retaining
// its ownership through the synchronous call. No load/save/default fallback.
// Source changes (including weak-cache refresh) refuse; a resolver needing a
// mutable lookup scratch value must use its own correctly scoped native value.
// bool ResolveLoaded(sourceAddress, const Packet& observed, uintptr_t& UClass).
// No resolver/native operation is invoked by Observe itself.
template<class Reader,class Resolver> bool ResolvePicked(Reader& reader,const promotion::Context& owner,DWORD currentThread,Resolver& resolver,
                                                       promotion::Context& out){
    out={};Budget<Reader> r(reader);auto c=owner;c.selectedClass=0;c.softProperty=0;c.picked={};c.pickedResolutionVerified=false;
    DWORD thread=0;unsigned char initialized=0;
    if(currentThread!=c.thread||!r.Read(c.module+cfgprobe::dispatch::ThreadIdRva,thread)||thread!=currentThread||
       !r.Read(c.module+cfgprobe::dispatch::ThreadFlagRva,initialized)||initialized!=1||
       !promotion::gate::OwnerCurrent(r,c,currentThread,c.modeVtable,true)||!ActorTypes(r,c))return false;
    cfgreflection::Field field;if(!PickedMetadata(r,c,field)||!r.Read(c.playerState+birthdisk::PickedCounselorClass,c.picked))return false;
    std::uint64_t name=0;std::uintptr_t chars=0;std::int32_t count=0,capacity=0;
    std::memcpy(&name,c.picked.bytes.data()+16,8);std::memcpy(&chars,c.picked.bytes.data()+24,8);
    std::memcpy(&count,c.picked.bytes.data()+32,4);std::memcpy(&capacity,c.picked.bytes.data()+36,4);
    if(!name||count<0||capacity<count||capacity>2048||(capacity&&!cfgprobe::ObjectPointer(chars))||(!capacity&&chars)||
       (count&&!cfgprobe::lifetime::Extent(chars,static_cast<std::uintptr_t>(count)*2)))return false;
    if(!resolver.ResolveLoaded(c.playerState+birthdisk::PickedCounselorClass,c.picked,c.selectedClass)||
       !CounselorClass(r,c,c.selectedClass))return false;
    promotion::Packet again;cfgreflection::Field finalField;
    if(!r.Read(c.playerState+birthdisk::PickedCounselorClass,again)||again.bytes!=c.picked.bytes||
       !PickedMetadata(r,c,finalField)||finalField.object!=field.object||finalField.type!=field.type||!(finalField.name==field.name)||
       !promotion::gate::OwnerCurrent(r,c,c.thread,c.modeVtable,true)||!ActorTypes(r,c))return false;
    c.softProperty=field.object;c.pickedResolutionVerified=true;out=c;return true;
}
}
