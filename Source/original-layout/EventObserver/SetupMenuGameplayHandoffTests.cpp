#include "../Core/SetupMenuGameplayHandoff.hpp"
#include <cstdio>
using namespace cfgmenu;
struct Fixture {
    Session session;GameplayHandoff handoff{session};MenuCatalogueEvidence menu;NewWorldEvidence world;
    Fixture(Mode mode=Mode::Counselor,unsigned total=5,unsigned maximumBots=9,bool nativeCustomization=false,unsigned duration=1,unsigned skill=1,bool settings=true,unsigned inventory=0,unsigned weapon=0,unsigned difficulty=1){
        Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        caps.verified[static_cast<unsigned>(RowId::Duration)]=settings;caps.verified[static_cast<unsigned>(RowId::Skill)]=settings;
        caps.verified[static_cast<unsigned>(RowId::Inventory)]=settings;
        session.Open(101,mode,caps,10);auto gen=session.Generation();
        if(nativeCustomization)session.PickNativeCustomization(101,gen,11,22);
        else session.Pick(101,gen,11,22,33);
        session.SetCount(gen,total);session.SetChoice(gen,RowId::Difficulty,difficulty);if(settings){session.SetChoice(gen,RowId::Duration,duration);session.SetChoice(gen,RowId::Skill,skill);session.SetChoice(gen,RowId::Inventory,inventory);session.SetChoice(gen,RowId::Weapon,weapon);}session.Commit(101,gen);
        menu.frontend=101;menu.generation=gen;menu.catalogueGeneration=9;menu.frontendWorld=0x10000;
        menu.map=11;menu.jason=22;menu.counselor=33;menu.maximumBots=maximumBots;menu.selectedJason[16]=7;
        menu.cfgMetadata=menu.ownedSelections=true;
        menu.preserveNativeCounselor=nativeCustomization;if(nativeCustomization)menu.counselor=0;
        world.generation=gen;world.catalogueGeneration=9;world.submission=1;
        world.map=11;world.jason=22;world.humanCounselor=33;world.identity={0x20000,0x30000,1,2};
        world.selectedIndex=1;world.resources.catalogue={0x40000,2,2};world.resources.entries[1]=menu.selectedJason;
        world.cfgMetadata=world.offlineSandbox=world.ownedSelections=true;
        world.nativeCounselorBirthVerified=true;
    }
};
int main(){
    unsigned failures=0;auto check=[&](bool x){if(!x)++failures;};cfgbundle::Configuration out;
    for(unsigned fault=0;fault<8;++fault){Fixture f;check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out));
        const auto prior=f.world.identity;auto world=std::uintptr_t{0x60000};auto serial=3;
        if(fault==1)world=prior.world;if(fault==1)serial=prior.worldSerial;
        if(fault==2)serial=0;if(fault==3)world=f.menu.frontendWorld;if(fault==4)f.session.Reset();
        const bool rearmed=f.handoff.RearmForRestart(prior.world,prior.worldSerial,world,serial);
        check(rearmed==(fault==0||fault>=5));
        if(!rearmed)continue;
        check(!f.handoff.RearmForRestart(prior.world,prior.worldSerial,world,serial));
        f.world.identity={world,0x70000,serial,4};
        if(fault==5)++f.world.identity.worldSerial;if(fault==6)++f.world.map;if(fault==7)f.world.nativeCounselorBirthVerified=false;
        check(f.handoff.Consume(f.world,out)==(fault==0));
        if(!fault)check(out.identity.world==world&&out.counselorBots==4&&out.submission==1);
    }
    for(unsigned difficulty=0;difficulty<3;++difficulty){Fixture f(Mode::Counselor,5,9,false,1,1,true,0,0,difficulty);
        check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out)&&out.difficulty==difficulty);
        auto invalid=out;invalid.difficulty=3;check(!cfgbundle::ValidConfiguration(invalid,9));}
    for(unsigned weapon=0;weapon<5;++weapon)for(unsigned inventory=0;inventory<6;++inventory){
        Fixture f(Mode::Counselor,5,9,false,1,1,true,inventory,weapon);f.menu.loadoutSeed=17;
        cfgloadout::Selection expected;check(cfgloadout::Commit({weapon,inventory},17,17*1664525u+1013904223u,expected));
        check(f.handoff.Arm(f.menu));MenuCatalogueEvidence evidence;Selection chosen;std::uint64_t id=0;
        check(f.handoff.InspectArmed(evidence,chosen,id)&&chosen.weapon==expected.weapon&&chosen.inventory==expected.inventory);
        check(f.handoff.Consume(f.world,out));unsigned restricted=0;
        if(expected.weapon||!restrictedinventory::WireChoice(expected.inventory,restricted))
            check(out.startingWeapon==expected.weapon&&out.startingInventory==expected.inventory&&!out.restrictedInventory);
        else check(out.restrictedInventory==restricted&&!out.startingWeapon&&!out.startingInventory);
        auto bad=out;bad.startingWeapon=1;check(!cfgbundle::ValidConfiguration(bad,9));
    }
    for(unsigned wire=0;wire<4;++wire){const std::array<unsigned,4> base{{0,2,3,5}};Fixture f(Mode::Counselor,5,9,false,1,1,true,base[wire]);
        check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out)&&out.restrictedInventory==wire&&out.version==cfgbundle::ConfigurationVersion);
        auto invalid=out;invalid.restrictedInventory=4;check(!cfgbundle::ValidConfiguration(invalid,9));}
    for(unsigned duration=0;duration<3;++duration)for(unsigned skill=0;skill<3;++skill){Fixture f(Mode::Counselor,5,9,false,duration,skill);cfgofflinesettings::Request request;
        check(!f.handoff.ConsumedSettings(request));check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out));
        check(out.matchSeconds==(duration+1)*1800&&out.nativeCounselorSkill==(skill==0?1u:skill==1?0u:2u));
        check(f.handoff.ConsumedSettings(request)&&request.Valid()&&request.seconds==out.matchSeconds&&request.nativeSkill==out.nativeCounselorSkill&&request.world==out.identity.world);
        f.handoff.Cancel();check(!f.handoff.ConsumedSettings(request));}
    {Fixture f(Mode::Counselor,5,9,false,1,1,false);cfgofflinesettings::Request request;
        check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out)&&f.handoff.ConsumedSettings(request)&&!request.seconds&&request.nativeSkill==3&&out.nativeCounselorSkill==3&&!out.matchSeconds);
        auto bad=out;bad.matchSeconds=1;check(!cfgbundle::ValidConfiguration(bad,9));bad=out;bad.nativeCounselorSkill=4;check(!cfgbundle::ValidConfiguration(bad,9));}
    {
        Fixture f(Mode::Counselor,5,8,true);
        // Independent exact capture32436 line16: native selected=None despite
        // FFFFFFFF in its weak-cache index. Reproduce the live handoff refusal.
        constexpr char captured[]="FFFFFFFF000000000000000000000000000000000000000000000000000000000000000000000000";
        const auto nibble=[](char c){return c<='9'?c-'0':c-'A'+10;};
        for(unsigned i=0;i<40;++i)f.world.resources.selected[i]=static_cast<unsigned char>((nibble(captured[2*i])<<4)|nibble(captured[2*i+1]));
        check(softpath::Empty(f.world.resources.selected)&&!softpath::Plain(f.world.resources.selected));
        check(f.handoff.Arm(f.menu)&&f.handoff.Consume(f.world,out)&&out.counselorBots==4&&out.selectedJason==f.menu.selectedJason&&f.handoff.LastFailure()==HandoffFailure::None);
        check(!f.handoff.Consume(f.world,out)&&!out.submission);
    }
    for(unsigned byte=16;byte<40;++byte){Fixture f;f.world.resources.selected[byte]=0xFF;
        check(!softpath::Empty(f.world.resources.selected));
        check(f.handoff.Arm(f.menu)&&!f.handoff.Consume(f.world,out)&&!out.submission&&f.handoff.LastFailure()==HandoffFailure::SelectedJason);
    }
    {Fixture f;MenuCatalogueEvidence source;Selection selected;std::uint64_t submission=99;
        check(!f.handoff.InspectArmed(source,selected,submission)&&!submission);
        check(f.handoff.Arm(f.menu));check(f.handoff.InspectArmed(source,selected,submission)&&submission==1&&source.selectedJason==f.menu.selectedJason);
        check(!f.handoff.Arm(f.menu));check(f.handoff.Consume(f.world,out));
        check(out.counselorBots==4&&out.jasonCatalogueIndex==1&&out.selectedJason==f.menu.selectedJason);
        check(!f.handoff.Consume(f.world,out)&&!out.submission);check(!f.handoff.InspectArmed(source,selected,submission)&&!submission);
        f.handoff.Cancel();check(!f.handoff.Arm(f.menu));}
    {Fixture f(Mode::Jason);check(!f.handoff.Arm(f.menu));}
    {Fixture f(Mode::Counselor,1,0);check(f.handoff.Arm(f.menu));check(f.handoff.Consume(f.world,out));check(out.counselorBots==0);}
    {Fixture f(Mode::Counselor,9,8);check(f.handoff.Arm(f.menu));check(f.handoff.Consume(f.world,out));check(out.counselorBots==8);}
    {Fixture f(Mode::Counselor,10,8);check(!f.handoff.Arm(f.menu));}
    {Fixture f;f.menu.cfgMetadata=false;check(!f.handoff.Arm(f.menu));}
    {Fixture f;f.menu.selectedJason={};check(!f.handoff.Arm(f.menu));}
    for(unsigned failure=0;failure<18;++failure){Fixture f;check(f.handoff.Arm(f.menu));
        switch(failure){
        case 0:++f.world.generation;break;case 1:++f.world.submission;break;
        case 2:++f.world.catalogueGeneration;break;case 3:++f.world.map;break;
        case 4:++f.world.jason;break;case 5:++f.world.humanCounselor;break;
        case 6:f.world.identity.world=f.menu.frontendWorld;break;case 7:f.world.identity.pawnSerial=0;break;
        case 8:f.world.cfgMetadata=false;break;case 9:f.world.offlineSandbox=false;break;
        case 10:f.world.ownedSelections=false;break;case 11:f.world.resources.killer=0x50000;break;
        case 12:f.world.resources.entries[1][16]=8;break;case 13:f.world.resources.entries[0]=f.menu.selectedJason;break;
        case 14:f.session.Reset();break;case 15:f.world.resources.selected[16]=8;break;
        case 16:f.world.selectedIndex=2;break;case 17:f.world.resources.catalogue.capacity=1;break;}
        check(!f.handoff.Consume(f.world,out)&&!out.submission);check(f.handoff.State()==HandoffState::Refused);
        check(!f.handoff.Consume(f.world,out));
    }
    {Fixture f;f.menu.generation++;check(!f.handoff.Arm(f.menu));}
    {Fixture f;f.menu.map++;check(!f.handoff.Arm(f.menu));}
    {Fixture f(Mode::Counselor,5,9,true);check(f.handoff.Arm(f.menu));check(f.handoff.Consume(f.world,out));check(out.counselorBots==4);}
    for(unsigned fault=0;fault<6;++fault){Fixture f(Mode::Counselor,5,9,true);
        if(fault==0)f.menu.preserveNativeCounselor=false;
        if(fault==1)f.menu.counselor=33;
        const auto armed=f.handoff.Arm(f.menu);check(armed==(fault>=2));
        if(!armed)continue;
        if(fault==2)f.world.nativeCounselorBirthVerified=false;
        if(fault==3)f.world.humanCounselor=0;
        if(fault==4)f.world.ownedSelections=false;
        if(fault==5)f.world.identity.pawnSerial=0;
        check(!f.handoff.Consume(f.world,out)&&!out.submission);
    }
    {Fixture f;f.menu.preserveNativeCounselor=true;check(!f.handoff.Arm(f.menu));}
    {Fixture f;check(f.handoff.Arm(f.menu));f.world.resources.entries[1][0]=42;
        f.world.resources.selected=f.world.resources.entries[1];f.world.resources.selected[8]=19;
        check(f.handoff.Consume(f.world,out));check(out.selectedJason==f.world.resources.entries[1]);
        check(out.selectedJason!=f.menu.selectedJason);}
    {Fixture f;check(f.handoff.Arm(f.menu));f.world.resources.entries[0]=f.menu.selectedJason;
        f.world.resources.entries[0][0]=42;check(!f.handoff.Consume(f.world,out));}
    for(unsigned byte=24;byte<40;++byte){Fixture f;f.menu.selectedJason[byte]=1;check(!f.handoff.Arm(f.menu));}
    {Fixture f;check(f.handoff.Arm(f.menu));f.world.resources.entries[1][32]=1;check(!f.handoff.Consume(f.world,out));}
    for(unsigned word=0;word<2;++word){Fixture f;const std::int32_t negative=-1;
        std::memcpy(f.menu.selectedJason.data()+16+4*word,&negative,4);check(!f.handoff.Arm(f.menu));}
    {Session s;Capabilities caps;caps.automaticJason=true;check(s.Open(101,Mode::Jason,caps,9));
        check(!s.PickNativeCustomization(101,s.Generation(),11,22));}
    std::printf("SetupMenuGameplayHandoffTests: %u failures; pure owned evidence only\n",failures);return failures?1:0;
}
