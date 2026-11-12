#define CFGMENU_BIRTH_FIXTURE_ONLY
#include "NativeCounselorBirthTests.cpp"
#include "NativeMenuArrival.hpp"
using namespace cfgmenu;
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<10;++fault){BirthFixture f;Session session;Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(session.Open(101,Mode::Counselor,caps,9)&&session.PickNativeCustomization(101,session.Generation(),11,22)&&
            session.Commit(101,session.Generation()),"native customization committed without default class");
        MenuCatalogueEvidence menu;menu.frontend=101;menu.generation=session.Generation();menu.catalogueGeneration=9;
        menu.frontendWorld=0x900000;menu.map=11;menu.jason=22;menu.maximumBots=8;menu.selectedJason[16]=7;
        menu.cfgMetadata=menu.ownedSelections=menu.preserveNativeCounselor=true;
        GameplayHandoff handoff(session);check(handoff.Arm(menu),"native preserved submission armed");
        NewWorldEvidence world;world.generation=session.Generation();world.catalogueGeneration=9;world.submission=1;
        world.map=11;world.jason=22;world.identity={f.World,f.Pawn,100,103};world.selectedIndex=0;
        world.resources.catalogue={0xA00000,1,1};world.resources.entries[0]=menu.selectedJason;
        world.cfgMetadata=world.offlineSandbox=world.ownedSelections=true;
        // These claims must NEVER substitute for the live ownership proof.
        world.humanCounselor=0xDEAD0000;world.nativeCounselorBirthVerified=true;
        if(fault==1)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==2)f.reader.Set(f.Pawn+cfgprobe::ObjectClass,f.KillerType);
        if(fault==3)++world.identity.pawnSerial;
        if(fault==4)++world.generation;
        if(fault==5)world.resources.killer=f.State;
        if(fault==6)world.resources.entries[0][16]=8;
        if(fault==7)world.submission++;
        if(fault==9)world.resources.entries[0][0]=42; // Native weak cache legitimately changed during travel.
        cfgbundle::Configuration out;unsigned reads=0;
        const auto okay=arrival::Consume(f.reader,handoff,world,f.plan,42,fault!=8,out,&reads);
        check(okay==(fault==0||fault==9),"configuration requires native counselor birth plus exact armed path");
        check(reads<=birth::ReadLimit,"one bounded native arrival proof");
        if(okay)check(out.identity==world.identity&&out.counselorBots==6&&out.selectedJason==world.resources.entries[0],"automatic AI configuration uses owned human birth and fresh native packet");
        else check(!out.submission&&handoff.State()==HandoffState::Refused,"no partial configuration or later-world retry");
        const auto before=f.reader.reads;reads=999;
        check(!arrival::Consume(f.reader,handoff,world,f.plan,42,true,out,&reads)&&reads==0&&f.reader.reads==before,
            "duplicate arrival performs zero reads/calls");
    }
    std::printf("CFG native menu/new-world AI association tests: %s (%u failures). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
