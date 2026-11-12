#include "../Core/JasonDeathCompletion.hpp"
#include "DefeatedOutroProperty.hpp"
#define main CfgShackFixtureMain
#include "ShackTrapTests.cpp"
#undef main
int main(){
    cfgkill::Policy kill;kill.identity={0x200000,0x220000,1};cfgdeathend::Policy policy;
    assert(!policy.Confirm(kill,true,1000));kill.interactionAttempted=true;
    assert(!policy.Confirm(kill,false,1000));assert(policy.Confirm(kill,true,1000));
    assert(!policy.Ready(25999)&&policy.Ready(26000));assert(policy.Confirm(kill,true,2000)&&policy.due==26000);
    auto wrong=kill;wrong.identity.serial=2;assert(!policy.Confirm(wrong,true,2000));
    assert(policy.Mark(26000)&&!policy.Mark(27000)&&!policy.Confirm(kill,true,28000));
    unsigned peak=0;
    for(unsigned fault=0;fault<6;++fault){Fixture f;cfgdeathend::OutroProperty p;
        f.Name(1,"bSkipLevelOutro");f.Name(2,"BoolProperty");f.r.Put(f.Field+0x34,int{1});
        f.r.Put(f.Field+0x70,std::array<std::uint8_t,4>{{1,0,4,4}});
        f.r.Put(f.Module+cfgdeathend::BoolAssignProof,cfgdeathend::BoolAssignCode);
        f.r.Put(f.Actor+0x800,std::uint8_t{0xA0});
        f.r.reads=0;assert(p.Warm(f.r,f.Module,f.Actor)&&p.stage==1);peak=(std::max)(peak,f.r.reads);
        switch(fault){case 1:f.r.Put(f.Field+0x70,std::array<std::uint8_t,4>{{1,0,3,3}});break;
        case 2:f.r.Put(f.Field+0x44,int{0x808});break;
        case 3:f.r.Put(f.Field+0x34,int{8});break;
        case 4:f.r.Put(f.Field+0x70,std::array<std::uint8_t,4>{{1,1,4,4}});break;
        case 5:f.r.Put(f.Module+cfgdeathend::BoolAssignProof,std::array<unsigned char,39>{});break;}
        std::uintptr_t address=0;std::uint8_t mask=0,value=0;f.r.reads=0;
        assert(p.Value(f.r,f.Module,f.Actor,address,mask,value)==(fault==0));
        if(!fault)assert(address==f.Actor+0x800&&mask==4&&value==0xA0&&(value|mask)==0xA4);
        peak=(std::max)(peak,f.r.reads);assert(f.r.reads<1536);
    }
    std::printf("Jason death completion PASS: native proof required,25s grace,one-shot,changed identity rejected,packed outro bool preserves peers; peak=%u/1536\n",peak);
}
