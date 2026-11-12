#include "PrimarySpawnObservation.hpp"
#include <cstdio>

struct Reader {
    unsigned reads=0;
    template<class T> bool Read(std::uintptr_t,T&) { ++reads; return true; }
};

int main() {
    Reader reader;
    const bool allowed=cfgprimary::LookupSafe(reader,0x140000000);
    if(allowed || reader.reads!=0 || cfgprimary::RuntimeLookupEligibilityVerified) {
        std::puts("FAIL unresolved runtime lookup eligibility must refuse before memory reads");
        return 1;
    }
    std::puts("PASS unresolved runtime lookup eligibility refuses before memory reads");
    return 0;
}
