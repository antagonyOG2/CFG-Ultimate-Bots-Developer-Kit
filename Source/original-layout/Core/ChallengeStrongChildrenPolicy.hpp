#pragma once
#include <cstdint>
namespace cfgmenu::challengeintro {
// Typed registry slot state, read-only; indices were captured while the
// retained parent proved both exact strong links. Never allocate a serial.
template<class Reader>bool ChildrenFlagsClear(Reader& r,std::uintptr_t data,int levelIndex,int settingsIndex){
 std::uint32_t a=0,b=0;
 return data&&levelIndex>=0&&settingsIndex>=0&&
  r.Read(data+24ULL*static_cast<unsigned>(levelIndex)+8,a)&&
  r.Read(data+24ULL*static_cast<unsigned>(settingsIndex)+8,b)&&
  !((a|b)&0x30000000u);
}
}
