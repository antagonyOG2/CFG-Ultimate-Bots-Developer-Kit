#include "NativePublicationWindow.hpp"
#include <cstdio>
int main(){unsigned failures=0;
    for(unsigned scenario=0;scenario<9;++scenario){unsigned attempts=0,tries=0,work=0,yields=0;std::uint64_t time=0;bool contentionOnly=true;
        const bool result=cfgretain::PublicationWindow([&](){return scenario!=1&&!(scenario==5&&tries);},
            [&](bool& acquired,bool& contended){++tries;
                if((scenario==0&&tries<3)||scenario==3||scenario==5||scenario==6||(scenario==8&&tries==1)){contended=true;return false;}
                if(scenario==8)return false;
                if(scenario==7){acquired=true;contended=true;++work;return false;}
                if(scenario==2)return false;
                acquired=true;++work;return scenario!=4;
            },[&](){return time;},[&](){++yields;time+=scenario==6?9:1;},attempts,&contentionOnly);
        bool okay=false;
        if(scenario==0)okay=result&&tries==3&&work==1&&yields==2;
        if(scenario==1)okay=!result&&!tries&&!work&&!yields;
        if(scenario==2)okay=!result&&tries==1&&!work&&!yields;
        if(scenario==3)okay=!result&&tries==8&&!work&&yields==7;
        if(scenario==4)okay=!result&&tries==1&&work==1&&!yields;
        if(scenario==5)okay=!result&&tries==1&&!work&&yields==1;
        if(scenario==6)okay=!result&&tries==1&&!work&&yields==1;
        if(scenario==7)okay=!result&&tries==1&&work==1&&!yields;
        if(scenario==8)okay=!result&&tries==2&&!work&&yields==1;
        okay=okay&&(contentionOnly==(scenario==3||scenario==6));
        if(!okay){++failures;std::printf("FAIL scenario%u\n",scenario);}
    }
    for(unsigned scenario=0;scenario<8;++scenario){unsigned attempts=0,tries=0,work=0,yields=0;std::uint64_t time=0;
        const bool okay=cfgretain::PublicationWindow([&](){time+=20;return scenario!=2||!tries;},
            [&](bool& acquired,bool& contended){++tries;
                if(tries==1||scenario==1){contended=true;return false;}
                acquired=true;++work;return scenario!=3;
            },[&](){return time;},[&](){++yields;time+=scenario==4?65:0;},attempts,nullptr,
            scenario==5?cfgretain::PublicationLimits{9,64}:scenario==6?cfgretain::PublicationLimits{8,65}:
            scenario==7?cfgretain::PublicationLimits{0,64}:cfgretain::StartupPublicationLimits);
        bool expected=false;
        if(scenario==0)expected=okay&&tries==2&&work==1&&yields==1;
        if(scenario==1)expected=!okay&&tries==4&&!work&&yields==3&&time==80;
        if(scenario==2)expected=!okay&&tries==1&&!work&&yields==1;
        if(scenario==3)expected=!okay&&tries==2&&work==1&&yields==1;
        if(scenario==4)expected=!okay&&tries==1&&!work&&yields==1;
        if(scenario>=5)expected=!okay&&!tries&&!work&&!yields;
        if(!expected){++failures;std::printf("FAIL startup timing scenario%u tries%u work%u time%llu\n",scenario,tries,work,static_cast<unsigned long long>(time));}
    }
    std::printf("Bounded pre-work lock acquisition and expensive startup proofs; no replay after work/guard refusal: %u failures.\n",failures);
    return failures?1:0;
}
