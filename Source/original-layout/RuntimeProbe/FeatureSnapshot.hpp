#pragma once
#include "FeatureSnapshotBindings.generated.hpp"
#include "PositionBindings.generated.hpp"
#include "ObjectLifetime.hpp"
#include "TypeObservations.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>

namespace cfgfeature {
using cfgprobe::lifetime::SlotObservation;
constexpr unsigned MaxWorldItems=256,MaxInventoryItems=32,MaxRecords=64;
struct Array {std::uintptr_t data=0;std::int32_t count=0,capacity=0;};
struct Vec {float x=0,y=0,z=0;};
static_assert(sizeof(Array)==16 && sizeof(Vec)==12,"Native snapshot layout");
inline bool SaneArray(const Array& a,unsigned max) noexcept {
    return a.count>=0 && static_cast<unsigned>(a.count)<=max && a.capacity>=a.count &&
        a.capacity<=65536 && (!a.count || (cfgprobe::ObjectPointer(a.data) &&
        cfgprobe::lifetime::Extent(a.data,static_cast<std::uintptr_t>(a.count)*8)));
}
inline bool SanePosition(const Vec& v) noexcept {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) &&
        std::fabs(v.x)<10000000 && std::fabs(v.y)<10000000 && std::fabs(v.z)<10000000;
}
struct DriverObservation {
    std::uintptr_t vehicle=0,driver=0;
    SlotObservation vehicleSlot{},driverSlot{};
};
template<class Reader> bool ObserveDriver(Reader& r,const cfgprobe::lifetime::ArrayHeader& table,
    std::uintptr_t vehicle,std::uintptr_t vehicleClass,std::uintptr_t counselorClass,DriverObservation& output) {
    output={};DriverObservation found;found.vehicle=vehicle;
    cfgprobe::Ancestry vehicleType,driverType;
    if(!cfgprobe::CaptureAncestry(r,vehicle,vehicleType) || !vehicleType.Contains(vehicleClass) ||
        !cfgprobe::lifetime::ReadSlot(r,table,vehicle,found.vehicleSlot) || !found.vehicleSlot.HasAssignedSerial() ||
        !r.Read(vehicle+disk::Driver,found.driver) || !cfgprobe::ObjectPointer(found.driver) ||
        !cfgprobe::CaptureAncestry(r,found.driver,driverType) || !driverType.Contains(counselorClass) ||
        !cfgprobe::lifetime::ReadSlot(r,table,found.driver,found.driverSlot) || !found.driverSlot.HasAssignedSerial()) return false;
    std::uintptr_t driverAgain=0;SlotObservation vehicleAgain{},driverAgainSlot{};
    if(!r.Read(vehicle+disk::Driver,driverAgain) || driverAgain!=found.driver ||
        !cfgprobe::StableAncestry(r,vehicle,vehicleType) || !cfgprobe::StableAncestry(r,found.driver,driverType) ||
        !cfgprobe::lifetime::ReadSlot(r,table,vehicle,vehicleAgain) || !(vehicleAgain==found.vehicleSlot) ||
        !cfgprobe::lifetime::ReadSlot(r,table,found.driver,driverAgainSlot) || !(driverAgainSlot==found.driverSlot)) return false;
    output=found;return true;
}
struct Record {
    const char* role="unknown";
    SlotObservation slot{},rootSlot{};
    // 'owner' is the source reference holder, NOT proof of native Actor.Owner.
    std::uintptr_t root=0,owner=0,related=0;
    Vec position{};
    bool positioned=false;
};
struct List {
    std::uintptr_t owner=0,offset=0;
    Array header{};
    std::array<std::uintptr_t,MaxWorldItems> items{};
};
struct Report {
    std::array<std::uintptr_t,disk::ClassCount> classes{};
    std::uintptr_t sceneClass=0,ability=0,abilityOwner=0;
    std::array<std::uintptr_t,2> inventories{},equipped{},cars{},drivers{};
    std::uintptr_t phone=0;
    unsigned char hasSweater=0,wearingSweater=0,unlimitedSweater=0,consumedCandidate=0;
    bool abilityObserved=false;
    std::array<Record,MaxRecords> records{};
    unsigned recordCount=0,unassigned=0,unclassified=0;
    std::array<List,3> lists{};
    unsigned listCount=0;
};
template<class Reader> class Capture {
    Reader& r;
    std::uintptr_t module;
    cfgprobe::lifetime::ArrayHeader table;
    cfgspawn::TypeObservations<Reader,512> types;
    Report result{};
    bool Typed(std::uintptr_t object,unsigned id) {
        return result.classes[id] && types.ObjectIs(object,result.classes[id]);
    }
    bool Add(std::uintptr_t object,const char* role,std::uintptr_t owner=0,std::uintptr_t related=0,bool actor=true) {
        if(!object) return true;
        for(unsigned i=0;i<result.recordCount;++i)
            if(result.records[i].slot.object==object) return true;
        if(result.recordCount==MaxRecords) return false;
        Record record;record.role=role;record.owner=owner;record.related=related;
        if(!cfgprobe::lifetime::ReadSlot(r,table,object,record.slot)) return false;
        if(!record.slot.HasAssignedSerial()) {++result.unassigned;return true;}
        if(actor && result.sceneClass) {
            if(!r.Read(object+cfgposition::disk::ActorRootComponent,record.root)) return false;
            if(record.root && types.ObjectIs(record.root,result.sceneClass)) {
                std::uintptr_t rootOwner=0;
                if(!cfgprobe::lifetime::ReadSlot(r,table,record.root,record.rootSlot) ||
                    !r.Read(record.root+0xE0,rootOwner) || rootOwner!=object) return false;
                if(record.rootSlot.HasAssignedSerial()) {
                    if(!r.Read(record.root+cfgposition::disk::SceneWorldTranslation,record.position) ||
                        !SanePosition(record.position)) return false;
                    record.positioned=true;
                }
            }
        }
        result.records[result.recordCount++]=record;return true;
    }
    bool Items(std::uintptr_t owner,std::uintptr_t offset,unsigned max,bool inventory) {
        if(result.listCount==result.lists.size()) return false;
        List list;list.owner=owner;list.offset=offset;
        if(!r.Read(owner+offset,list.header) || !SaneArray(list.header,max)) return false;
        if(list.header.count && !r.ReadBytes(list.header.data,list.items.data(),static_cast<std::size_t>(list.header.count)*8)) return false;
        for(std::int32_t i=0;i<list.header.count;++i) {
            const auto item=list.items[static_cast<unsigned>(i)];
            if(!item) continue;
            if(!cfgprobe::ObjectPointer(item)) return false;
            const char* kind=nullptr;
            if(Typed(item,disk::Sweater)) kind="sweater";
            else if(Typed(item,disk::Mask)) kind="killer_mask";
            else if(Typed(item,disk::Trap)) kind="trap_origin_unknown";
            else if(inventory && Typed(item,disk::Item)) kind="inventory_item";
            else {++result.unclassified;continue;}
            std::uintptr_t related=0;
            if(inventory && Typed(item,disk::Item) && !r.Read(item+disk::OuterInventory,related)) return false;
            if(!Add(item,kind,owner,related)) return false;
        }
        result.lists[result.listCount++]=list;return true;
    }
public:
    Capture(Reader& reader,std::uintptr_t base,const cfgprobe::lifetime::ArrayHeader& header):
        r(reader),module(base),table(header),types(reader) {}
    bool Run(std::uintptr_t state,std::uintptr_t pawn,bool counselor,Report& output) {
        output={};
        for(unsigned i=0;i<disk::Classes.size();++i) {
            std::uintptr_t a=0,b=0;
            if(!r.Read(module+disk::Classes[i].reflection,a) || !r.Read(module+disk::Classes[i].constructor,b)) return false;
            result.classes[i]=cfgprobe::MatchingClassCache(a,b);
        }
        std::uintptr_t sceneA=0,sceneB=0;
        if(!r.Read(module+cfgposition::disk::SceneClassCache,sceneA) ||
            !r.Read(module+cfgposition::disk::SceneConstructorCache,sceneB)) return false;
        result.sceneClass=cfgprobe::MatchingClassCache(sceneA,sceneB);
        std::array<unsigned char,cfgposition::disk::LocationBytes.size()> code{};
        if(!r.ReadBytes(module+cfgposition::disk::LocationThunk,code.data(),code.size()) ||
            code!=cfgposition::disk::LocationBytes || !Typed(state,disk::GameState) ||
            !Add(state,"game_state",0,0,false) || !Add(pawn,"human_pawn")) return false;
        if(counselor) {
            if(!Typed(pawn,disk::Counselor) || !r.Read(pawn+disk::AbilityField,result.ability) ||
                !r.Read(pawn+disk::HasSweater,result.hasSweater) ||
                !r.Read(pawn+disk::WearingSweater,result.wearingSweater) ||
                !r.Read(pawn+disk::UnlimitedSweater,result.unlimitedSweater)) return false;
            if(result.ability && Typed(result.ability,disk::Ability)) {
                if(!r.Read(result.ability+disk::AbilityOwner,result.abilityOwner) ||
                    !r.Read(result.ability+0x98,result.consumedCandidate) ||
                    !Add(result.ability,"sweater_ability",pawn,result.abilityOwner,false)) return false;
                result.abilityObserved=true; // +98 is provisional evidence ONLY; no write capability.
            }
            for(unsigned i=0;i<2;++i) {
                if(!r.Read(pawn+(i?disk::SmallInventory:disk::SpecialInventory),result.inventories[i])) return false;
                if(!result.inventories[i]) continue;
                if(!Typed(result.inventories[i],disk::Inventory) ||
                    !Add(result.inventories[i],i?"small_inventory":"special_inventory",pawn,0,false) ||
                    !r.Read(result.inventories[i]+disk::EquippedItem,result.equipped[i]) ||
                    !Items(result.inventories[i],disk::InventoryItems,MaxInventoryItems,true)) return false;
            }
        }
        for(unsigned i=0;i<2;++i) {
            if(!r.Read(state+(i?disk::Car2:disk::Car4),result.cars[i])) return false;
            if(!result.cars[i]) continue;
            if(!Typed(result.cars[i],disk::Vehicle) || !r.Read(result.cars[i]+disk::Driver,result.drivers[i]) ||
                !Add(result.cars[i],i?"car_2_seat":"car_4_seat",state,result.drivers[i])) return false;
        }
        if(!r.Read(state+disk::Phone,result.phone) || !Items(state,disk::WorldItems,MaxWorldItems,false)) return false;
        for(unsigned i=0;i<result.listCount;++i) {
            const auto& list=result.lists[i];Array fresh{};
            std::array<std::uintptr_t,MaxWorldItems> pointers{};
            if(!r.Read(list.owner+list.offset,fresh) || std::memcmp(&fresh,&list.header,sizeof(fresh)) ||
                (fresh.count && (!r.ReadBytes(fresh.data,pointers.data(),static_cast<std::size_t>(fresh.count)*8) ||
                std::memcmp(pointers.data(),list.items.data(),static_cast<std::size_t>(fresh.count)*8)))) return false;
        }
        cfgprobe::lifetime::ArrayHeader freshTable{};
        if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,freshTable) || !(freshTable==table)) return false;
        for(unsigned i=0;i<result.recordCount;++i) {
            const auto& record=result.records[i];SlotObservation fresh{};
            if(!cfgprobe::lifetime::ReadSlot(r,table,record.slot.object,fresh) || !(fresh==record.slot)) return false;
            if(record.positioned) {
                std::uintptr_t root=0,owner=0;
                if(!r.Read(record.slot.object+cfgposition::disk::ActorRootComponent,root) || root!=record.root ||
                    !r.Read(root+0xE0,owner) || owner!=record.slot.object ||
                    !cfgprobe::lifetime::ReadSlot(r,table,root,fresh) || !(fresh==record.rootSlot)) return false;
            }
        }
        for(unsigned i=0;i<disk::Classes.size();++i) {
            std::uintptr_t a=0,b=0;
            if(!r.Read(module+disk::Classes[i].reflection,a) || !r.Read(module+disk::Classes[i].constructor,b) ||
                cfgprobe::MatchingClassCache(a,b)!=result.classes[i]) return false;
        }
        std::uintptr_t sceneAgainA=0,sceneAgainB=0;
        if(!r.Read(module+cfgposition::disk::SceneClassCache,sceneAgainA) ||
            !r.Read(module+cfgposition::disk::SceneConstructorCache,sceneAgainB) ||
            cfgprobe::MatchingClassCache(sceneAgainA,sceneAgainB)!=result.sceneClass || !types.Revalidate()) return false;
        std::uintptr_t pointerAgain=0;
        if(!r.Read(state+disk::Phone,pointerAgain) || pointerAgain!=result.phone) return false;
        for(unsigned i=0;i<2;++i) {
            if(!r.Read(state+(i?disk::Car2:disk::Car4),pointerAgain) || pointerAgain!=result.cars[i] ||
                (result.cars[i] && (!r.Read(result.cars[i]+disk::Driver,pointerAgain) || pointerAgain!=result.drivers[i]))) return false;
            if(counselor && (!r.Read(pawn+(i?disk::SmallInventory:disk::SpecialInventory),pointerAgain) ||
                pointerAgain!=result.inventories[i] || (result.inventories[i] &&
                (!r.Read(result.inventories[i]+disk::EquippedItem,pointerAgain) || pointerAgain!=result.equipped[i])))) return false;
        }
        if(counselor && (!r.Read(pawn+disk::AbilityField,pointerAgain) || pointerAgain!=result.ability ||
            (result.abilityObserved && (!r.Read(result.ability+disk::AbilityOwner,pointerAgain) || pointerAgain!=result.abilityOwner)))) return false;
        output=result;return true;
    }
};
inline void Print(const Report& report,unsigned reads) {
    std::printf("FeatureSnapshot=CAPTURED\nFeatureReadCalls=%u\nFeatureReadCap=4096\n",reads);
    for(unsigned i=0;i<disk::Classes.size();++i)
        std::printf("Class_%s=%s\n",disk::Classes[i].name,report.classes[i]?"matching_native_caches":"unresolved");
    std::printf("HasSweater=%u\nWearingSweater=%u\nUnlimitedSweater=%u\nAbilityObserved=%s\nAbility=0x%llX\nAbilityOwner=0x%llX\nAbilityByte98_Unverified=%u\nWorldItemCount=%d\nUnclassifiedItemCount=%u\nUnassignedRecordCount=%u\nPhoneReference_Unclassified=0x%llX\n",
        static_cast<unsigned>(report.hasSweater),static_cast<unsigned>(report.wearingSweater),static_cast<unsigned>(report.unlimitedSweater),report.abilityObserved?"yes":"no",
        static_cast<unsigned long long>(report.ability),static_cast<unsigned long long>(report.abilityOwner),
        static_cast<unsigned>(report.consumedCandidate),report.lists[report.listCount-1].header.count,report.unclassified,report.unassigned,
        static_cast<unsigned long long>(report.phone));
    for(unsigned i=0;i<report.recordCount;++i) {
        const auto& record=report.records[i];
        std::printf("Record_%u=%s object:0x%llX serial:%d source:0x%llX related:0x%llX position:%s xyz:%.3f,%.3f,%.3f\n",i,record.role,
            static_cast<unsigned long long>(record.slot.object),record.slot.serial,
            static_cast<unsigned long long>(record.owner),static_cast<unsigned long long>(record.related),
            record.positioned?"sampled":"unavailable",record.position.x,record.position.y,record.position.z);
    }
    std::puts("CaptureIsReadOnly=yes\nNativeCalls=0\nMemoryWrites=0\nBackgroundWorkers=0\nTrapOriginAndPlacementUnverified=yes\nAbilityByte98NotWriteApproved=yes");
}
} // namespace cfgfeature
