    // Projection is read-only navigation work, not a counselor birth. Give it
    // a fresh callback: class/tree/spawn-helper proof belongs to the later
    // allocation callback and previously exhausted this query's read budget.
    void ProjectHelperStart(){
        using namespace cfgbundle;
        bundleReport.helperPreparation=4;
        cfgspawn::Vec3 human{};std::uintptr_t function=0,owner=0,cdo=0,currentHuman=0;
        cfgprobe::lifetime::SlotObservation slot{};
        if(!HelperContext()||!Known(0,currentHuman)||!Location(currentHuman,human)||
           !FeatureFunction(cfgvehicle::disk::Project,function,owner)||
           !r_.Read(owner+cfgspawn::ClassDefaultObject,cdo)||
           !cfgobserver::Same(r_,cdo+cfgprobe::ObjectClass,owner)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,cdo,slot)){
            bundleHelpers.Refuse(HelperFailure::Projection);return;
        }
        cfgcounselorstart::Marker marker{},freshMarker{};
        const auto start=(bundleRandomCounselors.random+bundleReport.bots+bundleHelperLocationAttempts++)%bundleCounselorStarts.Count();
        if(bundleHelperLocationAttempts>bundleCounselorStarts.Count()*2||
           !HelperMarkerCurrent(start,marker)){
            bundleHelpers.Refuse(HelperFailure::Projection);return;
        }
        cfgspawn::Vec3 origin{};
        if(!Location(marker.actor,origin)||!HelperMarkerCurrent(start,freshMarker)||
           freshMarker.actor!=marker.actor){bundleHelpers.Refuse(HelperFailure::Projection);return;}
        if(!bundleRandomCounselors.LocationAllowed(origin,human)){bundleSchedule.spawnAt=GetTickCount64()+100;return;}
        cfgvehicle::Projection p;p.world=plan.world;p.point=origin;
        p.point.z=origin.z-HelperStartup::CapsuleLift+25.f;p.extent={220,220,110};
        if(!HelperContext()||!Invoke(cdo,function,&p,false)){
            bundleHelpers.Refuse(HelperFailure::Projection);return;
        }
        bundleHelperStart=start;
        if(p.result!=1||!bundleHelpers.Projected(p.point,p.projected))bundleHelpers.Next(HelperFailure::Projection);
        bundleSchedule.spawnAt=GetTickCount64()+100;
    }
