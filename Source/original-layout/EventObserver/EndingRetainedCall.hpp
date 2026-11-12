#pragma once
#include "../Core/EndingMetadataPolicy.hpp"
namespace cfgending {
// Used by the real BundleAccess gateway. Retain exact inputs before entry;
// consume before any allocating/reentrant work; never replay ambiguous work.
template<class Host>bool RetainedSetter(Transaction& transaction,NativeSetter setter,Host& host){
    if(!transaction.BeginNative(setter))return false;
    // Root registration itself can allocate/reenter. Ownership is consumed
    // before preparation as well as before the final native setter.
    if(!host.Prepare()||!host.Begin()){transaction.EndNative(setter);return false;}
    ++transaction.calls;
    const bool returned=host.Invoke();
    const bool completed=returned&&host.Finish();
    return transaction.EndNative(setter)&&completed;
}
}
