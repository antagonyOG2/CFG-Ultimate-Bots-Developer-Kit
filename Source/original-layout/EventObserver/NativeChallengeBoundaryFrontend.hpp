// Included inside the resident frontend private namespace.
cfgchallengeboundary::Runtime frontendBoundaryRuntime;
cfgchallengeboundary::WindowsWriter frontendBoundaryWriter;
std::uint64_t frontendBoundaryNext=0;
unsigned frontendBoundaryPeakReads=0,frontendBoundaryErrors=0;
bool ArmChallengeBoundary(const cfgmenu::active::Context& source,bool enabled,std::uint64_t now){
 Reader raw;raw.cap=65536;cfgchallengeboundary::Budget<Reader> r{raw};
 const bool ok=frontendBoundaryRuntime.Arm(r,frontendBoundaryWriter,source,enabled,now);
 frontendBoundaryPeakReads=(std::max)(frontendBoundaryPeakReads,r.reads);frontendBoundaryNext=0;return ok;
}
void FrontendBoundaryAfter(bool nested,bool completed)noexcept{
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread||
    (!frontendBoundaryRuntime.Armed()&&!frontendBoundaryRuntime.CleanupPending()&&!frontendBoundaryWriter.Pending()))return;
 const auto now=GetTickCount64();if(now<frontendBoundaryNext)return;frontendBoundaryNext=now+50;
 try{
  Reader raw;raw.cap=65536;cfgchallengeboundary::Budget<Reader> r{raw};
  frontendBoundaryRuntime.Step(r,frontendBoundaryWriter,observerOwnerThread,now);
  frontendBoundaryPeakReads=(std::max)(frontendBoundaryPeakReads,r.reads);SyncChallengeReport();
 }catch(...){++frontendBoundaryErrors;frontendBoundaryRuntime.Cancel();}
}
