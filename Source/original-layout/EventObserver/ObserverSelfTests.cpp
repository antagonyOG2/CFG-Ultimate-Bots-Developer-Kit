#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "OfflineContextObservation.hpp"
#include "ReportObservation.hpp"
#include <cstdio>
#include <cwchar>

int wmain(int argc,wchar_t** argv) {
    if (argc!=2) return 2;
    wchar_t absolute[32768]{};
    const auto length=GetFullPathNameW(argv[1],32768,absolute,nullptr);
    if (!length || length>=32768) return 2;
    HMODULE dll=LoadLibraryW(absolute); // This diagnostic process only, never CFG.
    if (!dll) { std::printf("Observer own-process load failed: %lu\n",GetLastError()); return 1; }
    using Prepare=DWORD(WINAPI*)(const cfgobserver::Plan*);
    using Report=const cfgobserver::Report*(WINAPI*)();
    const auto prepare=reinterpret_cast<Prepare>(GetProcAddress(dll,"CfgPrepareObservation"));
    const auto getReport=reinterpret_cast<Report>(GetProcAddress(dll,"CfgObserverReport"));
    if (!prepare || !getReport) return 1;
    const auto* report=getReport();
    bool okay=report && report->magic==cfgobserver::Magic && report->version==cfgobserver::Version &&
        report->status==static_cast<DWORD>(cfgobserver::Status::NotStarted) && report->hookInstalled==0 &&
        report->publishedCount==0 && report->trampoline==0;
    // Syntactically valid plan with synthetic foreign context. Its module is
    // THIS test executable, so build identity must refuse before object reads,
    // allocation/preparation or a game operation is possible.
    cfgobserver::Plan p; p.bytes=sizeof(p); p.threadId=GetCurrentThreadId();
    p.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    p.engine=0x20000000; p.viewport=0x20010000; p.world=0x20020000; p.mode=0x20030000;
    p.controller=0x20040000; p.pawn=0x20050000; p.player=0x20060000;
    p.gameInstance=0x20070000; p.localPlayerData=0x20080000; p.objectArray={0x30000000,16};
    p.dispatch={1,p.threadId,0x20090000,p.module+cfgprobe::dispatch::NativeActorDispatcherRva};
    p.objects={cfgprobe::lifetime::SlotObservation{p.world,0,100},{p.mode,1,101},
        {p.controller,2,102},{p.pawn,3,103}};
    okay=okay && cfgobserver::SanePlan(p) &&
        prepare(&p)==static_cast<DWORD>(cfgobserver::Status::RefusedPlan);
    report=getReport();
    okay=okay && report->status==static_cast<DWORD>(cfgobserver::Status::RefusedPlan) &&
        report->bytes==sizeof(*report) && report->hookInstalled==0 && report->publishedCount==0 &&
        report->trampoline==0 && report->target==0 &&
        report->preparationStage==static_cast<DWORD>(cfgobserver::PreparationStage::Build) &&
        cfgobserver::SamePreparationFailure(*report,*report) && !cfgobserver::PublishedReport(*report) &&
        prepare(nullptr)==static_cast<DWORD>(cfgobserver::Status::RefusedPlan);
    // Resident until normal exit even in this test; no FreeLibrary path.
    std::printf("CFG observer own-process build-refusal test: %s (no game access)\n",okay?"PASS":"FAIL");
    return okay?0:1;
}
