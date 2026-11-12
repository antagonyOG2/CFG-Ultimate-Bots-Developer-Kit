#define CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
#include "AdditionalJasonBirthRuntimeTests.cpp"
#undef CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
#include "AdditionalConstructionTrace.hpp"
int main(){
    BirthFixture f;f.r_.cap=100000;
    const auto actor=f.r_.RealObject(f.bundleAdditional.selected.pawnClass);f.Serial(actor);f.PublishHeader();
    f.r_.Put(actor+0x3A0,std::uintptr_t{});
    cfgadditionalconstruction::Scope scope{f.plan.module,f.plan.world,f.plan.mode,f.spawnAssets.gameState,
        f.bundleAdditional.selected.pawnClass,f.bundleJason.pawn.object,f.plan.pawn,f.plan.threadId,true};
    cfgadditionalconstruction::Sample sample{};f.r_.reads=0;
    CHECK(cfgadditionalconstruction::Observe(f.r_,scope,actor,scope.thread,sample)&&sample.slot.serial==1&&f.r_.reads<=64);
    CHECK(!cfgadditionalconstruction::Observe(f.r_,scope,actor,scope.thread+1,sample));
    CHECK(!cfgadditionalconstruction::Observe(f.r_,scope,scope.primary,scope.thread,sample));
    {const auto iterator=f.r_.Object();f.PublishHeader();
     scope.actor=actor;scope.actorSlot=sample.slot;scope.primaryState=f.bundleAdditional.journal.primary.state;
     scope.primaryController=f.bundleJason.killerController.object;scope.primarySlot=f.bundleJason.pawn;
     scope.primaryControllerSlot=f.bundleJason.killerController;
     f.r_.Put(scope.primary+0x3A0,scope.primaryController);f.r_.Put(scope.primaryController+0x370,scope.primary);
     // Actual native order: PostSpawn witness, AddCharacter publishes extra,
     // then the production Begin gate admits the exact captured allocation.
     f.r_.Put(scope.gameState+cfgprimary::CurrentKiller,actor);
     f.r_.reads=0;f.r_.cap=160;CHECK(cfgadditionalconstruction::BeginAllowed(f.r_,scope,actor,scope.thread)&&f.r_.reads<=160);f.r_.cap=100000;
     cfgprobe::lifetime::SlotObservation cleanup{};f.r_.reads=0;
     CHECK(cfgadditionalconstruction::CleanupWitness(f.r_,scope,scope.thread,cleanup)&&cleanup==scope.actorSlot&&f.r_.reads<=160);
     auto noSerial=scope;noSerial.actorSlot.serial=0;f.r_.reads=0;
     CHECK(!cfgadditionalconstruction::CleanupWitness(f.r_,noSerial,scope.thread,cleanup));
     f.r_.reads=0;CHECK(!cfgadditionalconstruction::CleanupWitness(f.r_,scope,scope.thread+1,cleanup));
     for(auto registry:{std::uintptr_t{},scope.primary,iterator}){f.r_.Put(scope.gameState+cfgprimary::CurrentKiller,registry);
      f.r_.reads=0;CHECK(!cfgadditionalconstruction::BeginAllowed(f.r_,scope,actor,scope.thread));}
     f.r_.Put(scope.gameState+cfgprimary::CurrentKiller,actor);
     auto reused=scope;++reused.actorSlot.serial;f.r_.reads=0;
     CHECK(!cfgadditionalconstruction::BeginAllowed(f.r_,reused,actor,scope.thread));
     auto reusedController=scope;++reusedController.primaryControllerSlot.serial;f.r_.reads=0;
     CHECK(!cfgadditionalconstruction::BeginAllowed(f.r_,reusedController,actor,scope.thread));
     for(unsigned fault=0;fault<3;++fault){auto wrong=scope;
      if(fault==0)wrong.primaryState=actor;if(fault==1)wrong.actor=scope.primary;
      f.r_.reads=0;CHECK(!cfgadditionalconstruction::BeginAllowed(f.r_,wrong,actor,fault==2?scope.thread+1:scope.thread));}
     scope.beginActor=actor;f.r_.reads=0;CHECK(!cfgadditionalconstruction::BeginAllowed(f.r_,scope,actor,scope.thread));
     f.r_.Put(iterator+8,scope.world);f.r_.Put(iterator+0x24,std::uint8_t{});f.r_.Put(iterator+0x30,scope.primary);
     const auto caller=scope.module+cfgadditionalconstruction::disk::FirstIteratorReturn;
     f.r_.reads=0;CHECK(cfgadditionalconstruction::SkipPrimary(f.r_,scope,iterator,caller,scope.thread));
     unsigned advances=0,skips=0;
     cfgadditionalconstruction::Advance([&](void* p){CHECK(p==reinterpret_cast<void*>(iterator));++advances;},
      [&](){f.r_.reads=0;return cfgadditionalconstruction::SkipPrimary(f.r_,scope,iterator,caller,scope.thread);},
      [&](){++skips;++scope.primarySkipped;},reinterpret_cast<void*>(iterator));
     CHECK(advances==2&&skips==1);
     f.r_.reads=0;CHECK(!cfgadditionalconstruction::SkipPrimary(f.r_,scope,iterator,caller,scope.thread));
     scope.primarySkipped=0;
     for(unsigned refusal=0;refusal<7;++refusal){auto bad=scope;
      if(refusal==0)bad.active=false;if(refusal==1)bad.beginActor=0;if(refusal==2)bad.primarySlot.serial=0;
      if(refusal==3)bad.primaryController=actor;if(refusal==4)bad.primaryState=actor;
      const auto call=refusal==5?caller+1:caller;const auto thread=refusal==6?scope.thread+1:scope.thread;
      f.r_.reads=0;CHECK(!cfgadditionalconstruction::SkipPrimary(f.r_,bad,iterator,call,thread));
     }
     f.r_.Put(iterator+0x30,actor);f.r_.reads=0;CHECK(!cfgadditionalconstruction::SkipPrimary(f.r_,scope,iterator,caller,scope.thread));
     advances=skips=0;cfgadditionalconstruction::Advance([&](void*){++advances;},[](){return false;},[&](){++skips;},nullptr);
     CHECK(advances==1&&!skips);scope.beginActor=scope.actor=0;
     f.r_.Put(scope.gameState+cfgprimary::CurrentKiller,scope.primary);
    }
    const auto pair=f.bundleAdditional.journal.primary;unsigned forwarded=0,witnessed=0;
    const auto native=[&](void* p,const void* transform,void* owner,void* instigator,unsigned char remote,unsigned char noFail,unsigned char defer){
        ++forwarded;CHECK(p==reinterpret_cast<void*>(actor)&&transform==&scope&&owner==&pair&&instigator==&sample);
        CHECK(remote==1&&noFail==0&&defer==1);
        f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{}); // construction removed same temporary actor
        f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKillerState,std::uintptr_t{0x123400});
    };
    cfgadditionalconstruction::Forward(native,[&](bool after){++witnessed;f.r_.reads=0;
        CHECK(cfgadditionalconstruction::Observe(f.r_,scope,actor,scope.thread,sample));
        CHECK(sample.pawn==(after?0:pair.pawn));
    },reinterpret_cast<void*>(actor),&scope,const_cast<void*>(static_cast<const void*>(&pair)),&sample,1,0,1);
    CHECK(forwarded==1&&witnessed==2); // observation never restores a null or foreign pair
    cfgadditionaljournal::Pair observed{};f.r_.reads=0;CHECK(f.AdditionalReadPair(observed)&&observed.pawn==0&&observed.state==0x123400);
    scope.active=false;CHECK(!cfgadditionalconstruction::Observe(f.r_,scope,actor,scope.thread,sample));
    std::printf("Scoped construction observer/all7 native args/once-only/foreign-pair preservation: %d failures; native callbacks mocked.\n",failures);
    return failures?1:0;
}
