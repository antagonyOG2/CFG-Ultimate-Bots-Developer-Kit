#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "ObjectiveReadOnlyCapture.hpp"
#include <cassert>
int main(){
 wchar_t path[MAX_PATH]{};swprintf_s(path,L"CfgObjectiveExclusiveTest-%lu-%llu.tmp",GetCurrentProcessId(),GetTickCount64());
 FILE* exclusive=cfgobjectivecapture::OpenExclusive(path);assert(exclusive);
 assert(!cfgobjectivecapture::OpenExclusive(path));assert(std::fputs("owned test",exclusive)>=0);assert(std::fclose(exclusive)==0);
 assert(!cfgobjectivecapture::OpenExclusive(path));assert(DeleteFileW(path));
 for(unsigned i=0;i<cfgobjectivecapture::Names.size();++i)assert(cfgobjectivecapture::Wanted(cfgobjectivecapture::Names[i])==static_cast<int>(i));
 assert(cfgobjectivecapture::Wanted(u"Objective_Car_Widget_C_1")==-1);
 assert(cfgobjectivecapture::Wanted(u"OtherObjectivesWidget_C")==-1);
 cfgobjectivecapture::Reader reader(GetCurrentProcess());unsigned value=9;
 assert(!reader.Read(0,value)&&value==9);reader.reads=500000;
 assert(!reader.Read(reinterpret_cast<std::uintptr_t>(&value),value)&&reader.exhausted);
 std::puts("Objective capture PASS: exact eight names, readonly invalid-address/budget refusal; no game access");
}
