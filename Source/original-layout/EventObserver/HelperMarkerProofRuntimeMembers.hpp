    // Compatibility observation only: the unchanged authored-start validator
    // repeats full local-owner/world checks before and after its metadata reads.
    // Give EACH fresh observation its own bounded reader; native calls and the
    // placement policy remain on the original access/call budget and root lease.
    unsigned helperMarkerProofReads_=0;
    bool HelperMarkerCurrent(unsigned choice,cfgcounselorstart::Marker& marker){
        BundleAccess proof;
        const bool current=bundleCounselorStarts.Current(proof.r_,proof,choice,marker);
        helperMarkerProofReads_=proof.Reads();
        return current&&proof.Calls()==0;
    }
