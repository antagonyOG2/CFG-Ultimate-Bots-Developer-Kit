#include "KnifeOwnerObservation.hpp"
#define main CfgExistingKnifeOwnerMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    Fixture f;f.cap=1536;const auto ai=f.Object(f.classes[cfgspawn::KillerAI]),path=f.Object(),state=f.Object(f.classes[cfgspawn::PlayerState]);
    cfgjason::NewActorEvidence j;cfgprobe::lifetime::SlotObservation s;
    const auto serial=[&](std::uintptr_t p){std::int32_t i=0;CHECK(f.Read(p+0xC,i));f.Put(f.objects.data+24ULL*i+16,std::int32_t{100+i});};
    for(auto p:{f.pawn,ai})serial(p);
    CHECK(cfgspawn::ObjectSlot(f,f.objects,f.pawn,j.pawn));CHECK(cfgspawn::ObjectSlot(f,f.objects,ai,j.killerController));
    CHECK(cfgspawn::ObjectSlot(f,f.objects,path,j.path));CHECK(cfgspawn::ObjectSlot(f,f.objects,state,s));
    f.Put(f.pawn+0x3A0,ai);f.Put(ai+0x370,f.pawn);f.Put(ai+0x410,path);
    f.Put(path+0x20,ai);f.Put(path+cfgjason::disk::ComponentOwner,ai);
    f.Put(f.pawn+cfgescape::disk::PlayerState,state);f.Put(ai+cfgescape::disk::PlayerState,state);f.Put(state+0x88,ai);
    const std::array<std::uintptr_t,3> classes{f.pawnClass,f.classes[cfgspawn::KillerAI],0};
    const auto prove=[&](Fixture& r){return cfgknife::FreshOwnerLinks(r,r.objects,j,classes,s,f.classes[cfgspawn::PlayerState]);};
    f.reads=0;CHECK(prove(f));const auto cost=f.reads;
    // Measured real link reader, plus conservative full first-owner/target,
    // function/code/geometry allowance. The old repeated full walks exceeded it.
    f.reads=800;CHECK(prove(f)&&prove(f)&&prove(f));CHECK(f.reads<1536);
    for(auto at:{f.pawn+0x3A0,ai+0x370,ai+0x410,path+0x20,path+cfgjason::disk::ComponentOwner,
                 f.pawn+cfgescape::disk::PlayerState,ai+cfgescape::disk::PlayerState,state+0x88}){
        auto bad=f;bad.reads=0;bad.Put(at,std::uintptr_t{0});CHECK(!prove(bad));
    }
    auto reclaimed=f;reclaimed.reads=0;std::int32_t i=0;CHECK(reclaimed.Read(f.pawn+0xC,i));
    reclaimed.Put(reclaimed.objects.data+24ULL*i+16,std::int32_t{999});CHECK(!prove(reclaimed));
    auto exhausted=f;exhausted.reads=1536;CHECK(!prove(exhausted));
    std::printf("Knife actual callback owner links: %s; fresh=%u reads; composed=%u/1536; no game access.\n",failures?"FAIL":"PASS",cost,f.reads);
    return failures?1:0;
}
