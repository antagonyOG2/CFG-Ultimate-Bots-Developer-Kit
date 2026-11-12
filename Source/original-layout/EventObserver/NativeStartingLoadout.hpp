#pragma once
#include "NativeRestrictedLoadout.hpp"
#include "../Core/StartingLoadoutPolicy.hpp"
namespace cfgloadout::expanded {
struct Request {cfgofflinesettings::Request owner{};Slot slot=Slot::Complete;unsigned choice=0;
    bool Valid()const{return owner.Valid()&&!ClassName(slot,choice).empty();}};
struct Evidence {
    std::uintptr_t type=0,defaults=0,small=0,special=0,equipped=0;
    std::uint8_t large=0,isSpecial=0;int maximumSmall=0;
    std::array<restricted::List,2> lists{};
    std::array<cfgprobe::lifetime::SlotObservation,7> slots{};
    bool operator==(const Evidence& b)const{return type==b.type&&defaults==b.defaults&&small==b.small&&special==b.special&&equipped==b.equipped&&large==b.large&&isSpecial==b.isSpecial&&maximumSmall==b.maximumSmall&&lists==b.lists&&slots==b.slots;}
};
// Host.Current proves owner thread, no GC, exact live offline local counselor
// ownership and the committed world. Discovery supplies a loaded class; no
// native class pointer is retained across worlds. All grant calls use CFG ABI.
template<class Reader,class Host> class NativeAccess {
    Reader& reader_;Host& host_;Request request_;std::uintptr_t module_=0,type_=0;
    template<class R,class T> static bool Same(R& r,std::uintptr_t p,const T& v){T actual{};return r.Read(p,actual)&&actual==v;}
    template<class R> bool Pair(R& r,std::uintptr_t a,std::uintptr_t b,std::uintptr_t& type){std::uintptr_t other=0;return r.Read(module_+a,type)&&r.Read(module_+b,other)&&cfgprobe::MatchingClassCache(type,other);}
    template<class R> static bool Typed(R& r,std::uintptr_t object,std::uintptr_t type){cfgprobe::Ancestry chain;return cfgprobe::CaptureAncestry(r,object,chain)&&chain.Contains(type)&&cfgprobe::StableAncestry(r,object,chain);}
    template<class R> static bool List(R& r,std::uintptr_t inventory,restricted::List& out,bool allowEmpty=false){
        if(!r.Read(inventory+cfgfeature::disk::InventoryItems,out.header)||out.header.count<0||out.header.count>8||out.header.capacity<out.header.count||out.header.capacity>32||(out.header.count&&!cfgprobe::ObjectPointer(out.header.data)))return false;
        for(int i=0;i<out.header.count;++i){auto& item=out.items[static_cast<unsigned>(i)];if(!r.Read(out.header.data+8*static_cast<unsigned>(i),item))return false;
            if(!item){if(allowEmpty)continue;return false;} // large grants preserve native consumed-item holes
            if(!cfgprobe::ObjectPointer(item))return false;for(int j=0;j<i;++j)if(out.items[static_cast<unsigned>(j)]==item)return false;}return true;
    }
public:
    // 1scope,2types,3name,4inventory/defaults,5pickup/capacity,6identities,
    // 7existing items,8native ABI,9revalidation,20grant,30verification.
    unsigned lastReads=0,lastStage=0;bool grantDispatched=false,hostedOnline=false;
    NativeAccess(Reader& r,Host& host,Request request,std::uintptr_t module,std::uintptr_t type):reader_(r),host_(host),request_(request),module_(module),type_(type){}
    bool Observe(Evidence& out,bool ready){
        out={};restricted::Budget<Reader> r(reader_);struct Finish {unsigned& output;restricted::Budget<Reader>& r;~Finish(){output=r.reads;}} finish{lastReads,r};
        const auto& owner=request_.owner;lastStage=1;
        if(!request_.Valid()||!host_.Current(owner)||!cfgprobe::ObjectPointer(module_)||!cfgprobe::ObjectPointer(type_)||(!hostedOnline&&!Same(r,owner.world+cfgprobe::WorldNetDriver,std::uintptr_t{})))return false; // H2.58: a hosted world legitimately has a NetDriver; every other proof below is unchanged
        lastStage=2;std::uintptr_t item=0,counselor=0,inventory=0,meta=0,actualMeta=0;const auto& inv=cfgfeature::disk::Classes[cfgfeature::disk::Inventory];
        if(!Pair(r,restricted::itemdisk::SCItemClass,restricted::itemdisk::SCItemConstructor,item)||!Pair(r,restricted::itemdisk::SCCounselorCharacterClass,restricted::itemdisk::SCCounselorCharacterConstructor,counselor)||!Pair(r,inv.reflection,inv.constructor,inventory)||!Typed(r,owner.pawn,counselor)||!cfgmenu::maps::Subclass(r,type_,item)||!r.Read(item+cfgprobe::ObjectClass,meta)||!cfgprobe::ObjectPointer(meta)||!r.Read(type_+cfgprobe::ObjectClass,actualMeta)||!cfgmenu::maps::Subclass(r,actualMeta,meta))return false;
        lastStage=3;cfgnames::Observation<restricted::Budget<Reader>> names(r,module_);cfgnames::Text name,again;if(!names.Object(type_,name)||name.View()!=ClassName(request_.slot,request_.choice))return false;
        lastStage=4;Evidence e;e.type=type_;
        if(!r.Read(type_+restricted::itemdisk::ClassDefaultObject,e.defaults)||!cfgprobe::ObjectPointer(e.defaults)||!Same(r,e.defaults+cfgprobe::ObjectClass,type_)||!r.Read(e.defaults+restricteddisk::IsLarge,e.large)||e.large!=(request_.slot==Slot::Weapon?1u:0u)||!r.Read(e.defaults+restricteddisk::IsSpecial,e.isSpecial)||e.isSpecial>1||(e.large&&e.isSpecial)||!r.Read(owner.pawn+cfgfeature::disk::SmallInventory,e.small)||!r.Read(owner.pawn+cfgfeature::disk::SpecialInventory,e.special)||e.small==e.special||!Typed(r,e.small,inventory)||!Typed(r,e.special,inventory)||!Same(r,e.small+0x20,owner.pawn)||!Same(r,e.special+0x20,owner.pawn)||!r.Read(owner.pawn+restricteddisk::EquippedItem,e.equipped)||!r.Read(owner.pawn+restricteddisk::MaxSmallItems,e.maximumSmall)||e.maximumSmall<1||e.maximumSmall>8||!List(r,e.special,e.lists[0],e.large!=0)||!List(r,e.small,e.lists[1],e.large!=0))return false;
        lastStage=5;
        if(ready&&(!Same(r,owner.pawn+restricted::itemdisk::PickingItem,std::uintptr_t{})||!Same(r,owner.pawn+restricteddisk::TempWeapon,std::uintptr_t{})||(!e.large&&(e.isSpecial?e.lists[0].header.count!=0:e.lists[1].header.count>=e.maximumSmall))))return false;
        // Donor GiveStartingItem deliberately uses native AddOrSwap. A selected
        // weapon may replace a real already-equipped weapon, but never a foreign
        // pointer or an in-progress pickup. The native game owns the swap/drop.
        if(e.equipped&&(!Typed(r,e.equipped,item)||!Same(r,e.equipped+restricteddisk::OwningCharacter,owner.pawn)||
            !Same(r,e.equipped+restricteddisk::IsLarge,std::uint8_t{1})||!Same(r,e.equipped+restricteddisk::IsSpecial,std::uint8_t{})))return false;
        lastStage=6;cfgprobe::lifetime::ArrayHeader table;const std::array<std::uintptr_t,7> objects{{owner.world,owner.pawn,type_,e.defaults,e.small,e.special,item}};
        if(!cfgprobe::lifetime::ObserveBatch(r,module_+cfgprobe::lifetime::ObjectArrayRva,objects,table,e.slots)||e.slots[0].serial!=owner.worldSerial||e.slots[1].serial!=owner.pawnSerial)return false;
        lastStage=7;for(unsigned which=0;which<2;++which)for(int i=0;i<e.lists[which].header.count;++i){const auto object=e.lists[which].items[static_cast<unsigned>(i)];if(!object)continue;if(!Typed(r,object,item)||!Same(r,object+restricteddisk::OwningCharacter,owner.pawn)||!Same(r,object+cfgfeature::disk::OuterInventory,which?e.small:e.special))return false;}
        lastStage=8;std::uintptr_t vtable=0;if(!r.Read(owner.pawn,vtable)||!cfgprobe::ObjectPointer(vtable)||!Same(r,vtable+restricteddisk::AddOrSwapSlot,module_+restricteddisk::CounselorAddOrSwap)||!Same(r,module_+restricteddisk::CounselorAddOrSwap,restricteddisk::CounselorAddOrSwapCode)||!Same(r,module_+restricted::itemdisk::GiveStartingItem,restricted::itemdisk::GiveStartingItemCode))return false;
        lastStage=9;for(unsigned i=0;i<2;++i){restricted::List fresh;if(!List(r,i?e.small:e.special,fresh,e.large!=0)||!(fresh==e.lists[i]))return false;}
        if(!names.Object(type_,again)||again.View()!=name.View()||!Same(r,type_+cfgprobe::ObjectClass,actualMeta)||!Same(r,item+cfgprobe::ObjectClass,meta)||!cfgmenu::maps::Subclass(r,type_,item)||!Same(r,type_+restricted::itemdisk::ClassDefaultObject,e.defaults)||!Same(r,e.defaults+cfgprobe::ObjectClass,type_)||!Same(r,e.defaults+restricteddisk::IsLarge,e.large)||!Same(r,e.defaults+restricteddisk::IsSpecial,e.isSpecial)||!Same(r,owner.pawn+cfgfeature::disk::SmallInventory,e.small)||!Same(r,owner.pawn+cfgfeature::disk::SpecialInventory,e.special)||!Same(r,owner.pawn+restricteddisk::EquippedItem,e.equipped)||!cfgprobe::lifetime::RevalidateBatch(r,module_+cfgprobe::lifetime::ObjectArrayRva,table,e.slots)||!host_.Current(owner))return false;
        out=e;lastStage=0;return true;
    }
    bool Grant(Evidence& before){
        grantDispatched=false;Evidence fresh;if(!Observe(fresh,true))return false;
        // Native startup can finish its stock inventory between our two slices.
        // Keep the immutable class/owner/registry lease, but take the current,
        // fully validated bag/equipped state as the native AddOrSwap baseline.
        // Comparing an old array snapshot rejected a valid shotgun BEFORE call.
        if(fresh.type!=before.type||fresh.defaults!=before.defaults||fresh.small!=before.small||
           fresh.special!=before.special||fresh.slots!=before.slots||fresh.large!=before.large||
           fresh.isSpecial!=before.isSpecial||fresh.maximumSmall!=before.maximumSmall){lastStage=9;return false;}
        before=fresh;lastStage=20;grantDispatched=true;
        const bool okay=host_.InvokeGrant(module_+restricted::itemdisk::GiveStartingItem,request_.owner.pawn,before.type);if(okay)lastStage=0;return okay;}
    bool Verify(const Evidence& before){
        Evidence after;if(!Observe(after,false)||after.type!=before.type||after.defaults!=before.defaults||after.slots!=before.slots||after.small!=before.small||after.special!=before.special||after.large!=before.large||after.isSpecial!=before.isSpecial)return false;
        lastStage=30;std::uintptr_t created=0,inventory=0;
        if(before.large){if(!after.equipped||after.equipped==before.equipped||after.lists!=before.lists)return false;created=after.equipped;}
        else {if(before.equipped!=after.equipped)return false;const unsigned at=before.isSpecial?0u:1u;if(!(before.lists[1-at]==after.lists[1-at])||after.lists[at].header.count!=before.lists[at].header.count+1)return false;
            for(int i=0;i<after.lists[at].header.count;++i){const auto object=after.lists[at].items[static_cast<unsigned>(i)];bool found=false;for(int j=0;j<before.lists[at].header.count;++j)found|=object==before.lists[at].items[static_cast<unsigned>(j)];if(!found){if(created)return false;created=object;}}inventory=at?after.small:after.special;}
        restricted::Budget<Reader> r(reader_);cfgprobe::lifetime::ArrayHeader table;cfgprobe::lifetime::SlotObservation slot,again;
        const bool verified=created&&Same(r,created+cfgprobe::ObjectClass,after.type)&&Same(r,created+restricteddisk::OwningCharacter,request_.owner.pawn)&&Same(r,created+restricteddisk::IsLarge,after.large)&&Same(r,created+restricteddisk::IsSpecial,after.isSpecial)&&(before.large||Same(r,created+cfgfeature::disk::OuterInventory,inventory))&&cfgprobe::lifetime::ReadArrayHeader(r,module_+cfgprobe::lifetime::ObjectArrayRva,table)&&cfgprobe::lifetime::ReadSlot(r,table,created,slot)&&cfgprobe::lifetime::ReadSlot(r,table,created,again)&&slot==again&&Same(r,created+restricteddisk::OwningCharacter,request_.owner.pawn)&&host_.Current(request_.owner);
        if(verified)lastStage=0;return verified;
    }
};
}
