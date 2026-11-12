#include "ObjectiveHudWrite.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};
 auto memory=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 if(!memory)return 1;
 const auto address=reinterpret_cast<std::uintptr_t>(memory);
 auto& value=*reinterpret_cast<std::uintptr_t*>(memory);
 check(!cfgobjective::CompareWrite(address,0,0x100000)&&value==0); // script writer must still refuse
 check(cfgobjective::PublishEmptyPointer(address,0x100000)&&value==0x100000);
 check(!cfgobjective::PublishEmptyPointer(address,0x200000)&&value==0x100000); // no foreign replacement
 value=0;check(!cfgobjective::PublishEmptyPointer(address,0)&&value==0);
 check(!cfgobjective::PublishEmptyPointer(address+1,0x100000)&&value==0);
 DWORD old=0;check(VirtualProtect(memory,4096,PAGE_READONLY,&old)!=0);
 check(!cfgobjective::PublishEmptyPointer(address,0x100000)&&value==0);
 check(VirtualProtect(memory,4096,old,&old)!=0);
 check(cfgobjective::PublishEmptyPointer(address,0x300000)&&value==0x300000);
 check(!cfgobjective::PublishEmptyPointer(0,0x300000));
 VirtualFree(memory,0,MEM_RELEASE);
 std::printf("Real native empty-cache writer: %u failures; own-process allocated memory only.\n",failures);
 return failures?1:0;
}
