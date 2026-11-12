#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <cwchar>
namespace cfgclosedtrace {
// A few owner-thread milestones only; never auth/session data, a worker,
// global scan, playback adjustment or mutation authority.
// H2.97 release: the developer Diagnostics folder when it exists, otherwise a Diagnostics folder next to this DLL
// (player installs). Resolved once; never a different drive guess.
inline const wchar_t* Folder()noexcept{
 static wchar_t folder[MAX_PATH]{};static bool resolved=false;
 if(resolved)return folder;resolved=true;
 constexpr wchar_t developer[]=L"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics";
 const DWORD attributes=GetFileAttributesW(developer);
 if(attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY)){wcscpy_s(folder,developer);return folder;}
 HMODULE self=nullptr;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&Folder),&self))return folder;
 wchar_t module[MAX_PATH]{};const DWORD n=GetModuleFileNameW(self,module,MAX_PATH);if(!n||n>=MAX_PATH)return folder;
 for(DWORD i=n;i>0;--i)if(module[i-1]==L'\\'){module[i-1]=0;break;}
 if(swprintf_s(folder,L"%s\\Diagnostics",module)<0){folder[0]=0;return folder;}
 CreateDirectoryW(folder,nullptr);return folder;
}
inline void Mark(const char* stage,std::uintptr_t actor=0,std::uint64_t value=0)noexcept{
 const auto* folder=Folder();if(!folder[0])return;
 wchar_t path[384]{};swprintf_s(path,L"%s\\CFG-Cinematic-Trace-PID%lu.txt",folder,GetCurrentProcessId());
 HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return;
 char line[224]{};const int n=sprintf_s(line,"Tick=%llu Stage=%s Actor=0x%llX Value=%llu\r\n",
  static_cast<unsigned long long>(GetTickCount64()),stage,static_cast<unsigned long long>(actor),static_cast<unsigned long long>(value));
 DWORD written=0;if(n>0)WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(file);
}
}
