#include "NativeJasonCalls.hpp"
#include <cstdio>
namespace {
unsigned ctorCalls=0,spawnCalls=0,possessCalls=0;
unsigned char expectedCollision=2;
bool validFrame=false,validPossession=false,throwSpawn=false,throwPossess=false;
cfgspawn::Vec3 position{100,200,300},rotation{0,84,0};
void* __fastcall Constructor(void* value){
    ++ctorCalls;auto* p=static_cast<unsigned char*>(value);
    for(unsigned i=0;i<0x30;++i)if(p[i])return nullptr;
    p[0x2C]=8;return value;
}
std::uintptr_t __fastcall Spawn(void* world,void* klass,const void* location,const void* facing,const void* value){
    ++spawnCalls;const auto* p=static_cast<const unsigned char*>(value);std::uintptr_t owner=1;
    std::memcpy(&owner,p+0x10,8);
    validFrame=world==reinterpret_cast<void*>(0x10000)&&klass==reinterpret_cast<void*>(0x20000)&&
        location==&position&&facing==&rotation&&owner==0&&p[0x28]==expectedCollision&&p[0x2C]==8&&
        reinterpret_cast<std::uintptr_t>(value)%16==0;
    if(throwSpawn)RaiseException(0xE1234567,0,0,nullptr);
    return validFrame?0x30000:0;
}
void __fastcall Possess(void* controller,void* pawn){
    ++possessCalls;validPossession=controller==reinterpret_cast<void*>(0x40000)&&pawn==reinterpret_cast<void*>(0x30000);
    if(throwPossess)RaiseException(0xE1234568,0,0,nullptr);
}
}
int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    cfgnativejason::Fault fault{};std::uintptr_t result=0;
    const auto spawn=[&](){return cfgnativejason::detail::Spawn(reinterpret_cast<std::uintptr_t>(&Constructor),
        reinterpret_cast<std::uintptr_t>(&Spawn),0x10000,0x20000,&position,&rotation,result,fault);};
    check(spawn()&&result==0x30000&&validFrame&&ctorCalls==1&&spawnCalls==1&&!fault.code);
    check(cfgnativejason::detail::Possess(reinterpret_cast<std::uintptr_t>(&Possess),0x40000,result,fault)&&validPossession&&possessCalls==1);
    expectedCollision=1;
    check(cfgnativejason::detail::Spawn(reinterpret_cast<std::uintptr_t>(&Constructor),reinterpret_cast<std::uintptr_t>(&Spawn),
        0x10000,0x20000,&position,&rotation,result,fault,1)&&result==0x30000&&validFrame);
    expectedCollision=2;
    throwSpawn=true;result=99;
    check(!spawn()&&!result&&fault.code==0xE1234567&&fault.address&&spawnCalls==3&&ctorCalls==3);
    fault={};throwPossess=true;
    check(!cfgnativejason::detail::Possess(reinterpret_cast<std::uintptr_t>(&Possess),0x40000,0x30000,fault)&&
        fault.code==0xE1234568&&fault.address&&possessCalls==2);
    std::printf("CFG donor-style native Jason Windows ABI: %s (%u checks); actor params, authored transform, base possession, exception capture; no game access.\n",failures?"FAIL":"PASS",checks);
    return failures?1:0;
}
