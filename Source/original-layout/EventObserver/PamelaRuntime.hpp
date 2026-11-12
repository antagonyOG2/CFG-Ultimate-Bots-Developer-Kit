// Included inside Runtime. Pamela-only; legacy two-strike path is unchanged.
    template<class H>Step TickPamela(H& h,std::uint64_t now){
        if(policy.stage==Stage::AwaitContext&&!policy.stanceAttempted){
            const auto ready=detail::PrepareStance(h,0);
            if(ready==Preparation::Pending)return Step::Waiting;
            if(ready!=Preparation::Ready){failure=Failure::StancePreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        }
        Identity id{};Context death{};
        if(!h.Scope(id)||!(id==policy.identity)){failure=Failure::Scope;policy.stage=Stage::Uncertain;return Step::Uncertain;}
        if(!h.DeathContext(death)){failure=Failure::Context;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        if(!death.actor){
            if(policy.stage!=Stage::AwaitContext||policy.stanceAttempted)return Step::Waiting;
            bool off=false;if(!h.MaskOff(off)||!off)return Step::Waiting;
            // The earned damage command creates the native stance; no finisher
            // is pinned before the actual death context exists.
            const auto mark=[&](){return policy.RequestStance(now);};
            if(!detail::DispatchStance(h,mark,0)||!policy.stanceAttempted){
                failure=Failure::StanceDispatch;policy.stage=Stage::Uncertain;return Step::Uncertain;}
            ++stanceDispatches;return Step::StanceDispatched;
        }
        if(!policy.ObserveContext(death.actor,now,death.active)){failure=Failure::ContextChanged;return Step::Uncertain;}
        if(death.active){
            policy.interactionAttempted=true;policy.pamelaDispatchPending=false;
            return Step::NativeActive; // actual native ownership consumes the opportunity
        }
        if(policy.interactionAttempted)return Step::ContextReady;
        if(policy.pamelaDispatchPending){
            if(!policy.pamelaDispatchReturned){policy.stage=Stage::Uncertain;return Step::Uncertain;}
            policy.pamelaDispatchPending=false; // freshly inactive, completed call: not a started cinematic
        }
        const auto contextReady=detail::PreparePamelaContext(h,death,0);
        if(contextReady==Preparation::Pending)return Step::Waiting;
        if(contextReady!=Preparation::Ready){failure=Failure::InteractionPreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        if(policy.pamelaNativeContextOnly)return Step::ContextReady; // donor: player's next native interaction owns completion
        std::uintptr_t pawn=0,weapon=0;Weapon kind=Weapon::Other;
        if(!detail::PamelaFinisher(h,pawn,weapon,kind,0)){
            policy.finisher=policy.weapon=0;return Step::Waiting;
        }
        policy.finisher=pawn;policy.weapon=weapon; // current selection, never a pre-context pin
        if(now<policy.pamelaNextDispatch)return Step::Waiting;
        const auto ready=detail::PrepareInteraction(h,death.actor,pawn,0);
        if(ready==Preparation::Pending)return Step::Waiting;
        if(ready!=Preparation::Ready){failure=Failure::InteractionPreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        const auto mark=[&](){return policy.RequestPamelaInteraction(now);};
        if(!detail::FinalInteraction(h,death,pawn,mark,0)){
            failure=Failure::InteractionDispatch;
            if(!policy.pamelaDispatchPending){++interactionDeferrals;return Step::Waiting;}
            // A native exception/unreturned dispatch is ambiguous. Never retry.
            policy.stage=Stage::Uncertain;++unavailable;return Step::Unavailable;
        }
        if(!policy.pamelaDispatchPending){failure=Failure::InteractionMarker;policy.stage=Stage::Uncertain;return Step::Uncertain;}
        policy.pamelaDispatchReturned=true;failure=Failure::None;++interactionDispatches;
        return Step::InteractionDispatched; // stays open unless a later native-active proof consumes it
    }
