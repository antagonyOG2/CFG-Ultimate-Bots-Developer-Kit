#include "SpawnAssetObservation.hpp"
#include "ReportObservation.hpp"
#include <map>
#include <cstring>
#include <cstdio>
namespace {
int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
struct Fixture {
    std::map<std::uintptr_t,std::array<unsigned char,8>> memory;
    cfgprobe::lifetime::ArrayHeader array{0x20000000,16};
    std::uintptr_t module=0x10000000,helperClass=0x30000000,helperDefault=0x30010000,function=0x30020000;
    cfgprobe::lifetime::SlotObservation classSlot{helperClass,1,0},defaultSlot{helperDefault,2,0},functionSlot{function,3,0};
    std::uintptr_t mode=0x40000000,tree=0x40010000,treeClass=0x40020000,actorClass=0x40030000,locationFunction=0x40040000;
    cfgprobe::lifetime::SlotObservation treeSlot{tree,4,0},actorClassSlot{actorClass,5,0},locationSlot{locationFunction,6,0};
    unsigned reads=0,cap=128,triggerCount=0;
    std::uintptr_t trigger=0,mutation=0,replacement=0;
    template<class T> void Put(std::uintptr_t address,const T& value) {
        static_assert(sizeof(T)<=8,"bounded cell"); auto& bytes=memory[address]; bytes={}; std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& value) {
        if (reads++>=cap) return false;
        if (address==trigger && ++triggerCount==2) Put(mutation,replacement);
        const auto found=memory.find(address); if (found==memory.end()) return false;
        std::memcpy(&value,found->second.data(),sizeof(value)); return true;
    }
    void Slot(const cfgprobe::lifetime::SlotObservation& slot) {
        Put(slot.object+cfgprobe::lifetime::ObjectIndexOffset,slot.index);
        const auto at=array.data+static_cast<std::uintptr_t>(slot.index)*24;
        Put(at,slot.object); Put(at+8,std::uint32_t{0}); Put(at+16,slot.serial);
    }
    Fixture() {
        Slot(classSlot); Slot(defaultSlot); Slot(functionSlot); Slot(treeSlot); Slot(actorClassSlot); Slot(locationSlot);
        Put(module+cfgspawn::Classes[cfgspawn::Helper].reflection,helperClass);
        Put(module+cfgspawn::Classes[cfgspawn::Helper].constructor,helperClass);
        Put(helperClass+cfgspawn::ClassDefaultObject,helperDefault);
        Put(helperDefault+cfgprobe::ObjectClass,helperClass);
        Put(module+cfgspawn::SpawnFunctionCache,function);
        Put(function+0x20,helperClass);
        Put(mode+cfgspawn::ModeCounselorTree,tree);
        Put(module+cfgspawn::Classes[cfgspawn::Tree].reflection,treeClass);
        Put(module+cfgspawn::Classes[cfgspawn::Tree].constructor,treeClass);
        Put(tree+cfgprobe::ObjectClass,treeClass); Put(treeClass+cfgprobe::ClassSuper,std::uintptr_t{0});
        Put(module+cfgspawn::LocationFunctionCache,locationFunction);
        Put(module+cfgspawn::ActorClassReflectionCache,actorClass);
        Put(locationFunction+0x20,actorClass);
    }
    bool Verify() { reads=0; return cfgspawn::ObserveOwnedNativeHelper(*this,module,array,helperClass,helperDefault,function,classSlot,defaultSlot,functionSlot); }
    bool VerifyTree() { reads=0; return cfgspawn::ObserveModeTreeAndLocation(*this,module,mode,array,tree,treeClass,locationFunction,actorClass,treeSlot,locationSlot,actorClassSlot); }
};
}
int main() {
    Fixture good;
    CHECK(good.Verify() && good.reads<=48); // Zero serials are observations, not weak identity claims.
    CHECK(good.VerifyTree() && good.reads<=64);
    good.classSlot.serial=301; good.defaultSlot.serial=302; good.Slot(good.classSlot); good.Slot(good.defaultSlot);
    CHECK(good.Verify());
    for (unsigned index=0;index<=cfgspawn::SpawnAssetCount;++index) {
        Fixture f; cfgprobe::lifetime::SlotObservation observed;
        CHECK(cfgspawn::ReadSpawnAssetSlot(f,f.array,f.helperDefault,index,observed)==(index>=cfgspawn::HelperDefaultAssetIndex && index<cfgspawn::SpawnAssetCount));
    }
    for (unsigned change=0;change<10;++change) {
        Fixture f;
        if (change==0) f.Put(f.module+cfgspawn::Classes[cfgspawn::Helper].reflection,f.function);
        if (change==1) f.Put(f.module+cfgspawn::Classes[cfgspawn::Helper].constructor,f.function);
        if (change==2) f.Put(f.helperClass+cfgspawn::ClassDefaultObject,f.function);
        if (change==3) f.Put(f.helperDefault+cfgprobe::ObjectClass,f.function);
        if (change==4) f.Put(f.function+0x20,f.helperDefault);
        if (change==5) f.Put(f.module+cfgspawn::SpawnFunctionCache,f.helperDefault);
        if (change==6) f.Put(f.array.data+24,f.helperDefault);
        if (change==7) f.Put(f.array.data+48+16,std::int32_t{1});
        if (change==8) f.Put(f.array.data+48+8,std::uint32_t{0x20000000});
        if (change==9) f.Put(f.array.data+24+8,std::uint32_t{0x20000000});
        CHECK(!f.Verify());
    }
    for (unsigned change=0;change<4;++change) {
        Fixture f;
        f.trigger=change==0?f.helperClass+cfgspawn::ClassDefaultObject:
            change==1?f.function+0x20:change==2?f.module+cfgspawn::Classes[cfgspawn::Helper].reflection:f.array.data+48;
        f.mutation=f.trigger; f.replacement=f.function;
        CHECK(!f.Verify()); // Change between samples is not adopted.
    }
    Fixture bounded; bounded.cap=10; CHECK(!bounded.Verify());
    Fixture missing; missing.memory.erase(missing.function+0x20); CHECK(!missing.Verify());
    for (unsigned change=0;change<9;++change) {
        Fixture f;
        const std::uintptr_t at=change==0?f.mode+cfgspawn::ModeCounselorTree:
            change==1?f.module+cfgspawn::Classes[cfgspawn::Tree].reflection:
            change==2?f.module+cfgspawn::Classes[cfgspawn::Tree].constructor:
            change==3?f.tree+cfgprobe::ObjectClass:change==4?f.module+cfgspawn::ActorClassReflectionCache:
            change==5?f.module+cfgspawn::LocationFunctionCache:change==6?f.locationFunction+0x20:
            change==7?f.array.data+6*24:f.array.data+5*24;
        f.Put(at,f.function); CHECK(!f.VerifyTree());
        Fixture changing; changing.trigger=at; changing.mutation=at; changing.replacement=changing.function;
        CHECK(!changing.VerifyTree());
    }
    Fixture flagged; flagged.Put(flagged.array.data+4*24+8,std::uint32_t{0x20000000}); CHECK(!flagged.VerifyTree());
    Fixture treeBounded; treeBounded.cap=10; CHECK(!treeBounded.VerifyTree());
    CHECK(std::strcmp(cfgobserver::PreparationStageName(13),"SpawnBindings")==0);
    CHECK(std::strcmp(cfgobserver::PreparationStageName(14),"Unknown")==0);
    CHECK(std::strcmp(cfgobserver::PreparationStageName(0xFFFFFFFF),"Unknown")==0);
    cfgobserver::Report refusal; refusal.bytes=sizeof(refusal);
    refusal.status=static_cast<unsigned>(cfgobserver::Status::RefusedIdentity);
    refusal.preparationStage=static_cast<unsigned>(cfgobserver::PreparationStage::SpawnBindings);
    CHECK(cfgobserver::SamePreparationFailure(refusal,refusal));
    std::printf("CFG production spawn-asset ownership/refusal checks %s (%d failures). Owned synthetic memory only.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
