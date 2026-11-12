#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Pure policy for a future hosted selector/CanSpectate wrapper. This file
// never calls the engine, writes camera state, or establishes authority.
namespace cfghosted::spectator {
struct Identity {
    std::uintptr_t object=0;
    std::int32_t serial=0;
    bool operator==(const Identity& b) const { return object==b.object && serial==b.serial; }
    bool operator!=(const Identity& b) const { return !(*this==b); }
    explicit operator bool() const { return object!=0 && serial>0; }
};
// A serial-zero PlayerState is usable only when its caller performed the
// existing same-callback cfgspectator::OwnedPlayerState graph and supplies its
// positive owner actor identity. This value is transient and never retained.
struct PlayerStateIdentity {
    std::uintptr_t object=0;
    std::int32_t serial=0;
    Identity ownerActor{};
    bool stronglyOwnedThisCallback=false;
};
inline bool ValidPlayerState(const PlayerStateIdentity& state) {
    if(!state.object || !state.ownerActor) return false;
    return state.serial>0 || (state.serial==0 && state.stronglyOwnedThisCallback);
}
inline bool SamePlayerState(const PlayerStateIdentity& a,const PlayerStateIdentity& b) {
    if(!ValidPlayerState(a)||!ValidPlayerState(b)||a.object!=b.object||
       a.serial!=b.serial||a.ownerActor!=b.ownerActor) return false;
    return a.serial!=0 || (a.stronglyOwnedThisCallback&&b.stronglyOwnedThisCallback);
}
struct Proof {
    Identity world, viewer;
    PlayerStateIdentity registeredJason;
    std::uint64_t worldEpoch=0;
    bool knownHostedPhase=false, terminalViewer=false, livingViewer=false;
    bool exactAdoptedJason=false;
    bool localHost=false;
    bool remoteViewer=false;
    // These must come from the caller's current retained-world/terminal-viewer
    // observation; stale callbacks cannot nominate an older generation.
    bool currentHostedGeneration=false,currentTerminalViewer=false;
};
constexpr std::size_t MaximumViewers=8;
struct Slot { Identity world,viewer; std::uint64_t epoch=0; bool initialized=false,nextJason=false; };
struct State {
    std::array<Slot,MaximumViewers> slots{};
    Identity currentWorld{};
    std::uint64_t currentEpoch=0;
    bool hasGeneration=false,inSelector=false;
};

inline bool Fresh(const Proof& p) {
    return p.world && p.viewer && p.worldEpoch!=0 && p.knownHostedPhase &&
        p.terminalViewer && !p.livingViewer && !p.localHost && p.remoteViewer &&
        p.currentHostedGeneration && p.currentTerminalViewer;
}
inline bool ExactJason(const Proof& p,const PlayerStateIdentity& target) {
    return Fresh(p) && p.exactAdoptedJason && ValidPlayerState(p.registeredJason) &&
        SamePlayerState(target,p.registeredJason);
}

// Called only after the wrapper has called the original native selector once.
// `stock` is passed through on every refusal. Policy alternates a verified
// stock counselor and the exactly adopted Jason PlayerState; it never returns
// a pawn. Reentrancy/full capacity cannot reserve or advance a slot.
inline PlayerStateIdentity SelectAfterOriginal(State& state,const Proof& proof,PlayerStateIdentity stock,
                                   bool stockIsValidCounselor,int direction) {
    if(state.inSelector || !Fresh(proof) || !proof.exactAdoptedJason ||
       !ValidPlayerState(proof.registeredJason) || !ValidPlayerState(stock) || !stockIsValidCounselor ||
       (direction!=-1 && direction!=1)) return stock;
    struct Guard { bool& flag; explicit Guard(bool& f):flag(f){flag=true;} ~Guard(){flag=false;} } guard(state.inSelector);

    // Advance/reclaim only after every proof and selector argument is fresh.
    // currentHostedGeneration fences callbacks from an older lease; a lower
    // epoch in the same world is independently stale and cannot clear state.
    if(state.hasGeneration && state.currentWorld==proof.world && proof.worldEpoch<state.currentEpoch) return stock;
    if(!state.hasGeneration || state.currentWorld!=proof.world || state.currentEpoch!=proof.worldEpoch) {
        state.slots={}; state.currentWorld=proof.world; state.currentEpoch=proof.worldEpoch; state.hasGeneration=true;
    }
    // The currently proved viewer may reuse an address after respawn with a
    // new positive serial. Retire only that address's older serial in this
    // already-proved generation, never a different world/epoch's callback.
    for(auto& candidate:state.slots)
        if(candidate.initialized && candidate.world==proof.world && candidate.epoch==proof.worldEpoch &&
           candidate.viewer.object==proof.viewer.object && candidate.viewer.serial!=proof.viewer.serial)
            candidate={};

    Slot* slot=nullptr; Slot* empty=nullptr;
    for(auto& candidate:state.slots) {
        if(candidate.initialized && candidate.world==proof.world && candidate.viewer==proof.viewer && candidate.epoch==proof.worldEpoch) { slot=&candidate; break; }
        if(!candidate.initialized && !empty) empty=&candidate;
    }
    if(!slot) {
        if(!empty) return stock;
        slot=empty;
        // First successful proof returns stock and arms the following Jason
        // selection. State changes only after all proof/capacity checks.
        slot->world=proof.world; slot->viewer=proof.viewer; slot->epoch=proof.worldEpoch;
        slot->initialized=true; slot->nextJason=true;
        return stock;
    }
    if(slot->nextJason) { slot->nextJason=false; return proof.registeredJason; }
    slot->nextJason=true; return stock;
}

inline bool CanSpectateAfterOriginal(bool stockResult,const Proof& proof,const PlayerStateIdentity& target) {
    return stockResult || ExactJason(proof,target);
}
} // namespace cfghosted::spectator
