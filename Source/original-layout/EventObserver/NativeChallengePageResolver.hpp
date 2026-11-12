#pragma once
#include "NativeSoftClassLoader.hpp"
#include "MenuReflectionParameters.hpp"
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/ConvergenceBindings.generated.hpp"
#include "../RuntimeProbe/NativeMapSelectionObservation.hpp"
#include "../RuntimeProbe/MenuPush.generated.hpp"
#include <memory>
namespace cfgmenu::challengepage {
inline constexpr auto& Path=startupretain::ChallengePagePath;
struct NamePacket {reflection::String input;cfgnames::Words result{};};
static_assert(sizeof(NamePacket)==24&&offsetof(NamePacket,result)==16);
inline promotion::Packet SoftReference(cfgnames::Words name){
    promotion::Packet value{};const std::int32_t invalid=-1;
    std::memcpy(value.bytes.data(),&invalid,4);std::memcpy(value.bytes.data()+16,&name,8);return value;
}
inline bool NameSchema(const capture::FunctionEvidence& f){
    if(f.function.label.View()!=u"Conv_StringToName"||f.parameterBytes!=24||f.storageBytes!=24||
       f.script.count||f.script.data||f.script.capacity||f.destructorLink||f.fieldsCount!=2||f.flags!=0x14022401)return false;
    unsigned mask=0;
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];unsigned bit=0;
        if(p.field.owner!=f.function.object||p.dimension!=1||(p.flags&0x80)==0)return false;
        if((p.field.label.View()==u"InString"||p.field.label.View()==u"inString")&&p.field.kind.View()==u"StrProperty"&&
           p.elementBytes==16&&p.offset==0&&p.flags==0x0/*CFG3.5 address withheld*/00000280ULL)bit=1;
        if(p.field.label.View()==u"ReturnValue"&&p.field.kind.View()==u"NameProperty"&&p.elementBytes==8&&p.offset==16&&p.flags==0x0/*CFG3.5 address withheld*/40000780ULL)bit=2;
        if(!bit||(mask&bit))return false;mask|=bit;
    }return mask==3;
}
// Check the returned CLASS against the SCMenuWidget owner of PushMenu, not
// SCHUD (GetCurrentMenu's owner). The native factory bytes prove this owner
// cache belongs to the pinned menu method on the supported executable.
template<class R,class Current>bool InspectClass(R& r,active::Context& source,std::uintptr_t loaded,
    unsigned& stage,cfgprobe::lifetime::SlotObservation& before,Current&& current){
    cfgnames::Observation<R> names(r,source.module);cfgnames::Text text;std::uintptr_t kind=0,menu=0,ctor=0;
    stage=1;if(!current())return false;
    stage=2;if(!cfgprobe::lifetime::ReadSlot(r,source.registry,loaded,before))return false;
    stage=3;if(!names.Object(loaded,text)||text.View()!=u"SPChallengesSettingsMenuWidget_C")return false;
    stage=4;if(!r.Read(loaded+cfgprobe::ObjectClass,kind)||!names.Object(kind,text)||text.View()!=u"WidgetBlueprintGeneratedClass")return false;
    stage=5;if(!r.Read(source.module+pushdisk::MenuClass,menu)||!r.Read(source.module+pushdisk::MenuConstructor,ctor)||
        !cfgprobe::MatchingClassCache(menu,ctor))return false;
    stage=6;if(!names.Object(menu,text)||text.View()!=u"SCMenuWidget")return false;
    stage=7;std::array<unsigned char,challengecalls::PushMenuFactory.size()> code{};
    if(!r.ReadBytes(source.module+challengecalls::PushMenu.factory,code.data(),code.size())||code!=challengecalls::PushMenuFactory)return false;
    stage=8;if(!maps::Subclass(r,loaded,menu))return false;
    stage=9;return true;
}
// Connected prerequisite, NOT an expanded settings menu. One ordinary native
// load on a real standalone Challenges lobby's owner callback. No widgets or
// modifiers are created. Source invalidation/failure leaves stock Start intact.
class Resolver {
    active::Context source_{};cfgnames::Words name_{};
    std::uintptr_t converter_=0,library_=0;bool attempted_=false;
    template<class R>bool Current(R& r){
        active::Context fresh;cfgnames::Observation<R> names(r,source_.module);cfgnames::Text label;
        if(!active::Observe(r,source_.module,fresh)||!(fresh==source_)||!names.Object(fresh.type,label)||label.View()!=u"SP_LobbyWidget_C")return false;
        std::uintptr_t native=0,ctor=0,modeType=0;
        if(!r.Read(source_.module+execdisk::FrontendClass,native)||!r.Read(source_.module+execdisk::FrontendConstructor,ctor)||
           !cfgprobe::MatchingClassCache(native,ctor)||!r.Read(source_.mode+cfgprobe::ObjectClass,modeType)||!maps::Subclass(r,modeType,native))return false;
        source_.registry=fresh.registry;return true;
    }
    template<class R,class E>bool Converter(R& r,E& engine,bool warm){
        using namespace cfgconvergence::disk;const auto& d=Conv_StringToName;
        std::uintptr_t owner=0,ctor=0,function=0;std::array<unsigned char,47> factory{};
        std::array<unsigned char,Conv_StringToNameExecutorCode.size()> code{};
        if(!Current(r)||!r.Read(source_.module+d.ownerReflection,owner)||!r.Read(source_.module+d.ownerConstructor,ctor)||
           !cfgprobe::MatchingClassCache(owner,ctor)||!r.ReadBytes(source_.module+d.factory,factory.data(),factory.size())||factory!=d.code||
           !r.Read(source_.module+d.cache,function))return false;
        if(!function&&(!warm||!engine.Factory(source_.module+d.factory,function)||!Current(r)))return false;
        cfgreflection::Observation<R> reflection(r,source_.module);cfgreflection::Field field;
        auto evidence=std::make_unique<capture::FunctionEvidence>();cfgprobe::Ancestry ancestry;std::uintptr_t receiver=0;
        if(!reflection.ReadField(function,owner,field)||!capture::KnownFunction(r,source_.module,field,u"Conv_StringToName",*evidence)||
           !NameSchema(*evidence)||evidence->nativeExecutor!=source_.module+Conv_StringToNameExecutor||
           !r.ReadBytes(source_.module+Conv_StringToNameExecutor,code.data(),code.size())||code!=Conv_StringToNameExecutorCode||
           !r.Read(owner+cfgspawn::ClassDefaultObject,receiver)||!cfgprobe::ObjectPointer(receiver)||
           !cfgprobe::CaptureAncestry(r,receiver,ancestry)||!ancestry.Contains(owner)||!cfgprobe::StableAncestry(r,receiver,ancestry)||
           !Current(r))return false;
        if(converter_&&(converter_!=function||library_!=receiver))return false;
        converter_=function;library_=receiver;return true;
    }
    template<class R>bool Inspect(R& r,std::uintptr_t loaded){
        cfgprobe::lifetime::SlotObservation before,after;
        if(!InspectClass(r,source_,loaded,inspectStage,before,[&](){return Current(r);}))return false;
        pageClass=loaded;cfgreflection::Observation<R> reflection(r,source_.module);cfgreflection::Field field;
        // Record individual contracts. A mask is evidence, never call authority.
        for(unsigned i=0;i<3;++i){const std::array<std::u16string_view,3> wanted{{u"Construct",u"OnClicked_Start",u"OnClicked_Back"}};
            auto f=std::make_unique<capture::FunctionEvidence>();
            if(reflection.Find(loaded,wanted[i],field)&&capture::KnownFunction(r,source_.module,field,wanted[i],*f)&&
               !f->parameterBytes&&!f->destructorLink)schemaMask|=1u<<i;
        }
        cfgreflection::Property property;
        if(reflection.Find(loaded,u"DifficultyComboBox",field)&&reflection.ReadProperty(field,property)&&
           field.kind.View()==u"ObjectProperty"&&property.dimension==1&&property.elementBytes==8)schemaMask|=8;
        inspectStage=10;
        if(!Current(r)||!cfgprobe::lifetime::ReadSlot(r,source_.registry,loaded,after)||!(after==before))return false;
        inspectStage=0;return true;
    }
public:
    unsigned stage=0,failure=0,attempts=0,schemaMask=0;std::uintptr_t pageClass=0;
    unsigned reacquireFailure=0,inspectStage=0;
    // Reacquire through the verified owned soft reference, then consume the
    // class in this SAME callback. The diagnostic pageClass is never a lease.
    template<class R,class Consume>bool Reacquire(R& r,const active::Context& source,Consume consume){
        reacquireFailure=1;if(stage!=4||name_.index<=0||source.thread!=GetCurrentThreadId())return false;
        source_=source;reacquireFailure=2;if(!Current(r))return false;
        const auto loaded=population::ResolveClass(r,source.module,SoftReference(name_),source.thread,[&](){return Current(r);},true,
            CFG_J22_CHALLENGES_RETENTION?startupretain::Call::FrontendChallengePageClass:startupretain::Call::LoadClass);
        reacquireFailure=3;if(!loaded)return false;
        reacquireFailure=4;if(!Inspect(r,loaded))return false;
        reacquireFailure=5;if(!consume(loaded))return false;
        reacquireFailure=0;return true;
    }
    template<class R,class E>void Service(R& r,E& engine,const active::Context& context){
        if(stage>=4||context.thread!=GetCurrentThreadId())return;
        if(!attempted_){source_=context;attempted_=true;}
        const auto refuse=[&](unsigned why){failure=why;stage=5;};
        if(!(context==source_)||!Current(r)||!engine.BeginPhase()){refuse(1);return;}
        if(stage==0){if(!Converter(r,engine,true)){refuse(2);return;}stage=1;return;}
        if(stage==1){
            if(!Converter(r,engine,false)){refuse(3);return;}
            NamePacket packet{{Path,static_cast<int>(std::size(Path)),static_cast<int>(std::size(Path))},{}};
            if(!engine.Invoke(library_,converter_,&packet)||!Current(r)||packet.result.index<=0||packet.result.number<0){refuse(4);return;}
            name_=packet.result;stage=2;return;
        }
        if(stage==2){
            // Consume the attempt BEFORE synchronous allocation/native loading.
            ++attempts;const auto packet=SoftReference(name_);
            const auto loaded=population::ResolveClass(r,source_.module,packet,source_.thread,[&](){return Current(r);},true,
                CFG_J22_CHALLENGES_RETENTION?startupretain::Call::FrontendChallengePageClass:startupretain::Call::LoadClass);
            if(!loaded){refuse(5);return;}
            // Inspect on the SAME callback: a returned metadata pointer with
            // unassigned serial must not become a retained cross-event lease.
            if(!Inspect(r,loaded)){refuse(6);return;}stage=4;return;
        }
    }
};
}
