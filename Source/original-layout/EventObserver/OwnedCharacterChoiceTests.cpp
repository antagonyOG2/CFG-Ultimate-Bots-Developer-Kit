#include "OwnedCharacterChoice.hpp"
#include <cstdio>
#include <cstring>
struct ChoiceHost {
    unsigned currentCalls=0,initialize=0,get=0,request=0,destroy=0;
    unsigned fault=0;bool owns=false,faulted=false;std::uintptr_t allocation=0x800000;
    bool CurrentChoice(){return ++currentCalls==1?fault!=1:fault!=5;}
    bool BindingsReady(){return fault!=2;}
    bool Initialize(cfgmenu::choices::Packet& p){++initialize;if(fault==3)return false;if(fault==8){faulted=true;return false;}p={};owns=true;return true;}
    bool GetPreview(cfgmenu::choices::Packet& p){++get;std::memcpy(p.bytes.data()+24,&allocation,8);if(fault==9)faulted=true;return fault!=4&&fault!=9;}
    bool RequestChoice(cfgmenu::choices::Packet& p){++request;std::uintptr_t observed=0;std::memcpy(&observed,p.bytes.data()+24,8);if(observed!=allocation||!owns)faulted=true;if(fault==10)faulted=true;return fault!=6&&fault!=10;}
    bool Destroy(cfgmenu::choices::Packet& p){++destroy;if(!owns)faulted=true;if(fault==11)faulted=true;if(fault==7||fault==11)return false;p={};owns=false;return true;}
    bool Faulted()const{return faulted;}
};
int main(){using namespace cfgmenu::choices;unsigned failures=0;auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    ChoiceHost unchosen;check(SubmitPreview(unchosen,false)==Result::Refused&&!unchosen.currentCalls&&!unchosen.initialize,"no implicit save mutation/selection");
    for(unsigned fault=0;fault<12;++fault){ChoiceHost host;host.fault=fault;const auto result=SubmitPreview(host,true);
        check(result==(fault==0?Result::Submitted:fault>=8?Result::Indeterminate:fault==7?Result::CleanupFailed:Result::Refused),"explicit native submission/refusal state");
        check(host.initialize<=1&&host.get<=1&&host.request<=1&&host.destroy<=1,"one synchronous attempt, no retry/fanout");
        check(host.initialize+host.get+host.request+host.destroy<=4,"four native calls maximum, menu event only");
        if(fault==0)check(!host.owns&&host.destroy==1&&host.request==1,"getter return alive through request and destroyed exactly once");
        if(fault==4||fault==5||fault==6)check(host.destroy==1&&!host.owns,"normal refusal cleans initialized packet even if owner changed");
        if(fault==8||fault==9||fault==10)check(!host.destroy,"indeterminate native exception quarantined without cleanup retry");
        if(fault>=1&&fault<4)check(!host.request&&!host.destroy,"uninitialized packet never submitted or destroyed");
    }
    std::printf("CFG owned native character-choice packet tests: %s (%u failures). No game access/native calls.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
