#include "../Core/ComprehensiveCapture.hpp"
#include <cstdio>
#include <stdexcept>
#include <cstdint>
struct CaptureHost {
    unsigned calls=0,fail=0;bool throwFailure=false;std::uintptr_t process=0x1234;
    int Section(unsigned section){++calls;if(section!=calls||process!=0x1234)return 99;
        if(section==fail){if(throwFailure)throw std::runtime_error("diagnostic model failure");return 7;}return 0;}
};
int main(){unsigned failures=0;
    for(unsigned fail=0;fail<4;++fail)for(unsigned observer=0;observer<2;++observer)for(unsigned throwing=0;throwing<2;++throwing){
        CaptureHost host;host.fail=fail;host.throwFailure=throwing!=0;const auto results=cfgcapture::Collect(host,observer?4:0);
        if(host.calls!=3||results.exits[0]!=(observer?4:0)||results.passed!=4-(fail?1u:0u)-observer)++failures;
        for(unsigned section=1;section<4;++section)if((results.exits[section]!=0)!=(section==fail))++failures;
    }
    std::printf("Comprehensive same-handle best-effort capture: %u failures; no process access.\n",failures);return failures?1:0;
}
