void* __fastcall BundleNativeSummaryText(void* row,void* output){
    cfgsummarypresentation::Scope scope(bundleSummaryPresentationRow,reinterpret_cast<std::uintptr_t>(row));return bundleSummaryTextOriginal?bundleSummaryTextOriginal(row,output):output;
}
unsigned char __fastcall BundleNativeSummaryPanel(void* row){
    cfgsummarypresentation::Scope scope(bundleSummaryPresentationRow,reinterpret_cast<std::uintptr_t>(row));return bundleSummaryPanelOriginal?bundleSummaryPanelOriginal(row):1;
}
unsigned char __fastcall BundleNativeSummaryCharacter(void* row){
    cfgsummarypresentation::Scope scope(bundleSummaryPresentationRow,reinterpret_cast<std::uintptr_t>(row));return bundleSummaryCharacterOriginal?bundleSummaryCharacterOriginal(row):1;
}
bool __fastcall BundleNativeSummaryStateReady(void* state,bool includePending){
    const auto caller=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
    const bool native=bundleSummaryStateReadyOriginal&&bundleSummaryStateReadyOriginal(state,includePending);
    if(!cfgsummarypresentation::MayOverride(native,includePending,BundleSummaryStarted(),GetCurrentThreadId()==plan.threadId,
       cfgsummarypresentation::disk::Caller(plan.module,caller),bundleSummaryPresentationRow))return native;
    BundleAccess access;bool allowed=false;
    __try{allowed=access.SummaryPresentedState(bundleSummaryPresentationRow,reinterpret_cast<std::uintptr_t>(state));}
    __except(EXCEPTION_EXECUTE_HANDLER){allowed=false;}
    return allowed; // only this nested native UI call;no PlayerState byte changes
}
