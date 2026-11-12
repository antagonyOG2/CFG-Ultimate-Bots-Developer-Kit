#pragma once
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
namespace cfgmenu::softpath {
// Native None is the empty asset path/subobject, not zero-filled cache bytes.
// Captured CFG Sandbox SelectedKillerOpaque40 beginsFFFFFFFF (unassigned weak
// index) while bytes16..39 are zero. Cache bytes are never selection authority.
inline bool Empty(const cfgprimary::SoftClassBytes& packet)noexcept{
    for(unsigned i=16;i<40;++i)if(packet[i])return false;
    return true;
}
// Plain asset references use the FName at byte16 and an empty SubPathString.
// Weak cache bytes0..15 may change across travel. Native calls must use the
// fresh world's exact packet, never borrowed string storage from the menu.
inline bool Plain(const cfgprimary::SoftClassBytes& packet)noexcept{
    std::int32_t index=0,number=0;
    std::memcpy(&index,packet.data()+16,4);std::memcpy(&number,packet.data()+20,4);
    if(index<0||number<0||(!index&&!number))return false;
    for(unsigned i=24;i<40;++i)if(packet[i])return false;
    return true;
}
inline bool Same(const cfgprimary::SoftClassBytes& a,const cfgprimary::SoftClassBytes& b)noexcept{
    if(!Plain(a)||!Plain(b))return false;
    for(unsigned i=16;i<24;++i)if(a[i]!=b[i])return false;
    return true;
}
// Semantic menu token, never an object address or a native-call target.
inline std::uintptr_t Token(const cfgprimary::SoftClassBytes& packet)noexcept{
    std::uint64_t value=0;if(Plain(packet))std::memcpy(&value,packet.data()+16,8);return static_cast<std::uintptr_t>(value);
}
}
