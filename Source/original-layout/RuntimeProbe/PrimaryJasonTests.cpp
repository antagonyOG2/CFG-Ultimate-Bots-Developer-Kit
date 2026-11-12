#define main CfgAiResourceTestsMain
#include "AiResourceTests.cpp"
#undef main
#include "PrimaryJasonResources.hpp"
static_assert(cfgprimary::ModeReflection==0x0304C230&&cfgprimary::ModeConstructor==0x0304C238);
static_assert(cfgprimary::StateReflection==0x0304C470&&cfgprimary::StateConstructor==0x0304C478);
struct PrimaryFixture : Fixture {
    std::uintptr_t mode=0,nativeMode=0,sandbox=0,nativeState=0,catalogue=0x760000;
    PrimaryFixture() {
        nativeMode=Object(); sandbox=Object(); nativeState=Object(); mode=Object(sandbox);
        Put(sandbox+0x30,nativeMode); Put(nativeState+0x30,classes[cfgspawn::GameState]);
        Put(state+cfgprobe::ObjectClass,nativeState);
        Put(module+cfgprimary::ModeReflection,nativeMode); Put(module+cfgprimary::ModeConstructor,nativeMode);
        Put(module+cfgprimary::StateReflection,nativeState); Put(module+cfgprimary::StateConstructor,nativeState);
        Put(mode+cfgprimary::CounselorTree,tree); Catalogue(10);
    }
    void Catalogue(int count) {
        Put(mode+cfgprimary::KillerClasses,cfgspawn::PlayerArray{catalogue,count,count});
        for (int i=0;i<count && i<64;++i) {
            cfgprimary::SoftClassBytes bytes{};
            for (unsigned j=0;j<bytes.size();++j) bytes[j]=static_cast<unsigned char>(i+j);
            Put(catalogue+static_cast<std::uintptr_t>(i)*40,bytes);
        }
    }
    bool Primary(cfgprimary::Report& result) {
        return cfgprimary::Capture(*this,module,objects,mode,state,sandbox,bot,counselorBase,
            killerBase,classes[cfgspawn::PlayerState],classes[cfgspawn::Tree],result);
    }
};
int main() {
    CHECK(CfgAiResourceTestsMain()==0);
    cfgprimary::Report report{};
    { PrimaryFixture f; CHECK(f.Primary(report)); CHECK(report.EmptyPrimary()); CHECK(report.catalogue.count==10);
        CHECK(report.entries[9][39]==48); CHECK(f.reads<1024); }
    { PrimaryFixture f; f.Catalogue(64); CHECK(f.Primary(report)); CHECK(report.entries[63][39]==102); CHECK(f.reads<1024); }
    { PrimaryFixture f; f.Catalogue(0); CHECK(f.Primary(report)); CHECK(report.catalogue.count==0); }
    { PrimaryFixture f; f.Catalogue(65); CHECK(!f.Primary(report)); CHECK(report.catalogue.count==0); }
    { PrimaryFixture f; f.Catalogue(-1); CHECK(!f.Primary(report)); }
    CHECK(!cfgprimary::SaneCatalogue(cfgspawn::PlayerArray{0x760000,2,257}));
    CHECK(!cfgprimary::SaneCatalogue(cfgspawn::PlayerArray{0x760003,2,2}));
    CHECK(!cfgprimary::SaneCatalogue(cfgspawn::PlayerArray{0,2,2}));
    { PrimaryFixture f; f.Put(f.state+cfgprimary::CurrentKiller,f.pawn); CHECK(f.Primary(report)); CHECK(!report.EmptyPrimary()); }
    { PrimaryFixture f; f.Put(f.state+cfgprimary::CurrentKiller,f.bot); CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.Put(f.module+cfgprimary::ModeConstructor,f.sandbox); CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.Put(f.mode+cfgprobe::ObjectClass,f.nativeMode); CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.Put(f.mode+cfgprimary::CounselorTree,f.bot); CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.trigger=f.state+cfgprimary::CurrentKiller; f.mutation=f.trigger; f.newPointer=f.pawn;
        CHECK(!f.Primary(report)); CHECK(report.killer==0); }
    { PrimaryFixture f; f.trigger=f.catalogue; f.mutation=f.catalogue; f.newPointer=0x1234;
        CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.trigger=f.mode+cfgprimary::KillerClasses; f.mutation=f.trigger; f.newPointer=0x770000;
        CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.trigger=f.state+cfgprimary::CurrentKiller; f.mutation=f.state+cfgprobe::ObjectClass;
        f.newPointer=f.classes[cfgspawn::GameState]; CHECK(!f.Primary(report)); }
    { PrimaryFixture f; f.cap=5; CHECK(!f.Primary(report)); CHECK(f.reads==5); }
    std::printf("Primary Sandbox resource guards: %s; finite read-only capture, no game used.\n",failures?"FAIL":"PASS");
    return failures?1:0;
}
