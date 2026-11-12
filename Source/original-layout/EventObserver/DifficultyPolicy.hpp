#pragma once
#include <cmath>
#include <cstdint>
namespace cfgdifficulty {
enum class Source:unsigned {None,Jason,Weapon,Projectile};
constexpr float DamageScale(unsigned difficulty){return difficulty==0?0.5f:difficulty==2?2.f:1.f;}
constexpr float SpeedScale(unsigned difficulty){return difficulty==2?2.f:1.f;}
inline bool EligibleAmount(float damage){return std::isfinite(damage)&&damage>0.f&&damage<10000.f;}
inline float Scale(float damage,unsigned difficulty,Source source){
    if(difficulty>2||source==Source::None||!EligibleAmount(damage))return damage;
    const float result=damage*DamageScale(difficulty);
    return std::isfinite(result)?result:damage;
}
// Windows x64: RCX victim, XMM1 damage, R8 event, R9 instigator,
// caller stack argument5 causer; XMM0 return. Forward all five arguments once.
using TakeDamageOriginal=float(__fastcall*)(void*,float,void*,void*,void*);
}
