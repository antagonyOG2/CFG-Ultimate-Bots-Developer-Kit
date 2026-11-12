#include "J24FailedStartWorld.hpp"
cfgj24::FailedStartWorldCounters j24WorldCounters;
void J24RecordWorldForRetirement(SpawnAccess& access)noexcept{
 if(!CFG_J24_FAILED_START_RETIRE||
    (bundlePreviousWorld.world.object&&!bundlePreviousWorld.checked))return;
 Reader reader;reader.cap=96;auto objects=access.ObjectArray();std::uintptr_t packageType=0;
 if(!cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,objects)||
    !reader.Read(plan.module+cfgcompat::cfg35::worldMap::NativePackageCache,packageType))packageType=0;
 const auto result=cfgj24::RecordRootedWorld(reader,objects,plan.world,packageType,bundlePreviousWorld);
 j24WorldCounters.Recorded(result,bundlePreviousWorld,primaryRoots.Count());
}
void J24WorldInitialized()noexcept{j24WorldCounters.Initialized(bundlePreviousWorld);}
// Called once for the preceding match, only after a CURRENT successful native
// menu epoch. Diagnostic reads/trace only: no engine dispatch, roots or repair.
void ObservePreviousWorldAtMenu(const cfgmenu::active::Context& menu)noexcept{
    auto& previous=bundlePreviousWorld;
    if(previous.checked||!previous.world.object||!previous.level.object||
       GetCurrentThreadId()!=observerOwnerThread||menu.thread!=observerOwnerThread||menu.world==previous.world.object)return;
    Reader read;read.cap=8192;cfgprobe::lifetime::ArrayHeader objects{};cfgprobe::lifetime::SlotObservation fresh{};
    if(!cfgprimary::LookupVetoClearForObservation(read,menu.module)||
       !cfgprobe::lifetime::ReadArrayHeader(read,menu.module+cfgprobe::lifetime::ObjectArrayRva,objects))return;
    static std::uintptr_t observedWorld=0;
    if(observedWorld==previous.world.object)return;observedWorld=previous.world.object;
    std::uint32_t flags=0;const bool live=cfgworlddiag::Alive(read,objects,previous.world,previous.worldClass,flags);
    cfgclosedtrace::Mark(live?"previous-world-alive":"previous-world-observation-missing",previous.world.object,previous.level.object);
    if(!live)return;
    unsigned marks=0;
    const auto emitHolder=[&](const char* stage,std::uintptr_t object,std::uint64_t id){
        if(marks>=16)return;
        char tag[120]{};strcpy_s(tag,stage);
        std::uintptr_t type=0;cfgnames::Text label;
        if(read.Read(object+cfgprobe::ObjectClass,type)){
            cfgnames::Observation<Reader> names(read,menu.module);
            if(names.Object(type,label)){
                char name[49]{};unsigned n=0;
                for(const auto c:label.View()){if(n==48)break;name[n++]=c>=32&&c<127?static_cast<char>(c):'?';}
                strcat_s(tag,":");strcat_s(tag,name);
            }
        }
        ++marks;cfgclosedtrace::Mark(tag,object,id);
    };
    const auto bank=[&](auto& roots,unsigned id){
        // All banks have at most32 resident owners of32 opaque slots each.
        std::array<std::uintptr_t,1024> values{};unsigned count=0;
        if(!roots.ObserveSlots(values,count)){cfgclosedtrace::Mark("bank-observation-refused",id,0);return;}
        for(unsigned i=0;i<count&&marks<16&&read.reads+64<read.cap;++i)
            if(values[i]&&cfgworlddiag::Reaches(read,objects,values[i],previous))
                emitHolder("bank-holds-old-world",values[i],(static_cast<std::uint64_t>(id)<<32)|i);
    };
    bank(startupRootBank,1);bank(frontendRootBank,2);bank(bundleRootBank,3);
#ifdef CFG35_URINATION_NATIVE_PORT
    bank(urinationAssetBank,4);
#endif
    const auto holder=[&](std::uintptr_t at,unsigned id){
        std::uintptr_t object=0;if(marks<16&&read.reads+64<read.cap&&read.Read(at,object)&&object&&
            cfgworlddiag::Reaches(read,objects,object,previous))emitHolder("persistent-holds-old-world",object,id);
    };
    // Current menu infrastructure was just validated by FrontendEpochAfter.
    // Addresses are observed fields, not native getter calls or old-world work.
    holder(menu.viewport+cfgprobe::ViewportWorld,1);
    std::uintptr_t players=0,player=0,controller=0;int count=0,capacity=0;
    if(read.Read(menu.gameInstance+cfgprobe::GameInstanceLocalPlayers,players)&&cfgprobe::ObjectPointer(players)&&
       read.Read(menu.gameInstance+cfgprobe::GameInstanceLocalPlayers+8,count)&&count==1&&
       read.Read(menu.gameInstance+cfgprobe::GameInstanceLocalPlayers+12,capacity)&&capacity>=1&&capacity<=4&&
       read.Read(players,player)&&player==menu.player&&read.Read(player+cfgprobe::PlayerController,controller)&&controller){
        if(cfgworlddiag::Reaches(read,objects,controller,previous))emitHolder("persistent-holds-old-world",controller,2);
        holder(controller+cfgprobe::ControllerPawn,3);
        holder(controller+cfgprobe::ControllerAcknowledgedPawn,4);
        holder(controller+cfgprobe::ControllerSpectatorPawn,5);
        holder(controller+cfgprobe::ControllerCameraManager,6);
        std::uintptr_t camera=0;
        if(cfgworlddiag::Camera(read,objects,menu.module,controller,camera)){
            namespace d=cfgcompat::cfg35::cameradiagnostic;
            holder(camera+d::ViewTarget,7);holder(camera+d::ViewTargetState,8);
            holder(camera+d::PendingTarget,9);holder(camera+d::PendingState,10);
        }
    }
    cfgclosedtrace::Mark("previous-world-diagnostic-complete",previous.world.object,
        (static_cast<std::uint64_t>(marks)<<32)|read.reads);
}
