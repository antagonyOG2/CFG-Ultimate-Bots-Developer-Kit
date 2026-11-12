#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "NativePopulationCalls.hpp"
#include "../RuntimeProbe/SummaryPresentationBindings.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include <map>
#include <cstdio>
namespace {
unsigned failures=0,checks=0,uiCalls=0;
void Check(bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}}
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;
    bool image=true,executable=true;
    bool Image(std::uintptr_t,std::uintptr_t,std::size_t){return image;}
    bool Executable(std::uintptr_t,std::size_t){return executable;}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t count){
        auto* dest=static_cast<unsigned char*>(out);for(std::size_t i=0;i<count;++i){
            const auto found=bytes.find(at+i);if(found==bytes.end())return false;dest[i]=found->second;}return true;
    }
    template<std::size_t N>void Put(std::uintptr_t at,const std::array<unsigned char,N>& data){
        for(unsigned i=0;i<N;++i)bytes[at+i]=data[i];
    }
};
struct Fixture {
    static constexpr std::uintptr_t Image=0x100000,Target=Image+cfgmenu::populationdisk::HasFullyTraveled,Original=0x800000,Detour=0x900000;
    Memory memory;cfgownednative::Registry registry;
    Fixture(){memory.Put(Target,cfgmenu::populationdisk::HasFullyTraveledCode);
        for(unsigned i=0;i<6;++i)memory.bytes[Original+i]=cfgmenu::populationdisk::HasFullyTraveledCode[i];
        memory.Put(Original+6,cfgownednative::Jump(Target+6));memory.Put(Original+20,cfgownednative::Jump(Detour));}
    bool Prepare(){return registry.Prepare(memory,Image,Target,cfgmenu::populationdisk::HasFullyTraveledCode,Original,Detour,6);}
    void Patch(){const auto offset=static_cast<std::int32_t>(Original+20-(Target+5));
        memory.bytes[Target]=0xE9;std::array<unsigned char,4> value{};std::memcpy(value.data(),&offset,4);memory.Put(Target+1,value);}
    std::uintptr_t Resolve(){return registry.Resolve(memory,Image,Target,cfgmenu::populationdisk::HasFullyTraveledCode);}
};
bool __fastcall UiDetour(void*,bool){++uiCalls;return false;}
struct OwnedMemory:cfgownednative::CurrentProcess {
    std::uintptr_t target=0,image=0;
    // Fixture-only adapter admits ONE owned allocation. Production CurrentProcess
    // continues to require MEM_IMAGE and the exact allocation base.
    bool Image(std::uintptr_t at,std::uintptr_t module,std::size_t n){return at==target&&module==image&&n==32&&Executable(at,n);}
};
}
static_assert(cfgmenu::populationdisk::HasFullyTraveled==cfgsummarypresentation::disk::StateReady);
static_assert([]()constexpr{for(unsigned i=0;i<cfgmenu::populationdisk::HasFullyTraveledCode.size();++i)
    if(cfgmenu::populationdisk::HasFullyTraveledCode[i]!=cfgsummarypresentation::disk::StateReadyCode[i])return false;return true;}());
int main(){
    {Fixture f;Check(f.Resolve()==f.Target,"original direct without registry");Check(f.Prepare(),"prepare exact original/trampoline");
     Check(!f.Prepare(),"duplicate target cannot replace resident record");f.Patch();Check(!f.Resolve(),"disabled registry rejects JMP");
     Check(f.registry.Enable(f.memory,f.Target),"own installed patch enables");Check(f.Resolve()==f.Original,"owned detour selects trampoline");
     f.registry.Disable(f.Target);Check(!f.Resolve(),"disabled registry cannot admit installed JMP");
     f.memory.Put(f.Target,cfgmenu::populationdisk::HasFullyTraveledCode);Check(f.Resolve()==f.Target,"disabled hook original bytes direct");}
    {Fixture f;f.Patch();Check(!f.Resolve()&&!f.Prepare(),"foreign JMP cannot register or resolve");}
    {Fixture f;Check(f.Prepare(),"tail fixture prepare");f.Patch();Check(f.registry.Enable(f.memory,f.Target),"tail fixture enable");
     f.memory.bytes[f.Target+100]^=1;Check(!f.Resolve(),"changed unpatched tail refuses");}
    {Fixture f;Check(f.Prepare(),"relay fixture prepare");f.Patch();Check(f.registry.Enable(f.memory,f.Target),"relay fixture enable");
     f.memory.bytes[f.Original+26]^=1;Check(!f.Resolve(),"foreign relay target refuses");}
    {Fixture f;Check(f.Prepare(),"trampoline fixture prepare");f.Patch();Check(f.registry.Enable(f.memory,f.Target),"trampoline fixture enable");
     f.memory.bytes[f.Original]^=1;Check(!f.Resolve(),"changed stolen bytes refuse");}
    {Fixture f;Check(f.Prepare(),"jump fixture prepare");f.Patch();Check(f.registry.Enable(f.memory,f.Target),"jump fixture enable");
     f.memory.bytes[f.Target+1]^=1;Check(!f.Resolve(),"foreign entry JMP refuses");}
    {Fixture f;Check(f.Prepare(),"pin fixture prepare");f.Patch();Check(f.registry.Enable(f.memory,f.Target),"pin fixture enable");
     auto wrong=cfgmenu::populationdisk::HasFullyTraveledCode;wrong[20]^=1;
     Check(!f.registry.Resolve(f.memory,f.Image,f.Target,wrong),"different expected pin refuses");
     f.memory.image=false;Check(!f.Resolve(),"foreign image refuses");}
    // Real MinHook, exact same six stolen prologue bytes and bool(self,bool)
    // call ABI on OWNED synthetic code. The UI detour returns false; population
    // through its recorded original must preserve true and never enter that UI.
    auto* code=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));
    Check(code!=nullptr,"owned executable allocation");if(!code)return 1;
    std::array<unsigned char,32> expected{};expected.fill(0xCC);
    const unsigned char body[]{0x40,0x53,0x48,0x83,0xEC,0x20,0x0F,0xB6,0xC2,0x48,0x83,0xC4,0x20,0x5B,0xC3};
    std::memcpy(expected.data(),body,sizeof(body));std::memcpy(code,expected.data(),expected.size());
    FlushInstructionCache(GetCurrentProcess(),code,expected.size());
    const auto target=reinterpret_cast<std::uintptr_t>(code);OwnedMemory r;r.target=target;r.image=target-0x1000;
    cfgownednative::Registry bank;void* original=nullptr;
    Check(MH_Initialize()==MH_OK,"owned MinHook initialized");
    Check(MH_CreateHook(code,reinterpret_cast<void*>(&UiDetour),&original)==MH_OK,"owned summary hook created");
    Check(bank.Prepare(r,r.image,target,expected,reinterpret_cast<std::uintptr_t>(original),reinterpret_cast<std::uintptr_t>(&UiDetour),6),"real MinHook continuation/relay proof");
    Check(MH_EnableHook(code)==MH_OK&&bank.Enable(r,target),"owned summary hook enabled/registered");
    const auto function=bank.Resolve(r,r.image,target,expected);bool ready=false;
    Check(function==reinterpret_cast<std::uintptr_t>(original)&&cfgmenu::population::detail::Travel(function,0x50000,ready)&&ready&&uiCalls==0,
        "post-summary population readiness keeps original true, bypasses UI detour");
    // Native direct callers still see the unchanged summary hook behavior.
    Check(cfgmenu::population::detail::Travel(target,0x50000,ready)&&!ready&&uiCalls==1,"summary feature still detours direct native callers");
    Check(MH_DisableHook(code)==MH_OK,"owned hook disabled");bank.Disable(target);
    Check(bank.Resolve(r,r.image,target,expected)==target,"real disabled hook restores direct original");
    Check(MH_RemoveHook(code)==MH_OK&&MH_Uninitialize()==MH_OK,"owned fixture cleanup");VirtualFree(code,0,MEM_RELEASE);
    std::printf("Owned native detour gates: %u checks, %u failures; original/direct, own/trampoline, foreign refusal, summary behavior and real MinHook bool ABI. No game access.\n",checks,failures);
    return failures?1:0;
}
