#pragma once
#include <cstdint>
#ifndef CFG_J21_CONVERGENCE
#define CFG_J21_CONVERGENCE 0
#endif
#ifndef CFG_J21_EMOTE_MIRROR
#define CFG_J21_EMOTE_MIRROR 1
#endif
#ifndef CFG_J21_LEAN_CONVERGENCE
#define CFG_J21_LEAN_CONVERGENCE 1
#endif
namespace cfgj21 {
// Reviewed264/266 fallback: no claim that mocked native calls prove live cost.
// Keep both independent implementations available for separately profiled builds.
constexpr bool ConvergenceEnabled=CFG_J21_CONVERGENCE!=0,EmotesEnabled=CFG_J21_EMOTE_MIRROR!=0;
constexpr bool LeanEnabled=CFG_J21_LEAN_CONVERGENCE!=0;
constexpr bool ConvergenceLaneEnabled(bool configured,bool hosted){return configured||hosted;}
inline bool EarnedSweater(bool earned,bool observed,bool has){return earned||(observed&&has);}
struct Backoff {
    unsigned failures=0;std::uint64_t next=0;
    void Finish(std::uint64_t now,bool success){failures=success?0u:(failures<4?failures+1:4);
        next=now+(success?1000u:(250u<<(failures-1)));}
};
inline bool ObjectiveTurn(bool active,bool release,bool exclusive,bool urgent,bool previous,bool other,bool due){
    return active&&!release&&!exclusive&&!urgent&&!previous&&!other&&due;
}
enum Stage:unsigned {Hints,Held,Hunt,Convergence,Emotes,PE,Native,Roster,Cabin,Objectives,Summary,Knife,Traps,LegacyConvergence,LegacyEmotes,LegacyUrination,Lean,
    NativeSessionEntry,NativeLookupScope,NativeObjectiveScope,NativeRootScope,NativeRetirement,NativeRetain,NativeDonorService,StageCount};
struct Cost {
    std::uint64_t count=0,total=0,maximum=0,over4ms=0,overFrame=0;
    void Observe(std::uint64_t us){++count;total+=us;if(us>maximum)maximum=us;if(us>4000)++over4ms;if(us>16667)++overFrame;}
};
}
