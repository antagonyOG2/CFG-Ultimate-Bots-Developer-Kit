#pragma once
// Portable sequencing and fail-open-on-OPTIONAL-HOOK setup. This does not
// authorize a native gameplay call or any bypass of a hook's policy gate.
namespace cfgj29escape {
template<class PrepareEscape>bool InstallAfterNative(bool nativePairReady,PrepareEscape&& escape){
 if(!nativePairReady)return false;
 (void)escape(); // Escape Five<> owns rollback/quarantine; native pair untouched.
 return true;
}
template<class Cancel>bool AllowStart(bool armed,unsigned& failures,Cancel&& cancel){
 if(armed)return true;
 ++failures;cancel();return true;
}
}
