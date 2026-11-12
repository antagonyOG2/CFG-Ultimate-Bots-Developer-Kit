#include "SweaterOwnerRuntimeMembers.hpp"
#include "SweaterUseObservationMembers.hpp"
#include "SweaterClassRetentionMembers.hpp"
    bool CaptureSweaterCleanup(std::uintptr_t pawn,cfgsweater::Retention& token,
                              cfgprobe::lifetime::SlotObservation& identity){
        token={};identity={};std::uintptr_t ability=0,owner=0;std::uint8_t used=2;
        if(pawn!=bundleSweaterOwner.object||!BundleGameplayStarted()||!bundleSchedule.active||
           (bundleSweaterPolicy.stage!=cfgsweater::Stage::Owned&&bundleSweaterPolicy.stage!=cfgsweater::Stage::Waiting)||
           !bundleSweaterCleanupEnabled||!bundleSweaterCleanupOriginal||
           bundleSweaterCleanupTarget!=plan.module+cfgsweater::retaindisk::Cleanup||
           !SweaterContext()||!SweaterAlive()||
           !r_.Read(pawn+cfgfeature::disk::AbilityField,ability)||ability!=bundleSweaterAbility.object||
           !SweaterBinding(ability)||!r_.Read(ability+cfgfeature::disk::AbilityOwner,owner)||owner!=pawn||
           !r_.Read(ability+cfgsweater::disk::Used,used)||used!=1||
           !cfgspawn::ObjectSlot(r_,context.objectArray,ability,identity))return false;
        token={pawn,ability,true,true};return true;
    }
    bool RestoreSweaterCleanup(const cfgsweater::Retention& token,const cfgprobe::lifetime::SlotObservation& identity){
        struct Host {
            BundleAccess& a;const cfgprobe::lifetime::SlotObservation& identity;bool mutated=false;
            bool Fresh(const cfgsweater::Retention& t){
                bundleReport.sweaterRetentionFailure=2;cfgprobe::lifetime::SlotObservation fresh;std::uint8_t used=2;
                return BundleGameplayStarted()&&bundleSchedule.active&&
                    (bundleSweaterPolicy.stage==cfgsweater::Stage::Owned||bundleSweaterPolicy.stage==cfgsweater::Stage::Waiting)&&
                    t.pawn==bundleSweaterOwner.object&&t.ability==bundleSweaterAbility.object&&a.SweaterContext()&&a.SweaterAlive()&&
                    cfgspawn::ObjectSlot(a.r_,a.context.objectArray,t.ability,fresh)&&fresh==identity&&
                    a.TypedFeature(t.ability,cfgfeature::disk::Ability,false)&&
                    a.r_.Read(t.ability+cfgsweater::disk::Used,used)&&used==1;
            }
            bool Cleared(const cfgsweater::Retention& t){
                bundleReport.sweaterRetentionFailure=3;
                return cfgobserver::Same(a.r_,t.pawn+cfgfeature::disk::AbilityField,std::uintptr_t{})&&
                    cfgobserver::Same(a.r_,t.ability+cfgfeature::disk::AbilityOwner,std::uintptr_t{})&&
                    cfgescape::Same(a.r_,t.pawn+cfgfeature::disk::HasSweater,std::uint8_t{});
            }
            bool Reserve(){bundleReport.sweaterRetentionFailure=4;return a.r_.reads+16<a.r_.cap&&cfgprimary::LookupSafe(a.r_,plan.module);}
            bool Store(std::uintptr_t address,std::uintptr_t value,unsigned failure){
                bundleReport.sweaterRetentionFailure=failure;std::uintptr_t current=0;SIZE_T written=0;
                if(!a.r_.Read(address,current)||current)return false;
                mutated=true;
                if(!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&value,sizeof(value),&written)||written!=sizeof(value))return false;
                ++bundleReport.sweaterWrites;return cfgobserver::Same(a.r_,address,value);
            }
            bool Owner(const cfgsweater::Retention& t){return Store(t.ability+cfgfeature::disk::AbilityOwner,t.pawn,5);}
            bool Ability(const cfgsweater::Retention& t){return Store(t.pawn+cfgfeature::disk::AbilityField,t.ability,6);}
            bool Readback(const cfgsweater::Retention& t){bundleReport.sweaterRetentionFailure=7;return
                cfgobserver::Same(a.r_,t.pawn+cfgfeature::disk::AbilityField,t.ability)&&
                cfgobserver::Same(a.r_,t.ability+cfgfeature::disk::AbilityOwner,t.pawn)&&
                cfgescape::Same(a.r_,t.ability+cfgsweater::disk::Used,std::uint8_t{1})&&
                cfgescape::Same(a.r_,t.pawn+cfgfeature::disk::HasSweater,std::uint8_t{});}
        } host{*this,identity};
        if(!cfgsweater::RetainConsumed(host,token)){if(host.mutated){bundleSweaterPolicy.Refuse();
                cfgbundle::RecordSweaterFirstRefusal(bundleReport,204,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,GetTickCount64(),bundleReport.sweaterFreshFailure);}
            return false;}
        bundleReport.sweaterRetentionFailure=0;return true;
    }
#include "SweaterPickupObservationMembers.hpp"
    bool RetireSweaterPickup(){
        auto& s=bundleSweaterPickup;if(!s.clone||!s.installed)return true;
        if(!SweaterPickupSource()||!cfgobserver::Same(r_,s.source.object,s.clone+8)||!cfgprimary::LookupSafe(r_,plan.module))return false;
        const auto prior=InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(s.source.object),
            reinterpret_cast<void*>(plan.module+cfgsweater::pickupdisk::Vtable),reinterpret_cast<void*>(s.clone+8));
        if(prior!=reinterpret_cast<void*>(s.clone+8))return false;
        s.installed=0;s.stage=3;return true; // allocated clone remains pinned, even after restoration
    }
    bool SweaterPickupPhase(std::uint64_t now){
        auto& s=bundleSweaterPickup;if(!cfgsweater::pickupdisk::RuntimeVerified||s.stage>=2)return false;
        if(!s.deadline){s.deadline=now+90000;s.world=plan.world;s.module=plan.module;}
        if(now>=s.deadline||s.world!=plan.world||s.module!=plan.module){s.stage=3;++s.refused;s.lastFailure=1;return true;}
        if(s.stage==1){
            if(cfgsweater::PickupCloneAllocations>=cfgsweater::MaximumPickupClones){s.stage=3;++s.refused;s.lastFailure=5;return true;}
            s.lastFailure=3;
            if(!SweaterPickupSource()||!cfgobserver::Same(r_,s.source.object,plan.module+cfgsweater::pickupdisk::Vtable)||
                !cfgbundle::Code(r_,plan.module,cfgsweater::pickupdisk::Original,cfgsweater::pickupdisk::OriginalCode)||
                !cfgbundle::Code(r_,plan.module,cfgsweater::pickupdisk::Base,cfgsweater::pickupdisk::BaseCode))return true;
            cfgsweater::PickupTable table;
            if(!cfgsweater::BuildPickupTable(r_,plan.module,reinterpret_cast<std::uintptr_t>(&BundleSweaterCanInteract),table)||
                !cfgprimary::LookupSafe(r_,plan.module))return true;
            s.lastFailure=4;
            auto* clone=static_cast<std::uintptr_t*>(VirtualAlloc(nullptr,sizeof(table),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
            if(!clone){s.stage=3;++s.refused;return true;}
            std::memcpy(clone,table.data(),sizeof(table));DWORD old=0;
            if(!VirtualProtect(clone,sizeof(table),PAGE_READONLY,&old)){VirtualFree(clone,0,MEM_RELEASE);s.stage=3;++s.refused;return true;}
            const auto prior=InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(s.source.object),clone+1,reinterpret_cast<void*>(plan.module+cfgsweater::pickupdisk::Vtable));
            if(prior!=reinterpret_cast<void*>(plan.module+cfgsweater::pickupdisk::Vtable)){VirtualFree(clone,0,MEM_RELEASE);s.stage=3;++s.refused;return true;}
            // Like the context-kill clone, this immutable table is process-pinned;
            // never free an installed table while native engine callbacks can use it.
            ++cfgsweater::PickupCloneAllocations;s.clone=reinterpret_cast<std::uintptr_t>(clone);s.installed=1;s.stage=2;s.lastFailure=0;return true;
        }
        s.lastFailure=2;struct List{std::uintptr_t data=0;std::int32_t count=0,capacity=0;};List list;std::uintptr_t native=0;
        if(!SweaterContext()||!TypedFeature(spawnAssets.gameState,cfgfeature::disk::GameState,false)||
            !r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,list)||list.count<0||list.count>256||list.capacity<list.count||list.capacity>65536||
            (list.count&&!cfgprobe::ObjectPointer(list.data))||
            !NativeClass(cfgfeature::disk::Classes[cfgfeature::disk::Sweater].reflection,cfgfeature::disk::Classes[cfgfeature::disk::Sweater].constructor,native))return true;
        if(s.cursor>=static_cast<unsigned>(list.count))s.cursor=0;
        for(unsigned n=0;n<4&&s.cursor<static_cast<unsigned>(list.count);++n,++s.cursor){std::uintptr_t item=0,level=0;++s.scanned;
            if(!r_.Read(list.data+8*s.cursor,item))return true;
            if(!item||!cfgspawn::ObjectIs(r_,item,native))continue;
            if(!cfgspawn::ObjectSlot(r_,context.objectArray,item,s.source)||!r_.Read(item+0x20,level)||!cfgspawn::ObjectSlot(r_,context.objectArray,level,s.level))return true;
            s.index=s.cursor;s.stage=1;return true;
        }
        return true;
    }
    bool SweaterFunction(bool visual,std::uintptr_t& function,bool allowCold=true){
        if(!allowCold){std::uintptr_t cached=0;
            if(!r_.Read(plan.module+(visual?cfgsweater::disk::OnRep_WearingSweater.cache:cfgsweater::disk::HasBeenUsed.cache),cached)||!cached)return false;}
        std::uintptr_t owner=0;
        const bool prepared=visual?FeatureFunction(cfgsweater::disk::OnRep_WearingSweater,function,owner):
            FeatureFunction(cfgsweater::disk::HasBeenUsed,function,owner);
        if(!prepared)return false;
        if(visual)return cfgobserver::Same(r_,function+0xB0,plan.module+cfgsweater::disk::OnRep_WearingSweaterExecutor)&&
            cfgbundle::Code(r_,plan.module,cfgsweater::disk::OnRep_WearingSweaterExecutor,cfgsweater::disk::OnRep_WearingSweaterExecutorCode)&&
            cfgbundle::Code(r_,plan.module,cfgsweater::disk::Visual,cfgsweater::disk::VisualCode);
        return cfgobserver::Same(r_,function+0xB0,plan.module+cfgsweater::disk::HasBeenUsedExecutor)&&
            cfgbundle::Code(r_,plan.module,cfgsweater::disk::HasBeenUsedExecutor,cfgsweater::disk::HasBeenUsedExecutorCode);
    }
    bool SweaterBinding(std::uintptr_t ability){
        bundleSweaterPolicy.lastFailure=101;
        if(!ability)return false;
        std::uintptr_t nativeType=0;const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
        bundleSweaterPolicy.lastFailure=111;
        if(!NativeClass(binding.reflection,binding.constructor,nativeType))return false;
        cfgprobe::lifetime::SlotObservation typedSlot{},typedAgain{};
        bundleSweaterPolicy.lastFailure=112;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,ability,typedSlot))return false;
        bundleSweaterPolicy.lastFailure=113;
        if(!cfgspawn::ObjectIs(r_,ability,nativeType))return false;
        bundleSweaterPolicy.lastFailure=114;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,ability,typedAgain)||!(typedAgain==typedSlot))return false;
        cfgprobe::lifetime::SlotObservation fresh{};std::uintptr_t owner=0,current=0,sourceAbility=0;
        bundleSweaterPolicy.lastFailure=102;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,ability,fresh)||
            !r_.Read(ability+cfgfeature::disk::AbilityOwner,owner)||(owner&&owner!=bundleSweaterOwner.object)||
            !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,current))return false;
        if(!(fresh==bundleSweaterAbility)){
            const auto old=bundleSweaterAbility;
            cfgprobe::lifetime::SlotObservation again{};
            if(!cfgsweater::AssignedOwnedSlot(old.object,old.index,old.serial,fresh.object,fresh.index,fresh.serial,
                   current==ability&&owner==bundleSweaterOwner.object&&bundleSweaterOwner.HasAssignedSerial())||
               !cfgjason::SameSlot(r_,context.objectArray,bundleSweaterOwner)||
               !cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,ability)||
               !cfgobserver::Same(r_,ability+cfgfeature::disk::AbilityOwner,bundleSweaterOwner.object)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,ability,again)||!(again==fresh))return false;
            bundleSweaterAbility=fresh; // native weak assignment, not a new actor/instance or GC pin
            if(bundleSweaterClass.anchor==5&&bundleSweaterClass.earnedSlot==old&&bundleSweaterClass.earnedOwner==bundleSweaterOwner)
                bundleSweaterClass.earnedSlot=fresh;
        }
        if(current==ability){if(owner==bundleSweaterOwner.object){bundleSweaterPolicy.lastFailure=0;return true;}return false;}
        // A cleared pawn pointer may only reuse a still positively identified
        // native sweater's retained ability, never a stale bare pointer.
        if(current)return false;
        if(bundleSweaterSource.object&&TypedFeature(bundleSweaterSource.object,cfgfeature::disk::Sweater,false)&&
            r_.Read(bundleSweaterSource.object+cfgfeature::disk::SweaterAbility,sourceAbility)&&sourceAbility==ability){
            std::uintptr_t wearer=0;
            if(!r_.Read(bundleSweaterSource.object+cfgloadout::restricteddisk::OwningCharacter,wearer)||(wearer&&wearer!=bundleSweaterOwner.object))return false;
            if(bundleSweaterSource.HasAssignedSerial()&&cfgjason::SameSlot(r_,context.objectArray,bundleSweaterSource)){bundleSweaterPolicy.lastFailure=0;return true;}
            if(cfgsweater::StrongSource(r_,context.objectArray,plan.world,spawnAssets.gameState,bundleSweaterOwner.object,
                bundleSweaterSource,bundleSweaterAbility,bundleSweaterSourceIndex)){bundleSweaterPolicy.lastFailure=0;return true;}
        }
        // An assigned serial is a real weak lifetime check on this native game
        // thread (LookupSafe excludes GC). If GC removed it, ObjectSlot above
        // refuses. Never retain/rearm serial0 from a lost strong source link.
        bundleSweaterPolicy.lastFailure=103;
        const bool retained=bundleSweaterAbility.HasAssignedSerial()&&owner==bundleSweaterOwner.object;
        const bool detached=cfgsweater::DetachedEarnedAbility(bundleSweaterAbility.HasAssignedSerial(),
            bundleSweaterPolicy.lastUse!=0,current,owner);
        if((retained||detached)&&cfgobserver::Same(r_,ability+cfgfeature::disk::AbilityOwner,owner)&&
            cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,current)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,ability,fresh)&&fresh==bundleSweaterAbility){bundleSweaterPolicy.lastFailure=0;return true;}
        return false;
    }
#include "SweaterBindRuntimeMembers.hpp"
    bool DiscoverSweaterSettings(){
        const bool captured=bundleSweaterClass.CaptureSettings(r_,context.objectArray,plan.module,plan.world,bundleSweaterSettingsCursor);
        bundleReport.sweaterClassFailure=bundleSweaterClass.failure;
        if(!captured)return false;
        bundleSweaterClassAttempted=true;bundleReport.sweaterFreshClass=bundleSweaterClass.slot.object;
        bundleReport.sweaterClassAnchor=bundleSweaterClass.anchor;bundleReport.sweaterClassSerial=bundleSweaterClass.slot.serial;
        bundleReport.sweaterClassCdoSerial=bundleSweaterClass.defaultSlot.serial;return true;
    }
    bool DiscoverSweater(){
        // Tommy earns a NEW ability after his own mask pickup. A prior
        // current class may still be valid although its source garment is no
        // longer in WorldItems. Never borrow the old wearer's ability.
        if(cfgsweater::RetainCurrentClassForDiscovery(r_,context.objectArray,plan.module,
            bundleSweaterClass,bundleSweaterClassAttempted,bundleSweaterCursor,SweaterClassMetadataCurrent())){
            bundleReport.sweaterFreshClass=bundleSweaterClass.slot.object;
            bundleReport.sweaterClassAnchor=bundleSweaterClass.anchor;
            return true; // constructor remains a separately guarded phase
        }
        struct List{std::uintptr_t data=0;std::int32_t count=0,capacity=0;
            bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
        List list{},fresh{};
        if(!TypedFeature(spawnAssets.gameState,cfgfeature::disk::GameState,false)||
            !r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,list)||list.count<0||list.count>256||
            list.capacity<list.count||list.capacity>65536||(list.count&&!cfgprobe::ObjectPointer(list.data)))return false;
        if(bundleSweaterCursor>=static_cast<unsigned>(list.count))bundleSweaterCursor=0;
        std::uintptr_t native=0;
        if(!NativeClass(cfgfeature::disk::Classes[cfgfeature::disk::Sweater].reflection,
            cfgfeature::disk::Classes[cfgfeature::disk::Sweater].constructor,native))return false;
        // Only during mask-triggered cold discovery: four known ItemList cells
        // per phase, never GObjects or level scans and never per-frame discovery.
        for(unsigned step=0;step<4&&bundleSweaterCursor<static_cast<unsigned>(list.count);++step,++bundleSweaterCursor){
            std::uintptr_t item=0,ability=0,owner=0,wearer=0;
            const auto cell=list.data+8*bundleSweaterCursor;
            if(!r_.Read(cell,item))return false;if(!item||!cfgspawn::ObjectIs(r_,item,native))continue;
            if(!TypedFeature(item,cfgfeature::disk::Sweater,false)||!r_.Read(item+cfgfeature::disk::SweaterAbility,ability)||
                !r_.Read(item+cfgloadout::restricteddisk::OwningCharacter,wearer))return false;
            if(!bundleSweaterClassAttempted){
                bool living=false;
                if(wearer&&wearer!=bundleSweaterOwner.object&&(!Alive(wearer,living)||living))continue;
                cfgsweater::ClassLease::WorldSource declared;declared.world=plan.world;declared.state=spawnAssets.gameState;declared.index=bundleSweaterCursor;
                cfgsweater::ClassLease candidate;
                if(cfgspawn::ObjectSlot(r_,context.objectArray,item,declared.source)&&
                   candidate.CaptureDeclared(r_,context.objectArray,plan.module,declared)){
                    bundleSweaterClass=candidate;bundleSweaterClassAttempted=true;bundleReport.sweaterFreshClass=candidate.slot.object;
                    bundleReport.sweaterClassAnchor=7;bundleReport.sweaterClassSerial=candidate.slot.serial;
                    bundleReport.sweaterClassCdoSerial=candidate.defaultSlot.serial;bundleReport.sweaterClassFailure=0;
                    return true; // class declaration only; no borrowed/dead instance
                }
            }
            if(bundleSweaterPolicy.repairingOwned||!ability)continue; // repair is declaration-only; never borrow an instance
            if(!TypedFeature(ability,cfgfeature::disk::Ability,false)||
                !r_.Read(ability+cfgfeature::disk::AbilityOwner,owner))return false;
            ++bundleReport.sweaterDiscoveryCandidates;bundleReport.sweaterDiscoveryResult=0;
            bundleReport.sweaterDiscoverySource=item;bundleReport.sweaterDiscoveryAbility=ability;
            bundleReport.sweaterDiscoveryWearer=wearer;bundleReport.sweaterDiscoveryOwner=owner;
            const bool foreign=(wearer&&wearer!=bundleSweaterOwner.object)||(owner&&owner!=bundleSweaterOwner.object);
            if(foreign||!bundleSweaterClassAttempted){
                // Donor does not grant a second Pamela power while its other
                // carrier is alive. A native terminal carrier permits only
                // independent class acquisition here, never ability theft.
                bool living=false;
                if((wearer&&wearer!=bundleSweaterOwner.object&&(!Alive(wearer,living)||living))||
                   (owner&&owner!=bundleSweaterOwner.object&&(!Alive(owner,living)||living))){bundleReport.sweaterDiscoveryResult=1;continue;}
                cfgsweater::ClassLease::WorldSource source;source.world=plan.world;source.state=spawnAssets.gameState;source.index=bundleSweaterCursor;
                if(!cfgspawn::ObjectSlot(r_,context.objectArray,item,source.source)||
                   !cfgspawn::ObjectSlot(r_,context.objectArray,ability,source.ability))return false;
                cfgsweater::ClassLease replacement;
                bundleReport.sweaterDiscoveryResult=2;
                if(replacement.Capture(r_,context.objectArray,plan.module,ability,{},&source)){
                    bundleSweaterClass=replacement;bundleSweaterClassAttempted=true;bundleReport.sweaterFreshClass=replacement.slot.object;
                    bundleReport.sweaterClassAnchor=replacement.anchor;bundleReport.sweaterClassSerial=replacement.slot.serial;
                    bundleReport.sweaterClassCdoSerial=replacement.defaultSlot.serial;bundleReport.sweaterClassFailure=0;
                    bundleReport.sweaterDiscoveryResult=3;
                    // This is independent CURRENT world class evidence, including
                    // a sweater whose native cleanup cleared owner/wearer fields.
                    // Do not reuse its possibly consumed ability; construction
                    // remains in the separate Fresh phase with repeated proof.
                    return true;
                }
                if(foreign)continue;
            }
            if(owner&&owner!=bundleSweaterOwner.object)continue; // Never borrow another wearer's ability.
            if(!cfgobserver::Same(r_,cell,item)||!r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,fresh)||!(fresh==list)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,item,bundleSweaterSource)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,ability,bundleSweaterAbility))return false;
            bundleSweaterSourceIndex=bundleSweaterCursor;
            if(!cfgsweater::StrongSource(r_,context.objectArray,plan.world,spawnAssets.gameState,bundleSweaterOwner.object,
                bundleSweaterSource,bundleSweaterAbility,bundleSweaterSourceIndex))return false;
            bundleReport.sweaterBound=1;bundleReport.sweaterDiscoveryResult=4;return true;
        }
        const bool same=r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,fresh)&&fresh==list;
        if(same&&!bundleSweaterClassAttempted&&bundleSweaterCursor>=static_cast<unsigned>(list.count))
            return DiscoverSweaterSettings();
        return same;
    }
    template<class T> bool SweaterStore(std::uintptr_t address,const T& expected,const T& value){
        T current{};SIZE_T written=0;
        bundleSweaterPolicy.lastFailure=24;
        if(GetCurrentThreadId()!=plan.threadId||!r_.Read(address,current)||current!=expected)return false;
        bundleSweaterPolicy.BeginPublish(); // a failed/partial native write is never replayed
        if(!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&value,sizeof(value),&written)||written!=sizeof(value))return false;
        ++bundleReport.sweaterWrites;
        return r_.Read(address,current)&&current==value;
    }
    bool PublishSweater(bool rearm,std::uint64_t now){
        const auto ability=bundleSweaterAbility.object;
        std::uintptr_t current=0,owner=0,function=0;unsigned char has=0,wearing=0,unlimited=0,used=0;
        bool alive=false;
        bundleSweaterPolicy.lastFailure=10;
        if(!Known(0,current)||!Alive(current,alive)||!alive||!SweaterBinding(ability))return false;
        bundleSweaterPolicy.lastFailure=12;
        if(!cfgbundle::Code(r_,plan.module,cfgsweater::disk::HasSweaterSetter,cfgsweater::disk::HasSweaterSetterCode)||
            !cfgbundle::Code(r_,plan.module,cfgsweater::disk::WearingSweaterSetter,cfgsweater::disk::WearingSweaterSetterCode)||
            !cfgbundle::Code(r_,plan.module,cfgsweater::disk::UnlimitedSweaterSetter,cfgsweater::disk::UnlimitedSweaterSetterCode)||
            !cfgbundle::Code(r_,plan.module,cfgsweater::disk::Consume,cfgsweater::disk::ConsumeCode))return false;
        bundleSweaterPolicy.lastFailure=13;
        const bool clearUsed=rearm&&!bundleSweaterFreshPending;
        if(!SweaterFunction(false,function,false)||!Invoke(ability,function,&used,false)||used>1||used!=(clearUsed?1:0))return false;
        bundleSweaterPolicy.lastFailure=14;
        if(!SweaterFunction(true,function,false))return false;
        bundleSweaterPolicy.lastFailure=15;
        if(!r_.Read(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,current)||
            (current&&current!=ability)||!r_.Read(ability+cfgfeature::disk::AbilityOwner,owner)||(owner&&owner!=bundleSweaterOwner.object)||
            !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::HasSweater,has)||has>1||
            !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::WearingSweater,wearing)||wearing>1||
            !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::UnlimitedSweater,unlimited)||unlimited>1)return false;
        // All identity, API and expected-value gates complete before writes.
        // Publish once; partial/ambiguous write or call disables this feature,
        // never a hot-loop repair. Native visual callback preserves wheel/input.
        struct Writer{
            BundleAccess& host;std::uintptr_t ability,function;
            bool Fresh(const cfgsweater::Publication&){return host.SweaterContext()&&host.SweaterBinding(ability);}
            bool Owner(std::uintptr_t from,std::uintptr_t to){return host.SweaterStore(ability+cfgfeature::disk::AbilityOwner,from,to);}
            bool Ability(std::uintptr_t from,std::uintptr_t to){return host.SweaterStore(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,from,to);}
            bool Has(std::uint8_t from,std::uint8_t to){return host.SweaterStore(bundleSweaterOwner.object+cfgfeature::disk::HasSweater,from,to);}
            bool Wearing(std::uint8_t from,std::uint8_t to){return host.SweaterStore(bundleSweaterOwner.object+cfgfeature::disk::WearingSweater,from,to);}
            bool Unlimited(std::uint8_t from,std::uint8_t to){return host.SweaterStore(bundleSweaterOwner.object+cfgfeature::disk::UnlimitedSweater,from,to);}
            bool Used(std::uint8_t from,std::uint8_t to){return host.SweaterStore(ability+cfgsweater::disk::Used,from,to);}
            bool Visual(){return host.Invoke(bundleSweaterOwner.object,function,nullptr);}
        } writer{*this,ability,function};
        // Reserve bounded reads for Fresh's context/source revalidation and
        // all six expected-value writes/readbacks BEFORE the first mutation.
        bundleSweaterPolicy.lastFailure=16;
        if(!cfgsweater::PublicationBudget(r_.reads,cfgbundle::ReadCap,calls_,cfgbundle::CallCap))return false;
        bundleSweaterPolicy.lastFailure=17;
        if(!cfgsweater::Publish(writer,{bundleSweaterOwner.object,ability,current,owner,has,wearing,unlimited,used,clearUsed}))return false;
        bundleSweaterFreshPending=false;bundleSweaterPolicy.QueueHud(now,rearm);bundleReport.sweaterBound=1;bundleSweaterPolicy.lastFailure=0;
        return true;
    }
#include "SweaterHudNativeRuntimeMembers.hpp"
    bool PublishSweaterHud(){
        bundleSweaterPolicy.lastFailure=20;
        if(!SweaterContext()||!SweaterBinding(bundleSweaterAbility.object))return false;
        bundleSweaterPolicy.lastFailure=21;cfgsweater::HudEvidence hud;
        if(!cfgsweater::ObserveHud(r_,context.objectArray,plan.module,plan.objects[2],plan.module+cfgspawn::GenericDispatcherRva,hud))return false;
        bundleSweaterPolicy.lastFailure=22;
#if CFG_H248_TOMMY_SLOT
        // Report-only sub-clause of failure 22 (same predicates, same order, same short-circuit).
        const auto hudStep=[this](unsigned c,bool ok){
            if(!ok&&c<bundleReport.h248HudClause.size()&&bundleReport.h248HudClause[c]<UINT32_MAX)++bundleReport.h248HudClause[c];
            return ok;};
#else
        const auto hudStep=[](unsigned,bool ok){return ok;};
#endif
        if(!hudStep(1,cfgbundle::Code(r_,plan.module,cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix))||
            !hudStep(2,cfgobserver::Same(r_,plan.controller+0x370,bundleSweaterOwner.object))||
            !hudStep(3,cfgobserver::Same(r_,bundleSweaterOwner.object+0x3A0,plan.controller))||
            !hudStep(4,cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,bundleSweaterAbility.object))||
            !hudStep(5,cfgobserver::Same(r_,bundleSweaterAbility.object+cfgfeature::disk::AbilityOwner,bundleSweaterOwner.object))||
            !hudStep(6,cfgescape::Same(r_,bundleSweaterAbility.object+cfgsweater::disk::Used,std::uint8_t{}))||
            !hudStep(7,cfgprimary::LookupSafe(r_,plan.module))||!hudStep(8,calls_<cfgbundle::CallCap))return false;
        bundleSweaterPolicy.lastFailure=23;
        if(!SweaterHudNative(hud))return false;
        // The native HUD event can reenter gameplay and replace/retire the
        // ability. A returned call is not a recharge proof: count only after
        // the exact live owner's ability is still assigned and usable.
        bundleSweaterPolicy.lastFailure=24;
        std::uintptr_t liveAbility=0,liveOwner=0;std::uint8_t liveUsed=255,liveHas=0;
        if(!SweaterContext()||!SweaterBinding(bundleSweaterAbility.object)||
           !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,liveAbility)||liveAbility!=bundleSweaterAbility.object||
           !r_.Read(bundleSweaterAbility.object+cfgfeature::disk::AbilityOwner,liveOwner)||liveOwner!=bundleSweaterOwner.object||
           !r_.Read(bundleSweaterAbility.object+cfgsweater::disk::Used,liveUsed)||liveUsed!=0||
           !r_.Read(bundleSweaterOwner.object+cfgfeature::disk::HasSweater,liveHas)||liveHas!=1||
           !cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,liveAbility)||
           !cfgobserver::Same(r_,bundleSweaterAbility.object+cfgfeature::disk::AbilityOwner,liveOwner)||
           !cfgescape::Same(r_,bundleSweaterAbility.object+cfgsweater::disk::Used,liveUsed)||
           !cfgescape::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::HasSweater,liveHas)){
            bundleSweaterPolicy.Refuse();return false;
        }
        ++bundleSweaterPolicy.hudReturns;
        if(bundleSweaterPolicy.pendingRearm)++bundleReport.sweaterRecharges;else ++bundleReport.sweaterGrants;
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(bundleSweaterPolicy.pendingRearm)RecordBundleSweaterEvent(false,GetTickCount64());
#endif
        bundleSweaterPolicy.ConfirmPublish();bundleSweaterPolicy.lastFailure=0;return true;
    }
#include "SweaterKillArmRuntimeMembers.hpp"
    bool ConstructFreshSweater(std::uintptr_t& fresh)noexcept{
        __try{fresh=reinterpret_cast<std::uintptr_t(__fastcall*)(void*,void*)>(plan.module+cfgsweater::freshdisk::SpawnObject)(
            reinterpret_cast<void*>(bundleSweaterClass.slot.object),reinterpret_cast<void*>(bundleSweaterOwner.object));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool FreshSweaterWorldAfterAllocation(){
        return GetCurrentThreadId()==plan.threadId&&cfgprimary::LookupSafe(r_,plan.module)&&
            GameplayWorld(context)&&SpectatorLocalOwner();
    }
#include "SweaterFreshRuntimeMembers.hpp"
#include "SweaterClassCaptureRuntimeMembers.hpp"
#include "SweaterPhaseRuntimeMembers.hpp"
