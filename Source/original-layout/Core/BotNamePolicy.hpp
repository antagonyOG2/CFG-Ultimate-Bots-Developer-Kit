#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
namespace cfgbotname {
constexpr unsigned Maximum=31;
struct Name {std::array<wchar_t,32> text{};std::int32_t count=0;};
inline Name Make(unsigned index,unsigned seed){
    Name out;if(index>=Maximum)return out;
    constexpr const wchar_t* first[]={L"Neon",L"Pixel",L"Rusty",L"Ghost",L"Turbo",L"Quiet",L"Lucky",L"Midnight",L"Fuzzy",L"Static",L"Cosmic",L"Cobalt",L"Velvet",L"Silver",L"Dizzy",L"Atomic"};
    constexpr const wchar_t* last[]={L"Moth",L"Badger",L"Lantern",L"Waffle",L"Raccoon",L"Pine",L"Cactus",L"Cricket",L"Otter",L"Mushroom",L"Fox",L"Noodle",L"Comet",L"Gecko",L"Sparrow",L"Pebble"};
    const unsigned key=(seed+index*73u)&255u;
    for(const auto* word:{first[key&15u],last[key>>4]})for(const auto* p=word;*p&&out.count<30;++p)out.text[static_cast<unsigned>(out.count++)]=*p;
    ++out.count;return out;
}
struct String {wchar_t* data=nullptr;std::int32_t count=0,capacity=0;};
static_assert(sizeof(String)==16&&offsetof(String,count)==8);
struct Queue {unsigned cursor=0,attempts=0,confirmed=0,skipped=0,failure=0,seed=0;std::array<std::uintptr_t,Maximum> claimedStates{};};
}
