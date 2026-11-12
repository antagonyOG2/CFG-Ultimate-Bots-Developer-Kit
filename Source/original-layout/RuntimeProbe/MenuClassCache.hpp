#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
namespace cfgmenu::capture {
// Read-only discovery acceleration, not class lifetime/call authority. A large
// mod's unrelated types must not make the bounded menu discovery abort. Exact
// pointer keys are compared; collisions evict, never alias or allocate more.
class MenuClassCache {
    struct Entry {std::uintptr_t type=0;bool menu=false;};
    std::array<Entry,4096> entries_{};
    static std::size_t Index(std::uintptr_t type) noexcept {
        return static_cast<std::size_t>((type>>4)^(type>>16))&4095;
    }
public:
    unsigned hits=0,misses=0,evictions=0;
    bool Lookup(std::uintptr_t type,bool& menu) noexcept {
        const auto& entry=entries_[Index(type)];
        if(type&&entry.type==type){menu=entry.menu;++hits;return true;}
        ++misses;return false;
    }
    void Store(std::uintptr_t type,bool menu) noexcept {
        if(!type)return;
        auto& entry=entries_[Index(type)];
        if(entry.type&&entry.type!=type)++evictions;
        entry={type,menu};
    }
};
}
