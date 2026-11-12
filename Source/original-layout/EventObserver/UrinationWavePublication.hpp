#pragma once
#include "UrinationSharedSlotObservation.hpp"
namespace cfgurination {
// One publication of the donor's retained montage. Writer.Current() must prove
// the SAME live registry window and real gameplay tick; this object grants no
// write permission. Partial writes remain recorded, never guessed or replayed.
class WavePublication {
    bool attempted_=false,verified_=false,hadVerified_=false,shared_=false,restoreAttempted_=false;unsigned fields_=0;
public:
    bool Attempted()const noexcept{return attempted_;}
    bool Verified()const noexcept{return verified_;}
    unsigned Fields()const noexcept{return fields_;}
    bool Shared()const noexcept{return shared_;}
    bool PlaybackEligible()const noexcept{return hadVerified_;}
    bool Suspended()const noexcept{return restoreAttempted_;}
    template<class R,class W>bool Publish(R& r,W& writer,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
        const std::array<std::uintptr_t,3>& roots,std::uintptr_t loaded){
        if(attempted_||!loaded||!writer.Current())return false;
        AssetProof proof{};SharedSlot slot{};
        if(!ObserveAssets(r,objects,module,roots,loaded,proof))return false;
        const bool female=roots[2]&&ObserveSharedSlot(r,module,loaded,proof.skeleton.object,slot);
        if(!writer.Current()||!ObserveAssets(r,objects,module,roots,loaded,proof))return false;
        attempted_=true; // consume before the first mutation
        if(!writer.Compare(proof.defaults.object+disk::MaleMontage,roots[1],loaded))return false;
        fields_=1;
        if(!writer.Current()||!ObserveAssets(r,objects,module,roots,loaded,proof,fields_))return false;
        if(female){
            std::uint64_t original=0,cinematic=0;std::memcpy(&original,&slot.original,8);std::memcpy(&cinematic,&slot.cinematic,8);
            if(writer.Compare(slot.address,original,cinematic)){
                shared_=true;
                if(writer.Current()&&writer.Compare(proof.defaults.object+disk::FemaleMontage,roots[2],loaded))fields_|=2;
            }
        }
        verified_=writer.Current()&&ObserveAssets(r,objects,module,roots,loaded,proof,fields_);
        hadVerified_=verified_;
        return verified_;
    }
    template<class R,class W>bool Restore(R& r,W& writer,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
        const std::array<std::uintptr_t,3>& roots,std::uintptr_t loaded){
        if(!hadVerified_||restoreAttempted_||!writer.Current())return false;
        AssetProof proof{};
        if(!ObserveAssets(r,objects,module,roots,loaded,proof,fields_)||!writer.Current())return false;
        restoreAttempted_=true;verified_=false;
        // Both original fields were proved before either mutation. Restore
        // only the values we published. Keep the resident montage/shared slot
        // intact for already-playing bots, exactly like the donor fallback.
        if(fields_&2){
            if(!writer.Compare(proof.defaults.object+disk::FemaleMontage,loaded,roots[2]))return false;fields_&=~2u;
        }
        if(fields_&1){
            if(!writer.Current()||!writer.Compare(proof.defaults.object+disk::MaleMontage,loaded,roots[1]))return false;fields_&=~1u;
        }
        return writer.Current()&&ObserveAssets(r,objects,module,roots,loaded,proof,fields_);
    }
};
}
