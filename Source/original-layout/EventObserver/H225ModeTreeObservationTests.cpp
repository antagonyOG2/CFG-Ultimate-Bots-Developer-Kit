#include "SpawnAssetObservation.hpp"
#include "PrimarySpawnObservation.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>

namespace {
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    template<class T> void Put(std::uintptr_t address,const T& value){
        static_assert(sizeof(T)<=16);auto& bytes=memory[address];bytes={};std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& value){
        const auto found=memory.find(address);if(found==memory.end())return false;
        std::memcpy(&value,found->second.data(),sizeof(value));return true;
    }
};
constexpr std::uintptr_t Module=0x10000000,Mode=0x20000000,Data=0x21000000,
    Tree=0x30000000,TreeClass=0x31000000,ActorClass=0x32000000,Location=0x33000000;
struct Fixture : Reader {
    cfgprobe::lifetime::ArrayHeader array{Data,16};
    cfgprobe::lifetime::SlotObservation treeSlot{Tree,5,0},functionSlot{Location,6,0},actorSlot{ActorClass,7,0};
    Fixture(){
        const auto slot=[&](cfgprobe::lifetime::SlotObservation s){
            Put(s.object+cfgprobe::lifetime::ObjectIndexOffset,s.index);const auto at=Data+static_cast<std::uintptr_t>(s.index)*cfgprobe::lifetime::ItemBytes;
            Put(at,s.object);Put(at+8,std::uint32_t{0});Put(at+cfgprobe::lifetime::ItemSerialOffset,s.serial);
        };
        slot(treeSlot);slot(functionSlot);slot(actorSlot);
        Put(Mode+cfgspawn::ModeCounselorTree,Tree);Put(Module+cfgspawn::Classes[cfgspawn::Tree].reflection,TreeClass);
        Put(Module+cfgspawn::Classes[cfgspawn::Tree].constructor,TreeClass);Put(Tree+cfgprobe::ObjectClass,TreeClass);
        Put(TreeClass+cfgprobe::ClassSuper,std::uintptr_t{});
        Put(Module+cfgspawn::LocationFunctionCache,Location);Put(Module+cfgspawn::ActorClassReflectionCache,ActorClass);
        Put(Location+0x20,ActorClass);
    }
    bool Observe(){return cfgspawn::ObserveModeTreeAndLocation(*this,Module,Mode,array,Tree,TreeClass,Location,
        ActorClass,treeSlot,functionSlot,actorSlot);}
};
}
int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    Fixture valid;cfgprobe::lifetime::SlotObservation tree{};
    check(cfgspawn::ReadSpawnAssetSlot(valid,valid.array,Tree,5,tree)&&tree.serial==0&&valid.Observe());
    Fixture replaced;replaced.Put(Data+5*cfgprobe::lifetime::ItemBytes,Location);check(!replaced.Observe());
    Fixture changedCache;changedCache.Put(Module+cfgspawn::Classes[cfgspawn::Tree].constructor,ActorClass);check(!changedCache.Observe());
    Fixture wrongType;wrongType.Put(Tree+cfgprobe::ObjectClass,ActorClass);check(!wrongType.Observe());
    Fixture invalidOwner;invalidOwner.Put(Location+0x20,Tree);check(!invalidOwner.Observe());
    Fixture deniedSlot;deniedSlot.Put(Data+5*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{0x20000000});check(!deniedSlot.Observe());
    std::printf("H225 existing mode-owned tree protocol: %u checks, %u failures; serial-zero accepted only with matching mode/cache/type/slot/owner proofs.\n",checks,failures);
    return failures?1:0;
}
