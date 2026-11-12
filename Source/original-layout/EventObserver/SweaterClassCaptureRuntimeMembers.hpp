    bool CaptureEarnedSweaterClass(){
        // Capture AT BIND before the optional ItemList source survey. A later
        // owner callback remains a bounded retry if admission was unavailable.
        if(bundleReport.sweaterClassAttempts>=8)return false;
        ++bundleReport.sweaterClassAttempts;bundleReport.sweaterClassFailure=1;
        if(!SweaterContext()||!SweaterAlive()||!SweaterBinding(bundleSweaterAbility.object))return false;
        bundleSweaterClass={};bundleSweaterClassAttempted=false;
        bundleReport.sweaterFreshClass=0;bundleReport.sweaterClassAnchor=0;
        bundleReport.sweaterClassFailure=2;
        if(!bundleSweaterClass.Capture(r_,context.objectArray,plan.module,bundleSweaterAbility.object,bundleSweaterOwner)){
            // Existing finite earned-class attempts only. Inspect independent
            // template evidence while the source is current; never constructor
            // authority and never revisit the consumed ability from this path.
            if(bundleSweaterClass.failure==5&&r_.reads+180<cfgbundle::ReadCap){
                std::uintptr_t earnedClass=0;
                if(r_.Read(bundleSweaterAbility.object+cfgprobe::ObjectClass,earnedClass)){
                    const auto e=cfgsweater::ObserveTemplate(r_,context.objectArray,bundleSweaterSource,earnedClass);
                    bundleReport.sweaterTemplateStage=e.stage;bundleReport.sweaterTemplateSourceClass=e.sourceClass;
                    bundleReport.sweaterTemplateSourceDefault=e.sourceDefault;bundleReport.sweaterTemplateObject=e.abilityTemplate;
                    bundleReport.sweaterTemplateClass=e.abilityClass;bundleReport.sweaterTemplateSourceSerial=e.sourceSerial;
                    bundleReport.sweaterTemplateSerial=e.templateSerial;
                }
            }
            bundleReport.sweaterClassFlags=bundleSweaterClass.observedClassFlags;
            bundleReport.sweaterClassCdoFlags=bundleSweaterClass.observedDefaultFlags;
            bundleReport.sweaterClassFailure=bundleSweaterClass.failure;
            bundleReport.sweaterClassSerial=bundleSweaterClass.observedSerial;
            bundleReport.sweaterClassCdoSerial=bundleSweaterClass.observedDefaultSerial;return false;
        }
        // Anchor4/5 depends on the earned INSTANCE and cannot survive its
        // retirement. Promote only after verified match metadata retention.
        if(!RetainSweaterClassMetadata()){
            bundleSweaterClass={};bundleSweaterClassAttempted=false;
            bundleReport.sweaterFreshClass=0;bundleReport.sweaterClassAnchor=0;
            bundleReport.sweaterClassFailure=33;return false;
        }
        bundleSweaterClass.anchor=8;bundleSweaterClass.earnedSlot={};bundleSweaterClass.earnedOwner={};
        if(HostedRoutineActive())
            cfgh241sweater::memo={plan.world,plan.module,bundleSweaterClass.slot.object,bundleSweaterClass.defaults,
                bundleSweaterClass.slot,bundleSweaterClass.defaultSlot};
        bundleSweaterClassAttempted=true;bundleReport.sweaterFreshClass=bundleSweaterClass.slot.object;
        bundleReport.sweaterClassFlags=bundleSweaterClass.observedClassFlags;
        bundleReport.sweaterClassCdoFlags=bundleSweaterClass.observedDefaultFlags;
        bundleReport.sweaterClassSerial=bundleSweaterClass.slot.serial;bundleReport.sweaterClassCdoSerial=bundleSweaterClass.defaultSlot.serial;
        bundleReport.sweaterClassAnchor=bundleSweaterClass.anchor;bundleReport.sweaterClassFailure=0;return true;
    }
