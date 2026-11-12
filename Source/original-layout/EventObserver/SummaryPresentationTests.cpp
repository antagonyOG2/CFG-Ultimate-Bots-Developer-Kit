#include "../Core/SummaryPresentationPolicy.hpp"
#include "../RuntimeProbe/SummaryPresentationBindings.hpp"
#include <cstdio>
int main(){unsigned failed=0;const auto check=[&](bool b){if(!b)++failed;};
 using namespace cfgsummarypresentation;
 for(unsigned fault=0;fault<7;++fault)check(MayOverride(fault==1,fault==2,fault!=3,fault!=4,fault!=5,fault==6?0:0x20000)==(fault==0));
 std::uintptr_t row=0;unsigned originals=0;int output=123;
 {Scope text(row,0x20000);++originals;check(row==0x20000);
  {Scope nested(row,0x30000);++originals;check(row==0x30000);}check(row==0x20000);}
 check(!row&&originals==2&&output==123);
 try{Scope text(row,0x20000);throw 1;}catch(int){}check(!row);
 constexpr auto module=std::uintptr_t{0x140000000};
 for(const auto offset:{disk::TextReturn,disk::PanelReturn,disk::CharacterReturn})check(disk::Caller(module,module+offset));
 check(!disk::Caller(module,module+disk::TextReturn+1)&&!disk::Caller(module,0));
 std::printf("Scoped native Summary presentation/thread/caller/argument/nesting: %u failures;no PlayerState/native UI writes.\n",failed);
 return failed?1:0;
}
