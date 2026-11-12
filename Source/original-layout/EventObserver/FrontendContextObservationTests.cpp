#define CFG_FRONTEND_BUNDLE
#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "OfflineContextObservation.hpp"
struct BoundedContextReader {
    OwnedByteReader& r;unsigned reads=0,cap=256;
    template<class T>bool Read(std::uintptr_t address,T& out){
        if(reads>=cap)return false;++reads;return r.Read(address,out);
    }
};
struct FrontendContextFixture : NativeFixture {
    static constexpr std::uintptr_t Vtable=0x1800000;
    cfgobserver::Plan plan;
    FrontendContextFixture(){auto& r=fixture.r;Register(Mode,104);
        r.Set(HUD,Vtable);r.Set(Vtable+cfgprobe::dispatch::EventSlotBytes,
            MenuFixture::Base+cfgprobe::dispatch::NativeActorDispatcherRva);
        plan.bytes=sizeof(plan);plan.module=MenuFixture::Base;plan.threadId=42;
        plan.engine=Engine;plan.viewport=Viewport;plan.world=World;plan.mode=Mode;
        plan.controller=Controller;plan.pawn=HUD;plan.player=Player;plan.gameInstance=Instance;plan.localPlayerData=Players;
        cfgmenu::active::Observe(r,plan.module,plan.frontend);
        const std::array<std::uintptr_t,4> objects{{World,Mode,Controller,HUD}};
        cfgprobe::lifetime::ObserveBatch(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,objects,plan.objectArray,plan.objects);
        cfgprobe::dispatch::Observe(r,plan.module,HUD,plan.dispatch);
    }
};
int main(){unsigned failures=0,checks=0;
    const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
    using F=FrontendContextFixture;using namespace cfgprobe;using namespace cfgobserver;
    unsigned passingReads=0;
    for(unsigned fault=0;fault<17;++fault){F f;auto& r=f.fixture.r;const auto registry=f.plan.module+lifetime::ObjectArrayRva;
        if(fault==1)r.Set(registry+lifetime::ArrayCountOffset,f.plan.objectArray.count+4);
        if(fault==2)r.Set(registry+lifetime::ArrayCountOffset,f.plan.objectArray.count-1);
        if(fault==3)r.Set(registry+lifetime::ArrayDataOffset,std::uintptr_t{0x1900000});
        if(fault==4)r.Set(F::Registry+f.plan.objects[1].index*lifetime::ItemBytes+lifetime::ItemSerialOffset,999);
        if(fault==5)r.Set(F::Registry+f.plan.objects[1].index*lifetime::ItemBytes,std::uintptr_t{0x1900000});
        if(fault==6)r.Set(f.plan.frontend.array.data+cfgmenu::execdisk::RecordWidget,std::uintptr_t{0x1900000});
        if(fault==7)r.Set(F::Stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{F::Records,1,2});
        if(fault==8)r.Set(F::World+WorldNetDriver,std::uintptr_t{0x1900000});
        if(fault==9)r.Set(F::Viewport+ViewportWorld,std::uintptr_t{0x1900000});
        if(fault==10)r.Set(F::Player+PlayerController,std::uintptr_t{0x1900000});
        if(fault==11)r.Set(f.plan.module+dispatch::ThreadIdRva,std::uint32_t{43});
        if(fault==12)r.Set(F::Vtable+dispatch::EventSlotBytes,f.plan.dispatch.target+16);
        if(fault==13)r.Set(F::Registry+f.plan.objects[1].index*lifetime::ItemBytes+lifetime::ItemSerialOffset,0);
        if(fault==14)r.Set(F::HUD+cfgmenu::execdisk::HUDStack,std::uintptr_t{0x1900000});
        if(fault==15)f.plan.objects[1].serial=0;
        BoundedContextReader reader{r};if(fault==16)reader.cap=passingReads-1;
        FrontendContextStage stage{};const bool ok=FrontendContextCurrent(reader,f.plan,&stage);
        check(ok==(fault==0||fault==1),"only stable owners or append growth accepted");
        check(reader.reads<=256,"unchanged default budget sufficient");
        check(ok?(stage==FrontendContextStage::None):(stage!=FrontendContextStage::None),"failure stage populated");
        if(fault==0){passingReads=reader.reads;std::printf("Frontend passing revalidation reads=%u/256\n",passingReads);}
        if(fault==1){BoundedContextReader old{r};
            check(!lifetime::RevalidateBatch(old,registry,f.plan.objectArray,f.plan.objects),"old exact count guard reproduces append refusal");
            BoundedContextReader current{r};check(ContextUnchanged(current,f.plan),"production frontend path accepts proven append");}
    }
    for(unsigned fault=0;fault<7;++fault){F f;auto& r=f.fixture.r;
        const auto registry=f.plan.module+lifetime::ObjectArrayRva;
        if(fault==0)r.Set(registry+lifetime::ArrayCountOffset,f.plan.objectArray.count+4);
        if(fault==1)r.Set(registry+lifetime::ArrayCountOffset,f.plan.objectArray.count-1);
        if(fault==2){const std::uintptr_t moved=0x1900000;
            r.Set(registry+lifetime::ArrayDataOffset,moved);
            for(const auto& slot:f.plan.objects){
                r.Set(moved+slot.index*lifetime::ItemBytes,slot.object);
                r.Set(moved+slot.index*lifetime::ItemBytes+lifetime::ItemSerialOffset,slot.serial);}}
        if(fault==3)r.Set(F::Registry+f.plan.objects[1].index*lifetime::ItemBytes+lifetime::ItemSerialOffset,999);
        if(fault==4)r.Set(F::Registry+f.plan.objects[1].index*lifetime::ItemBytes,std::uintptr_t{0x1900000});
        if(fault==5)f.plan.objects[1].serial=0;
        if(fault==6){r.changeAt=f.plan.objects[3].object+lifetime::ObjectIndexOffset;
            r.changeSlot=registry+lifetime::ArrayDataOffset;r.newPointer=0x1900000;}
        BoundedContextReader reader{r};check(FrontendOwnersCurrent(reader,f.plan)==(fault==0),
            "owner guard allows only append, rejects relocation even with copied identical slots and mid-sample relocation");
    }
    // Growth is frontend-only: the generic strict lifetime validator stays intact.
    std::printf("CFG frontend preload context: %s (%u checks, %u failures). Owned memory only.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
