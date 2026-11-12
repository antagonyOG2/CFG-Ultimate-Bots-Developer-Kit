#define CFG_MULTI_MATCH_MAIN H212LifecycleMain
#include "MultiMatchLifecycleProductionTests.cpp"
#include <cstdlib>
int H212TestFault(EXCEPTION_POINTERS* e){
 const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
 std::printf("FAIL H212 lifecycle exception=0x%lX RVA=0x%llX\n",e->ExceptionRecord->ExceptionCode,
     static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(e->ExceptionRecord->ExceptionAddress)-base));
 return EXCEPTION_EXECUTE_HANDLER;
}
int main(){
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 _set_error_mode(_OUT_TO_STDERR);_set_abort_behavior(0,_WRITE_ABORT_MSG|_CALL_REPORTFAULT);
 __try{return H212LifecycleMain();}__except(H212TestFault(GetExceptionInformation())){return 1;}
}
