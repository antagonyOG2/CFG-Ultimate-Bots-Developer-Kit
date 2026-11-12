#pragma once
#include <windows.h>
#include <intrin.h>
#include <array>
#include <cstdint>
#include <cstring>
namespace cfgchallengeboundary {
// Atomic2-byte conditional edit of an already proved heap Script buffer only.
// The caller owns schema/source/current-object proof; this primitive neither
// finds code nor calls game functions. SEH contains only primitive locals.
inline bool BoundaryCompare(std::uintptr_t at,short expected,short replacement)noexcept{
 __try{return _InterlockedCompareExchange16(reinterpret_cast<volatile short*>(at),replacement,expected)==expected;}
 __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
class WindowsWriter {
 std::uintptr_t pendingAt_=0;DWORD pendingProtection_=0;
 bool Recover()noexcept{
  if(!pendingAt_)return true;DWORD ignored=0;
  if(!VirtualProtect(reinterpret_cast<void*>(pendingAt_),2,pendingProtection_,&ignored)){++errors;return false;}
  pendingAt_=0;pendingProtection_=0;return true;
 }
public:
 unsigned errors=0;
 bool Pending()const{return pendingAt_!=0;}
 // Only after the owning function's registry item definitively retired. Never
 // restore protection on freed/recycled memory just because the address exists.
 void Forget(std::uintptr_t at)noexcept{if(pendingAt_==at){pendingAt_=0;pendingProtection_=0;}}
 bool Exchange(std::uintptr_t at,const std::array<unsigned char,2>& expected,const std::array<unsigned char,2>& replacement)noexcept{
  if((pendingAt_&&pendingAt_!=at)||!Recover()||!at||(at&1)||at>UINTPTR_MAX-2)return false;
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(reinterpret_cast<void*>(at),&info,sizeof(info))||info.State!=MEM_COMMIT||info.Type!=MEM_PRIVATE||
     (info.Protect&(PAGE_GUARD|PAGE_NOACCESS))||(info.Protect!=PAGE_READONLY&&info.Protect!=PAGE_READWRITE))return false;
  const auto base=reinterpret_cast<std::uintptr_t>(info.BaseAddress);
  if(at<base||info.RegionSize<2||at-base>info.RegionSize-2)return false;
  short oldValue=0,newValue=0;std::memcpy(&oldValue,expected.data(),2);std::memcpy(&newValue,replacement.data(),2);
  DWORD oldProtection=0;
  if(!VirtualProtect(reinterpret_cast<void*>(at),2,PAGE_READWRITE,&oldProtection))return false;
  pendingAt_=at;pendingProtection_=oldProtection;
  const bool exchanged=BoundaryCompare(at,oldValue,newValue);const bool restored=Recover();
  return exchanged&&restored;
 }
};
}
