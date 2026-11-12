#include "PrimarySpawnObservation.hpp"
#include "../Core/NativeSpawnParameters.hpp"
#include "CrowdObservation.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>

namespace {
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    unsigned reads=0;
    template<class T> void Put(std::uintptr_t address,const T& value){
        static_assert(sizeof(T)<=16);auto& bytes=memory[address];bytes={};std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& value){
        ++reads;const auto found=memory.find(address);if(found==memory.end())return false;
        std::memcpy(&value,found->second.data(),sizeof(value));return true;
    }
};
constexpr std::uintptr_t Module=0x10000000,Array=0x20000000,Data=0x21000000,Path=0x30000000,Controller=0x31000000;
struct Fixture : Reader {
    cfgprobe::lifetime::ArrayHeader header{Data,8};
    cfgprobe::lifetime::SlotObservation slot{Path,3,7};
    Fixture(){
        Put(Path+cfgprobe::lifetime::ObjectIndexOffset,std::int32_t{3});
        Put(Data+3*cfgprobe::lifetime::ItemBytes,Path);Put(Data+3*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{0});
        Put(Data+3*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemSerialOffset,std::int32_t{7});
        Put(Path+0x20,Controller);Put(Path+0xE0,Controller);Put(Path+cfgcrowd::PathStatus,std::uint32_t{0});
        SetKind(2);
    }
    void SetKind(unsigned kind){
        const auto table=kind==1?cfgcrowd::OrdinaryVtable:kind==2?cfgcrowd::CrowdVtable:cfgcrowd::SCVtable;
        Put(Path,Module+table);Put(Module+table+cfgcrowd::FollowSlot,Module+(kind==1?cfgcrowd::OrdinaryFollow:cfgcrowd::CrowdFollow));
        if(kind!=1){Put(Module+table+cfgcrowd::SetterSlot,Module+cfgcrowd::Setter);Put(Path+cfgcrowd::SimulationState,std::uint8_t{0});
            Put(Path+cfgcrowd::RegistrationFlags,std::uint8_t{0x10});}
    }
    bool Observe(cfgcrowd::Observation& out){return cfgcrowd::Observe(*this,Module,header,slot,Controller,out);}
};
}

int main(){
    unsigned checks=0,failures=0;
    const auto check=[&](bool value,const char* label){++checks;if(!value){++failures;std::printf("FAIL %s\n",label);}};
    Fixture ordinary;ordinary.SetKind(1);cfgcrowd::Observation before{};
    check(ordinary.Observe(before)&&before.kind==1&&cfgcrowd::OptionalSetter(before)==cfgcrowd::OptionalSetterAction::Skip,
        "verified ordinary PathFollowing skips crowd-only setter");
    unsigned setterCalls=0;if(ordinary.Observe(before)&&cfgcrowd::OptionalSetter(before)==cfgcrowd::OptionalSetterAction::Apply)++setterCalls;
    check(setterCalls==0,"ordinary component makes zero optional setter calls");
    Fixture crowd;check(crowd.Observe(before)&&before.kind==2&&
        cfgcrowd::OptionalSetter(before)==cfgcrowd::OptionalSetterAction::Apply,"verified CrowdComponent admits setter");
    ++setterCalls;crowd.Put(Path+cfgcrowd::SimulationState,cfgcrowd::DisabledState);
    crowd.Put(Path+cfgcrowd::RegistrationFlags,std::uint8_t{0});cfgcrowd::Observation after{};
    check(crowd.Observe(after)&&cfgcrowd::SetterPostcondition(before,after),"setter state and registration postcondition");
    Fixture wrongTable;wrongTable.Put(Module+cfgcrowd::CrowdVtable+cfgcrowd::SetterSlot,Module+0x1234);
    check(!wrongTable.Observe(before),"wrong crowd vtable slot refuses before call");
    Fixture wrongKind;wrongKind.Put(Path,std::uintptr_t{Module+0x7777});
    check(!wrongKind.Observe(before),"unrecognized receiver refuses before call");
    Fixture busy;busy.Put(Path+cfgcrowd::PathStatus,std::uint32_t{1});
    check(busy.Observe(before)&&cfgcrowd::OptionalSetter(before)==cfgcrowd::OptionalSetterAction::Refuse,
        "busy component refuses optional call");
    Fixture inconsistent;inconsistent.Put(Path+cfgcrowd::SimulationState,cfgcrowd::DisabledState);
    inconsistent.Put(Path+cfgcrowd::RegistrationFlags,std::uint8_t{0x10});
    check(inconsistent.Observe(before)&&cfgcrowd::OptionalSetter(before)==cfgcrowd::OptionalSetterAction::Refuse,
        "already disabled but registered never enters the no-op setter");
    Fixture recycled;recycled.Put(Data+3*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemSerialOffset,std::int32_t{8});
    check(!recycled.Observe(before),"changed registry slot refuses receiver proof");
    check(!cfgcrowd::SetterPostcondition(cfgcrowd::Observation{2,0,0,0},cfgcrowd::Observation{3,0,2,0}),
        "postcondition rejects changed native class kind");
    std::printf("H225 crowd receiver seam: %u checks, %u failures; simulated local setter calls=%u.\n",checks,failures,setterCalls);
    return failures?1:0;
}
