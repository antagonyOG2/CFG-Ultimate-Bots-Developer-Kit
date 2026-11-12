#pragma once
#include <cstdint>
namespace cfgprimary {
// Reader-typed, thread-local capability. Never a global eligibility switch.
// A caller must independently prove its completed native callback before
// construction; every use repeats the supplied current-root/context validator.
template<class Reader>class ScopedLookupEligibility {
public:
    using Validator=bool(*)(Reader&,void*);
private:
    struct State {std::uintptr_t module=0;void* context=nullptr;Validator validate=nullptr;};
    inline static thread_local State active_{};
    bool owns_=false;
public:
    ScopedLookupEligibility(std::uintptr_t module,void* context,Validator validate,bool admitted)noexcept {
        if(admitted&&module&&context&&validate&&!active_.validate){active_={module,context,validate};owns_=true;}
    }
    ~ScopedLookupEligibility(){if(owns_)active_={};}
    ScopedLookupEligibility(const ScopedLookupEligibility&)=delete;
    ScopedLookupEligibility& operator=(const ScopedLookupEligibility&)=delete;
    bool Entered()const noexcept{return owns_;}
    static bool Current(Reader& reader,std::uintptr_t module){
        return active_.validate&&module==active_.module&&active_.validate(reader,active_.context);
    }
};
}
