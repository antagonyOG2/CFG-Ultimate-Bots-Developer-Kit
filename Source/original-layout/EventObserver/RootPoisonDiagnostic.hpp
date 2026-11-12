#pragma once
#include <cstdint>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
namespace cfgretain {
enum class PoisonSite:unsigned {Explicit=1,GrowRegistration,RetainCommit,CancelOutput,CompleteState,CompleteOutput,
 CloseState,CloseRelease,CompleteWindow,DeferredRelease,ScopeClose,NativeGateway};
struct PoisonEvidence {unsigned site=0,claims=0,flags=0;std::uintptr_t pc=0;};
using PoisonObserver=void(*)(void*,const PoisonEvidence&)noexcept;
}
#if defined(_MSC_VER)
#define CFG_ROOT_POISON_PC reinterpret_cast<std::uintptr_t>(_ReturnAddress())
#else
#define CFG_ROOT_POISON_PC std::uintptr_t{}
#endif
