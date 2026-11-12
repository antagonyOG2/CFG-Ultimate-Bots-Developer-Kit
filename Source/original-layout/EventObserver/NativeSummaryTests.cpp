#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "NativeSummaryObservation.hpp"
#include "NativeSummaryBotRoster.hpp"
#include "../Core/EndingSummaryPolicy.hpp"
#include "EndingSummaryOpportunity.hpp"
namespace cfgclosedtrace {template<class A,class B>void Mark(const char*,A,B){}}
struct SummaryFixture:SpectatorFixture {
    std::uintptr_t hud=0,root=0,display=0,state=0,rows=0;
    void Name(std::uintptr_t object,int index,const char* text){
        auto entry=std::uintptr_t{0x830000}+static_cast<unsigned>(index)*256;
        r_.Put(0x810000+8ULL*index,entry);r_.Put(entry,std::uint32_t{});
        std::array<char,128> chars{};for(unsigned i=0;text[i]&&i<127;++i)chars[i]=text[i];
        r_.Put(entry+cfgnames::disk::EntryText,chars);r_.Put(object+cfgnames::disk::ObjectName,cfgnames::Words{index,0});
    }
    SummaryFixture(){r_.cap=100000;
        auto hudType=r_.Object(),rootType=r_.Object(),displayType=r_.Object(),listType=r_.Object(),itemType=r_.Object(),rowType=r_.Object();
        Name(rootType,1,"ScoreboardWidget_C");Name(displayType,2,"ScoreboardDisplayWidget_C");Name(rowType,3,"ScoreboardPlayerListItem_C");
        r_.Put(0x800000+cfgnames::disk::ElementCount,std::array<int,2>{4,1});
        r_.Put(displayType+cfgprobe::ClassSuper,listType);r_.Put(rowType+cfgprobe::ClassSuper,itemType);
        auto rowDefault=r_.Object(rowType);r_.Put(rowType+cfgspawn::ClassDefaultObject,rowDefault);
        hud=r_.Object(hudType);root=r_.Object(rootType);display=r_.Object(displayType);
        for(auto object:{hud,root,display})Serial(object);
        r_.Put(plan.module+cfgsweater::huddisk::SCInGameHUDReflection,hudType);r_.Put(plan.module+cfgsweater::huddisk::SCInGameHUDConstructor,hudType);
        r_.Put(plan.module+cfgsummary::disk::ListClass,listType);r_.Put(plan.module+cfgsummary::disk::ListConstructor,listType);
        r_.Put(plan.module+cfgsummary::disk::ItemClass,itemType);r_.Put(plan.module+cfgsummary::disk::ItemConstructor,itemType);
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateReflection,r_.classes[cfgspawn::PlayerState]);
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,r_.classes[cfgspawn::PlayerState]);
        r_.Put(plan.controller+cfgsweater::huddisk::ControllerHUD,hud);r_.Put(hud+0x368,plan.controller);
        r_.Put(hud+cfgsummary::disk::Scoreboard,root);r_.Put(root+cfgsummary::disk::Display,display);
        r_.Put(display,std::uintptr_t{0x940000});r_.Put(0x940000+cfgsummary::disk::RegisterSlot,plan.module+cfgsummary::disk::Register);
        r_.Put(plan.module+cfgsummary::disk::Register,cfgsummary::disk::RegisterCode);
        r_.Put(display+cfgsummary::disk::RowClass,rowType);
        state=r_.Object(r_.classes[cfgspawn::PlayerState]); // legitimate strong PS with serial0
        r_.Put(state+0x88,r_.botController);r_.Put(0x950000,state);
        r_.Put(spawnAssets.gameState+cfgsummary::disk::Players,cfgsummary::List{0x950000,1,1});
        rows=0x960000;r_.Put(display+cfgsummary::disk::Registered,cfgsummary::List{rows,0,4});
        r_.Put(display+cfgsummary::disk::Rows,cfgsummary::List{});
        r_.reads=0;r_.cap=1536;
    }
    bool Scope(){return Context()&&SpectatorLocalOwner();}
    bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& out){return cfgspawn::ObjectSlot(r_,context.objectArray,object,out);}
    bool StateType(std::uintptr_t object){return cfgspawn::ObjectIs(r_,object,spawnAssets.resources.classes[cfgspawn::PlayerState]);}
    bool Observe(cfgsummary::Proof& p){r_.reads=0;return cfgsummary::Observe(r_,*this,plan.module,plan.controller,spawnAssets.gameState,state,0,p);}
};
struct StatusFixture:SummaryFixture {
    std::uintptr_t stack=0,statusRoot=0,statusType=0,statusDisplay=0,property=0,propertyType=0;
    cfgstatussurface::Runtime surface;
    StatusFixture(){r_.cap=100000;auto stackType=r_.Object();stack=r_.Object(stackType);Serial(stack);
        statusType=r_.Object();statusRoot=r_.Object(statusType);Serial(statusRoot);Name(statusType,6,"CounselorStatusPanel_C");
        propertyType=r_.Object();Name(propertyType,5,"ObjectProperty");property=r_.Object(propertyType);Name(property,4,"LiveStatusPlayers");
        r_.Put(0x800000+cfgnames::disk::ElementCount,std::array<int,2>{7,1});
        r_.Put(plan.module+cfgmenu::parentdisk::Owner,stackType);r_.Put(plan.module+cfgmenu::parentdisk::Constructor,stackType);
        r_.Put(hud+cfgmenu::execdisk::HUDStack,stack);r_.Put(stack+0x20,hud);r_.Put(statusRoot+0x20,stack);
        r_.Put(stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{0x9A0000,1,1});
        r_.Put(0x9A0000+cfgmenu::execdisk::RecordWidget,statusRoot);
        std::uintptr_t type=0,rowType=0;CHECK(r_.Read(display+cfgprobe::ObjectClass,type));CHECK(r_.Read(display+cfgsummary::disk::RowClass,rowType));
        statusDisplay=r_.Object(type);Serial(statusDisplay);r_.Put(statusDisplay,std::uintptr_t{0x940000});
        r_.Put(statusDisplay+cfgsummary::disk::RowClass,rowType);r_.Put(statusDisplay+cfgsummary::disk::Registered,cfgsummary::List{});
        r_.Put(statusDisplay+cfgsummary::disk::Rows,cfgsummary::List{});r_.Put(statusDisplay+0x3D8,std::uint8_t{});
        r_.Put(statusType+cfgreflection::disk::Children,property);r_.Put(property+0x20,statusType);r_.Put(property+0x28,std::uintptr_t{});
        r_.Put(property+cfgreflection::disk::PropertyDimensions,1);r_.Put(property+cfgreflection::disk::PropertyElementSize,8);
        r_.Put(property+cfgreflection::disk::PropertyFlags,std::uint64_t{});r_.Put(property+cfgreflection::disk::PropertyOffset,0x500);
        r_.Put(statusRoot+0x500,statusDisplay);r_.reads=0;r_.cap=1536;
    }
};
struct SummaryRuntimeFixture:SummaryFixture {
    using BundleAccess=SummaryRuntimeFixture;
    unsigned Reads()const{return r_.reads;}
    cfgsummary::Runtime bundleSummary;cfgsummarybots::Roster bundleSummaryBots;
    cfgsummary::Runtime bundleStatusRows;cfgsummarybots::Roster bundleStatusBots;
    cfgstatussurface::Runtime bundleStatusSurface;unsigned bundleSummarySurfaceTurn=0;
    bool bundleSummaryFilterEnabled=true;
    bool bundleSummaryRowReadyEnabled=true;
    bool bundleSummaryPresentationEnabled=true;
    bool bundleSummaryStackBound=false;
    struct {unsigned bots=1;struct Bot{std::uintptr_t controller=0;};std::array<Bot,9> roster{};}bundleReport;
    std::array<cfgprobe::lifetime::SlotObservation,9> bundleControllerSlots{};
    unsigned nativeRegistrations=0,registerMode=0;bool multiple=false;
    bool PrepareBundleSummaryFilter(std::uintptr_t){return false;}
    bool PrepareBundleSummaryRowReady(std::uintptr_t){return false;}
    bool PrepareBundleSummaryPresentation(std::uintptr_t){return false;}
    bool NativeEnvironmentAllowed(){return GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&cfgprimary::LookupSafe(r_,plan.module);}
    SummaryRuntimeFixture(){r_.cap=100000;r_.Put(state+0x394,4u);r_.Put(state+0x13C,std::uint8_t{});
        bundleReport.roster[0].controller=r_.botController;Serial(r_.botController);
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,r_.botController,bundleControllerSlots[0]));
        r_.Put(r_.botController+cfgescape::disk::PlayerState,state);r_.reads=0;r_.cap=1536;
    }
    bool FixtureSummaryRegister(std::uintptr_t widget,std::uintptr_t value){++nativeRegistrations;
        if(multiple){
            cfgsummary::List registered{},items{};std::uintptr_t rowType=0;
            CHECK(widget==display&&r_.Read(display+cfgsummary::disk::Registered,registered)&&r_.Read(display+cfgsummary::disk::Rows,items));
            CHECK(registered.count==items.count&&registered.count<32&&r_.Read(display+cfgsummary::disk::RowClass,rowType));
            const auto row=r_.Object(rowType);Serial(row);r_.Put(row+0x20,display);r_.Put(row+cfgsummary::disk::RowPlayerState,value);
            r_.Put(rows+8ULL*static_cast<unsigned>(registered.count),value);r_.Put(0x980000+8ULL*static_cast<unsigned>(items.count),row);
            r_.Put(display+cfgsummary::disk::Registered,cfgsummary::List{rows,registered.count+1,4});
            r_.Put(display+cfgsummary::disk::Rows,cfgsummary::List{0x980000,items.count+1,4});return true;
        }
        CHECK(widget==display&&value==state);r_.Put(rows,state);
        r_.Put(display+cfgsummary::disk::Registered,cfgsummary::List{rows,1,4});
        if(registerMode==1)return true; // registered but no actual row: ambiguous
        std::uintptr_t rowType=0;CHECK(r_.Read(display+cfgsummary::disk::RowClass,rowType));
        const auto row=r_.Object(rowType);Serial(row);r_.Put(row+0x20,display);
        r_.Put(row+cfgsummary::disk::RowPlayerState,state);r_.Put(0x980000,row);
        r_.Put(display+cfgsummary::disk::Rows,cfgsummary::List{0x980000,1,1});return true;
    }
#define CFG_SUMMARY_MEMBER_FIXTURE_ONLY
#define CFG_CALLBACK_OBSERVATION_MEMBER_FIXTURE_ONLY
#pragma warning(push)
#pragma warning(disable:4458)
#include "NativeSummaryRuntimeMembers.hpp"
#pragma warning(pop)
#undef CFG_SUMMARY_MEMBER_FIXTURE_ONLY
#undef CFG_CALLBACK_OBSERVATION_MEMBER_FIXTURE_ONLY
};
#ifndef CFG_SUMMARY_SCOPE_FIXTURE_ONLY
#ifndef CFG_SUMMARY_TEST_MAIN
#define CFG_SUMMARY_TEST_MAIN main
#endif
int CFG_SUMMARY_TEST_MAIN(){
    if(CFG_J3_END_SUMMARY||CFG_J22_END_SUMMARY){StatusFixture f;
     f.Name(f.statusType,6,"CounselorHUDWidget_C");
     f.r_.Put(f.hud+cfgsweater::huddisk::HUDWidget,f.statusRoot);
     f.r_.Put(f.stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{0x9A0000,0,1});
     bool ready=false;for(unsigned turn=0;turn<48&&!ready;++turn){f.r_.reads=0;ready=f.surface.Step(f.r_,f,f.plan.module,f.plan.controller);CHECK(f.r_.reads<=1536);}
     CHECK(ready&&f.surface.binding.hudRoot&&f.surface.binding.root.object==f.statusRoot&&f.surface.binding.display.object==f.statusDisplay);
     f.r_.reads=0;CHECK(cfgstatussurface::Current(f.r_,f,f.plan.module,f.plan.controller,f.surface.binding));
     f.r_.Put(f.hud+cfgsweater::huddisk::HUDWidget,f.root);f.r_.reads=0;
     CHECK(!cfgstatussurface::Current(f.r_,f,f.plan.module,f.plan.controller,f.surface.binding));
    }
    {SummaryFixture f;f.r_.cap=1536;
     for(const auto object:{f.hud,f.root,f.display}){f.r_.reads=0;
      CHECK(cfgsummaryending::WidgetCandidate(f.r_,f.plan.controller,object)&&f.r_.reads<=16);}
     f.r_.reads=0;CHECK(!cfgsummaryending::WidgetCandidate(f.r_,f.plan.controller,f.state));}
    {StatusFixture f;f.r_.reads=0;CHECK(cfgsummaryending::WidgetCandidate(f.r_,f.plan.controller,f.statusRoot)&&f.r_.reads<=16);
     f.r_.Put(f.stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{0x9A0000,65,65});f.r_.reads=0;
     CHECK(!cfgsummaryending::WidgetCandidate(f.r_,f.plan.controller,f.statusRoot));}
    {SummaryRuntimeFixture f;StatusFixture ending;
     static_cast<SummaryFixture&>(f)=static_cast<const SummaryFixture&>(ending);
     f.display=ending.statusDisplay;f.r_.cap=100000;
     f.r_.Put(f.state+0x394,4u);f.r_.Put(f.state+0x13C,std::uint8_t{});f.r_.Put(f.state+0x88,std::uintptr_t{});
     f.r_.Put(f.state+cfgescape::disk::bDead,std::uint8_t{1});
     f.r_.Put(f.hud+cfgsummary::disk::Scoreboard,std::uintptr_t{}); // gameplay scoreboard no longer owns results
     f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{});
     f.r_.Put(f.bundleJason.killerController.object+0x370,std::uintptr_t{});
     f.r_.cap=1536;unsigned peak=0;
     for(unsigned now=100;now<=1600;now+=100){f.r_.reads=0;CHECK(f.SummaryPhase(now,true));peak=(std::max)(peak,f.r_.reads);CHECK(f.r_.reads<=1536);}
     CHECK(f.nativeRegistrations==1&&f.bundleSummaryStackBound);
     std::uintptr_t row=0;CHECK(f.r_.Read(0x980000,row));f.r_.reads=0;CHECK(f.SummaryRowReady(row));
     f.r_.Put(f.state+0x451,std::uint8_t{});f.r_.reads=0;CHECK(f.SummaryPresentedState(row,f.state));
     f.r_.Put(0x9A0000+cfgmenu::execdisk::RecordWidget,f.root);f.r_.reads=0;CHECK(!f.SummaryRowReady(row));
     f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);f.r_.reads=0;CHECK(f.SummaryPhase(1700,true)&&f.nativeRegistrations==1);
     std::printf("Actual ending row reconstruction: no primary/controller owner/gameplay scoreboard, current native stack: peak%u/1536.\n",peak);}
    {SummaryRuntimeFixture f;f.multiple=true;f.bundleReport.bots=0;f.r_.cap=100000;
     // Native Jason + dead/escaped bots not dependent on helper-controller
     // tracking or a live pawn. All use native role/outcome/portrait getters.
     for(unsigned i=1;i<3;++i){const auto state=f.r_.Object(f.r_.classes[cfgspawn::PlayerState]);
      f.r_.Put(state+0x88,f.r_.botController);f.r_.Put(state+0x394,4u);f.r_.Put(state+0x13C,std::uint8_t{});
      f.r_.Put(state+(i==1?cfgescape::disk::bDead:cfgescape::disk::bEscaped),std::uint8_t{1});
      f.r_.Put(0x950000+8ULL*i,state);}
     f.r_.Put(f.spawnAssets.gameState+cfgsummary::disk::Players,cfgsummary::List{0x950000,3,3});
     f.r_.cap=1536;for(unsigned now=100;now<=1200;now+=100){f.r_.reads=0;CHECK(f.SummaryPhase(now));CHECK(f.r_.reads<=1536);}
     CHECK(f.nativeRegistrations==3);}
    {SummaryRuntimeFixture f;f.multiple=true;f.bundleReport.bots=3;f.r_.cap=100000;
     std::array<std::uintptr_t,3> states{{f.state,0,0}};
     for(unsigned i=1;i<3;++i){const auto controller=f.r_.Object(f.r_.classes[cfgspawn::CounselorAI]);f.Serial(controller);
        const auto state=f.r_.Object(f.r_.classes[cfgspawn::PlayerState]);states[i]=state;
        f.r_.Put(state+0x88,controller);f.r_.Put(state+0x394,4u);f.r_.Put(state+0x13C,std::uint8_t{});
        f.r_.Put(controller+cfgescape::disk::PlayerState,state);f.bundleReport.roster[i].controller=controller;
        CHECK(cfgspawn::ObjectSlot(f.r_,f.context.objectArray,controller,f.bundleControllerSlots[i]));f.r_.Put(0x950000+8ULL*i,state);}
     f.r_.Put(f.spawnAssets.gameState+cfgsummary::disk::Players,cfgsummary::List{0x950000,3,3});f.r_.cap=1536;
     for(unsigned now=100;now<=1200;now+=100){f.r_.reads=0;CHECK(f.SummaryPhase(now));CHECK(f.r_.reads<=1536);}
     CHECK(f.nativeRegistrations==3);
     // Ending reconstruction after two terminal outcomes and primary teardown.
     f.r_.Put(states[0]+cfgescape::disk::bDead,std::uint8_t{1});f.r_.Put(states[1]+cfgescape::disk::bEscaped,std::uint8_t{1});
     for(const auto state:states)f.r_.Put(state+0x88,std::uintptr_t{}); // native PS survives its controller
     for(unsigned i=0;i<3;++i)f.Serial(f.bundleReport.roster[i].controller,99);
     f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{});
     f.r_.Put(f.bundleJason.killerController.object+0x370,std::uintptr_t{});
     f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{});f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{});
     for(unsigned now=1300;now<=2400;now+=100){f.r_.reads=0;CHECK(f.SummaryPhase(now));CHECK(f.r_.reads<=1536);}
     CHECK(f.nativeRegistrations==6);cfgsummary::List list{};CHECK(f.r_.Read(f.display+cfgsummary::disk::Registered,list)&&list.count==3);
     std::uint8_t outcome=0;CHECK(f.r_.Read(states[0]+cfgescape::disk::bDead,outcome)&&outcome==1);
     CHECK(f.r_.Read(states[1]+cfgescape::disk::bEscaped,outcome)&&outcome==1);
    }
    {SummaryRuntimeFixture f;
     // Presentation survives the exact kind of killer registry/possession
     // teardown that deliberately makes gameplay Context refuse.
     CHECK(f.SummaryPhase(100)&&f.nativeRegistrations==1);
     f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{});
     f.r_.Put(f.bundleJason.killerController.object+0x370,std::uintptr_t{});
     f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{});
     f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{});
     f.r_.reads=0;CHECK(!f.Context());f.r_.reads=0;
     CHECK(f.SummaryPhase(200)&&f.nativeRegistrations==2);
     f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);f.r_.reads=0;
     CHECK(f.SummaryPhase(300)&&f.nativeRegistrations==2);}
    for(unsigned fault=0;fault<11;++fault){SummaryRuntimeFixture f;
     CHECK(f.SummaryPhase(100)&&f.nativeRegistrations==1);
     std::uintptr_t row=0;CHECK(f.r_.Read(0x980000,row));f.r_.Put(f.state+0x451,std::uint8_t{});
     if(fault==1)f.r_.Put(row+0x20,f.root);
     if(fault==2)f.r_.Put(row+cfgsummary::disk::RowPlayerState,f.localState);
     if(fault==3)f.Serial(row,0);
     if(fault==4)f.r_.Put(f.state+0x394,0u);
     if(fault==5)f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{});
     if(fault==6)f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{});
     if(fault==7)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.r_.botController);
     if(fault==8)f.r_.Put(f.display+0x3D8,std::uint8_t{1});
     if(fault==9)f.r_.Put(f.hud+cfgsummary::disk::Scoreboard,f.display);
     if(fault==10){f.r_.Put(0x980008,row);f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{0x980000,2,2});}
     f.r_.reads=0;CHECK(f.SummaryRowReady(row)==(fault==0));
     f.r_.reads=0;CHECK(f.SummaryPresentedState(row,f.state)==(fault==0));
     f.r_.reads=0;CHECK(!f.SummaryPresentedState(row,f.localState));
     CHECK(f.r_.reads<=1536);std::uint8_t travel=1;CHECK(f.r_.Read(f.state+0x451,travel)&&!travel);
     CHECK(f.nativeRegistrations==1&&!f.selections&&!f.teleports);
    }
    for(unsigned fault=0;fault<8;++fault){StatusFixture f;
     if(fault==1)f.r_.Put(f.hud+cfgmenu::execdisk::HUDStack,f.root);
     if(fault==2)f.r_.Put(f.statusRoot+0x20,f.hud);
     if(fault==3)f.r_.Put(f.stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{0x9A0000,65,65});
     if(fault==4)f.Serial(f.statusDisplay,0);
     if(fault==5)f.r_.Put(f.property+cfgreflection::disk::PropertyOffset,0x400);
     if(fault==6)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.r_.botController);
     if(fault==7)f.r_.Put(f.statusRoot+0x500,f.root);
     bool ready=false;unsigned peak=0;
     for(unsigned i=0;i<4&&!ready;++i){f.r_.reads=0;ready=f.surface.Step(f.r_,f,f.plan.module,f.plan.controller);peak=(std::max)(peak,f.r_.reads);}
     CHECK(ready==(fault==0));CHECK(!f.calls_&&!f.selections&&!f.teleports);
     if(!fault){cfgsummary::Proof proof{};f.r_.reads=0;
      CHECK(cfgsummary::Observe(f.r_,f,f.plan.module,f.plan.controller,f.spawnAssets.gameState,f.state,0,proof,&f.surface.binding));
      CHECK(proof.display.object==f.statusDisplay&&proof.display.object!=f.display);
      std::printf("Current owned Status-list reflection discovery peak=%u/1536; list proof=%u/1536.\n",peak,f.r_.reads);
      cfgsummarybots::Roster roster;roster.entries[0].state=proof.state;roster.display=proof.display.object;roster.displaySlot=proof.display;
      roster.hud=proof.hud;roster.root=proof.root;f.r_.Put(f.state+0x394,4u);f.r_.Put(f.state+0x13C,std::uint8_t{});f.r_.Put(0x990000,f.statusDisplay);
      f.r_.reads=0;CHECK(cfgsummarybots::Eligible(f.r_,f,roster,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state,&f.surface.binding,f.plan.module));
      f.r_.cap=100000;std::uintptr_t rowClass=0;CHECK(f.r_.Read(f.statusDisplay+cfgsummary::disk::RowClass,rowClass));
      const auto row=f.r_.Object(rowClass);f.Serial(row);f.r_.Put(row+0x20,f.statusDisplay);
      f.r_.Put(row+cfgsummary::disk::RowPlayerState,f.state);f.r_.Put(0x9B0000,row);f.r_.Put(0x9B0100,f.state);
      f.r_.Put(f.statusDisplay+cfgsummary::disk::Rows,cfgsummary::List{0x9B0000,1,1});
      f.r_.Put(f.statusDisplay+cfgsummary::disk::Registered,cfgsummary::List{0x9B0100,1,1});
      f.r_.reads=0;f.r_.cap=1536;
      CHECK(cfgsummarybots::RowReady(f.r_,f,roster,row,f.plan.module,f.plan.controller,f.spawnAssets.gameState,&f.surface.binding));
      f.r_.Put(0x9A0000+cfgmenu::execdisk::RecordWidget,f.root);f.r_.reads=0;
      CHECK(!cfgsummarybots::Eligible(f.r_,f,roster,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state,&f.surface.binding,f.plan.module));
     }
    }
    {SummaryRuntimeFixture f;CHECK(f.SummaryPhase(100)&&f.nativeRegistrations==1&&f.bundleSummary.confirmed==1);
     std::printf("Included Summary adapter registration/row proof reads=%u/1536.\n",f.r_.reads);
     // A dead/escaped pawn/controller no longer determines the roster. The
     // actual participant PlayerState remains in the native strong array.
     f.Serial(f.r_.botController,99);f.r_.reads=0;
     CHECK(f.SummaryPhase(200)&&f.nativeRegistrations==1&&f.bundleSummary.refused==0);
     // A native reconstruction can be repaired only after BOTH lists clear.
     f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{});
     f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{});f.r_.reads=0;
     CHECK(f.SummaryPhase(300)&&f.nativeRegistrations==2&&f.bundleSummary.confirmed==2);
    }
    {SummaryRuntimeFixture f;f.registerMode=1;CHECK(f.SummaryPhase(100)&&f.nativeRegistrations==1&&f.bundleSummary.confirmed==0);
     f.r_.reads=0;CHECK(f.SummaryPhase(200)&&f.nativeRegistrations==1);
     f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{});f.r_.reads=0;
     CHECK(f.SummaryPhase(300)&&f.nativeRegistrations==1);}
    for(unsigned fault=0;fault<13;++fault){SummaryFixture f;cfgsummary::Proof p;
     CHECK(f.Observe(p));cfgsummarybots::Roster roster;roster.entries[0].state=p.state;
     roster.display=f.display;roster.displaySlot=p.display;roster.hud=p.hud;roster.root=p.root;
     f.r_.Put(f.state+0x394,4u);f.r_.Put(f.state+0x13C,std::uint8_t{});
     f.r_.Put(f.display+0x3D8,std::uint8_t{});f.r_.Put(0x990000,f.display);
     if(fault==1)f.r_.Put(0x990000,f.root); // another lobby/widget never relaxed
     if(fault==2)f.r_.Put(f.state+0x394,0u); // human
     if(fault==3)f.r_.Put(f.state+0x394,0x14u); // spectator
     if(fault==4)f.r_.Put(f.state+0x394,0x24u); // inactive
     if(fault==5)f.r_.Put(f.state+0x13C,std::uint8_t{4}); // native exclusion
     if(fault==6)f.r_.Put(f.display+0x3D8,std::uint8_t{1}); // local-client deferral
     if(fault==7)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.r_.botController); // online
     if(fault==8){f.r_.Put(0x950008,f.state);f.r_.Put(f.spawnAssets.gameState+cfgsummary::disk::Players,cfgsummary::List{0x950000,2,2});}
     if(fault==9)f.r_.Put(0x950000,f.localState); // no longer a participant
     if(fault==10)f.Serial(f.display,0); // stale widget
     if(fault==11)roster.entries={}; // foreign bot
     if(fault==12)f.r_.Put(f.hud+cfgsummary::disk::Scoreboard,f.display); // replaced HUD chain
     f.r_.reads=0;
     CHECK(cfgsummarybots::Eligible(f.r_,f,roster,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state)==(fault==0));
     f.r_.reads=0;
     CHECK(cfgsummarybots::Admit(f.r_,f,roster,f.plan.module,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state)==(fault==0));
     CHECK(!f.calls_&&!f.selections&&!f.teleports);
     if(!fault)std::printf("Offline bot native filter eligibility reads=%u/1536; no state writes/calls.\n",f.r_.reads);
    }
    {SummaryFixture f;cfgsummarybots::Roster roster;CHECK(f.Slot(f.state,roster.entries[0].state));
     f.r_.Put(f.state+0x394,4u);f.r_.Put(f.state+0x13C,std::uint8_t{});
     f.r_.Put(f.display+0x3D8,std::uint8_t{});f.r_.Put(0x990000,f.display);f.r_.reads=0;
     CHECK(cfgsummarybots::Admit(f.r_,f,roster,f.plan.module,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state));
     CHECK(roster.display==f.display);std::printf("Native Summary reconstructed HUD binding reads=%u/1536.\n",f.r_.reads);
     f.Serial(f.display,11);f.r_.reads=0;
     CHECK(cfgsummarybots::Admit(f.r_,f,roster,f.plan.module,0x990000,f.plan.controller,f.spawnAssets.gameState,f.state));
     CHECK(roster.displaySlot.serial==11&&!f.calls_&&!f.selections&&!f.teleports);
    }
    {cfgsummary::Runtime s;CHECK(s.MayRegister(1,false,false));s.Attempt(1);
     CHECK(!s.MayRegister(1,false,false));s.Confirm(1);
     CHECK(!s.MayRegister(1,true,false)&&!s.MayRegister(1,false,true));
     CHECK(s.MayRegister(1,false,false));s.Attempt(1);CHECK(!s.MayRegister(1,false,false));}
    {SummaryFixture f;cfgsummary::Proof p;CHECK(f.Observe(p)&&!p.present&&p.state.serial==0&&p.display.object==f.display);
     std::printf("Composed native Summary ownership proof reads=%u/1536; no registration called.\n",f.r_.reads);
     f.r_.Put(f.rows,f.state);f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{f.rows,1,4});
     CHECK(f.Observe(p)&&p.present);CHECK(!f.calls_&&!f.selections&&!f.teleports);}
    for(unsigned fault=0;fault<9;++fault){SummaryFixture f;cfgsummary::Proof p;
        if(fault==0)f.r_.Put(f.hud+cfgsummary::disk::Scoreboard,std::uintptr_t{});
        if(fault==1)f.Serial(f.display,0);
        if(fault==2)f.r_.Put(f.hud+0x368,f.r_.botController);
        if(fault==3)f.r_.Put(0x940000+cfgsummary::disk::RegisterSlot,f.plan.module+cfgsummary::disk::Register+1);
        if(fault==4)f.r_.Put(0x950000,f.localState);
        if(fault==5)f.r_.Put(f.display+cfgsummary::disk::Registered,cfgsummary::List{0,33,64});
        if(fault==6)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.r_.botController);
        if(fault==7)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,1u);
        if(fault==8)f.r_.cap=1;
        CHECK(!f.Observe(p));CHECK(!f.calls_&&!f.selections&&!f.teleports);
    }
    {SummaryFixture f;cfgsummary::Proof p;f.r_.cap=100000;std::uintptr_t rowType=0;
     CHECK(f.r_.Read(f.display+cfgsummary::disk::RowClass,rowType));
     const auto row=f.r_.Object(rowType);f.Serial(row);f.r_.Put(row+0x20,f.display);
     f.r_.Put(row+cfgsummary::disk::RowPlayerState,f.state);f.r_.Put(0x980000,row);
     f.r_.Put(f.display+cfgsummary::disk::Rows,cfgsummary::List{0x980000,1,1});
     CHECK(f.Observe(p)&&p.rowPresent&&!p.present);
     f.r_.Put(row+0x20,f.root);CHECK(!f.Observe(p));CHECK(!f.calls_&&!f.selections&&!f.teleports);}
    std::printf("Native Summary proof: %d failures; strong local chain and duplicate detection.\n",failures);return failures?1:0;
}
#endif
