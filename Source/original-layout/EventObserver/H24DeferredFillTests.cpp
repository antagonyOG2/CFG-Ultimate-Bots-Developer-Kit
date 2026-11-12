#include "HostedDeferredFill.hpp"
#include <cstdio>
int main(){unsigned n=0,bad=0;auto check=[&](bool ok){++n;if(!ok)++bad;};
 using cfghostedfill::DeferredReady;
 check(!DeferredReady(true,true,false,false,false,0,false,4000,0,0)); // Reason5 OFF is preserved.
 check(!DeferredReady(true,false,true,false,false,0,false,4000,0,0));
 check(!DeferredReady(false,true,true,false,false,0,false,4000,0,0));
 check(!DeferredReady(true,true,true,false,false,0,false,2999,0,0));
 check(DeferredReady(true,true,true,false,false,0,false,3000,0,0));
 check(!DeferredReady(true,true,true,false,false,1,false,4000,0,0)); // one queued birth, fresh roster before next.
 check(!DeferredReady(true,true,true,true,false,0,false,4000,0,0));
 check(!DeferredReady(true,true,true,false,true,0,false,4000,0,0)); // entered failure cannot replay.
 check(!DeferredReady(true,true,true,false,false,0,true,4000,0,0));
 check(!DeferredReady(true,true,true,false,false,0,false,4000,0,5000));
 check(DeferredReady(true,true,true,false,false,0,false,5000,0,5000));
 check(!DeferredReady(true,true,true,false,false,0,false,5000,6000,0));
 std::printf("H24 stable-frame fill admission: %u checks, %u failures; no native calls.\n",n,bad);return bad?1:0;
}
