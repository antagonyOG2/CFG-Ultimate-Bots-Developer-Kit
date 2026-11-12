#include <cstdio>
#include "../Core/HostedMovementFallbackPolicy.hpp"
int main(){
    using namespace cfghostedmovement;
    if(UseLegacyGapCloser(true,false)||UseLegacyGapCloser(false,false)||UseLegacyGapCloser(true,true))return 1;
    if(LegacyGapCloserDue(true,false,4999.f*4999.f,20000,20000)||
       LegacyGapCloserDue(true,false,5000.f*5000.f,19999,20000)||
       LegacyGapCloserDue(true,false,5000.f*5000.f,20000,20000))return 2;
    if(!FootBurstAllowed(false)||!FootBurstAllowed(true))return 3;
    std::puts("PASS hosted movement fallback removed; J2.19 movement/lease path owns morph and FootBurst");return 0;
}
