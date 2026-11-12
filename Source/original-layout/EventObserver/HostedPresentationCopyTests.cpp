// H2.43 fixture: hosted killer-presentation copy with a serial-zero newborn AI PlayerState.
//   * Core/HostedPresentationCopy.hpp Sequence (the SAME ordered clause code production runs) reaches BOTH setters,
//     skin before weapon, when the target PlayerState slot has serial 0.
//   * the H2.41 strict cfgjason::SameSlot refuses that same target at clause 37 (the live H2.42 refusal) -> old=37, new=0.
//   * the retention roots hold no native code address and never name the receipt's pawn/controller, and
//     cfghosted::adoption::Current still passes after the whole copy (also when retention assigns a serial to the
//     target PlayerState, because the receipt stores no PlayerState slot).
#include "../Core/HostedPresentationCopy.hpp"
#include "HostedAdoption.hpp"
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include <array>
namespace {
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    unsigned reads=0,cap=4096;
    template<class T> void Put(std::uintptr_t address,const T& value){
        static_assert(sizeof(T)<=16,"Fixture cell bound");
        auto& bytes=memory[address];bytes={};std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& output){
        if(reads>=cap)return false;++reads;
        const auto found=memory.find(address);if(found==memory.end())return false;
        T value{};std::memcpy(&value,found->second.data(),sizeof(value));output=value;return true;
    }
};
int failures=0,checks=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(false)
using namespace cfgprobe;
constexpr std::uintptr_t Module=0x10000000,SetSkinAddr=Module+0x3D2C70,SetWeaponAddr=Module+0x3D2D00;
constexpr std::uintptr_t Pawn=0x21000000,Ctl=0x21100000,State=0x21300000,Human=0x21500000,HumanCtl=0x21600000,
    Skin=0x22000000,SelectedClass=0x22100000,SkinClass=0x22200000;
struct Scene {
    Reader r;cfgobserver::Plan plan;cfghosted::adoption::Receipt receipt;
    cfgprobe::lifetime::SlotObservation targetSlot{},sourceSlot{};
    explicit Scene(std::int32_t stateSerial){
        plan.objectArray={0x30000000,64};plan.world=0x20020000;plan.mode=0x20030000;plan.controller=0x20040000;plan.pawn=0x20050000;
        const auto slot=[&](std::uintptr_t object,std::int32_t index,std::int32_t serial){
            r.Put(object+lifetime::ObjectIndexOffset,index);
            r.Put(plan.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes,object);
            r.Put(plan.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes+lifetime::ItemSerialOffset,serial);
            r.Put(plan.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes+8,std::uint32_t{0});};
        slot(Pawn,8,201);slot(Ctl,9,202);slot(State,11,stateSerial);slot(Human,12,203);
        r.Put(Pawn+0x3A0,Ctl);r.Put(Ctl+0x370,Pawn);r.Put(Ctl+0x388,State);r.Put(Pawn+0x388,State);r.Put(State+0x88,Ctl);
        receipt.valid=true;receipt.pawn={Pawn,8,201};receipt.controller={Ctl,9,202};receipt.controllerState=State;
        receipt.displacedState=Human;receipt.world=plan.world;receipt.mode=plan.mode;receipt.possessReturned=true;
        receipt.botsBefore=0;receipt.botsAfter=1;receipt.rosterBefore=2;receipt.rosterAfter=3;
        cfgspawn::ObjectSlot(r,plan.objectArray,State,targetSlot);cfgspawn::ObjectSlot(r,plan.objectArray,Human,sourceSlot);
        r.reads=0;
    }
    bool Current(){r.reads=0;return cfghosted::adoption::Current(r,plan.objectArray,receipt,plan);}
};
struct Host {
    Scene& w;bool strict;bool retainAssignsSerial=false;bool nativeAlive=true;
    std::vector<int> calls;std::vector<std::uintptr_t> retained;
    bool SlotOk(const cfgprobe::lifetime::SlotObservation& s){
        w.r.reads=0;return strict?cfgjason::SameSlot(w.r,w.plan.objectArray,s):cfgjason::OwnedComponentSlot(w.r,w.plan.objectArray,s);
    }
    template<class A> bool RetainRoots(const A& roots){
        retained.assign(roots.begin(),roots.end());
        if(retainAssignsSerial) // worst case: the collector bank gives the newborn state a weak serial
            w.r.Put(w.plan.objectArray.data+11*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{777});
        return true;
    }
    bool Retain(){return RetainRoots(cfghostedpresent::Roots(State,Human,HumanCtl,Skin,SkinClass,SelectedClass,SetSkinAddr,SetWeaponAddr));}
    bool World(){return true;}
    bool TargetSlot(){return SlotOk(w.targetSlot);}
    bool SourceSlot(){return SlotOk(w.sourceSlot);}
    bool SourceOwner(){return true;}
    bool SourceController(){return true;}
    bool TargetCurrent(){return true;}
    bool SourceSkinUnchanged(){return true;}
    bool SourceWeaponUnchanged(bool){return true;}
    bool NativeAllowed(){return nativeAlive;}
    bool Begin(){return true;}
    bool Finish(){return true;}
    int CallSkin(){calls.push_back(1);return 0;}
    bool SkinApplied(){return true;}
    int CallWeapon(){calls.push_back(2);return 0;}
    bool WeaponApplied(){return true;}
};
}
int main(){
    // ---- target PlayerState has serial 0 (newborn, CFG never allocates one) ----
    {Scene w(0);CHECK(w.targetSlot.object==State&&w.targetSlot.serial==0&&!w.targetSlot.HasAssignedSerial());
     CHECK(w.receipt.Complete()&&w.Current());
     // H2.41/H2.42 strict identity: refused at clause 37 with NO setter reached (the live H2.42 symptom).
     Host oldHost{w,true};CHECK(cfghostedpresent::Sequence(oldHost,false)==37&&oldHost.calls.empty());
     // H2.43 identity: both stock setters reached, skin then weapon, and adoption stays current.
     Host host{w,false};CHECK(cfghostedpresent::Sequence(host,false)==0);
     CHECK(host.calls==std::vector<int>({1,2}));
     CHECK(w.Current());
     // F1 style (selected defaults) uses the same sequence.
     Host extra{w,false};CHECK(cfghostedpresent::Sequence(extra,true)==0&&extra.calls==std::vector<int>({1,2}));
     // Roots: only UObjects, no native code address, never the receipt's pawn/controller.
     bool code=false,receiptSlot=false;for(const auto v:host.retained){
         if(v>=Module&&v<Module+0x01000000)code=true;
         if(v==w.receipt.pawn.object||v==w.receipt.controller.object)receiptSlot=true;}
#if CFG_H243_PRESENTATION
     CHECK(!code&&host.retained.size()==6);
#else
     CHECK(code&&host.retained.size()==8);
#endif
     CHECK(!receiptSlot);
    }
    // ---- worst case: retention assigns the newborn state a serial -> identity changed -> refuse, setters never run,
    //      and the receipt is STILL current because it stores no PlayerState slot ----
    {Scene w(0);Host host{w,false,true};CHECK(cfghostedpresent::Sequence(host,false)==37&&host.calls.empty());
     CHECK(w.receipt.Complete()&&w.Current());}
    // ---- positive serial target works under both identities ----
    {Scene w(55);Host s{w,true};CHECK(cfghostedpresent::Sequence(s,false)==0&&s.calls.size()==2);
     Scene v(55);Host n{v,false};CHECK(cfghostedpresent::Sequence(n,false)==0&&n.calls.size()==2&&v.Current());}
    // ---- identity still protects: index/object/serial replaced -> refused before any setter ----
    {Scene w(0);w.r.Put(w.plan.objectArray.data+11*lifetime::ItemBytes,std::uintptr_t{0x2FFFFFF0});
     Host h{w,false};CHECK(cfghostedpresent::Sequence(h,false)==37&&h.calls.empty());}
    {Scene w(0);Host h{w,false};h.nativeAlive=false;CHECK(cfghostedpresent::Sequence(h,false)==47&&h.calls.empty());}
    // ---- the receipt itself is broken by pawn/controller identity changes, never by the copy ----
    {Scene w(0);Host h{w,false};CHECK(cfghostedpresent::Sequence(h,false)==0);
     w.r.Put(w.plan.objectArray.data+9*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});CHECK(!w.Current());}
    CHECK(cfghostedpresent::RetrySafe(37)&&cfghostedpresent::RetrySafe(48)&&!cfghostedpresent::RetrySafe(49)&&
          !cfghostedpresent::RetrySafe(0)&&!cfghostedpresent::RetrySafe(62));
    std::printf("H243 presentation copy (CFG_H243_PRESENTATION=%d): %d checks, %d failures.\n",CFG_H243_PRESENTATION,checks,failures);
    return failures?1:0;
}
