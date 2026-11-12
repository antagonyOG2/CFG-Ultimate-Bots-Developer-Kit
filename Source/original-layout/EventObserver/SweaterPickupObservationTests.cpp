#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_SUMMARY_SCOPE_FIXTURE_ONLY
#include "NativeSummaryTests.cpp"
#include "SweaterPickup.hpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
struct PickupFixture:SummaryRuntimeFixture {
    cfgsweater::PickupState bundleSweaterPickup;
    cfgprobe::lifetime::SlotObservation bundleSweaterOwner;
    bool BundleGameplayStarted(){return bundleStarted;}
    bool bundleStarted=true,living=true;struct {bool active=true;}bundleSchedule;
    std::uintptr_t pickupItem=0,pickupLevel=0,itemList=0x9D0000;
    bool TypedFeature(std::uintptr_t object,unsigned id,bool assigned=true){
        if(id>=cfgfeature::disk::Classes.size())return false;
        const auto& binding=cfgfeature::disk::Classes[id];std::uintptr_t native=0;
        cfgprobe::lifetime::SlotObservation slot{},fresh{};
        return NativeClass(binding.reflection,binding.constructor,native)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,object,slot)&&(!assigned||slot.HasAssignedSerial())&&
            cfgspawn::ObjectIs(r_,object,native)&&cfgspawn::ObjectSlot(r_,context.objectArray,object,fresh)&&fresh==slot;
    }
    // Living-roster policy supplied here; production source/scope/serial/itemList/
    // class/code checks below are included unmodified and use real byte reads.
    bool KnownLiving(std::uintptr_t object){return living&&object==bundleSweaterOwner.object&&
        cfgjason::SameSlot(r_,context.objectArray,bundleSweaterOwner);}
#include "SweaterPickupObservationMembers.hpp"
    PickupFixture(){
        r_.cap=100000;bundleSweaterOwner=plan.objects[3];
        const auto sweaterType=r_.Object(),gameStateType=r_.Object(),levelType=r_.Object();
        for(const auto pair:{std::pair<unsigned,std::uintptr_t>{cfgfeature::disk::Sweater,sweaterType},
            {cfgfeature::disk::GameState,gameStateType}}){const auto& c=cfgfeature::disk::Classes[pair.first];
            r_.Put(plan.module+c.reflection,pair.second);r_.Put(plan.module+c.constructor,pair.second);}
        r_.Put(spawnAssets.gameState+cfgprobe::ObjectClass,gameStateType);
        const auto defaults=r_.Object(sweaterType);r_.Put(sweaterType+cfgspawn::ClassDefaultObject,defaults);
        pickupItem=r_.Object(sweaterType);pickupLevel=r_.Object(levelType);Serial(pickupItem);Serial(pickupLevel);
        r_.Put(plan.module+cfgkill::disk::LevelReflection,levelType);r_.Put(plan.module+cfgkill::disk::LevelConstructor,levelType);
        r_.Put(pickupLevel+cfgkill::disk::OwningWorld,plan.world);r_.Put(pickupItem+0x20,pickupLevel);
        r_.Put(spawnAssets.gameState+cfgfeature::disk::WorldItems,cfgsweater::PickupList{itemList,1,1});r_.Put(itemList,pickupItem);
        auto& s=bundleSweaterPickup;s.world=plan.world;s.module=plan.module;s.clone=0x9E0000;s.installed=1;
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,pickupItem,s.source));CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,pickupLevel,s.level));
        r_.Put(pickupItem,s.clone+8);r_.Put(plan.module+cfgsweater::pickupdisk::Original,cfgsweater::pickupdisk::OriginalCode);
        r_.Put(plan.module+cfgsweater::pickupdisk::Base,cfgsweater::pickupdisk::BaseCode);r_.reads=0;r_.cap=1536;
    }
};
int main(){unsigned peak=0;
    for(unsigned fault=0;fault<12;++fault){PickupFixture f;auto& s=f.bundleSweaterPickup;const auto owner=f.bundleSweaterOwner.object;
        CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));f.r_.reads=0;
        if(fault==1)++f.plan.threadId;
        if(fault==2)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==3)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==4)f.r_.Put(f.itemList,f.pickupLevel);
        if(fault==5)++s.source.serial;
        if(fault==6)f.r_.Put(f.pickupLevel+cfgkill::disk::OwningWorld,f.pickupItem);
        if(fault==7)f.r_.Put(f.pickupItem,s.clone+16);
        if(fault==8)f.living=false;
        if(fault==9)f.r_.Put(f.plan.module+cfgsweater::pickupdisk::Base,std::uint8_t{});
        if(fault==10)f.bundleSchedule.active=false;
        if(fault==11)f.bundleSweaterOwner.object=f.pickupLevel;
        CHECK(f.SweaterPickupAllowed(f.pickupItem,owner)==(fault==0));
        peak=(std::max)(peak,f.r_.reads);CHECK(f.r_.reads<1536&&f.nativeRegistrations==0);
        f.r_.reads=0;CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));
        f.r_.reads=0;CHECK(!f.NativeEnvironmentAllowed());
        if(!fault){f.r_.reads=0;CHECK(!f.SweaterPickupSource());} // mutation/install still needs real tick
    }
    std::printf("Actual pickup observation with REAL retained gate: %d failures, peak%u/1536; current native query only, no borrowed tick/native authority.\n",failures,peak);
    return failures?1:0;
}
