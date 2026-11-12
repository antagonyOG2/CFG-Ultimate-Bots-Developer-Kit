#pragma once
#include "NativeOfflineCounselorBirth.hpp"
#include "NativeOfflineLifecycleCalls.hpp"
#include "NativeOfflineIntroMetadata.hpp"
#include "NativeIntroPresentation.hpp"
#include "NativeOfflineRestart.hpp"
#include "NativeSoftClassLoader.hpp"
#include "NativeSelectedJasonPath.hpp"
#include "../RuntimeProbe/OfflineSettingsBindings.generated.hpp"
#include <intrin.h>
#include <memory>
#include "../Core/NativeStartupDiagnostic.hpp"
#include "../Core/NativeCounselorPopulationPolicy.hpp"
#include "OfflineDiagnosticTrace.hpp"
namespace cfgmenu::offlinestartup {
inline bool SkillExchange(std::uintptr_t address,std::uint8_t previous,std::uint8_t next)noexcept{
    __try{return static_cast<std::uint8_t>(_InterlockedCompareExchange8(reinterpret_cast<volatile CHAR*>(address),static_cast<CHAR>(next),static_cast<CHAR>(previous)))==previous;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
enum class Stage {Empty,Armed,Owner,Classes,Bots,Human,Metadata,Intro,AwaitMatch,Match,Restore,Ready,Refused};
struct Report {
    NativeStartupDiagnostic detail{};
    Stage stage=Stage::Empty;unsigned failure=0,bots=0;
    std::uintptr_t world=0,mode=0,human=0;
    bool introCalled=false,matchCalled=false,skillVerified=false,cleanupAttempted=false,cleanupRestored=false;
    unsigned nativeSkill=3;
    std::array<offlinepopulation::Born,31> roster{};
};
// Actual donor startup coordinator. Hooks and frontend own its process-long
// lifetime. It handles only one explicitly armed, fresh offline world.
template<class Reader> class Startup {
    Reader& r_;freshgame::Source source_{};promotion::Packet counselor_{},killer_{};
    unsigned requested_=0,skill_=3;bool busy_=false;promotion::Context owner_{};
    std::uintptr_t claimedReceiver_=0;
    std::uintptr_t counselorClass_=0,killerClass_=0;
    cfgselectedjason::Choice selectedClass_{};bool selectedPathAttempted_=false;
    std::unique_ptr<offlinelifecycle::Calls> native_;
    std::unique_ptr<intrometadata::WindowsHost<Reader>> metadata_;
    std::unique_ptr<intrometadata::Borrow<intrometadata::WindowsHost<Reader>>> borrow_;
    Report report_{};
    cfgending::skin::IntroPresentation presentation_{};
    cfgnames::Text package_{};
    bool Fresh(){
        cfgprobe::lifetime::ArrayHeader fresh{};
        if(!cfgprobe::lifetime::ReadArrayHeader(r_,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh)||
           !cfgprobe::lifetime::RevalidateBatch(r_,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh,owner_.identities))return false;
        owner_.registry=fresh;return offlineowner::Current(r_,owner_,GetCurrentThreadId());
    }
    bool Failure(unsigned code){
        if(borrow_&&borrow_->Status()==intrometadata::State::Borrowed){
            report_.cleanupAttempted=true;report_.cleanupRestored=borrow_->Restore();
        }
        report_.failure=code;report_.stage=Stage::Refused;return false;
    }
    bool Skill(){
        if(!Fresh())return false;
        if(skill_<3){std::uint8_t current=3;
            if(!promotion::detail::Code(owner_.module,owner_.module+cfgofflinesettings::disk::SkillConsumer,cfgofflinesettings::disk::SkillConsumerCode)||
               !r_.Read(owner_.mode+cfgofflinesettings::disk::CounselorSkill,current)||current>2||!Fresh()||
               (current!=skill_&&!SkillExchange(owner_.mode+cfgofflinesettings::disk::CounselorSkill,current,static_cast<std::uint8_t>(skill_)))||
               !Fresh()||!offlineowner::Same(r_,owner_.mode+cfgofflinesettings::disk::CounselorSkill,static_cast<std::uint8_t>(skill_)))return false;
        }
        report_.nativeSkill=skill_;report_.skillVerified=true;return true;
    }
    bool Human(){
        offlineowner::Human human{};
        report_.detail.human.stage=17;
        if(!Fresh()||!offlineowner::VerifyHuman(r_,owner_,counselor_,counselorClass_,GetCurrentThreadId(),human,nullptr,&report_.detail.human))return false;
        report_.human=human.pawn;return true;
    }
    bool Population(unsigned nativeRequested){
        auto& diag=report_.detail;diag.phase=3;
        std::uintptr_t type=0,counselorBase=0;cfgreflection::Field field;
        std::int32_t bytes=0,dim=0,offset=0;cfgspawn::PlayerArray roster{};
        if(!Fresh()||!r_.Read(owner_.module+cfgofflinesettings::disk::SCGameModeClass,type)||
           !offlineowner::Same(r_,owner_.module+cfgofflinesettings::disk::SCGameModeConstructor,type)||!cfgspawn::ObjectIs(r_,owner_.mode,type)||
           !r_.Read(owner_.module+birthdisk::SCCounselorCharacterClass,counselorBase))return false;
        cfgreflection::Observation<Reader> reflection(r_,owner_.module);
        diag.phase=4;if(!reflection.Find(type,u"CounselorCharacterClasses",field,false))return false;
        diag.field=field.object;diag.phase=5;if(field.kind.View()!=u"ArrayProperty"||
           !r_.Read(field.object+0x30,dim)||!r_.Read(field.object+0x34,bytes)||!r_.Read(field.object+0x44,offset))return false;
        diag.dimensions=dim;diag.elementBytes=bytes;diag.offset=offset;
        if(dim!=1||bytes!=16||offset!=0x560)return false;
        diag.phase=6;if(!r_.Read(owner_.mode+offset,roster))return false;
        diag.count=roster.count;diag.capacity=roster.capacity;
        if(roster.count<1||roster.count>64||roster.capacity<roster.count||roster.capacity>256||
           !cfgprobe::ObjectPointer(roster.data))return false;
        const unsigned start=static_cast<unsigned>(GetTickCount64()%roster.count);
        for(unsigned candidate=0;candidate<static_cast<unsigned>(roster.count)&&report_.bots<nativeRequested;++candidate){
            const unsigned index=(start+candidate)%static_cast<unsigned>(roster.count);
            diag.phase=7;diag.rosterIndex=index;
            promotion::Packet packet{},again{};
            if(!Fresh()||!r_.Read(roster.data+40*index,packet))return false;
            if(!cfgending::EmptySubpath(packet.bytes))continue;
            if(cfgending::SamePath(packet.bytes,counselor_.bytes))continue;
            const auto selected=population::ResolveClass(r_,owner_.module,packet,owner_.thread,[&](){return Fresh();},true);
            diag.selectedClass=selected;diag.phase=8;
            cfgspawn::PlayerArray current{};
            if(!Fresh()||!r_.Read(owner_.mode+offset,current)||current.data!=roster.data||current.count!=roster.count||
               !r_.Read(roster.data+40*index,again)||again.bytes!=packet.bytes)return false;
            if(!selected||selected==counselorClass_||!classchoice::MatchesPath(r_,owner_.module,selected,counselorBase,packet.bytes))continue;
            offlinepopulation::Birth<Reader> birth(r_,owner_);
            diag.phase=9;const bool born=birth.Run(selected,packet);const auto& result=birth.Report();
            diag.birthPhase=result.phase;diag.controller=result.controller;diag.state=result.state;diag.pawn=result.pawn;
            diag.controllerSerial=result.identities[0].serial;diag.stateSerial=result.identities[1].serial;
            diag.pawnSerial=result.identities[2].serial;diag.entered=result.entered?1u:0u;
            if(!born){cfgclosedtrace::Mark("population-birth-refused",result.state,
                (static_cast<std::uint64_t>(result.phase)<<16)|result.travelRefusal);return false;}
            report_.roster[report_.bots++]=birth.Report();
        }
        diag.phase=10;return report_.bots==nativeRequested&&Fresh();
    }
    bool BirthHuman(){
        report_.stage=Stage::Human;
        owner_.selectedClass=counselorClass_;owner_.picked=counselor_;owner_.pickedResolutionVerified=true;
        promotion::WindowsCalls calls(owner_.module,owner_.thread,true);auto selected=counselor_;
        if(!Fresh()||!calls.Consume(owner_,selected)||!Fresh())return Failure(4);
        native_=std::make_unique<offlinelifecycle::Calls>(owner_);
        if(!native_->RestartSelectedHuman(r_))return Failure(501);
        report_.detail.human.restartReturned=1;
        return Human()||Failure(502);
    }
    bool BirthBots(unsigned nativeRequested){
        report_.stage=Stage::Bots;
        if(nativeRequested&&!population::WarmCounselorController(r_,owner_.module,owner_.thread,[&](){return Fresh();},true))return Failure(202);
        if(!Population(nativeRequested))return Failure(report_.detail.phase==9?400+report_.detail.birthPhase:300+report_.detail.phase);
        return true;
    }
public:
    explicit Startup(Reader& r):r_(r){}
    const Report& Status()const{return report_;}
    const promotion::Context& Owner()const{return owner_;}
    const cfgending::skin::IntroPresentation& Presentation()const{return presentation_;}
    bool CanResetEpoch()const noexcept{return !busy_&&report_.stage==Stage::Ready&&borrow_&&borrow_->Status()==intrometadata::State::Restored;}
    bool ResetEpoch()noexcept{
        if(!CanResetEpoch())return false;
        // No old-world native calls/destructor rollback. Parent hooks are
        // process-resident and remain installed. All metadata was restored.
        borrow_.reset();metadata_.reset();native_.reset();source_={};owner_={};counselor_={};killer_={};
        claimedReceiver_=counselorClass_=killerClass_=0;selectedClass_={};selectedPathAttempted_=false;
        requested_=0;skill_=3;report_={};package_={};presentation_={};return true;
    }
    bool RestartCandidate(std::uintptr_t receiver,std::uint32_t thread,promotion::Context& fresh){
        fresh={};return CanResetEpoch()&&offlinerestart::Observe(r_,source_,owner_,package_,receiver,thread,fresh);
    }
    bool Restart(std::uintptr_t receiver,std::uint32_t thread){
        promotion::Context fresh;if(!RestartCandidate(receiver,thread,fresh))return false;
        const auto source=source_;const auto counselor=counselor_,killer=killer_;const auto selected=selectedClass_;const auto requested=requested_,skill=skill_;
        return ResetEpoch()&&Arm(source,counselor,killer,requested,skill,selected);
    }
    void Abort(unsigned code){Failure(code);}
    bool Arm(const freshgame::Source& source,const promotion::Packet& counselor,const promotion::Packet& killer,unsigned helpers,unsigned skill=3,
        const cfgselectedjason::Choice& selected={}){
        if(report_.stage!=Stage::Empty||!cfgpopulation::Make(helpers).valid||skill>3||!source.nativeOffline||!source.thread||!source.frontendWorld||
           !cfgending::EmptySubpath(counselor.bytes)||(!selected.Empty()&&!selected.Valid())||
           (selected.Empty()&&!cfgending::EmptySubpath(killer.bytes)))return false;
        source_=source;counselor_=counselor;killer_=killer;selectedClass_=selected;
        requested_=helpers;skill_=skill;report_.stage=Stage::Armed;return true;
    }
    bool Claimed(std::uintptr_t receiver,DWORD thread)const{
        return thread==source_.thread&&report_.stage!=Stage::Empty&&
            (owner_.mode?receiver==owner_.mode:claimedReceiver_?receiver==claimedReceiver_:report_.stage==Stage::Armed)&&
            // A retired mode address can be reused by an unrelated next match.
            // Never swallow that native intro/match using address equality.
            (report_.stage!=Stage::Ready||offlineowner::Current(r_,owner_,thread));
    }
    bool Intro(std::uintptr_t receiver){
        if(busy_||report_.stage!=Stage::Armed)return report_.introCalled;
        struct Scope{bool& b;Scope(bool& value):b(value){b=true;}~Scope(){b=false;}}scope(busy_);
        claimedReceiver_=receiver;
        cfgclosedtrace::Mark("intro-hook-entry",receiver);
        report_.stage=Stage::Owner;
        unsigned ownerStage=0;
        if(!offlineowner::Observe(r_,source_,receiver,GetCurrentThreadId(),true,owner_,nullptr,&ownerStage))return Failure(100+ownerStage);
        if(!offlinerestart::Package(r_,owner_,package_))return Failure(111);
        report_.world=owner_.world;report_.mode=owner_.mode;report_.stage=Stage::Classes;
        // Preserve the exact loaded choice before even the counselor loader
        // can allocate/reenter. Identity checks alone never pin a UObject.
        if(selectedClass_.Valid()){
            cfgprobe::lifetime::ArrayHeader registry;std::uintptr_t base=0;
            if(!r_.Read(owner_.module+birthdisk::SCKillerCharacterClass,base)||
                !cfgprobe::lifetime::ReadArrayHeader(r_,owner_.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
                !cfgselectedjason::Current(r_,owner_.module,registry,base,selectedClass_)||
                !startupretain::Retain(owner_.module,std::array<std::uintptr_t,2>{selectedClass_.identity.object,selectedClass_.defaultsIdentity.object})||!Fresh())return Failure(2);
        }
        counselorClass_=population::ResolveClass(r_,owner_.module,counselor_,owner_.thread,[&](){return Fresh();},true);
        std::uintptr_t counselorBase=0,killerBase=0;
        if(selectedClass_.Valid()){
            if(selectedPathAttempted_||!r_.Read(owner_.module+birthdisk::SCKillerCharacterClass,killerBase))return Failure(2);
            selectedPathAttempted_=true; // consume before allocating/reentrant engine work
            if(!cfgselectedjason::MakePath(r_,owner_.module,selectedClass_,killerBase,[&](){return Fresh();},killer_))return Failure(2);
            killerClass_=selectedClass_.identity.object;
        }else killerClass_=population::ResolveClass(r_,owner_.module,killer_,owner_.thread,[&](){return Fresh();},true);
        if(!counselorClass_||!killerClass_||!Fresh()||
           !r_.Read(owner_.module+birthdisk::SCCounselorCharacterClass,counselorBase)||
           !r_.Read(owner_.module+birthdisk::SCKillerCharacterClass,killerBase)||
           !classchoice::MatchesPath(r_,owner_.module,counselorClass_,counselorBase,counselor_.bytes)||
           !classchoice::MatchesPath(r_,owner_.module,killerClass_,killerBase,killer_.bytes))return Failure(2);
        report_.detail.phase=1;if(!Skill())return Failure(201);
        cfgclosedtrace::Mark("intro-classes-ready",receiver);
        report_.detail.skillVerified=1;report_.detail.phase=2;
        // Reserve the human's native authored start BEFORE any bot can consume
        // it. The ten-total path previously created nine bots first, exhausted
        // native starts, and accepted a human born at world origin.
        struct BirthHost {
            Startup& startup;
            bool Human(){return startup.BirthHuman();}
            bool Population(unsigned count){return startup.BirthBots(count);}
        }birth{*this};
        if(!cfgpopulation::Birth(birth,requested_))return false;
        cfgclosedtrace::Mark("intro-population-ready",receiver,requested_);
        intrometadata::Context context;context.owner=owner_;context.pawn=report_.human;context.killerClass=killerClass_;
        context.diagnostic=&report_.detail.metadata;
        context.counselor=counselor_;context.killer=killer_;
        context.counselorSource=reinterpret_cast<std::uintptr_t>(&counselor_);context.killerSource=reinterpret_cast<std::uintptr_t>(&killer_);
        if(!r_.Read(owner_.world+cfgprobe::WorldGameState,context.gameState)||!Fresh()||
           !cfgprobe::lifetime::ReadSlot(r_,owner_.registry,context.gameState,context.identities[0])||
           !cfgprobe::lifetime::ReadSlot(r_,owner_.registry,context.pawn,context.identities[1]))return Failure(6);
        report_.stage=Stage::Metadata;
        metadata_=std::make_unique<intrometadata::WindowsHost<Reader>>(r_,context);
        borrow_=std::make_unique<intrometadata::Borrow<intrometadata::WindowsHost<Reader>>>(*metadata_);
        const bool prepared=borrow_->Prepare();
        report_.detail.metadata.borrowStage=borrow_->Progress();report_.detail.metadata.borrowState=static_cast<unsigned>(borrow_->Status());
        if(!prepared)return Failure(7);
        cfgclosedtrace::Mark("intro-before-parent",receiver);
        report_.stage=Stage::Intro;
        if(!native_->ParentIntro(r_))return Failure(8);
        cfgclosedtrace::Mark("intro-parent-returned",receiver);
        report_.introCalled=true;report_.stage=Stage::AwaitMatch;return true;
    }
    bool Match(std::uintptr_t receiver){
        if(busy_||report_.stage!=Stage::AwaitMatch||receiver!=owner_.mode)return report_.matchCalled;
        struct Scope{bool& b;Scope(bool& value):b(value){b=true;}~Scope(){b=false;}}scope(busy_);
        report_.stage=Stage::Match;
        cfgclosedtrace::Mark("intro-match-entry",receiver);
        // Read-only donor presentation witness BEFORE restoring borrowed intro
        // ownership. Failure must not block human possession/native gameplay.
        std::uintptr_t gameState=0;
        if(Fresh()&&metadata_&&borrow_->Status()==intrometadata::State::Borrowed&&
           r_.Read(owner_.world+cfgprobe::WorldGameState,gameState))
            presentation_.Capture(r_,owner_.module,owner_.registry,owner_.world,gameState,
                owner_.controller,owner_.playerState,killer_.bytes,[&](){return metadata_->Fresh();});
        cfgclosedtrace::Mark("intro-selected-skin-proof",presentation_.observedSkin,presentation_.stage);
        if(!Fresh()||!native_->ParentMatchStart(r_))return Failure(9);
        report_.matchCalled=true;report_.stage=Stage::Restore;
        // Restore before handing the empty primary slot to the existing guarded
        // AI backend. Intro has completed and human possession stays unchanged.
        if(!borrow_->Restore()||!Human())return Failure(10);
        // Keep the diagnostic borrow fields current (was only the Prepare snapshot).
        report_.detail.metadata.borrowStage=borrow_->Progress();report_.detail.metadata.borrowState=static_cast<unsigned>(borrow_->Status());
        report_.stage=Stage::Ready;return true;
    }
};
}
