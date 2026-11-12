#include "H226HostedRuntimePolicy.hpp"
#include <cassert>
#include <cstdio>
int main(){
    assert(cfgh226::ContinueDonorAfterTargetProjectionFailure(true));
    assert(!cfgh226::ContinueDonorAfterTargetProjectionFailure(false));
    assert(cfgh226::UseHostedAimSibling(true,false));
    assert(!cfgh226::UseHostedAimSibling(false,false));
    assert(!cfgh226::UseHostedAimSibling(true,true));
    std::puts("H226 hosted policy PASS: only adopted hosted hunt bypasses optional projection refusal; only outer hosted knife aim receives a sibling reader.");
}
