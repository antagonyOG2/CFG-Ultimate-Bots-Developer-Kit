#pragma once
#include <cstdint>
#include <cstddef>
#include <map>
// Synthetic byte-addressable fixture only. Never maps or reads a game process.
struct OwnedByteReader {
    std::map<std::uintptr_t,unsigned char> bytes;
    unsigned reads=0;std::uintptr_t changeAt=0,changeSlot=0,newPointer=0;
    template<class T> void Set(std::uintptr_t at,const T& x){const auto* p=reinterpret_cast<const unsigned char*>(&x);for(size_t i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    // A fault at a field byte must also fire when a coherent range contains
    // that byte. Exact-start-only injection misses faults in grouped reads.
    template<class T> bool Read(std::uintptr_t at,T& x){++reads;if(changeSlot&&at<=changeAt&&changeAt-at<sizeof(T))Set(changeSlot,newPointer);
        auto* p=reinterpret_cast<unsigned char*>(&x);for(size_t i=0;i<sizeof(T);++i){auto it=bytes.find(at+i);if(it==bytes.end())return false;p[i]=it->second;}return true;}
};
