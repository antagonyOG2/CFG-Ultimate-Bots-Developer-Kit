#include "../Core/HostedKillerPresentation.hpp"
#include <cstdio>
#include <map>

namespace {
struct Reader {
    std::map<std::uintptr_t,std::uintptr_t> values;
    bool Read(std::uintptr_t address,std::uintptr_t& value){
        const auto found=values.find(address);if(found==values.end())return false;
        value=found->second;return true;
    }
};
unsigned checks=0,failures=0;
#define CHECK(value) do{++checks;if(!(value)){++failures;std::printf("Hosted Jason presentation source check failed at line %d\n",__LINE__);}}while(false)
}
int main(){
    constexpr std::uintptr_t pawn=0x100000,classType=0x200000,killerBase=0x300000,root=0x400000,otherBase=0x500000;
    Reader reader;reader.values[pawn+cfgprobe::ObjectClass]=classType;
    reader.values[classType+cfgprobe::ClassSuper]=killerBase;
    reader.values[killerBase+cfgprobe::ClassSuper]=root;
    reader.values[root+cfgprobe::ClassSuper]=0;
    std::uintptr_t selected=0;
    CHECK(cfghostedpresentation::SelectedKillerClass(reader,pawn,killerBase,selected)&&selected==classType);
    selected=0xDEAD;
    CHECK(!cfghostedpresentation::SelectedKillerClass(reader,pawn,otherBase,selected)&&selected==0);
    reader.values[classType+cfgprobe::ClassSuper]=root;selected=0xDEAD;
    CHECK(!cfghostedpresentation::SelectedKillerClass(reader,pawn,killerBase,selected)&&selected==0);
    std::printf("Hosted Jason presentation source proof: %u checks, %u failures; selected class comes from current Jason pawn, not post-adoption template.\n",checks,failures);
    return failures?1:0;
}
