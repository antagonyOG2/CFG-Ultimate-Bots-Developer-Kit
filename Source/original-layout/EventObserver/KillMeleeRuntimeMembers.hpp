// Included inside BundleAccess. Donor DriveCounselorFinalKill's native
// bCanAttackToInteract on the ACTUAL SCInteractComponent, not a guessed field
// on SCContextKillComponent (which is not an interactable subclass in CFG).
    bool ArmFinishingStrike(const cfgkill::Context& death,std::uintptr_t pawn){
        std::uintptr_t interactClass=0;std::uint8_t before=2;
        if(death.active||death.actor!=bundleKill.policy.context||pawn!=bundleKill.policy.finisher||
           death.actor!=bundleKillActorSlot.object||death.interactable!=bundleKillInteractSlot.object||
           death.killComponent!=bundleKillComponentSlot.object||!KnownLiving(pawn)||
           !cfgprimary::LookupSafe(r_,plan.module)||
           !cfgbundle::Code(r_,plan.module,cfgkill::disk::bCanAttackToInteractSetter,cfgkill::disk::bCanAttackToInteractSetterCode)||
           !NativeClass(cfgkill::disk::SCInteractComponentReflection,cfgkill::disk::SCInteractComponentConstructor,interactClass)||
           !cfgspawn::ObjectIs(r_,death.interactable,interactClass)||
           !cfgjason::SameSlot(r_,context.objectArray,bundleKillActorSlot)||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,bundleKillInteractSlot)||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,bundleKillComponentSlot)||
           !cfgobserver::Same(r_,death.interactable+0x20,death.actor)||!cfgobserver::Same(r_,death.interactable+0xE0,death.actor)||
           !cfgobserver::Same(r_,death.killComponent+0x20,death.actor)||!cfgobserver::Same(r_,death.killComponent+0xE0,death.actor)||
           !cfgobserver::Same(r_,death.actor+cfgkill::disk::InteractComponent,death.interactable)||
           !cfgobserver::Same(r_,death.actor+cfgkill::disk::ContextKillComponent,death.killComponent)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfgkill::disk::JasonKillObject,death.actor)||
           !r_.Read(death.interactable+cfgkill::disk::bCanAttackToInteract,before)||before>1||
           !cfgescape::Same(r_,death.killComponent+cfgkill::disk::bIsKillActive,std::uint8_t{}))return false;
        if(r_.reads>r_.cap||r_.cap-r_.reads<4)return false;
        if(!before&&ExchangeByte(death.interactable+cfgkill::disk::bCanAttackToInteract,0,1)!=cfgofflinesettings::Write::Changed)return false;
        return cfgescape::Same(r_,death.interactable+cfgkill::disk::bCanAttackToInteract,std::uint8_t{1});
        // Native melee/contact/weapon/range eligibility still decides the kill.
    }
