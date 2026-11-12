// Included inside BundleAccess; DifficultyPolicy.hpp and DifficultyBindings.generated.hpp
// must be included at namespace scope. Called by the native TakeDamage detour
// with a NEW BundleAccess, never with another gameplay phase's spent reader.
    bool DifficultyActorWorld(std::uintptr_t actor){
        std::uintptr_t level=0,type=0;
        cfgprobe::lifetime::SlotObservation slot{};
        return r_.Read(actor+0x20,level)&&level&&
            NativeClass(cfgkill::disk::LevelReflection,cfgkill::disk::LevelConstructor,type)&&
            cfgspawn::ObjectIs(r_,level,type)&&cfgspawn::ObjectSlot(r_,context.objectArray,level,slot)&&
            cfgobserver::Same(r_,level+cfgkill::disk::OwningWorld,plan.world);
    }
    float DifficultyDamage(unsigned difficulty,std::uintptr_t victim,float incoming,
        std::uintptr_t instigator,std::uintptr_t causer){
        using cfgdifficulty::Source;
        if(difficulty>2||difficulty==1||!cfgdifficulty::EligibleAmount(incoming)||!victim||!causer||
           !bundleStarted||!bundleSchedule.active||GetCurrentThreadId()!=plan.threadId||
           bundleKill.policy.Exclusive(GetTickCount64())||!Context()||!Jason()||
           victim==bundleJason.pawn.object||!KnownLiving(victim)||
           (instigator&&instigator!=bundleJason.killerController.object))return incoming;
        cfgprobe::lifetime::SlotObservation victimSlot{},causerSlot{};
        unsigned char authority=0;bool killerAlive=false;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,victim,victimSlot)||!victimSlot.HasAssignedSerial()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,causer,causerSlot)||!causerSlot.HasAssignedSerial()||
           !r_.Read(bundleJason.pawn.object+cfgknife::disk::Role,authority)||authority!=3||
           !Alive(bundleJason.pawn.object,killerAlive)||!killerAlive||
           !DifficultyActorWorld(victim)||!DifficultyActorWorld(causer))return incoming;
        Source source=Source::None;std::uintptr_t native=0;
        if(causer==bundleJason.pawn.object)source=Source::Jason;
        else if(HeldWeapon(bundleJason.pawn.object,causer))source=Source::Weapon;
        else if(NativeClass(cfgknife::disk::ThrowableReflection,cfgknife::disk::ThrowableConstructor,native)&&
                cfgspawn::ObjectIs(r_,causer,native)&&
                cfgobserver::Same(r_,causer+cfgknife::disk::ThrownBy,bundleJason.pawn.object))source=Source::Projectile;
        // The projectile may outlive Jason.Driver; use native ThrownBy and its
        // own current registry identity, never the transient held-knife link.
        if(source==Source::None||!cfgjason::SameSlot(r_,context.objectArray,victimSlot)||
           !cfgjason::SameSlot(r_,context.objectArray,causerSlot)||
           !cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)||
           !cfgjason::SameSlot(r_,context.objectArray,bundleJason.killerController)||
           !cfgprimary::LookupSafe(r_,plan.module))return incoming;
        return cfgdifficulty::Scale(incoming,difficulty,source);
    }
