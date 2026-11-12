#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#include <cstdio>
#include <thread>
struct Reader {bool veto=false;unsigned reads=0;template<class T>bool Read(std::uintptr_t,T& value){++reads;value=veto?T{1}:T{};return true;}};
struct Context {bool live=true;unsigned checks=0;};
bool Validate(Reader&,void* opaque){auto& c=*static_cast<Context*>(opaque);++c.checks;return c.live;}
int main(){unsigned failures=0;const auto check=[&](bool value){if(!value)++failures;};
    constexpr std::uintptr_t module=0x140000000;Reader reader;Context context;
    check(!cfgprimary::LookupSafe(reader,module)&&!reader.reads&&!context.checks);
    {cfgprimary::ScopedLookupEligibility<Reader> denied(module,&context,Validate,false);
     check(!denied.Entered()&&!cfgprimary::LookupSafe(reader,module));}
    {cfgprimary::ScopedLookupEligibility<Reader> scope(module,&context,Validate,true);
     check(scope.Entered()&&cfgprimary::LookupSafe(reader,module));
     const auto reads=reader.reads;check(!cfgprimary::LookupSafe(reader,module+1)&&reader.reads==reads);
     {cfgprimary::ScopedLookupEligibility<Reader> nested(module,&context,Validate,true);
      check(!nested.Entered()&&cfgprimary::LookupSafe(reader,module));}
     check(cfgprimary::LookupSafe(reader,module));
     bool foreign=false;std::thread worker([&]{Reader other;foreign=cfgprimary::LookupSafe(other,module)||other.reads;});worker.join();check(!foreign);
     context.live=false;const auto before=reader.reads;
     check(!cfgprimary::LookupSafe(reader,module)&&reader.reads==before);
     context.live=true;reader.veto=true;check(!cfgprimary::LookupSafe(reader,module));reader.veto=false;
    }
    const auto before=reader.reads;check(!cfgprimary::LookupSafe(reader,module)&&reader.reads==before);
    check(!cfgprimary::RuntimeLookupEligibilityVerified);
    std::printf("Scoped lookup admission/module/thread/nesting/expiry/veto/root-loss/default-refusal: %u failures\n",failures);
    return failures?1:0;
}
