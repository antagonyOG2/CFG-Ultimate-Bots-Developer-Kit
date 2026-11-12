#include "HostedTakeoverFixture.hpp"
int main(){unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
    HostedFixture f;t::Frame frame;unsigned reads=0;
    const bool okay=t::Capture(f.r,f.a,99,frame,&reads);
    check(okay&&frame.eligibility==t::Eligibility::Ready&&frame.count==2&&frame.explicitPick==HostedFixture::humanState);
    check(reads<n::ReadLimit);const auto successReads=reads;
    if(!okay){std::printf("H1 initial capture refused after %u reads.\n",reads);return 1;}
    for(unsigned index=1;index<=successReads;++index){HostedFixture bad;bad.r.failAt=index;t::Frame result=frame;
        const bool captured=t::Capture(bad.r,bad.a,99,result);
        // A missing optional remote/local proof is a captured NOT-READY state;
        // it must never accidentally remain READY after a read failure.
        check(!captured||result.eligibility!=t::Eligibility::Ready);
    }
    HostedFixture host;host.r.Put(HostedFixture::killer+cfgescape::disk::Controller,Fixture::controller);
    host.r.Put(Fixture::controller+cfgprobe::ControllerPawn,HostedFixture::killer);
    host.r.Put(Fixture::state+b::CurrentKillerState,HostedFixture::hostState);
    check(t::Capture(host.r,host.a,99,frame)&&frame.eligibility==t::Eligibility::HostSelected);
    HostedFixture empty;empty.r.Put(Fixture::state+b::CurrentKiller,std::uintptr_t{});empty.r.Put(Fixture::state+b::CurrentKillerState,std::uintptr_t{});
    check(t::Capture(empty.r,empty.a,99,frame)&&frame.eligibility==t::Eligibility::NoKiller);
    for(unsigned fault=0;fault<8;++fault){HostedFixture bad;
        if(fault==0)bad.r.Put(HostedFixture::connection+b::ConnectionDriver,Fixture::state);
        if(fault==1)bad.r.Put(HostedFixture::connection+b::ConnectionActor,Fixture::controller);
        if(fault==2)bad.r.Put(HostedFixture::connection+d::PlayerController,Fixture::controller);
        if(fault==3)bad.r.Put(HostedFixture::human+b::ControllerConnection,std::uintptr_t{});
        if(fault==4)bad.r.Put(HostedFixture::humanState+b::BotFlags,std::uint32_t(4));
        if(fault==5)bad.r.Put(HostedFixture::connection+cfgprobe::ObjectClass,HostedFixture::stateType);
        if(fault==6)bad.r.Put(Fixture::controller+cfgprobe::ControllerPawn,std::uintptr_t{});
        if(fault==7)bad.r.Put(HostedFixture::localPawn+cfgescape::disk::Controller,HostedFixture::human);
        check(t::Capture(bad.r,bad.a,99,frame)&&frame.eligibility!=t::Eligibility::Ready);
    }
    HostedFixture weak;weak.r.Put(Fixture::table+10*24+16,std::int32_t{});
    check(t::Capture(weak.r,weak.a,99,frame)&&frame.eligibility==t::Eligibility::Ready&&frame.humanState.serial==0);
    HostedFixture duplicate;duplicate.r.Put(HostedFixture::roster+8,HostedFixture::hostState);
    check(!t::Capture(duplicate.r,duplicate.a,99,frame));
    HostedFixture capacity;capacity.r.Put(Fixture::state+b::PlayerArray,h::LocalPlayers{HostedFixture::roster,9,9});
    check(!t::Capture(capacity.r,capacity.a,99,frame));
    HostedFixture full;full.r.Put(full.a.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(19));
    for(unsigned i=2;i<8;++i){const auto ps=std::uintptr_t(0x710000+0x1000*i);full.Object(ps,11+i,HostedFixture::stateType);full.r.Put(ps+0x20,full.a.world);full.r.Put(HostedFixture::roster+8*i,ps);}
    full.r.Put(Fixture::state+b::PlayerArray,h::LocalPlayers{HostedFixture::roster,8,8});unsigned fullReads=0;
    check(t::Capture(full.r,full.a,99,frame,&fullReads)&&frame.count==8&&fullReads<n::ReadLimit);
    for(unsigned fault=0;fault<4;++fault){HostedFixture bad;bad.r.mutateAt=successReads/2;
        bad.r.mutateAddress=fault==0?HostedFixture::killer+cfgescape::disk::Controller:
            fault==1?HostedFixture::connection+b::ConnectionActor:fault==2?Fixture::table+10*24+16:HostedFixture::human+b::ControllerConnection;
        bad.r.mutateValue=1;const bool result=t::Capture(bad.r,bad.a,99,frame);check(!result||frame.eligibility!=t::Eligibility::Ready);
    }
    std::printf("H1 remote takeover observation: %u checks, %u failures; 2-player=%u reads,8-player=%u cap%u. No calls/writes/retention implied.\n",checks,failures,successReads,fullReads,n::ReadLimit);
    return failures?1:0;
}
