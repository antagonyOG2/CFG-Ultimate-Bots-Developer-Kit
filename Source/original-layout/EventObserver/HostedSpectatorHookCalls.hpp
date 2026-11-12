#pragma once
#include <cstdint>

namespace cfgh216hooks {
// The only post-original reads permitted before the context factory are the
// already-safe current-thread value and the process-resident owner atomic.
template<class Result,class Context>
Result OwnerThreadAfterOriginal(Result stock,std::uint32_t callbackThread,std::uint32_t ownerThread,Context&& context) {
    if(!ownerThread||callbackThread!=ownerThread)return stock;
    return context(callbackThread,ownerThread,stock);
}

// Shared by the production wrappers and fixture. This is a read-only window
// after the native original: no callback root lease is required because the
// collector itself re-proves current world, adoption, roster and owner links.
template<class Result,class Observation,class Authorize,class Observe,class Decide>
Result ReadOnlyAfterOriginal(Result stock,bool shouldObserve,bool hooksReady,
    std::uint32_t callbackThread,std::uint32_t ownerThread,bool exactMode,
    Authorize&& authorize,Observe&& observe,Decide&& decide) {
    if(!shouldObserve||!hooksReady||callbackThread!=ownerThread||!exactMode)return stock;
    if(!authorize())return stock;
    Observation proof{};
    if(!observe(proof))return stock;
    return decide(stock,proof);
}

// The guard spans the native original and policy decision. Nested calls still
// invoke their original once, but cannot consume or advance a selector edge.
template<class Original,class Decide>
auto SelectorCall(bool& busy,Original original,void* controller,std::int32_t direction,Decide&& decide)
    -> decltype(original(controller,direction)) {
    if(busy)return original(controller,direction);
    struct Guard { bool& value;explicit Guard(bool& v):value(v){value=true;}~Guard(){value=false;} } guard(busy);
    const auto stock=original(controller,direction);
    return decide(stock,direction);
}
// A separate guard lets the selector's original invoke native CanSpectate;
// recursive CanSpectate calls remain stock-only.
template<class Original,class Decide>
auto CanSpectateCall(bool& busy,Original original,void* mode,void* viewer,void* state,Decide&& decide)
    -> decltype(original(mode,viewer,state)) {
    if(busy)return original(mode,viewer,state);
    struct Guard { bool& value;explicit Guard(bool& v):value(v){value=true;}~Guard(){value=false;} } guard(busy);
    const auto stock=original(mode,viewer,state);
    return decide(stock,mode,viewer,state);
}
} // namespace cfgh216hooks
