#pragma once
#include <cstdint>
namespace cfgh241move {
// The donor MoveToLocation packet is a fixed 40-byte scalar in/out frame.
// In hosted adoption it needs actor/function retention, not reflected UObject
// output discovery; no packet member is an object pointer.
constexpr bool UseHostedFixedMoveFrame(bool hostedRoutine,std::uintptr_t observedExecutor,
    std::uintptr_t module,std::uintptr_t moveExecutor) noexcept {
    return hostedRoutine&&module&&moveExecutor&&observedExecutor==module+moveExecutor;
}
template<class Reader>bool HostedFixedMoveFrame(Reader& reader,bool hostedRoutine,std::uintptr_t functionObject,
    std::uintptr_t module,std::uintptr_t moveExecutor) noexcept {
    std::uintptr_t observed=0;
    return hostedRoutine&&functionObject&&reader.Read(functionObject+0xB0,observed)&&
        UseHostedFixedMoveFrame(true,observed,module,moveExecutor);
}
}
