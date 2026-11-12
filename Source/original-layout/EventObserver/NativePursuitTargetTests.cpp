#include "NativePursuitTargetObservation.hpp"
#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    Fixture f;const auto gs=f.Object(),controller=f.Object(f.classes[cfgspawn::CounselorAI]);
    const std::uintptr_t entries=0x850000;
    f.Put(gs+cfgescape::disk::PlayerCharacterList,cfgnativepursuit::List{entries,1,1});f.Put(entries,f.bot);
    f.Put(f.bot+cfgescape::disk::bHasEscaped,std::uint8_t{0});f.Put(f.bot+cfghunter::disk::Health,100.f);
    f.Put(f.bot+cfgescape::disk::PlayerState,std::uintptr_t{0});
    f.Put(f.bot+cfgescape::disk::Controller,controller);f.Put(controller+cfgescape::disk::Pawn,f.bot);
    const auto serial=[&](std::uintptr_t object,int number){int i=0;CHECK(f.Read(object+0xC,i));f.Put(f.objects.data+24*static_cast<unsigned>(i)+16,number);};
    serial(f.bot,101);serial(controller,102);
    cfgnativepursuit::Proof proof{};
    const auto observe=[&](Fixture& value,cfgnativepursuit::Proof& out,const cfgnativepursuit::Proof* retained=nullptr){
        value.reads=0;return cfgnativepursuit::Observe(value,value.objects,value.module,gs,value.counselorBase,
            value.classes[cfgspawn::BaseAI],value.pawn,0,value.bot,out,retained);};
    CHECK(observe(f,proof)&&proof.pawn.object==f.bot&&proof.controller.object==controller);
    CHECK(f.reads<300);cfgnativepursuit::Proof fresh{};CHECK(observe(f,fresh,&proof));
    auto bad=f;bad.Put(f.bot+cfgescape::disk::bHasEscaped,std::uint8_t{1});CHECK(!observe(bad,fresh));
    bad=f;bad.Put(f.bot+cfghunter::disk::Health,0.f);CHECK(!observe(bad,fresh));
    bad=f;bad.Put(controller+cfgescape::disk::Pawn,bad.pawn);CHECK(!observe(bad,fresh));
    bad=f;bad.Put(controller+cfgprobe::ObjectClass,bad.killerBase);CHECK(!observe(bad,fresh));
    bad=f;bad.Put(gs+cfgescape::disk::PlayerCharacterList,cfgnativepursuit::List{entries,17,17});CHECK(!observe(bad,fresh));
    bad=f;bad.Put(entries,bad.pawn);CHECK(!observe(bad,fresh));
    bad=f;int i=0;CHECK(bad.Read(f.bot+0xC,i));bad.Put(f.objects.data+24*static_cast<unsigned>(i)+16,std::int32_t{103});CHECK(!observe(bad,fresh,&proof));
    bad=f;bad.cap=10;CHECK(!observe(bad,fresh));
    bad=f;bad.trigger=entries;bad.triggerAt=2;bad.mutation=entries;bad.newPointer=bad.pawn;CHECK(!observe(bad,fresh));
    bad=f;bad.Put(gs+cfgescape::disk::PlayerCharacterList,cfgnativepursuit::List{entries,1,2});CHECK(!observe(bad,fresh,&proof));
    CHECK(!cfgnativepursuit::Observe(f,f.objects,f.module,gs,f.counselorBase,f.classes[cfgspawn::BaseAI],f.bot,0,f.bot,fresh));
    std::printf("Native pursuit roster production observation: %d failures; no game calls/writes\n",failures);return failures?1:0;
}
