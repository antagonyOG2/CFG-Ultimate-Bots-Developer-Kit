#include "J24FailedStartWorld.hpp"
#define CFG_NATIVE_TICK_TEST_MAIN J24PreviousProductionMain
#include "J23ProductionTests.cpp"
#include <map>
namespace J24RecordFixture {
struct Memory {
    std::map<std::uintptr_t,unsigned char> data;bool fail=false;
    template<class T>void Put(std::uintptr_t at,const T& value){const auto* bytes=reinterpret_cast<const unsigned char*>(&value);for(std::size_t i=0;i<sizeof(T);++i)data[at+i]=bytes[i];}
    template<class T>bool Read(std::uintptr_t at,T& value){if(fail)return false;T temp{};auto* bytes=reinterpret_cast<unsigned char*>(&temp);for(std::size_t i=0;i<sizeof(T);++i){const auto found=data.find(at+i);if(found==data.end())return false;bytes[i]=found->second;}value=temp;return true;}
};
struct Fixture {
    Memory r;cfgprobe::lifetime::ArrayHeader objects{0x800000,6};
    static constexpr std::uintptr_t PackageType=0x500300,A=0x600000,B=0x700000;
    Fixture(){
        for(unsigned world=0;world<2;++world){const auto base=world?B:A;const auto level=base+0x1000,package=base+0x2000;
            const std::array<std::uintptr_t,3> pointers{{base,level,package}};
            for(unsigned j=0;j<3;++j){const auto index=world*3+j;const auto at=objects.data+index*cfgprobe::lifetime::ItemBytes;
                r.Put(pointers[j]+cfgprobe::lifetime::ObjectIndexOffset,static_cast<std::int32_t>(index));r.Put(at,pointers[j]);r.Put(at+8,std::uint32_t{});r.Put(at+16,std::int32_t{});
                r.Put(pointers[j]+cfgprobe::ObjectClass,j==2?PackageType:std::uintptr_t(0x500100+0x100*j));}
            r.Put(base+cfgoutro::disk::PersistentLevel,level);r.Put(base+0x20,package);r.Put(package+0x20,std::uintptr_t{});
        }
    }
};
}
#ifndef CFG_J24_PRODUCTION_TEST_MAIN
#define CFG_J24_PRODUCTION_TEST_MAIN main
#endif
int CFG_J24_PRODUCTION_TEST_MAIN(){
    if(J24PreviousProductionMain())return 1;
    using namespace J24RecordFixture;unsigned checks=0,failures=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    Fixture f;cfgworlddiag::Previous previous;cfgj24::FailedStartWorldCounters counters;
    auto result=cfgj24::RecordRootedWorld(f.r,f.objects,Fixture::A,Fixture::PackageType,previous,true);
    counters.Recorded(result,previous,2);counters.Initialized(previous);previous.checked=true;
    check(result==cfgj24::WorldRecord::Recorded&&previous.generation==1);
    bundlePreviousWorld=previous;worldRootRetirement={};worldRootRetirement.previous=previous;worldRootRetirement.stage=cfgretain::WorldRetirementStage::Ready;
    check(FrontendPreviousWorldAllowsLoad());
    result=cfgj24::RecordRootedWorld(f.r,f.objects,Fixture::B,Fixture::PackageType,previous,true);counters.Recorded(result,previous,2);
    bundlePreviousWorld=previous;
    check(result==cfgj24::WorldRecord::Recorded&&previous.generation==2&&!previous.checked&&previous.world.object==Fixture::B);
    check(!FrontendPreviousWorldAllowsLoad()); // Stale world-A Ready cannot admit B -> next LoadMap.
    const auto saved=previous;
    check(cfgj24::RecordRootedWorld(f.r,f.objects,Fixture::B,Fixture::PackageType,previous,true)==cfgj24::WorldRecord::Skipped&&previous.generation==saved.generation);
    worldRootRetirement.previous=previous;worldRootRetirement.stage=cfgretain::WorldRetirementStage::Armed;check(!FrontendPreviousWorldAllowsLoad());
    worldRootRetirement.stage=cfgretain::WorldRetirementStage::Ready;check(FrontendPreviousWorldAllowsLoad());
    counters.Retired(previous.generation);counters.Retired(previous.generation);check(counters.records==2&&counters.retirements==1&&counters.rootsAtRecord==2);
    previous.checked=true;f.r.fail=true;
    result=cfgj24::RecordRootedWorld(f.r,f.objects,Fixture::A,Fixture::PackageType,previous,true);counters.Recorded(result,previous,2);bundlePreviousWorld=previous;
    check(result==cfgj24::WorldRecord::Failed&&previous.recordAttempted&&!previous.world.object&&!FrontendPreviousWorldAllowsLoad());
    check(counters.failures==1);
    const auto before=previous;
    check(cfgj24::RecordRootedWorld(f.r,f.objects,Fixture::A,Fixture::PackageType,previous,false)==cfgj24::WorldRecord::Skipped&&previous.recordAttempted==before.recordAttempted&&previous.generation==before.generation);
    Fixture identity;cfgprobe::lifetime::SlotObservation captured{};std::uint32_t flags=0;
    check(cfgworlddiag::DiagnosticSlot(identity.r,identity.objects,Fixture::A,captured,flags));
    cfgj24::Refusal why;why.Phase(1);
    check(J24SlotSame(identity.r,identity.objects,Fixture::A,captured,&why,cfgj24::ResourceCheck::ClassSlot,2));
    identity.r.Put(identity.objects.data+16,std::int32_t(42));
    check(!J24SlotSame(identity.r,identity.objects,Fixture::A,captured,&why,cfgj24::ResourceCheck::ClassSlot,2)&&
        why.slotMask==8&&why.capturedSerial==0&&why.freshSerial==42&&why.index==2);
    why.Fail(cfgj24::ResourceCheck::Dispatch,7);check(why.check==static_cast<unsigned>(cfgj24::ResourceCheck::ClassSlot)&&why.index==2);
    cfgj24::Refusal different;
    check(!J24SlotSame(identity.r,identity.objects,Fixture::B,captured,&different,cfgj24::ResourceCheck::ClassSlot)&&
        (different.slotMask&6)==6); // Must observe actual binding, never the captured pointer instead.
    for(unsigned route=0;route<3;++route){
        spawnAssets.classes[0]=Fixture::A;spawnAssets.conversionFunction=Fixture::A+0x100;
        spawnRuntime.report.catalogueIndex=route==1?0xFFFFFFFF:1;
        bundleConfiguration.selectedClass.identity={Fixture::A,0,7};
        bundleConfiguration.selectedClass.defaultsIdentity={Fixture::A+0x1000,1,0};
        constexpr std::u16string_view selected=u"/Game/Characters/Killers/Test.Test_C";
        bundleConfiguration.selectedClass.length=static_cast<unsigned>(selected.size());
        std::copy(selected.begin(),selected.end(),bundleConfiguration.selectedClass.path.begin());
        check(bundleConfiguration.selectedClass.Valid());
        BundleResetEpoch();
        check(!spawnAssets.classes[0]&&!spawnAssets.conversionFunction&&!bundleConfiguration.selectedClass.Valid());
        SpawnAccess fresh;check(fresh.CatalogueIndex()==0xFFFFFFFF&&fresh.ClassConversions()==0);
        spawnAssets.classes[0]=Fixture::B;spawnAssets.conversionFunction=Fixture::B+0x100;
        const auto resets=j24WorldAssetResets;J24ResetSpawnAssets();
        check(!spawnAssets.classes[0]&&!spawnAssets.conversionFunction&&j24WorldAssetResets==resets+1);
    }
    std::printf("J24 failed-start world recording/real LoadMap gate: %u checks, %u failures. No native calls.\n",checks,failures);
    return failures?1:0;
}
