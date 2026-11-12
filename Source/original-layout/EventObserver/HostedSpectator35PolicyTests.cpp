#include "../RuntimeProbe/HostedSpectatorRuntime.hpp"
#include "../RuntimeProbe/HostedSpectator35.generated.hpp"
#include <cassert>
#include <cstdio>
using namespace cfghosted::spectator;
static_assert(cfghosted::spectator35disk::SelectorBytes==463 &&
              cfghosted::spectator35disk::HuntCanSpectateFirstFragmentBytes==101 &&
              cfghosted::spectator35disk::HuntCanSpectateEnvelopeBytes==592,
              "H216 generated disk-binding envelope contract");

struct Adapter {
    State state;
    unsigned originalCalls=0;
    Identity world{0x10,1},viewer{0x20,2},jasonOwner{0x40,4},stockOwner{0x30,3};
    PlayerStateIdentity stock{0x100,1,stockOwner,false};
    PlayerStateIdentity jason{0x200,2,jasonOwner,false};
    Proof proof{};
    Adapter(){
        proof.world=world;proof.viewer=viewer;proof.registeredJason=jason;proof.worldEpoch=7;
        proof.knownHostedPhase=proof.terminalViewer=proof.exactAdoptedJason=true;
        proof.remoteViewer=proof.currentHostedGeneration=proof.currentTerminalViewer=true;
    }
    PlayerStateIdentity Select(int direction=1,bool stockValid=true) {
        // Models the future wrapper's required ordering: original native
        // selection exactly once, then pure hosted policy over that result.
        const PlayerStateIdentity original=[&](){++originalCalls;return stock;}();
        return SelectAfterOriginal(state,proof,original,stockValid,direction);
    }
};
static void Same(PlayerStateIdentity a,const PlayerStateIdentity& b){if(!SamePlayerState(a,b)){std::fprintf(stderr,"state mismatch actual=%p/%d owner=%p/%d strong=%d expected=%p/%d owner=%p/%d strong=%d\n",reinterpret_cast<void*>(a.object),a.serial,reinterpret_cast<void*>(a.ownerActor.object),a.ownerActor.serial,a.stronglyOwnedThisCallback,reinterpret_cast<void*>(b.object),b.serial,reinterpret_cast<void*>(b.ownerActor.object),b.ownerActor.serial,b.stronglyOwnedThisCallback);assert(false);}}

int main(){
    // Donor order, both signed directions: stock -> exact Jason -> stock.
    for(int direction: {-1,1}){Adapter a;Same(a.Select(direction),a.stock);Same(a.Select(direction),a.jason);Same(a.Select(direction),a.stock);assert(a.originalCalls==3);}

    // Refusals preserve stock and cannot consume a previously armed Jason edge.
    {Adapter a;Same(a.Select(),a.stock);Proof good=a.proof;
     a.proof.livingViewer=true;Same(a.Select(),a.stock);a.proof=good;
     a.proof.knownHostedPhase=false;Same(a.Select(),a.stock);a.proof=good;
     a.proof.terminalViewer=false;Same(a.Select(),a.stock);a.proof=good;
     a.proof.remoteViewer=false;Same(a.Select(),a.stock);a.proof=good;
     a.proof.currentTerminalViewer=false;Same(a.Select(),a.stock);a.proof=good;
     a.proof.localHost=true;Same(a.Select(),a.stock);a.proof=good;
     a.proof.exactAdoptedJason=false;Same(a.Select(),a.stock);a.proof=good;
     Same(a.Select(0),a.stock);Same(a.Select(1,false),a.stock);
     a.proof.viewer.serial++;a.proof.currentTerminalViewer=false;Same(a.Select(),a.stock);a.proof=good;
     Same(a.Select(),a.jason);assert(a.originalCalls==12);}

    // Strict actor identities reject serial zero. PlayerState identities
    // accept serial zero only with same-callback strong ownership and a
    // positive owner actor. Owner/state mismatch and missing strong proof fail.
    {Adapter a;const PlayerStateIdentity stock0{0x100,0,a.stockOwner,true};
     const PlayerStateIdentity badOwner{0x100,0,Identity{0x30,0},true};
     const PlayerStateIdentity badNoProof{0x100,0,a.stockOwner,false};
     const Identity zeroActorOwner{0x30,0};assert(!zeroActorOwner);assert(!ValidPlayerState(badOwner));assert(!ValidPlayerState(badNoProof));
     assert(!CanSpectateAfterOriginal(false,a.proof,stock0));
     a.jason={0x200,0,a.jasonOwner,true};a.proof.registeredJason=a.jason;
     assert(ValidPlayerState(a.jason));assert(CanSpectateAfterOriginal(false,a.proof,a.jason));
     PlayerStateIdentity staleOwner=a.jason;staleOwner.ownerActor.serial++;
     PlayerStateIdentity differentState=a.jason;differentState.object++;
     assert(!CanSpectateAfterOriginal(false,a.proof,staleOwner));
     assert(!CanSpectateAfterOriginal(false,a.proof,differentState));
     a.proof.registeredJason=badOwner;
     assert(!CanSpectateAfterOriginal(false,a.proof,badOwner));Same(a.Select(),a.stock);
     a.proof.registeredJason.stronglyOwnedThisCallback=false;
     assert(!CanSpectateAfterOriginal(false,a.proof,a.jason));
     a.proof.registeredJason=a.jason;
     Same(a.Select(),a.stock);Same(a.Select(),a.jason);
     a.proof.registeredJason=badNoProof;Same(a.Select(),a.stock);
     a.proof.registeredJason=a.jason;Same(a.Select(),a.stock);Same(a.Select(),a.jason);}

    // Full epoch A cannot be reset by failed/stale proof. A positively current
    // new world/epoch retires all eight entries and starts with stock.
    {Adapter a;for(unsigned i=0;i<MaximumViewers;++i){a.proof.viewer={0x1000+i,static_cast<std::int32_t>(i+1)};Same(a.Select(),a.stock);}
     assert(a.state.hasGeneration&&a.state.currentWorld==a.world&&a.state.currentEpoch==7);
     a.proof.world={0x50,5};a.proof.worldEpoch=8;a.proof.viewer={0x60,6};a.proof.currentHostedGeneration=false;
     Same(a.Select(),a.stock);assert(a.state.currentWorld==a.world&&a.state.currentEpoch==7);
     a.proof.currentHostedGeneration=true;Same(a.Select(),a.stock);assert(a.state.currentWorld==a.proof.world&&a.state.currentEpoch==8);
     // A late callback from A cannot erase B after B has begun alternating.
     a.proof.world=a.world;a.proof.worldEpoch=7;a.proof.currentHostedGeneration=false;Same(a.Select(),a.stock);
     const Identity worldB{0x50,5};assert((a.state.currentWorld==worldB&&a.state.currentEpoch==8));
     a.proof.world={0x50,5};a.proof.worldEpoch=8;a.proof.currentHostedGeneration=true;Same(a.Select(),a.jason);}

    // Epoch changes in the same world reset after proof; same viewer address
    // with a new positive serial replaces its prior slot instead of exhausting capacity.
    {Adapter a;Same(a.Select(),a.stock);Same(a.Select(),a.jason);
     a.proof.worldEpoch++;Same(a.Select(),a.stock);Same(a.Select(),a.jason);
     a.proof.viewer.serial++;Same(a.Select(),a.stock);
     unsigned occupied=0;for(const auto& s:a.state.slots)if(s.initialized)++occupied;
     assert(occupied==1);Same(a.Select(),a.jason);}

    // Reentrancy and eight-slot capacity remain fail-closed.
    {Adapter a;a.state.inSelector=true;Same(a.Select(),a.stock);assert(!a.state.hasGeneration);a.state.inSelector=false;
     for(unsigned i=0;i<MaximumViewers;++i){a.proof.viewer={0x2000+i,static_cast<std::int32_t>(i+1)};Same(a.Select(),a.stock);}
     a.proof.viewer={0x3000,9};Same(a.Select(),a.stock);for(const auto& s:a.state.slots)assert(s.initialized);
     assert(a.originalCalls==MaximumViewers+2);}

    // CanSpectate preserves stock unless the exact adopted state+owner proof
    // belongs to the current remote terminal viewer; positive and strong-zero paths.
    {Adapter a;assert(!CanSpectateAfterOriginal(false,a.proof,a.stock));
     assert(CanSpectateAfterOriginal(false,a.proof,a.jason));assert(CanSpectateAfterOriginal(true,a.proof,a.stock));
     a.proof.livingViewer=true;assert(!CanSpectateAfterOriginal(false,a.proof,a.jason));
     a.proof.livingViewer=false;a.proof.localHost=true;assert(!CanSpectateAfterOriginal(false,a.proof,a.jason));
     a.proof.localHost=false;a.proof.currentTerminalViewer=false;assert(!CanSpectateAfterOriginal(false,a.proof,a.jason));
     a.proof.currentTerminalViewer=true;PlayerStateIdentity stale=a.jason;stale.serial++;
     assert(!CanSpectateAfterOriginal(false,a.proof,stale));}
    std::puts("PASS H216 hosted spectator policy: generation retirement/fencing, serial-zero owner-bound states, refusal no-consume, alternation, bounded capacity, CanSpectate exact state");
}
