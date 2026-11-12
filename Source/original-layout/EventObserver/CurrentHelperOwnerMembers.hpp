// Explicit F3 belongs to the current living counselor of the same local
// controller, including native Tommy return. Never rewrite the install Plan.
    unsigned helperContextStage_=0;
    bool HelperContext(){
        std::uintptr_t pawn=0;bool alive=false;
        helperContextStage_=1;if(!Context())return false;
        helperContextStage_=2;if(!SpectatorLocalOwner())return false;
        helperContextStage_=3;if(!Known(0,pawn))return false;
        helperContextStage_=4;if(!bundleHumanTarget.HasAssignedSerial()||pawn!=bundleHumanTarget.object)return false;
        helperContextStage_=5;if(!cfgjason::SameSlot(r_,context.objectArray,bundleHumanTarget))return false;
        helperContextStage_=6;if(!cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,pawn))return false;
        helperContextStage_=7;if(!cfgobserver::Same(r_,pawn+cfgescape::disk::Controller,plan.controller))return false;
        helperContextStage_=8;if(!Alive(pawn,alive)||!alive)return false;
        helperContextStage_=0;return true;
    }
