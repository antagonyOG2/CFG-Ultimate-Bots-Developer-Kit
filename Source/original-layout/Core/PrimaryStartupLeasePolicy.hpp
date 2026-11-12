#pragma once
namespace cfgstartuplease {
inline bool CanRetain(bool bootstrapOpen,bool ownReturnedTickLease){return bootstrapOpen||ownReturnedTickLease;}
inline bool ReturnedPending(bool pending,bool bootstrapOpen,bool hosted,bool localPawn){
    return pending&&!bootstrapOpen&&!hosted&&localPawn;
}
}
