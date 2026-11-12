// Included inside existing BundleAccess and its focused owned-call fixture.
    bool KnifeRetainedRelease(cfgknife::Identity expected){
        bundleKnifeReleaseDiagnostics={};bundleKnifeReleaseDiagnostics.stage=1;
        struct Meter {BundleAccess& a;~Meter(){bundleKnifeReleaseDiagnostics.reads=a.r_.reads;bundleKnifeReleaseDiagnostics.calls=a.calls_;}}meter{*this};
        cfgknife::ReleaseFrame frame;
        // The release Snapshot was just identity-checked by existing Tick and
        // KnifeRelease. Publish its inputs BEFORE Before's allocating getters;
        // then Before renews the full identities around those native calls.
        bundleKnifeReleaseDiagnostics.stage=2;
        bool prepared=!bundleKnifeReleaseFrame&&bundleRootLease&&!bundleRootLease->NativeEntered()&&
            bundleKnife.release.jason.object==bundleJason.pawn.object&&bundleKnife.release.target.object&&
            bundleKnife.release.driver==expected&&
            RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{bundleKnife.release.jason.object,expected.object,bundleKnife.release.target.object});
        if(prepared){bundleKnifeReleaseDiagnostics.stage=1;
            prepared=KnifeNotifyBefore(bundleJason.pawn.object,frame.window)&&frame.window.driver==expected&&
                frame.window.jason==bundleKnife.release.jason&&frame.window.target==bundleKnife.release.target;}
        if(!prepared){
            bundleKnifeReleaseDiagnostics.failure=bundleKnifeReleaseDiagnostics.stage;
            ++bundleKnifeNotifyAdmissionRefused;
            // A refused correction never suppresses native release/cleanup.
            const bool result=KnifeSimpleCall(cfgknife::disk::Release);KnifeLifecycleSample(expected.object,2);return result;
        }
        frame.lease=bundleRootLease;frame.access=this;frame.thread=plan.threadId;frame.depth=bundleKnifeReleaseDepth+1;
        frame.returnPc=plan.module+cfgcompat::cfg35::constructBoundary::ReturnRva;
        frame.current=[](void* source,const cfgknife::ReleaseFrame& value){return static_cast<BundleAccess*>(source)->KnifeReleaseEntryCurrent(value);};
        struct Scope {
            cfgknife::ReleaseFrame* oldFrame;cfgknife::NotifyWindow* oldWindow;
            explicit Scope(cfgknife::ReleaseFrame& value):oldFrame(bundleKnifeReleaseFrame),oldWindow(bundleKnifeNotifyWindow){
                bundleKnifeReleaseFrame=&value;bundleKnifeNotifyWindow=&value.window;
            }
            ~Scope(){bundleKnifeReleaseFrame=oldFrame;bundleKnifeNotifyWindow=oldWindow;}
        } scope(frame);
        bundleKnifeReleaseDiagnostics.stage=3;
        const bool released=KnifeSimpleCall(cfgknife::disk::Release); // includes FinishBundleNative
        if(frame.faulted){bundleKnifeReleaseDiagnostics.failure=6;exception_=true;BundleRetentionFailed();return false;}
        KnifeLifecycleSample(expected.object,2);
        bundleKnifeNotifyEntries+=frame.entries;
        bundleKnifeNotifyMaxReads=(std::max)(bundleKnifeNotifyMaxReads,r_.reads);
        if(!released){bundleKnifeReleaseDiagnostics.failure=3;return false;}
        bundleKnifeReleaseDiagnostics.stage=4;
        if(!frame.Completed(bundleRootLease,GetCurrentThreadId(),bundleRootLease&&bundleRootLease->NativeEntered())){
            bundleKnifeReleaseDiagnostics.failure=4;
            ++bundleKnifeNotifyRefused;return true; // bounded existing Notify cleanup, no guessed launch
        }
        bundleKnifeReleaseDiagnostics.stage=5;
        bool corrected=false;
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()&&HostedAdoptionOwnsDriver()){
            // Release has returned and its original lease is closed. Give the
            // existing after-proof its own bounded reader while carrying call debt.
            BundleAccess completion(cfgbundle::ReadCap,calls_);completion.context=context;
            ChildAccounting accounting(*this,completion);
            corrected=completion.KnifeNotifyAfter(frame.window);
            exception_=exception_||completion.exception_;
            bundleKnifeNotifyMaxReads=(std::max)(bundleKnifeNotifyMaxReads,completion.Reads());
        }else
#endif
        corrected=KnifeNotifyAfter(frame.window);
        bundleKnifeNotifyMaxReads=(std::max)(bundleKnifeNotifyMaxReads,r_.reads);
        if(corrected){bundleKnifeReleaseDiagnostics.stage=6;++bundleKnifeNotifyCorrections;knifeFailure=0;return true;}
        if(frame.window.launched&&!frame.window.stopped){
            bundleKnifeReleaseDiagnostics.stage=7;
            if(frame.window.geometryUsed)++bundleKnifeNotifyCorrections;
            ++bundleReport.knifeNativeForwards;
            return bundleKnife.CompleteNativeNotify(frame.window.driver,GetTickCount64());
        }
        bundleKnifeReleaseDiagnostics.stage=8;bundleKnifeReleaseDiagnostics.failure=5;
        ++bundleKnifeNotifyRefused;return false;
    }
