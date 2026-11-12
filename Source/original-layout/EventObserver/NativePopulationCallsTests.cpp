#include "NativePopulationCalls.hpp"
#include <cstdio>
namespace {
bool validFrame=false,validTravel=false;
std::uintptr_t __fastcall ClassGetter(){return 0x60000;}
void* __fastcall Construct(void* frame){
    auto* p=static_cast<unsigned char*>(frame);
    for(unsigned i=0;i<0x30;++i)if(p[i])return nullptr;
    p[0x2C]=8;return frame;
}
std::uintptr_t __fastcall Spawn(void* world,void* klass,const void* location,const void* rotation,const void* frame){
    const auto* p=static_cast<const unsigned char*>(frame);std::uintptr_t owner=0;
    std::memcpy(&owner,p+0x10,sizeof(owner));
    validFrame=world==reinterpret_cast<void*>(0x10000)&&klass==reinterpret_cast<void*>(0x20000)&&
        !location&&!rotation&&owner==0x30000&&p[0x28]==1&&p[0x2C]==8&&
        (reinterpret_cast<std::uintptr_t>(p)%16)==0;
    return validFrame?0x40000:0;
}
bool __fastcall Travel(void* state,bool update){validTravel=state==reinterpret_cast<void*>(0x50000)&&update;return false;}
}
int main(){
    std::uintptr_t result=0;bool ready=true;
    if(!cfgmenu::population::detail::GetClass(reinterpret_cast<std::uintptr_t>(&ClassGetter),result)||result!=0x60000)return 3;
    if(!cfgmenu::population::detail::Spawn(reinterpret_cast<std::uintptr_t>(&Construct),reinterpret_cast<std::uintptr_t>(&Spawn),
        0x10000,0x20000,0x30000,result)||!validFrame||result!=0x40000)return 1;
    if(!cfgmenu::population::detail::Travel(reinterpret_cast<std::uintptr_t>(&Travel),0x50000,ready)||!validTravel||ready)return 2;
    std::puts("Native population Windows x64 frame and bool-result ABI PASS; no game access.");return 0;
}
