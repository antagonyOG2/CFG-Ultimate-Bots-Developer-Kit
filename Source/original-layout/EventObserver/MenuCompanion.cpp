#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Core/MenuCompanionPolicy.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdio>
#include <functional>
namespace {
constexpr UINT StatusMessage=WM_APP+1,DoneMessage=WM_APP+2;
constexpr int EnableButton=1,SaveButton=2;
HWND window=nullptr,statusText=nullptr,enableButton=nullptr,saveButton=nullptr;
std::wstring folder;
std::thread worker;
std::atomic<bool> busy{false},cancel{false};
bool attempted=false;
struct Result {DWORD code=4;std::string output;};
struct Update {std::wstring text;};
void Status(std::wstring_view text){
    auto update=std::make_unique<Update>();update->text.assign(text);
    if(PostMessageW(window,StatusMessage,0,reinterpret_cast<LPARAM>(update.get())))update.release();
}
Result Run(const wchar_t* action,const std::function<void(std::string_view)>& stream={}){
    Result result;const auto executable=folder+L"\\Tests\\EventObserver\\CfgFrontendBundleHarness.exe";
    auto command=L"\""+executable+L"\" "+action;
    SECURITY_ATTRIBUTES security{sizeof(security),nullptr,TRUE};HANDLE reader=nullptr,writer=nullptr;
    if(!CreatePipe(&reader,&writer,&security,0))return result;
    if(!SetHandleInformation(reader,HANDLE_FLAG_INHERIT,0)){CloseHandle(reader);CloseHandle(writer);return result;}
    HANDLE input=CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,OPEN_EXISTING,0,nullptr);
    if(input==INVALID_HANDLE_VALUE){CloseHandle(reader);CloseHandle(writer);return result;}
    STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    startup.wShowWindow=SW_HIDE;startup.hStdOutput=writer;startup.hStdError=writer;startup.hStdInput=input;
    PROCESS_INFORMATION process{};
    const bool created=CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,
        folder.c_str(),&startup,&process)!=FALSE;
    CloseHandle(writer);CloseHandle(input);
    if(!created){CloseHandle(reader);return result;}
    bool overflow=false,readFailure=false,finished=false;
    const bool readOnly=std::wstring_view(action)!=L"--install";
    const bool watching=std::wstring_view(action)==L"--watch-menu";
    const auto deadline=watching?185000ULL:30000ULL;
    const auto started=GetTickCount64();
    // Drain while the child runs: a full pipe must never block installation.
    // No forced termination of an in-progress native install transaction.
    for(;;){
        DWORD available=0;
        if(PeekNamedPipe(reader,nullptr,0,nullptr,&available,nullptr)&&available){
            char bytes[4096];DWORD received=0;
            if(!ReadFile(reader,bytes,available<sizeof(bytes)?available:static_cast<DWORD>(sizeof(bytes)),&received,nullptr))readFailure=true;
            else if(cfgmenu::companion::AppendOutput(result.output,{bytes,received},watching)){if(stream)stream({bytes,received});}
            else overflow=true;
            continue;
        }
        // Drain final bytes AFTER child exit as well as during execution.
        if(finished)break;
        if(readOnly&&(cancel||GetTickCount64()-started>deadline)){
            // Only our read-only reporter is stoppable; NEVER the installer
            // transaction and NEVER CFG itself.
            TerminateProcess(process.hProcess,3);
        }
        const auto waited=WaitForSingleObject(process.hProcess,50);
        if(waited==WAIT_FAILED){readFailure=true;break;}
        finished=waited==WAIT_OBJECT_0;
    }
    DWORD code=4;if(GetExitCodeProcess(process.hProcess,&code)&&!overflow&&!readFailure)result.code=code;
    CloseHandle(reader);CloseHandle(process.hThread);CloseHandle(process.hProcess);return result;
}
bool Save(const std::string& report){
    SYSTEMTIME time{};GetSystemTime(&time);static unsigned serial=0;
    wchar_t name[160]{};
    swprintf_s(name,L"\\Diagnostics\\CFG-Windowed-Menu-%04u%02u%02u-%02u%02u%02u-%lu-%u.txt",
        time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,GetCurrentProcessId(),++serial);
    const auto path=folder+name;
    HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;const bool saved=WriteFile(file,report.data(),static_cast<DWORD>(report.size()),&written,nullptr)&&written==report.size();
    CloseHandle(file);return saved;
}
void Complete(){busy=false;PostMessageW(window,DoneMessage,0,0);}
void Enable(){
    if(attempted||busy)return;attempted=true;busy=true;cancel=false;
    EnableWindow(enableButton,FALSE);EnableWindow(saveButton,FALSE);
    if(worker.joinable())worker.join();
    worker=std::thread([](){
        Status(L"Enabling once on the first Offline Play choice screen. No game launch or other DLL is loaded.");
        const auto install=Run(L"--install");const bool installSaved=Save(install.output);
        if(install.code||install.output.find("InstallState=INSTALLED")==std::string::npos){
            Status(installSaved?L"Enable failed. See the saved Diagnostics log. Do not retry in this CFG session; no bypass or repair was attempted.":
                L"Enable failed and the Diagnostics log could not be saved. Do not retry in this CFG session.");Complete();return;
        }
        unsigned finalState=0,route=0,callbacks=0,labels=0,order=0;bool parsed=true,complete=false,failed=false;cfgmenu::companion::Lines lines;
        const auto report=Run(L"--watch-menu",[&](std::string_view bytes){
            if(!lines.Feed(bytes,[&](std::string_view line){unsigned state=0;
                bool changed=false;
                if(cfgmenu::companion::Number(line,"FrontendState",state)&&state<=10){finalState=state;changed=true;}
                if(cfgmenu::companion::Number(line,"CounselorRouteState",state)&&state<=10){route=state;changed=true;}
                if(cfgmenu::companion::Number(line,"ClassLocalCallbackCells",state)&&state<=10){callbacks=state;changed=true;}
                if(cfgmenu::companion::Number(line,"NativeEntryStage",state)&&state<=6){labels=state;changed=true;}
                if(cfgmenu::companion::Number(line,"NativeEntryOrderState",state)&&state<=9){order=state;changed=true;}
                if(changed)Status(cfgmenu::companion::Instruction(finalState,route,callbacks,labels,order));
                if(line=="AIStartupComplete=yes"||line=="AIStartupComplete=yes\r"){
                    complete=true;Status(L"AI Jason and requested counselor bots have spawned. Return to gameplay; automatic startup monitoring now stops.");}
                if(line=="NativeJasonStartupComplete=yes"||line=="NativeJasonStartupComplete=yes\r"){
                    complete=true;Status(L"Native Jason match started. The selected Jason settings are committed; return to gameplay.");}
                if(line=="NativeJasonMenuFailed=yes"||line=="NativeJasonMenuFailed=yes\r"){
                    failed=true;Status(L"Jason settings could not be prepared. Save the report; use B Back to leave settings. The selected game mode was preserved.");}
                if(line=="AIStartupFailed=yes"||line=="AIStartupFailed=yes\r"){
                    failed=true;Status(L"AI startup failed or was incomplete. No second install was attempted. Save the gameplay report, then close CFG normally.");}
            }))parsed=false;
        });
        const bool saved=Save(report.output);
        if(!saved||!installSaved)Status(L"The Diagnostics log could not be saved. No reinstall was attempted; use Save gameplay report before closing CFG.");
        else if(report.code||!parsed||(!complete&&!failed&&!cfgmenu::companion::Finished(finalState)))
            Status(L"Readiness watch ended, was cancelled, or CFG closed. Diagnostics saved. No reinstall or bypass was attempted.");
        Complete();
    });
}
void Capture(){
    if(busy)return;busy=true;cancel=false;EnableWindow(saveButton,FALSE);
    if(worker.joinable())worker.join();
    worker=std::thread([](){const auto report=Run(L"--read-report");
        if(Save(report.output))Status(report.code?L"Report attempt saved in Diagnostics. CFG may be closed; no writes or reinstall attempted.":
            L"Gameplay report saved in Diagnostics. You can now close CFG normally.");
        else Status(L"Could not save the Diagnostics report. Keep the game open until the report is saved.");
        Complete();});
}
LRESULT CALLBACK Procedure(HWND hwnd,UINT message,WPARAM w,LPARAM l){
    switch(message){
    case WM_CREATE:{
        const auto font=reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        statusText=CreateWindowW(L"STATIC",L"Development build, not full-port acceptance. Only enable when directed: launch CFG normally and stay on the first Offline Play choice screen. Do not enter stock Offline Bots first.",
            WS_CHILD|WS_VISIBLE,20,22,610,95,hwnd,nullptr,nullptr,nullptr);
        enableButton=CreateWindowW(L"BUTTON",L"Enable Offline Counselor Bots",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
            20,135,270,34,hwnd,reinterpret_cast<HMENU>(static_cast<INT_PTR>(EnableButton)),nullptr,nullptr);
        saveButton=CreateWindowW(L"BUTTON",L"Save gameplay report",WS_CHILD|WS_VISIBLE|WS_TABSTOP,
            310,135,230,34,hwnd,reinterpret_cast<HMENU>(static_cast<INT_PTR>(SaveButton)),nullptr,nullptr);
        const auto footer=CreateWindowW(L"STATIC",L"Development candidate; use only when a test is requested. Full base-mod parity is not yet approved. No old Back/Sandbox workaround. The separate hook DLL is never auto-loaded.",
            WS_CHILD|WS_VISIBLE,20,190,610,55,hwnd,nullptr,nullptr,nullptr);
        for(auto child:{statusText,enableButton,saveButton,footer})SendMessageW(child,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);
        return 0;}
    case WM_COMMAND:if(HIWORD(w)==BN_CLICKED){if(LOWORD(w)==EnableButton)Enable();else if(LOWORD(w)==SaveButton)Capture();}return 0;
    case StatusMessage:{std::unique_ptr<Update> update(reinterpret_cast<Update*>(l));SetWindowTextW(statusText,update->text.c_str());return 0;}
    case DoneMessage:EnableWindow(saveButton,TRUE);return 0;
    case WM_CLOSE:
        if(busy){cancel=true;SetWindowTextW(statusText,L"Stopping the readiness watcher after its current operation. An in-progress install will not be interrupted. Close this window once it finishes.");return 0;}
        if(worker.joinable())worker.join();DestroyWindow(hwnd);return 0;
    case WM_DESTROY:PostQuitMessage(0);return 0;
    default:return DefWindowProcW(hwnd,message,w,l);
    }
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,wchar_t* arguments,int show){
    wchar_t path[32768]{};const auto length=GetModuleFileNameW(nullptr,path,static_cast<DWORD>(std::size(path)));
    if(!length||length>=std::size(path))return 2;folder.assign(path,length);
    const auto slash=folder.find_last_of(L"\\/");if(slash==folder.npos)return 2;folder.resize(slash);
    if(arguments&&std::wstring_view(arguments)==L"--self-test"){
        // Exercise the exact hidden child/pipe/exit-code path against the
        // existing OWN-PROCESS harness tests. Never install or inspect CFG.
        const auto report=Run(L"--self-test");const bool saved=Save(report.output);
        return !report.code&&saved&&report.output.find("Integration harness own-process metadata/hash/read tests: PASS")!=report.output.npos?0:1;
    }
    HANDLE singleton=CreateMutexW(nullptr,TRUE,L"Local\\CfgOfflineCounselorMenuCompanion");
    if(!singleton)return 2;
    if(GetLastError()==ERROR_ALREADY_EXISTS){
        MessageBoxW(nullptr,L"The menu helper is already running. Open its existing taskbar window.",L"CFG Offline Counselor Bots",MB_OK|MB_ICONINFORMATION);
        CloseHandle(singleton);return 0;
    }
    WNDCLASSW cls{};cls.hInstance=instance;cls.lpszClassName=L"CfgOfflineCounselorMenuCompanion";
    cls.lpfnWndProc=Procedure;cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassW(&cls)){CloseHandle(singleton);return 2;}
    window=CreateWindowW(cls.lpszClassName,L"Camp Forest Green - Offline Counselor Bots",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,670,300,nullptr,nullptr,instance,nullptr);
    if(!window){CloseHandle(singleton);return 2;}ShowWindow(window,show);MSG message{};
    while(GetMessageW(&message,nullptr,0,0)>0){if(!IsDialogMessageW(window,&message)){TranslateMessage(&message);DispatchMessageW(&message);}}
    if(worker.joinable())worker.join();CloseHandle(singleton);return 0;
}
