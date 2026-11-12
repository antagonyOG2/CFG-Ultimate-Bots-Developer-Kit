#include "HostedListenAuthorityFixture.hpp"
int main(){
    unsigned errors=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++errors;};
    Fixture f;h::Context c;unsigned reads=0;check(h::Capture(f.r,f.a,99,c,&reads));
    check(c.session.World()==f.a.world&&c.local[2].object==Fixture::controller&&reads<n::ReadLimit);
    const auto successReads=reads;
    {Fixture bad;n::Trace trace;bad.r.Put(Fixture::table+4*24+16,std::int32_t{});check(!h::Capture(bad.r,bad.a,99,c,nullptr,&trace)&&trace.stage==130&&trace.object==Fixture::instance&&trace.expected==bad.types[4]);}
    {Fixture bad;n::Trace trace;bad.r.Put(Fixture::controller+d::ControllerPlayer,Fixture::state);check(!h::Capture(bad.r,bad.a,99,c,nullptr,&trace)&&trace.stage==140);}
    {Fixture bad;n::Trace trace;bad.r.Put(0x520010,std::array<char,16>{'W','a','i','t'});check(!h::Capture(bad.r,bad.a,99,c,nullptr,&trace)&&trace.stage==31);}
    {Fixture multi;multi.r.Put(Fixture::instance+d::GameInstanceLocalPlayers,h::LocalPlayers{Fixture::players,2,9});
     check(h::Capture(multi.r,multi.a,99,c)&&c.local[1].object==Fixture::player);}
    // Exhaust every actual read: the caller never sees a partially valid context.
    for(unsigned fail=1;fail<=successReads;++fail){Fixture bad;bad.r.failAt=fail;h::Context result=c;
        check(!h::Capture(bad.r,bad.a,99,result)&&!result.session.World()&&!result.local[2].object);}
    for(unsigned fault=0;fault<22;++fault){Fixture bad;auto a=bad.a;auto thread=99U;
        if(fault==0)bad.r.Put(Fixture::driver+d::ServerConnection,std::uintptr_t(0x600000)); // Client.
        if(fault==1)bad.r.Put(Fixture::driver+d::DriverWorld,Fixture::state);
        if(fault==2)bad.r.Put(Fixture::instance+d::GameInstanceLocalPlayers,h::LocalPlayers{Fixture::players,0,1}); // Dedicated/no local player.
        if(fault==3)bad.r.Put(Fixture::instance+d::GameInstanceLocalPlayers,h::LocalPlayers{Fixture::players,17,17});
        if(fault==4)bad.r.Put(Fixture::instance+d::GameInstanceLocalPlayers,h::LocalPlayers{Fixture::players,1,17});
        if(fault==5)bad.r.Put(Fixture::controller+d::ControllerPlayer,Fixture::instance);
        if(fault==6)bad.r.Put(Fixture::controller+d::ActorRole,std::uint8_t(2));
        if(fault==7)bad.r.Put(Fixture::controller+0x20,Fixture::controller);
        if(fault==8)bad.r.Put(Fixture::controller+cfgprobe::ObjectClass,bad.types[2]);
        if(fault==9)bad.r.Put(Fixture::player+cfgprobe::ObjectClass,bad.types[4]);
        if(fault==10)bad.r.Put(Fixture::table+4*24+16,std::int32_t{});
        if(fault==11)bad.r.Put(Fixture::table+5*24+16,std::int32_t{});
        if(fault==12)bad.r.Put(Fixture::table+6*24+16,std::int32_t{});
        if(fault==13)bad.r.Put(a.module+d::WorldClassConstructor,std::uint8_t(0xE9));
        if(fault==14)bad.r.Put(a.module+d::HuntClassConstructor,std::uint8_t(0xE9));
        if(fault==15)bad.r.Put(a.module+d::NetDriverClassConstructor,std::uint8_t(0xE9));
        if(fault==16)bad.r.Put(a.module+d::SCGameInstanceClassConstructor,std::uint8_t(0xE9));
        if(fault==17)bad.r.Put(a.module+d::ILLLocalPlayerClassConstructor,std::uint8_t(0xE9));
        if(fault==18)thread=100;if(fault==19)a.imagePinned=false;if(fault==20)a.requested=false;
        if(fault==21)bad.r.Put(Fixture::mode+cfgescape::disk::MatchState,cfgnames::Words{0,1});
        check(!h::Capture(bad.r,a,thread,c)&&!c.session.World());
    }
    // Changes during the read-only census are caught by the closing proof.
    for(unsigned fault=0;fault<5;++fault){Fixture bad;bad.r.mutateAt=successReads/2;
        bad.r.mutateAddress=fault==0?Fixture::controller+d::ActorRole:
            fault==1?Fixture::driver+d::ServerConnection:fault==2?Fixture::driver+d::DriverWorld:
            fault==3?Fixture::table+6*24+16:Fixture::player+d::PlayerController;
        bad.r.mutateValue=1;check(!h::Capture(bad.r,bad.a,99,c)&&!c.session.World());
    }
    // Old 3.4 caches cannot satisfy the new adapter, even when valid-looking.
    Fixture legacy;legacy.Pair(cfghosted::disk::WorldReflection,cfghosted::disk::WorldConstructor,legacy.types[0]);
    legacy.r.Put(legacy.a.module+d::WorldReflection,std::uintptr_t{});check(!h::Capture(legacy.r,legacy.a,99,c));
    static_assert(d::WorldNetDriver==cfgprobe::WorldNetDriver&&d::WorldGameMode==cfgprobe::WorldGameMode&&d::ActorRole==cfghosted::disk::ActorRole);
    std::printf("H1 current-image listen-host observation: %u checks, %u failures, %u reads (cap%u). No native calls/writes.\n",checks,errors,successReads,n::ReadLimit);
    return errors?1:0;
}
