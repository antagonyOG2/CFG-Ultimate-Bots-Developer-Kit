#pragma once
#include <array>
#include <cstdint>
namespace cfgending {
using Packet=std::array<unsigned char,40>;
// Native registration is empty-or-equal only. Never replace another killer or
// register the local counselor; caller proves typed strong ownership twice.
inline bool CanRegisterState(std::uintptr_t owned,std::uintptr_t human,std::uintptr_t previous){
    return owned&&owned!=human&&(!previous||previous==owned);
}
enum class Stage:unsigned { Prepare,Class,Skin,Weapon,Publish,Ready,Refused,Active,ActiveInFlight,SkinInFlight };
enum class NativeSetter:unsigned { Class,Active,Skin,Weapon };
// The native class setter consumes its by-value FString. Restrict the local
// packet to a genuinely empty subpath, so it owns no borrowed allocation.
inline bool EmptySubpath(const Packet& p){
    bool named=false;for(unsigned i=16;i<24;++i)named|=p[i]!=0;
    for(unsigned i=24;i<40;++i)if(p[i])return false;return named;
}
inline bool SamePath(const Packet& a,const Packet& b){
    if(!EmptySubpath(a)||!EmptySubpath(b))return false;
    for(unsigned i=16;i<24;++i)if(a[i]!=b[i])return false;return true;
}
inline bool EmptyPath(const Packet& p){for(unsigned i=16;i<40;++i)if(p[i])return false;return true;}
struct Transaction {
    Stage stage=Stage::Prepare;unsigned failure=0,calls=0;
    std::uintptr_t playerState=0,skin=0,selectedSource=0,weaponSource=0,playerStateType=0;
    std::int32_t playerStateSerial=0,playerStateIndex=-1;Packet selected{},weapon{};
    unsigned nativeConsumed=0;bool nativeBusy=false;
    bool nativeOwned=false; // exact hosted Hunt ownership already published; no offline presentation rewrite
    bool Finished()const{return nativeOwned||stage==Stage::Ready||stage==Stage::Refused;}
    bool Refuse(unsigned reason){failure=reason;stage=Stage::Refused;return false;}
    static Stage NativeStage(NativeSetter setter){
        switch(setter){
        case NativeSetter::Class:return Stage::Class;
        case NativeSetter::Active:return Stage::ActiveInFlight;
        case NativeSetter::Skin:return Stage::SkinInFlight;
        case NativeSetter::Weapon:return Stage::Weapon;
        }return Stage::Refused;
    }
    bool NativeAvailable(NativeSetter setter)const{
        const auto index=static_cast<unsigned>(setter);
        return index<4&&!nativeBusy&&!Finished()&&stage==NativeStage(setter)&&!(nativeConsumed&(1u<<index));
    }
    bool BeginNative(NativeSetter setter){
        if(!NativeAvailable(setter))return false;
        nativeConsumed|=1u<<static_cast<unsigned>(setter);nativeBusy=true;return true;
    }
    bool EndNative(NativeSetter setter){
        const bool same=nativeBusy&&stage==NativeStage(setter)&&!Finished();nativeBusy=false;return same;
    }
    // Consume before entering native code. Reentry or an ambiguous return may
    // never replay this setter, nor allow publication of partial metadata.
    bool BeginActive(const Packet& before){
        if(stage!=Stage::Active||!EmptySubpath(selected)||(!EmptyPath(before)&&!SamePath(before,selected)))return Refuse(16);
        stage=Stage::ActiveInFlight;return true;
    }
    bool CompleteActive(bool fresh,const Packet& after){
        if(stage!=Stage::ActiveInFlight||!fresh||!SamePath(after,selected))return Refuse(17);
        stage=Stage::Skin;return true;
    }
};
}
