#define wmain ExistingOfflineSnapshotEntry
#include "OfflineSnapshot.cpp"
#undef wmain
#include "GcReferencerObservation.hpp"

namespace {
constexpr auto CfgPath=L"G:\\iknowimiconic\\CampForestGreenLauncher\\Games\\cfg-live\\CampForestGreen\\Binaries\\Win64\\SummerCamp-Win64-Shipping.exe";
int CaptureReferencer(HANDLE process){
    std::array<HMODULE,64> modules{};DWORD bytes=0;
    if(!EnumProcessModulesEx(process,modules.data(),static_cast<DWORD>(sizeof(modules)),&bytes,LIST_MODULES_64BIT)||bytes<sizeof(HMODULE))return 4;
    wchar_t path[32768]{};
    if(!GetModuleFileNameExW(process,modules[0],path,32768)||_wcsicmp(path,CfgPath))return 3;
    const auto module=reinterpret_cast<std::uintptr_t>(modules[0]);Reader reader(process,256);
    cfggcretain::Observation evidence;
    const bool captured=cfggcretain::ObserveReferencer(reader,module,evidence);
    std::printf("GCReferencerSnapshot=%s\nGCReferencerReadCalls=%u\n",captured?"CAPTURED":"REFUSED",reader.reads);
    if(captured){
        PrintPointer("GCReferencer",evidence.object);PrintPointer("GCReferencerClass",evidence.type);
        PrintPointer("RegisteredOwnerArray",evidence.owners.data);
        std::printf("RegisteredOwnerCount=%d\nRegisteredOwnerCapacity=%d\nReferencerSerial=%d\nReferencerClassSerial=%d\n",
            evidence.owners.count,evidence.owners.capacity,evidence.objectSlot.serial,evidence.classSlot.serial);
        // Fixed first8 interface observations only; no engine callback invocation.
        for(int i=0;i<(std::min)(evidence.owners.count,8);++i){
            std::uintptr_t owner=0,vtable=0,collect=0;
            if(!reader.Read(evidence.owners.data+8ULL*static_cast<unsigned>(i),owner)||!Pointer(owner)||
               !reader.Read(owner,vtable)||!Pointer(vtable)||!reader.Read(vtable+8,collect)){
                std::printf("Owner_%d=REFUSED\n",i);break;
            }
            std::printf("Owner_%d=0x%llX Vtable=0x%llX Collect=0x%llX CollectInCfgImage=%s\n",i,
                static_cast<unsigned long long>(owner),static_cast<unsigned long long>(vtable),
                static_cast<unsigned long long>(collect),TargetInExecutableImage(process,module,collect)?"yes":"no");
        }
        cfggcretain::Observation again;
        const bool stable=cfggcretain::ObserveReferencer(reader,module,again)&&again.object==evidence.object&&
            again.type==evidence.type&&again.owners==evidence.owners&&again.objectSlot==evidence.objectSlot&&again.classSlot==evidence.classSlot;
        std::printf("GCReferencerEndpointMatch=%s\n",stable?"yes":"no");
    }
    PrintPointer("GCReferencerFirstReadFailure",reader.failureAddress);
    std::printf("GCReferencerWin32Error=%lu\n",reader.failureError);
    std::puts("RegistrationCalls=0\nCollectorCalls=0\nMemoryWrites=0\nInjection=none\nPolling=none\nRuntimeRetentionApproved=no\nGameplayReady=no");
    return captured?0:5;
}
}
int wmain(int argc,wchar_t** argv){
    if(argc==2&&!std::wcscmp(argv[1],L"--self-test"))return ExistingOfflineSnapshotEntry(argc,argv);
    if(argc!=2||std::wcscmp(argv[1],L"--capture")){std::puts("Use --capture or --self-test");return 1;}
    std::string hash;
    if(!HashFile(CfgPath,hash)||hash!=cfgcompat::cfg35::Sha256){std::puts("Exact CFG image refused");return 3;}
    DWORD pid=0;if(!FindProcess(CfgPath,pid)){std::puts("One running CFG process required");return 2;}
    Handle process(OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid));
    if(!process.value){std::printf("ReadOnlyOpenError=%lu\n",GetLastError());return 4;}
    std::printf("Verified CFG executable SHA256=%s\nPID=%lu\n",hash.c_str(),pid);
    if(const int gate=Snapshot(process.value,CfgPath);gate)return gate;
    return CaptureReferencer(process.value);
}
