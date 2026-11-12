#pragma once
#include <cstdint>
namespace cfginput {
// Preserve held-key edge behavior and also accept Windows' short-press latch
// between the existing100ms polls. Foreground ownership still gates requests;
// an event consumed by another app/thread is never invented or replayed.
inline bool ForegroundKeyEdge(bool foreground,std::int16_t state,bool& wasDown)noexcept{
    const auto bits=static_cast<std::uint16_t>(state);
    const bool down=foreground&&(bits&0x8000u)!=0;
    const bool edge=foreground&&!wasDown&&((bits&0x8001u)!=0);
    wasDown=down;return edge;
}
}
