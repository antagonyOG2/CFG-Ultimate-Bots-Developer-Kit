#pragma once
#include "../Core/RestrictedLoadoutPolicy.hpp"
#include "../Core/GameplayConfiguration.hpp"
#include "../RuntimeProbe/StartingLoadoutBindings.generated.hpp"
#include "../RuntimeProbe/RestrictedLoadoutBindings.generated.hpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../RuntimeProbe/OfflineSettingsBindings.generated.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include "../RuntimeProbe/NativeClassPathChoice.hpp"
#include "../Core/SoftClassPathIdentity.hpp"
namespace cfgloadout::restricted {
enum class RefusalReason : unsigned {None,ItemClass,Defaults,DefaultsClass,LargeItem,SpecialFlag,
    MissingSmallInventory,MissingSpecialInventory,AliasedInventories,SmallInventoryClass,SpecialInventoryClass,
    SmallInventoryOwner,SpecialInventoryOwner,PickupBusy,TemporaryWeaponBusy,InvalidCapacity,
    InvalidSpecialList,InvalidSmallList,InvalidItemEvidence,
    ListHeaderRead,ListCount,ListCapacity,ListData,ListEntryRead,ListNullEntry,ListInvalidEntry,ListDuplicateEntry};
inline Request FromConfiguration(const cfgbundle::Configuration& config){
    if(!cfgbundle::ValidConfiguration(config,31))return {};
    return {{config.submission,config.identity.world,config.identity.pawn,config.identity.worldSerial,config.identity.pawnSerial,
        config.matchSeconds,config.nativeCounselorSkill},static_cast<Choice>(config.restrictedInventory)};
}
namespace itemdisk=cfgmenu::loadoutdisk;
struct Context {std::uintptr_t module=0,mode=0;bool nativeOffline=false;};
struct List {
    cfgspawn::PlayerArray header{};std::array<std::uintptr_t,MaximumItems> items{};
    bool operator==(const List& b)const{return header.data==b.header.data&&header.count==b.header.count&&header.capacity==b.header.capacity&&items==b.items;}
};
struct Evidence {
    bool resolved=false;
    std::uintptr_t type=0,defaults=0,field=0,small=0,special=0;int maximumSmall=0;
    std::uint8_t large=0,isSpecial=0;cfgprimary::SoftClassBytes resource{};
    std::array<List,2> lists{};std::array<cfgprobe::lifetime::SlotObservation,7> slots{};
    std::array<std::array<cfgprobe::lifetime::SlotObservation,MaximumItems>,2> itemSlots{};
};
template<class Reader> class Budget {
    Reader& reader_;unsigned cap_=ReadLimit;
public:
    unsigned reads=0;explicit Budget(Reader& r,unsigned cap=ReadLimit):reader_(r),cap_(cap){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads>=cap_)return false;++reads;return reader_.Read(at,out);}
};
// Host.Current(owner) must prove owner thread, no GC/serialization, exact native
// local human possession, current offline world, alive/unescaped pawn and one
// committed startup. InvokeGrant(entry,pawn,class) dispatches the proved VOID
// native helper; true means normal return only, not successful item ownership.
template<class Reader,class Host> class NativeAccess {
    Reader& reader_;Host& host_;Request request_;Context context_;
    template<class H> static auto Resolve(H& host,const cfgofflinesettings::Request& owner,std::uintptr_t field,
        const cfgprimary::SoftClassBytes& resource,std::uintptr_t& type,int)
        -> decltype(host.ResolveLoadedClass(owner,field,resource,type)) {return host.ResolveLoadedClass(owner,field,resource,type);}
    template<class H> static bool Resolve(H&,const cfgofflinesettings::Request&,std::uintptr_t,
        const cfgprimary::SoftClassBytes&,std::uintptr_t&,long){return false;}
    template<class R,class T> static bool Same(R& r,std::uintptr_t at,const T& value){T actual{};return r.Read(at,actual)&&actual==value;}
    template<class R> bool Pair(R& r,std::uintptr_t a,std::uintptr_t b,std::uintptr_t& out){std::uintptr_t other=0;
        return r.Read(context_.module+a,out)&&r.Read(context_.module+b,other)&&cfgprobe::MatchingClassCache(out,other);}
    template<class R> static bool Typed(R& r,std::uintptr_t object,std::uintptr_t type){cfgprobe::Ancestry a;
        return cfgprobe::CaptureAncestry(r,object,a)&&a.Contains(type)&&cfgprobe::StableAncestry(r,object,a);}
    template<class R> bool ListRead(R& r,std::uintptr_t owner,List& out){
        // Same guards/reads as before; name the exact refusal instead of
        // treating every sweater-related special-list failure as reason16.
        const auto refuse=[&](RefusalReason reason){lastReason=reason;return false;};
        if(!r.Read(owner+cfgfeature::disk::InventoryItems,out.header))return refuse(RefusalReason::ListHeaderRead);
        if(out.header.count<0||out.header.count>static_cast<int>(MaximumItems))return refuse(RefusalReason::ListCount);
        if(out.header.capacity<out.header.count||out.header.capacity>32)return refuse(RefusalReason::ListCapacity);
        if(out.header.count&&!cfgprobe::ObjectPointer(out.header.data))return refuse(RefusalReason::ListData);
        for(int i=0;i<out.header.count;++i){auto& item=out.items[static_cast<unsigned>(i)];
            if(!r.Read(out.header.data+8*static_cast<unsigned>(i),item))return refuse(RefusalReason::ListEntryRead);
            // A native-consumed item can leave a null TArray cell. It owns
            // no UObject, but its exact cell/header must remain stable. Do
            // not compact it or count it as free native grant capacity.
            if(!item)continue;
            if(!cfgprobe::ObjectPointer(item))return refuse(RefusalReason::ListInvalidEntry);
            for(int j=0;j<i;++j)if(out.items[static_cast<unsigned>(j)]==item)return refuse(RefusalReason::ListDuplicateEntry);
        }
        return true;
    }
public:
    // Diagnostic only: no added reads, calls, or admission changes.
    // 1 owner, 2 native types, 3 soft path, 4 cached class, 5 class/defaults/
    // inventories, 6 capacity, 7 items, 8 native grant, 9 identities,
    // 10 lists, 11 item recheck, 12 final proof, 20 grant, 30 verification.
    unsigned lastReads=0,lastStage=0;bool grantDispatched=false;RefusalReason lastReason=RefusalReason::None;
    NativeAccess(Reader& reader,Host& host,Request request,Context context):reader_(reader),host_(host),request_(request),context_(context){}
    bool Observe(Evidence& out,bool ready,const Evidence* supplied=nullptr){
        out={};lastReason=RefusalReason::None;Budget<Reader> r(reader_,ReadLimit);struct Finish {unsigned& output;Budget<Reader>& r;~Finish(){output=r.reads;}} finish{lastReads,r};
        const auto& owner=request_.owner;Evidence e;
        const auto familyCache=context_.nativeOffline?cfgmenu::offlinelifecycledisk::OfflineBotsClass:cfgofflinesettings::disk::SCGameMode_SandboxClass;
        lastStage=1;if(!request_.Valid()||request_.choice==Choice::None||!host_.Current(owner)||!cfgprobe::ObjectPointer(context_.module)||!cfgprobe::ObjectPointer(context_.mode))return false;
        std::uintptr_t modeType=0,sandbox=0,counselor=0,itemType=0,inventoryType=0;
        const auto& inv=cfgfeature::disk::Classes[cfgfeature::disk::Inventory];
        lastStage=2;if(!Pair(r,itemdisk::SCGameModeClass,itemdisk::SCGameModeConstructor,modeType)||
           !Pair(r,familyCache,familyCache+8,sandbox)||
           !Pair(r,itemdisk::SCCounselorCharacterClass,itemdisk::SCCounselorCharacterConstructor,counselor)||
           !Pair(r,itemdisk::SCItemClass,itemdisk::SCItemConstructor,itemType)||!Pair(r,inv.reflection,inv.constructor,inventoryType)||
           !Typed(r,context_.mode,modeType)||
           !Typed(r,context_.mode,sandbox)||!Typed(r,owner.pawn,counselor)||
           !Same(r,owner.world+cfgprobe::WorldNetDriver,std::uintptr_t{})||!Same(r,owner.world+cfgprobe::WorldGameMode,context_.mode))return false;
        if(context_.nativeOffline&&(!Same(r,context_.mode+cfgprobe::ObjectClass,sandbox)||!Same(r,context_.mode,context_.module+cfgmenu::offlinelifecycledisk::OfflineBotsVtable)))return false;
        e.field=context_.mode+(request_.choice==Choice::PocketKnife?itemdisk::PocketKnifeClass:
            request_.choice==Choice::FirstAid?itemdisk::MedSprayClass:itemdisk::MapClass);
        lastStage=3;if(!r.Read(e.field,e.resource)||!cfgmenu::softpath::Plain(e.resource))return false;
        std::int32_t index=-1,serial=0;std::memcpy(&index,e.resource.data(),4);std::memcpy(&serial,e.resource.data()+4,4);
        cfgprobe::lifetime::ArrayHeader table;
        lastStage=4;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,context_.module+cfgprobe::lifetime::ObjectArrayRva,table))return false;
        cfgprobe::lifetime::SlotObservation cached;
        const bool cachedCurrent=index>=0&&index<table.count&&serial>0&&
            r.Read(table.data+24*static_cast<unsigned>(index),e.type)&&cfgprobe::ObjectPointer(e.type)&&
            cfgprobe::lifetime::ReadSlot(r,table,e.type,cached)&&cached.index==index&&cached.serial==serial;
        if(supplied&&supplied->resolved){
            if(supplied->field!=e.field||supplied->resource!=e.resource)return false;
            e.type=supplied->type;e.resolved=true;
        }else if(!cachedCurrent){
            if(!Resolve(host_,owner,e.field,e.resource,e.type,0)||
               !Same(r,e.field,e.resource)||!host_.Current(owner))return false;
            e.resolved=true;
        }
        if(!cfgprobe::ObjectPointer(e.type))return false;
        cfgprobe::lifetime::SlotObservation classSlot;
        if(!cfgprobe::lifetime::ReadSlot(r,table,e.type,classSlot)||
           (!e.resolved&&(classSlot.index!=index||classSlot.serial!=serial))||
           (supplied&&supplied->resolved&&!(classSlot==supplied->slots[3])))return false;
        cfgmenu::catalogues::CharacterList one;one.header={e.field,1,1};one.entries[0]=e.resource;cfgmenu::classchoice::Choice choice;
        lastStage=5;lastReason=RefusalReason::None;
        const auto refused=[&](RefusalReason reason){lastReason=reason;return false;};
        if(!cfgmenu::classchoice::MatchLoaded(r,context_.module,e.type,itemType,one,choice))return refused(RefusalReason::ItemClass);
        if(!r.Read(e.type+itemdisk::ClassDefaultObject,e.defaults)||!cfgprobe::ObjectPointer(e.defaults))return refused(RefusalReason::Defaults);
        if(!Same(r,e.defaults+cfgprobe::ObjectClass,e.type))return refused(RefusalReason::DefaultsClass);
        if(!r.Read(e.defaults+restricteddisk::IsLarge,e.large)||e.large)return refused(RefusalReason::LargeItem);
        if(!r.Read(e.defaults+restricteddisk::IsSpecial,e.isSpecial)||e.isSpecial>1)return refused(RefusalReason::SpecialFlag);
        if(!r.Read(owner.pawn+cfgfeature::disk::SmallInventory,e.small)||!cfgprobe::ObjectPointer(e.small))return refused(RefusalReason::MissingSmallInventory);
        if(!r.Read(owner.pawn+cfgfeature::disk::SpecialInventory,e.special)||!cfgprobe::ObjectPointer(e.special))return refused(RefusalReason::MissingSpecialInventory);
        if(e.small==e.special)return refused(RefusalReason::AliasedInventories);
        if(!Typed(r,e.small,inventoryType))return refused(RefusalReason::SmallInventoryClass);
        if(!Typed(r,e.special,inventoryType))return refused(RefusalReason::SpecialInventoryClass);
        if(!Same(r,e.small+0x20,owner.pawn))return refused(RefusalReason::SmallInventoryOwner);
        if(!Same(r,e.special+0x20,owner.pawn))return refused(RefusalReason::SpecialInventoryOwner);
        if(!Same(r,owner.pawn+itemdisk::PickingItem,std::uintptr_t{}))return refused(RefusalReason::PickupBusy);
        if(!Same(r,owner.pawn+restricteddisk::TempWeapon,std::uintptr_t{}))return refused(RefusalReason::TemporaryWeaponBusy);
        if(!r.Read(owner.pawn+restricteddisk::MaxSmallItems,e.maximumSmall)||e.maximumSmall<1||e.maximumSmall>static_cast<int>(MaximumItems))return refused(RefusalReason::InvalidCapacity);
        if(!ListRead(r,e.special,e.lists[0]))return false;
        if(!ListRead(r,e.small,e.lists[1]))return false;
        lastStage=6;if(ready&&(e.isSpecial?e.lists[0].header.count!=0:e.lists[1].header.count>=e.maximumSmall)){lastReason=RefusalReason::InvalidCapacity;return false;}
        lastStage=7;
        for(unsigned which=0;which<2;++which)for(int i=0;i<e.lists[which].header.count;++i){
            const auto item=e.lists[which].items[static_cast<unsigned>(i)];const auto inventory=which?e.small:e.special;
            if(!item)continue;
            if(!Typed(r,item,itemType)||!Same(r,item+restricteddisk::OwningCharacter,owner.pawn)||
               !Same(r,item+cfgfeature::disk::OuterInventory,inventory)||
               !cfgprobe::lifetime::ReadSlot(r,table,item,e.itemSlots[which][static_cast<unsigned>(i)])){
                lastReason=RefusalReason::InvalidItemEvidence;return false;}
        }
        std::uintptr_t vtable=0;
        lastStage=8;if(!r.Read(owner.pawn,vtable)||!cfgprobe::ObjectPointer(vtable)||!Same(r,vtable+restricteddisk::AddOrSwapSlot,context_.module+restricteddisk::CounselorAddOrSwap)||
           !Same(r,context_.module+restricteddisk::CounselorAddOrSwap,restricteddisk::CounselorAddOrSwapCode)||
           !Same(r,context_.module+itemdisk::GiveStartingItem,itemdisk::GiveStartingItemCode))return false;
        const std::array<std::uintptr_t,7> objects{{owner.world,owner.pawn,context_.mode,e.type,e.defaults,e.special,e.small}};
        cfgprobe::lifetime::ArrayHeader ownedTable;
        lastStage=9;if(!cfgprobe::lifetime::ObserveBatch(r,context_.module+cfgprobe::lifetime::ObjectArrayRva,objects,ownedTable,e.slots)||!(ownedTable==table)||
           e.slots[0].serial!=owner.worldSerial||e.slots[1].serial!=owner.pawnSerial||!(e.slots[3]==classSlot)||
           (supplied&&supplied->resolved&&(!(e.slots[4]==supplied->slots[4])||e.defaults!=supplied->defaults)))return false;
        lastStage=10;for(unsigned i=0;i<2;++i){List again;if(!ListRead(r,i?e.small:e.special,again)||!(again==e.lists[i]))return false;}
        lastStage=11;
        for(unsigned which=0;which<2;++which)for(int i=0;i<e.lists[which].header.count;++i){const auto& id=e.itemSlots[which][static_cast<unsigned>(i)];
            if(!e.lists[which].items[static_cast<unsigned>(i)])continue;
            cfgprobe::lifetime::SlotObservation again;
            if(!cfgprobe::lifetime::ReadSlot(r,table,id.object,again)||!(id==again)||
               !Same(r,id.object+restricteddisk::OwningCharacter,owner.pawn)||!Same(r,id.object+cfgfeature::disk::OuterInventory,which?e.small:e.special))return false;
        }
        lastStage=12;if(!Same(r,e.field,e.resource)||!Same(r,e.type+itemdisk::ClassDefaultObject,e.defaults)||
           !Same(r,e.defaults+restricteddisk::IsLarge,e.large)||!Same(r,e.defaults+restricteddisk::IsSpecial,e.isSpecial)||
           !Same(r,owner.pawn+restricteddisk::MaxSmallItems,e.maximumSmall)||!Same(r,owner.pawn+cfgfeature::disk::SmallInventory,e.small)||
           !Same(r,owner.pawn+cfgfeature::disk::SpecialInventory,e.special)||!Same(r,owner.pawn+itemdisk::PickingItem,std::uintptr_t{})||
           !Same(r,owner.pawn+restricteddisk::TempWeapon,std::uintptr_t{})||
           !Same(r,context_.module+itemdisk::SCGameModeClass,modeType)||!Same(r,context_.module+itemdisk::SCGameModeConstructor,modeType)||
           (!Same(r,context_.module+familyCache,sandbox)||!Same(r,context_.module+familyCache+8,sandbox))||
           !Same(r,context_.module+itemdisk::SCCounselorCharacterClass,counselor)||!Same(r,context_.module+itemdisk::SCCounselorCharacterConstructor,counselor)||
           !Same(r,context_.module+itemdisk::SCItemClass,itemType)||!Same(r,context_.module+itemdisk::SCItemConstructor,itemType)||
           !Same(r,context_.module+inv.reflection,inventoryType)||!Same(r,context_.module+inv.constructor,inventoryType)||
           !cfgprobe::lifetime::RevalidateBatch(r,context_.module+cfgprobe::lifetime::ObjectArrayRva,table,e.slots)||!host_.Current(owner))return false;
        if(context_.nativeOffline&&(!Same(r,context_.mode+cfgprobe::ObjectClass,sandbox)||!Same(r,context_.mode,context_.module+cfgmenu::offlinelifecycledisk::OfflineBotsVtable)))return false;
        out=e;lastStage=0;return true;
    }
    bool Grant(const Evidence& before){
        grantDispatched=false;
        Evidence now;if(!Observe(now,true,&before))return false;lastStage=20;
        if(now.type!=before.type||now.defaults!=before.defaults||now.resource!=before.resource||
            now.isSpecial!=before.isSpecial||now.small!=before.small||now.special!=before.special||now.maximumSmall!=before.maximumSmall||
            now.lists!=before.lists||now.slots!=before.slots||now.itemSlots!=before.itemSlots)return false;
        grantDispatched=true; // Distinguish reversible preflight refusal from ambiguous native entry.
        const bool okay=host_.InvokeGrant(context_.module+itemdisk::GiveStartingItem,request_.owner.pawn,before.type);
        if(okay)lastStage=0;return okay;
    }
    bool Verify(const Evidence& before,std::uintptr_t* createdOutput=nullptr){
        if(createdOutput)*createdOutput=0;
        Evidence after;if(!Observe(after,false,&before))return false;lastStage=30;
        if(after.type!=before.type||after.defaults!=before.defaults||after.resource!=before.resource||
            after.isSpecial!=before.isSpecial||after.small!=before.small||after.special!=before.special||after.slots!=before.slots)return false;
        const unsigned at=before.isSpecial?0u:1u;const auto& old=before.lists[at];const auto& next=after.lists[at];
        if(!(before.lists[1-at]==after.lists[1-at])||before.itemSlots[1-at]!=after.itemSlots[1-at]||next.header.count!=old.header.count+1)return false;
        std::uintptr_t created=0;
        // The grant must preserve empty cells too; no inferred overwrite,
        // compaction or swap is accepted as an append-only success.
        for(int i=0;i<old.header.count;++i)if(!old.items[static_cast<unsigned>(i)]&&next.items[static_cast<unsigned>(i)])return false;
        for(int i=0;i<next.header.count;++i){const auto p=next.items[static_cast<unsigned>(i)];bool found=false;
            for(int j=0;j<old.header.count;++j)if(p==old.items[static_cast<unsigned>(j)]){
                if(!(before.itemSlots[at][static_cast<unsigned>(j)]==after.itemSlots[at][static_cast<unsigned>(i)]))return false;found=true;}
            if(!found){if(created)return false;created=p;}}
        if(!created)return false;
        Budget<Reader> r(reader_,ReadLimit);cfgprobe::lifetime::ArrayHeader table;cfgprobe::lifetime::SlotObservation slot,again;
        const auto inventory=at?after.small:after.special;
        if(!Same(r,created+cfgprobe::ObjectClass,after.type)||!Same(r,created+restricteddisk::OwningCharacter,request_.owner.pawn)||
           !Same(r,created+cfgfeature::disk::OuterInventory,inventory)||!Same(r,created+restricteddisk::IsLarge,std::uint8_t{})||
           !Same(r,created+restricteddisk::IsSpecial,after.isSpecial)||
           !cfgprobe::lifetime::ReadArrayHeader(r,context_.module+cfgprobe::lifetime::ObjectArrayRva,table)||
           !cfgprobe::lifetime::ReadSlot(r,table,created,slot)||!cfgprobe::lifetime::ReadSlot(r,table,created,again)||!(again==slot)||
           !Same(r,created+restricteddisk::OwningCharacter,request_.owner.pawn)||!Same(r,created+cfgfeature::disk::OuterInventory,inventory)||
           !host_.Current(request_.owner))return false;
        if(createdOutput)*createdOutput=created;
        lastStage=0;return true;
    }
};
class Runtime {
    State state_=State::Empty;std::uint64_t next_=0,deadline_=0;unsigned attempts_=0;Evidence before_{};bool servicing_=false,prepared_=false;
public:
    bool Begin(const Request& request,std::uint64_t now){if(state_!=State::Empty||!request.Valid())return false;
        state_=request.choice==Choice::None?State::Preserved:State::Waiting;next_=now+3000;deadline_=now+18000;return true;}
    // The host owns a single bounded async resource request. Waiting does not
    // consume grant attempts; a dispatched grant can NEVER be reopened.
    bool AwaitResource(std::uint64_t now){if(servicing_||state_!=State::Waiting)return false;
        next_=now;deadline_=now+18000;return true;}
    bool FailResource(){if(servicing_||state_!=State::Waiting)return false;state_=State::Unavailable;return true;}
    template<class Access> bool Service(Access& access,std::uint64_t now){
        if(servicing_||(state_!=State::Waiting&&state_!=State::Dispatched)||now<next_)return false;
        struct Guard {bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(servicing_);
        if(now>deadline_){state_=state_==State::Waiting?State::Unavailable:State::Indeterminate;return false;}next_=now+1000;
        if(state_==State::Dispatched){state_=access.Verify(before_)?State::Verified:State::Indeterminate;return state_==State::Verified;}
        if(!prepared_){
            if(++attempts_>MaximumAttempts){state_=State::Unavailable;return false;}
            if(!access.Observe(before_,true))return false;
            prepared_=true;return true; // fresh owner/read budget for Grant's complete revalidation
        }
        state_=State::Dispatched; // Consume BEFORE the potentially reentrant native helper.
        if(!access.Grant(before_))state_=State::Indeterminate;
        return true;
    }
    State CurrentState()const{return state_;}
};
}
