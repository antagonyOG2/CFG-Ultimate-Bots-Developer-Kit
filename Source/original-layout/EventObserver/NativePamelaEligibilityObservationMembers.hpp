// Engine-origin predicate forwarding only; no factory/dispatch/tick authority.
    bool NativePamelaEligibility(std::uintptr_t component,std::uintptr_t pawn,cfgkill::EligibilityOriginal original,
        const cfgkill::EligibilityVector* location,const cfgkill::EligibilityVector* direction,int& result){
        const auto now=GetTickCount64();const auto& current=bundleKill.policy;
        if(!current.PamelaAccepted()||!current.Exclusive(now))return false;
        // This is forwarding the ENGINE'S own predicate, not initiating a
        // native dispatch. Like SweaterPickupAllowed, use the read-only native
        // callback observation proof; no completed-tick lease is fabricated.
        // No discovery factory/allocation or new native call occurs here.
        cfgkill::Identity id{plan.world,bundleJason.pawn.object,bundleJason.pawn.serial};
        cfgkill::Context death{};std::uintptr_t weapon=0;
        if(!SummaryObservationScope()||!Jason()||!(id==current.identity)||!DeathContext(death)||death.actor!=current.context||death.active||
           death.killComponent!=component||!PamelaHeld(pawn,weapon)||
           !EligibilityCurrent(component,pawn,true))return false;
        auto policy=current;policy.finisher=pawn;policy.weapon=weapon;
        const cfgkill::EligibilityTicket ticket{id,death.actor,component,pawn,true,true,true,true,true};
        return PamelaEligibilityCall(policy,component,pawn,original,location,direction,result,&ticket);
    }
