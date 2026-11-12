#include "DonorCounselorObservation.hpp"
#include "DonorJasonDecisionFrame.hpp"
#define CFG35_NATIVE_TICK_FIXTURE_ONLY
#include "NativeJasonControllerTickObservationTests.cpp"
#undef CFG35_NATIVE_TICK_FIXTURE_ONLY
#include <limits>
struct CounselorFixture:TickFixture {
    const std::uintptr_t state=0x40070000,stateClass=0x50070000;
    CounselorFixture(){
        using namespace cfgcompat::cfg35::j1a;
        Put(plan.module+ContextKillSetter,ContextKillSetterCode);
        Put(plan.module+cfgescape::disk::SCCounselorCharacterReflection,counselorClass);
        Put(plan.module+cfgescape::disk::SCCounselorCharacterConstructor,counselorClass);
        Put(plan.module+cfgescape::disk::ControllerReflection,aiClass);
        Put(plan.module+cfgescape::disk::ControllerConstructor,aiClass);
        Put(pawn+cfghunter::disk::Health,100.0f);Put(pawn+CounselorContextKill,std::uint8_t{0});
        Put(pawn+cfgescape::disk::bHasEscaped,std::uint8_t{0});Put(pawn+cfgescape::disk::PlayerState,state);
        Slot(state,46,506);Put(state+cfgprobe::ObjectClass,stateClass);Put(stateClass+cfgprobe::ClassSuper,std::uintptr_t{0});
        Put(plan.module+cfgescape::disk::SCPlayerStateReflection,stateClass);
        Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,stateClass);
        Put(state+cfgescape::disk::bEscaped,std::uint8_t{0});reads=0;
    }
    bool Observe(cfgdonorhunt::CounselorObservation& out){reads=0;return cfgdonorhunt::ObserveDonorCounselor(*this,
        cfgprobe::lifetime::ArrayHeader{array,64},plan.module,plan.world,owner.actor.pawn,counselorClass,out);}
};
int main(){const auto earlier=PriorCounselorResultMain();unsigned checks=0;auto check=[&](bool b){++checks;if(!b)std::printf("failed check #%u\n",checks);CHECK(b);};
    cfgdonorhunt::CounselorObservation out;CounselorFixture good;
    check(good.Observe(out)&&out.Live()&&out.pawn==good.owner.actor.pawn&&out.controller==good.owner.actor.killerController);
    const auto readCount=good.reads;check(readCount<180);
    {CounselorFixture f;f.Slot(f.state,46,0);
        check(f.Observe(out)&&out.Live()&&out.playerState.object==f.state&&out.playerState.serial==0);}
    for(unsigned fault=0;fault<12;++fault){CounselorFixture f;
        switch(fault){
        case 0:f.Put(f.pawn+cfghunter::disk::Health,0.0f);break;
        case 1:f.Put(f.pawn+cfghunter::disk::Health,-1.0f);break;
        case 2:f.Put(f.pawn+cfgcompat::cfg35::j1a::CounselorContextKill,std::uint8_t{1});break;
        case 3:f.Put(f.pawn+cfgescape::disk::bHasEscaped,std::uint8_t{1});break;
        case 4:f.Put(f.state+cfgescape::disk::bEscaped,std::uint8_t{1});break;
        case 5:f.Put(f.killer+cfgescape::disk::Pawn,std::uintptr_t{0});break;
        case 6:f.Put(f.pawn+cfgescape::disk::Controller,std::uintptr_t{0});break;
        case 7:f.Put(f.pawn+cfghunter::disk::Health,1.0e-10f);break;
        case 8:f.Put(f.pawn+cfgescape::disk::PlayerState,std::uintptr_t{0});break;
        case 9:f.Put(f.state+cfgescape::disk::bDead,std::uint8_t{1});break; // donor does not substitute PlayerState.bDead
        case 10:f.Put(f.pawn+cfgcompat::cfg35::j1a::CounselorContextKill+1,std::uint8_t{1});break;
        case 11:f.Put(f.pawn+cfghunter::disk::Health,1.0f);break;
        }
        check(f.Observe(out)&&out.Live()==(fault>=7));
    }
    for(unsigned fault=0;fault<12;++fault){CounselorFixture f;
        switch(fault){
        case 0:f.Put(f.pawn+cfghunter::disk::Health,std::numeric_limits<float>::quiet_NaN());break;
        case 1:f.Put(f.pawn+cfgescape::disk::bHasEscaped,std::uint8_t{2});break;
        case 2:f.Put(f.state+cfgescape::disk::bEscaped,std::uint8_t{2});break;
        case 3:f.Put(f.array+40*24+16,std::int32_t{999});break;
        case 4:f.Put(f.array+42*24+16,std::int32_t{0});break;
        case 5:f.Put(f.state+cfgprobe::ObjectClass,f.aiClass);break;
        case 6:f.Put(f.plan.module+cfgescape::disk::SCCounselorCharacterConstructor,f.aiClass);break;
        case 7:f.Put(f.killer+0x20,f.plan.world);break;
        case 8:f.Put(f.plan.module+cfgcompat::cfg35::j1a::ContextKillSetter,std::uint8_t{0x90});break;
        case 9:f.Put(std::uintptr_t{0x40060000}+cfgkill::disk::OwningWorld,std::uintptr_t{0x777000});break;
        case 10:f.Slot(f.state,46,0);f.Put(f.array+46*24+8,std::uint32_t{0x20000000});break;
        case 11:f.Slot(f.state,46,0);f.Put(f.array+46*24,std::uintptr_t{0x777000});break;
        }
        check(!f.Observe(out)&&!out.pawn.object);
    }
    for(unsigned cap=0;cap<readCount;++cap){CounselorFixture f;f.cap=cap;check(!f.Observe(out)&&!out.pawn.object);}
    // Same actual observer for one local, two native AI and two manually born
    // F3 counselors. F3 may lack PlayerState until its native birth completes;
    // controller/Level/World/serial and reciprocal possession remain mandatory.
    {CounselorFixture f;f.cap=8192;
     cfgdonorframe::Source source;source.owner=f.owner;source.count=5;source.revision=1;
     cfgdonorhunt::Targets targets;targets.count=5;
     const std::uintptr_t counselorAI=0x50081000;
     f.Put(counselorAI+cfgprobe::ClassSuper,f.aiClass);
     for(unsigned i=0;i<5;++i){const std::uintptr_t pawn=0x40100000+0x10000*i,controller=pawn+0x2000,state=pawn+0x4000;
        const auto slot=48+static_cast<int>(i)*3;f.Slot(pawn,slot,600+static_cast<int>(i));
        f.Slot(controller,slot+1,700+static_cast<int>(i));f.Slot(state,slot+2,i?0:800);
        f.Put(pawn+cfgprobe::ObjectClass,f.pawnClass);f.Put(pawn+0x20,std::uintptr_t{0x40060000});
        f.Put(pawn+cfghunter::disk::Health,100.0f);f.Put(pawn+cfgcompat::cfg35::j1a::CounselorContextKill,std::uint8_t{});
        f.Put(pawn+cfgescape::disk::bHasEscaped,std::uint8_t{});f.Put(pawn+cfgescape::disk::Controller,controller);
        f.Put(pawn+cfgescape::disk::PlayerState,i==4?std::uintptr_t{}:state);
        f.Put(controller+cfgprobe::ObjectClass,i?counselorAI:f.aiClass);f.Put(controller+0x20,std::uintptr_t{0x40060000});
        f.Put(controller+cfgescape::disk::Pawn,pawn);f.Put(state+cfgprobe::ObjectClass,f.stateClass);
        f.Put(state+cfgescape::disk::bEscaped,std::uint8_t{});
        source.targets[i]={pawn,slot,600+static_cast<int>(i)};targets.actors[i]=pawn;
     }
     struct MixedHost {CounselorFixture& f;
        bool Current(const cfgjasontick::Owner&){return true;}bool BudgetExhausted(){return f.reads>=f.cap;}
        bool TargetBudgetAvailable(){return f.reads+180<f.cap;}
        bool Origin(const cfgprobe::lifetime::SlotObservation&,cfgdonorhunt::Position& p){p={};return true;}
        bool Observe(const cfgprobe::lifetime::SlotObservation& slot,cfgdonorhunt::CounselorObservation& value,cfgdonorhunt::Position& p){
            const auto i=(slot.object-0x40100000)/0x10000;
            const float distances[5]={9000,300,100,200,400};p={distances[i],0,0};
            return cfgdonorhunt::ObserveDonorCounselor(f,cfgprobe::lifetime::ArrayHeader{f.array,64},f.plan.module,f.plan.world,slot,f.counselorClass,value);
        }
     }host{f};cfgdonorframe::Frame frame;f.reads=0;
     check(frame.Step(host,source)==cfgdonorframe::State::Ready);
     for(unsigned i=0;i<source.count;++i)check(frame.ObservedLive(source.targets[i].object));
     check(frame.Nearest(targets,1000)==source.targets[2].object); // primary nearest native bot
     check(frame.Nearest(targets,1000,source.targets[0].object)==source.targets[2].object); // F1 excludes local
     float distance=0;check(frame.ObservedDistance(source.targets[2].object,distance)&&distance==100&&distance<8000);
     targets.blockedUntil[2]=2000;
     check(frame.Nearest(targets,1000,source.targets[0].object)==source.targets[3].object); // retarget to F3
    }
    // H2.48: sweater-wearing/mask-holding counselor. Neighbouring state bytes (sweater/wearing flags, adjacent
    // to the context-kill byte) set non-zero must not change liveness; per-clause attribution and raw bytes.
    {CounselorFixture f;using namespace cfgcompat::cfg35::j1a;
     for(int d=-4;d<=4;++d)if(d)f.Put(f.pawn+CounselorContextKill+d,std::uint8_t{1});
     check(f.Observe(out)&&out.Live()&&out.clause==0&&out.rawContext==0&&out.rawEscaped==0);}
    {CounselorFixture f;f.Put(f.pawn+cfgcompat::cfg35::j1a::CounselorContextKill,std::uint8_t{2});
     check(!f.Observe(out)&&out.failure==1&&out.clause==7&&out.rawContext==2);}   // value>1 still refuses
    {CounselorFixture f;f.Put(f.pawn+cfgescape::disk::bHasEscaped,std::uint8_t{3});
     check(!f.Observe(out)&&out.failure==1&&out.clause==8&&out.rawEscaped==3);}
    {CounselorFixture f;f.Put(f.pawn+cfghunter::disk::Health,std::numeric_limits<float>::quiet_NaN());
     check(!f.Observe(out)&&out.clause==6);}
    {CounselorFixture f;f.Put(f.array+42*24+16,std::int32_t{0});
     check(!f.Observe(out)&&out.failure==3&&!out.clause);}   // controller serial0 identity still refuses (later stage, no pawn clause)
    {CounselorFixture f;f.Put(f.plan.module+cfgescape::disk::SCCounselorCharacterConstructor,f.aiClass);
     check(!f.Observe(out)&&out.clause==2);}
    // H2.48 Tommy slot: report-only clause-4 reasons; verdicts unchanged (still refuse, never accept).
    {CounselorFixture f;f.Put(f.array+40*24+16,std::int32_t{999});
     check(!f.Observe(out)&&out.clause==4&&out.slotWhy==6&&out.slotFreshSerial==999&&out.slotExpectedSerial==f.owner.actor.pawn.serial);}
    {CounselorFixture f;f.Put(f.array+40*24+8,std::uint32_t{0x20000000});
     check(!f.Observe(out)&&out.clause==4&&out.slotWhy==4&&out.slotFlags==0x20000000u);}  // truly PendingKill stays refused
    {CounselorFixture f;f.Put(f.array+40*24+8,std::uint32_t{0x40000000});               // RootSet-only flag still accepted
     check(f.Observe(out)&&out.Live()&&out.slotWhy==0);}
    {CounselorFixture f;f.Put(f.array+40*24+16,std::int32_t{0});
     check(!f.Observe(out)&&out.clause==4&&out.slotWhy==6);}
    std::printf("Donor native counselor value snapshot: %u checks/%u reads positive; health>0,context-kill,dual escape,reciprocal possession,current world/slot/type and bounded-read failure.\n",checks,readCount);
    return earlier||failures?1:0;
}
