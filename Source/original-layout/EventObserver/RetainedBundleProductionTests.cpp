#define CFG_SPAWN_ADAPTER
#define CFG_JASON_APPROACH
#define CFG_PRIMARY_JASON
#define CFG_GAMEPLAY_BUNDLE
#define CFG_RETAINED_PRIMARY_JASON
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "OfflineEventObserver.cpp"
#include <cstdio>
namespace {unsigned forwarded=0,trapForwarded=0;
void __fastcall Forward(void*,void*,void*){++forwarded;}
int __fastcall TrapForward(void* victim,void* trap,cfgspawn::Vec3* output){
    if(victim!=reinterpret_cast<void*>(0x20000)||trap!=reinterpret_cast<void*>(0x30000)||!output)return -1;
    ++trapForwarded;*output={1,2,3};return 1;
}}
int main(){
    originalActorDispatcher=&Forward;plan.threadId=GetCurrentThreadId();plan.pawn=0x10000;
    for(unsigned field=0;field<7;++field){auto saved=plan;
        switch(field){case 0:saved.module=1;break;case 1:saved.world=1;break;case 2:saved.mode=1;break;
        case 3:saved.controller=1;break;case 4:saved.pawn=1;break;case 5:++saved.threadId;break;
        default:++saved.objects[0].serial;break;}
        Reader identityReader;
        if(ValidateBundleLookupScope(identityReader,&saved)||identityReader.reads)return 1;
    }
    BundleAccess access;bool output=false;
    // Exhausted phase may yield, but NEVER grants native/root work. The real
    // adapter stops deferring after the finite consecutive-failure limit.
    {BundleAccess spent(0);bundleContextDeferrals=0;bundleContextDeferralsTotal=0;
     if(spent.ContextFailureStage()!=3)return 1;
     for(unsigned i=0;i<BundleContextDeferralLimit;++i)if(!spent.ContextTransient())return 1;
     if(spent.ContextTransient()||bundleContextDeferrals!=BundleContextDeferralLimit||spent.Calls()||spent.Reads())return 1;
     bundleContextDeferrals=0;const auto owner=plan.threadId;plan.threadId=0;
     if(spent.ContextFailureStage()!=1||spent.ContextTransient())return 1;plan.threadId=owner;
     bundleContextDeferrals=0;bundleContextDeferralsTotal=0;bundleContextTraces=0;}
    bundleTrapSetterOriginal=&TrapForward;cfgspawn::Vec3 foot{};
    if(BundleNativeTrappedState(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),&foot)!=1||
       trapForwarded!=1||foot.x!=1||foot.y!=2||foot.z!=3)return 1;
    bundleTrapSetterOriginal=nullptr;
    // Historical finite bootstrap cannot become a fallback retention provider
    // for arbitrary feature callbacks after the completed-tick bridge begins.
    primaryBootstrapRootsOpen=false;
    {Reader closedBootstrap;
     if(RetainPrimaryObjects(closedBootstrap,std::array<std::uintptr_t,1>{0x30000})||closedBootstrap.reads||gcRegistryReport.calls)return 1;}
    primaryBootstrapRootsOpen=true;
    // A supplied AI actor is not authority on its own: without the exact
    // completed-callback lease the real registry adapter cannot initialize.
    {GcRegistryAccess unleased(0x30000);
     if(unleased.ProvenSlotWindow())return 1;
     cfgretain::SynchronizedRootOwner unpublished;unpublished.module=0x100000;
     cfggcprobe::Report residentReport;
     if(unleased.RegisterResidentOwner(unpublished,residentReport)||residentReport.calls||
        unpublished.native.state.load()!=cfggcretain::State::Empty)return 1;
     cfgretain::ResidentRootBank<32> bank;
     cfgretain::RetainedNativeTransaction<cfgretain::ResidentRootBank<32>> transaction(bank);
     std::array<std::uintptr_t,1> input{0x30000};
     if(transaction.Prepare(unleased,0x100000,input,1)||bank.Owners()||
        transaction.EnteredNative(unleased)||transaction.CommitReturned(unleased,input)||
        transaction.ReleaseCompleted(unleased)||gcRegistryReport.calls)return 1;
     const bool began=unleased.Begin(0x40000,0x50000);unleased.End();
     if(began||gcRegistryReport.calls)return 1;}
    if(access.NativeCallAllowed()||access.NativeBool(cfghunter::disk::CanAttack,output)||
       access.Input(false)||access.Invoke(plan.pawn,0x20000,nullptr)||
       access.InvokeGrant(plan.module+cfgmenu::loadoutdisk::GiveStartingItem,plan.pawn,0x30000)||access.Calls())return 1;
    // These must not acquire a scoped capability or start the full scheduler.
    ServiceQueuedGameplay(reinterpret_cast<void*>(plan.pawn),nullptr,true,true,0);
    ServiceQueuedGameplay(reinterpret_cast<void*>(plan.pawn),nullptr,false,false,0);
    ServiceQueuedGameplay(reinterpret_cast<void*>(0x20000),nullptr,false,true,0);
    ServiceQueuedGameplay(reinterpret_cast<void*>(plan.pawn),nullptr,false,true,0);
    plan.threadId=0;ServiceQueuedGameplay(reinterpret_cast<void*>(plan.pawn),nullptr,false,true,0);
    plan.threadId=GetCurrentThreadId();
    Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);
    Reader reader;
    const bool okay=!bundleStarted&&!bundleReport.phases&&!bundleReport.calls&&!gcRegistryReport.calls&&
        !primaryRoots.Count()&&!cfgprimary::LookupSafe(reader,plan.module)&&forwarded==1;
    std::printf("Existing bundle retained production no-scope/nested/unreturned/foreign/non-tick admission: %s; own-process only\n",okay?"PASS":"FAIL");
    return okay?0:1;
}
