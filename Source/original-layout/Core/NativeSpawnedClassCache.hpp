#pragma once
#include <cstdint>
namespace cfgmenu {
// Optional native bookkeeping, NOT class-selection authority. Callers must
// independently prove the selected asset, actual pawn class and possession.
// CFG OfflineBots RestartPlayer can leave this cache empty (live PID12252).
inline bool SpawnedClassCacheAgrees(std::uintptr_t cached,std::uintptr_t actual){
    return actual!=0&&(!cached||cached==actual);
}
}
