#pragma once
#include "PrimaryJasonParameters.hpp"
namespace cfgcounselorload {
// Donor preloads the whole native catalogue, not one cold choice per birth.
// Pace requests across callbacks and give the engine one bounded warm window.
struct Preload {
    unsigned cursor=0;std::uint64_t warmUntil=0,deadline=0;
    bool Loading(unsigned count)const{return cursor<count;}
    void Requested(unsigned count,std::uint64_t now){++cursor;if(cursor==count){warmUntil=now+2000;deadline=now+30000;}}
    bool Warming(std::uint64_t now)const{return warmUntil&&now<warmUntil;}
    bool Pending(std::uint64_t now)const{return deadline&&now<deadline;}
};
// Native FScriptDelegate and FLatentActionInfo. The delegate is deliberately
// unbound and Linkage=-1: only the engine-owned async resource request is used.
// No mod function/object is retained by the latent action.
struct Delegate {std::int32_t index=-1,serial=0;std::uint64_t function=0;};
struct Latent {std::int32_t linkage=-1,uuid=0;std::uint64_t function=0;std::uintptr_t target=0;};
struct Packet {std::uintptr_t world=0;cfgprimary::SoftClassBytes source{};Delegate onLoaded{};Latent latent{};};
static_assert(sizeof(Delegate)==16&&sizeof(Latent)==24&&sizeof(Packet)==88&&offsetof(Packet,source)==8&&offsetof(Packet,onLoaded)==48&&offsetof(Packet,latent)==64);
inline bool Build(std::uintptr_t world,std::uintptr_t owner,unsigned index,const cfgprimary::SoftClassBytes& source,Packet& out){
    out={};cfgprimary::ConversionParameters validated;
    if(!cfgprobe::ObjectPointer(world)||!cfgprobe::ObjectPointer(owner)||index>=64||!cfgprimary::BuildConversion(source,validated))return false;
    out.world=world;out.source=source;out.latent.target=owner;
    out.latent.uuid=0x43464700+static_cast<std::int32_t>(index);return true;
}
struct Pending {
    bool active=false;unsigned index=0;std::uint64_t deadline=0;
    bool Begin(unsigned entry,std::uint64_t now){if(active||entry>=64)return false;active=true;index=entry;deadline=now+30000;return true;}
    bool Waiting(std::uint64_t now)const{return active&&now<deadline;}
    void Reset(){*this={};}
};
}
