// Read-only B1 diagnostics. No getter, factory, native dispatch, write or target
// selection is allowed here. A failed observation never changes kill policy.
    std::uintptr_t KillDiagnosticClass(std::uintptr_t object){
        cfgprobe::lifetime::SlotObservation slot{},again{};std::uintptr_t type=0;
        return object&&cfgspawn::ObjectSlot(r_,context.objectArray,object,slot)&&
            r_.Read(object+cfgprobe::ObjectClass,type)&&cfgprobe::ObjectPointer(type)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,object,again)&&slot==again&&
            cfgobserver::Same(r_,object+cfgprobe::ObjectClass,type)?type:0;
    }
    void TraceKillHit(const cfgkilldiag::Hit& hit){
        const auto key=hit.function^(static_cast<std::uint64_t>(hit.clause)<<48)^
            (static_cast<std::uint64_t>(hit.kind)<<40)^hit.causer^hit.damageType;
        if(!bundleKillDiagnostics.Take(key))return;
        const auto instigator=KillDiagnosticClass(hit.instigator),causer=KillDiagnosticClass(hit.causer);
        auto cm=hit.range&&hit.squared<=10000000000.f?static_cast<unsigned>(std::sqrt(hit.squared)):~0u;
        unsigned mask=2;unsigned char maskOn=2;
        if(cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)&&
           r_.Read(bundleJason.pawn.object+cfgkill::disk::bMaskOn,maskOn)&&maskOn<=1)mask=maskOn?0u:1u;
        const auto candidate=hit.causer?hit.causer:bundleKill.policy.wearer;
        cfgspawn::Vec3 here{},there{};
        if(!hit.range&&candidate&&KillDiagnosticPosition(bundleJason.pawn.object,here)&&KillDiagnosticPosition(candidate,there)){
            const auto dx=here.x-there.x,dy=here.y-there.y,dz=here.z-there.z,squared=dx*dx+dy*dy+dz*dz;
            if(std::isfinite(squared)&&squared>=0&&squared<=10000000000.f)cm=static_cast<unsigned>(std::sqrt(squared));
        }
        char text[128]{};
        sprintf_s(text,"kill-hit-observe rej=%u E=%u P=%u iT=%llX cT=%llX wk=%u mask=%u cm=%u",
            hit.clause,hit.event?1u:0u,hit.packet?1u:0u,
            static_cast<unsigned long long>(instigator),static_cast<unsigned long long>(causer),
            static_cast<unsigned>(hit.kind),mask,cm);
        cfgclosedtrace::Mark(text,hit.weapon,hit.damageType);
    }
    bool KillDiagnosticPosition(std::uintptr_t pawn,cfgspawn::Vec3& position){
        std::uintptr_t root=0;cfgprobe::lifetime::SlotObservation slot{};
        return r_.Read(pawn+cfgposition::disk::ActorRootComponent,root)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,root,slot)&&
            cfgobserver::Same(r_,root+0xE0,pawn)&&
            r_.Read(root+cfgposition::disk::SceneWorldTranslation,position)&&
            std::isfinite(position.x)&&std::isfinite(position.y)&&std::isfinite(position.z)&&
            cfgjason::SameSlot(r_,context.objectArray,slot)&&
            cfgobserver::Same(r_,pawn+cfgposition::disk::ActorRootComponent,root);
    }
    void KillNativeContextDiagnostic(std::uint64_t now){
        if(!bundleKillDiagnostics.Available()||now<bundleKillDiagnostics.nextContext||
           !bundleKill.policy.usedAt||context.world!=plan.world||r_.reads>r_.cap||r_.cap-r_.reads<256)return;
        bundleKillDiagnostics.nextContext=now+500;
        if(!cfgprimary::LookupSafe(r_,plan.module)||!cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn))return;
        std::uintptr_t native=0;bool stunned=false,off=false;
        if(!r_.Read(bundleJason.pawn.object+cfgkill::disk::JasonKillObject,native)||!InStun(stunned)||!MaskOff(off))return;
        const auto index=bundleKillDiagnostics.candidate++%(bundleReport.bots+1);
        const auto pawn=index?bundlePawnSlots[index-1]:(bundleHumanTarget.object?bundleHumanTarget:plan.objects[3]);
        if(!pawn.object||!cfgjason::SameSlot(r_,context.objectArray,pawn)||
           !cfgspawn::ObjectIs(r_,pawn.object,bundleCounselorClass))return;
        cfgspawn::Vec3 here{},there{};unsigned cm=~0u;
        if(KillDiagnosticPosition(bundleJason.pawn.object,here)&&KillDiagnosticPosition(pawn.object,there)){
            const auto dx=here.x-there.x,dy=here.y-there.y,dz=here.z-there.z;
            const auto squared=dx*dx+dy*dy+dz*dz;
            if(std::isfinite(squared)&&squared>=0&&squared<=10000000000.f)cm=static_cast<unsigned>(std::sqrt(squared));
        }
        // Record distant candidates only when their state changes; include
        // the distance so absence of an eligible nearby finisher is explicit.
        std::uintptr_t weapon=0,type=0,weaponBase=0;unsigned kind=0;
        if(r_.Read(pawn.object+cfgloadout::restricteddisk::EquippedItem,weapon)&&weapon&&
           r_.Read(plan.module+cfgconvergence::disk::SCWeaponReflection,weaponBase)&&weaponBase&&
           cfgobserver::Same(r_,plan.module+cfgconvergence::disk::SCWeaponConstructor,weaponBase)&&
           cfgkill::OwnedHeldWeapon(r_,context.objectArray,pawn.object,weapon,weaponBase)){
            type=KillDiagnosticClass(weapon);cfgprobe::Ancestry chain;
            if(cfgprobe::CaptureAncestry(r_,weapon,chain)&&cfgprobe::StableAncestry(r_,weapon,chain)){
                cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text name;
                for(unsigned i=0;i<chain.count;++i){if(!names.Object(chain.classes[i],name))break;
                    if(name.View()==u"CounselorTwoHandedAxe_C")kind=1;
                    if(name.View()==u"CounselorMachete_C")kind=2;}
            }
        }
        unsigned char gender=2,hunter=2;
        namespace d=cfgcompat::cfg35::sweaterpickup;
        constexpr std::uintptr_t genderField=d::GenderCode[3]|(d::GenderCode[4]<<8)|(d::GenderCode[5]<<16)|(d::GenderCode[6]<<24);
        if(!cfgbundle::Code(r_,plan.module,d::Gender,d::GenderCode)||!r_.Read(pawn.object+genderField,gender)||gender>1)gender=2;
        if(!cfgbundle::Code(r_,plan.module,cfgkill::disk::bIsHunterSetter,cfgkill::disk::bIsHunterSetterCode)||
           !r_.Read(pawn.object+cfgkill::disk::bIsHunter,hunter)||hunter>1)hunter=2;
        if(!cfgjason::SameSlot(r_,context.objectArray,pawn)||!cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfgkill::disk::JasonKillObject,native))return;
        const auto key=native^pawn.object^type^(static_cast<std::uint64_t>(stunned)<<60)^
            (static_cast<std::uint64_t>(off)<<59)^(static_cast<std::uint64_t>(gender)<<56)^
            (static_cast<std::uint64_t>(hunter)<<54)^(static_cast<std::uint64_t>(cm<=600)<<53);
        if(!bundleKillDiagnostics.Take(key))return;
        char text[128]{};
        sprintf_s(text,"kill-native-context wt=%llX wk=%u g=%u h=%u cm=%u stun=%u mask=%u wearer=%llX",
            static_cast<unsigned long long>(type),kind,gender,hunter,cm,stunned?1u:0u,off?1u:0u,
            static_cast<unsigned long long>(bundleKill.policy.wearer));
        cfgclosedtrace::Mark(text,pawn.object,native);
    }
