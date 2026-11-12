#define CFG_URINATION_SHARED_FIXTURE_ONLY
#include "UrinationSharedSlotObservationTests.cpp"
#include "UrinationWavePublication.hpp"
namespace {
struct Writer {
    SharedFixture& f;bool allowed=true;unsigned writes=0,refuseAt=0,reenterAt=0;cfgurination::WavePublication* publication=nullptr;
    bool Current(){return allowed;}
    bool Compare(std::uintptr_t address,std::uint64_t expected,std::uint64_t value){
        if(!Current())return false;++writes;
        if(writes==refuseAt)return false;std::uint64_t current=0;
        if(!f.r.Read(address,current)||current!=expected)return false;f.r.Set(address,value);
        if(writes==reenterAt){CHECK(publication&&publication->Attempted());allowed=false;}
        return true;
    }
};
}
int main(){using namespace cfgurination;
    {SharedFixture f;Writer w{f};WavePublication publication;w.publication=&publication;
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        CHECK(publication.Verified()&&publication.Fields()==3&&publication.Shared()&&w.writes==3);
        CHECK(!publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&w.writes==3);
        AssetProof proof;CHECK(ObserveAssets(f.r,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded,proof,3));}
    {SharedFixture f;Writer w{f};WavePublication publication;
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        CHECK(publication.Restore(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        CHECK(publication.Suspended()&&!publication.Verified()&&publication.PlaybackEligible()&&!publication.Fields());
        cfgnames::Words slot{};CHECK(f.r.Read(f.Tracks,slot)&&slot==f.cinematic);AssetProof proof;CHECK(f.Fixture::Observe(proof));
        const auto writes=w.writes;CHECK(!publication.Restore(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&w.writes==writes);}
    {SharedFixture f;Writer w{f};WavePublication publication;
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        f.r.Set(f.Defaults+disk::MaleMontage,f.Female);const auto writes=w.writes;
        CHECK(!publication.Restore(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&w.writes==writes&&!publication.Suspended());}
    {SharedFixture f;Writer w{f};WavePublication publication;
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));w.refuseAt=5;
        CHECK(!publication.Restore(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&publication.Fields()==1&&publication.Suspended());
        const auto writes=w.writes;CHECK(!publication.Restore(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&w.writes==writes);}
    {SharedFixture f;Writer w{f};WavePublication publication;f.r.Set(f.SlotNames,f.original);
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        CHECK(publication.Fields()==1&&!publication.Shared()&&w.writes==1);}
    {SharedFixture f;Writer w{f};WavePublication publication;
        CHECK(publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,0},f.Loaded)&&publication.Fields()==1);}
    for(unsigned refused=1;refused<=3;++refused){SharedFixture f;Writer w{f};w.refuseAt=refused;WavePublication publication;
        const auto result=publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded);
        CHECK(publication.Attempted()&&result==(refused!=1)&&publication.Fields()==(refused==1?0u:1u));
        const auto writes=w.writes;CHECK(!publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&w.writes==writes);}
    {SharedFixture f;Writer w{f};WavePublication publication;w.allowed=false;
        CHECK(!publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&!publication.Attempted()&&!w.writes);}
    {SharedFixture f;Writer w{f};WavePublication publication;w.publication=&publication;w.reenterAt=1;
        CHECK(!publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded));
        CHECK(publication.Fields()==1&&!publication.Verified()&&w.writes==1);}
    {SharedFixture f;Writer w{f};WavePublication publication;f.r.Set(f.Loaded+disk::Skeleton,f.Emote);
        CHECK(!publication.Publish(f.r,w,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded)&&!publication.Attempted()&&!w.writes);}
    std::printf("Actual retained Wave publication: %u failures; same-skeleton native male field, optional verified shared-gender slot/female field, exact compare-and-exchange, partial ownership/no retry, stale-window refusal. Owned bytes only.\n",failures);
    return failures?1:0;
}
