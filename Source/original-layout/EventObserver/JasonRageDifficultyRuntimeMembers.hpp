    bool RageDifficultyPhase(cfgragethreshold::Rage& s,const cfgprobe::lifetime::SlotObservation& pawn,bool extra=false){
        if(s.attempted)return false;
        std::uintptr_t type=0;float before=0;
        if(!Context()||!(extra?AdditionalLinked():Jason())||!cfgjason::SameSlot(r_,context.objectArray,pawn)||
           !NativeClass(cfgofflinesettings::disk::SCKillerCharacterClass,cfgofflinesettings::disk::SCKillerCharacterConstructor,type)||
           !cfgspawn::ObjectIs(r_,pawn.object,type)||
           !r_.Read(pawn.object+cfgofflinesettings::disk::RageUnlockTime,before)){
            s.attempted=true;cfgclosedtrace::Mark("difficulty-rage-owner-refused",pawn.object,Reads());return true;
        }
        if(!s.Prepare(pawn.object,pawn.serial,bundleConfiguration.difficulty,before)){
            s.attempted=true;cfgclosedtrace::Mark("difficulty-rage-threshold-refused",pawn.object,Reads());return true;
        }
        if(!Context()||!cfgjason::SameSlot(r_,context.objectArray,pawn)||!(extra?AdditionalLinked():Jason())||
           !cfgescape::Same(r_,pawn.object+cfgofflinesettings::disk::RageUnlockTime,before)){
            s.attempted=true;cfgclosedtrace::Mark("difficulty-rage-revalidation-refused",pawn.object,Reads());return true;
        }
        s.attempted=true; // once per positive pawn identity, including ambiguous CAS
        s.applied=(before==s.desired||CarSpeedWrite(pawn.object+cfgofflinesettings::disk::RageUnlockTime,before,s.desired))&&
            cfgescape::Same(r_,pawn.object+cfgofflinesettings::disk::RageUnlockTime,s.desired);
        cfgclosedtrace::Mark(s.applied?"difficulty-rage-threshold-applied":"difficulty-rage-threshold-unconfirmed",pawn.object,Reads());return true;
    }
