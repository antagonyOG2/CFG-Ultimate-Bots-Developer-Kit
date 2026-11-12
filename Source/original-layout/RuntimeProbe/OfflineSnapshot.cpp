#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <bcrypt.h>
#include <array>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>
#include "SnapshotBindings.hpp"
#include "../Core/ComprehensiveCapture.hpp"
#include "ClassIdentity.hpp"
#include "HostedAuthority35.generated.hpp"
#include "EntryPrefixObservation.hpp"
#include "NamedExportEvidence.hpp"
#include "ObjectLifetime.hpp"
#include "DispatchObservation.hpp"
#include "AiResourceSnapshot.hpp"
#if defined(CFG_FEATURE_SNAPSHOT)
#include "FeatureSnapshot.hpp"
#include "InventoryFunctionCapture.hpp"
#endif
#if defined(CFG_PRIMARY_RESOURCES)
#include "PrimaryJasonResources.hpp"
#endif

namespace {
struct Handle {
    HANDLE value = nullptr;
    explicit Handle(HANDLE handle = nullptr) : value(handle) {}
    ~Handle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
};
class Sha256 {
public:
    ~Sha256() {
        if (hash_) BCryptDestroyHash(hash_);
        if (algorithm_) BCryptCloseAlgorithmProvider(algorithm_,0);
    }
    bool Initialize() {
        DWORD bytes = 0, size = 0;
        if (BCryptOpenAlgorithmProvider(&algorithm_,BCRYPT_SHA256_ALGORITHM,nullptr,0) < 0 ||
            BCryptGetProperty(algorithm_,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),
                sizeof(size),&bytes,0) < 0) return false;
        storage_.resize(size);
        return BCryptCreateHash(algorithm_,&hash_,storage_.data(),size,nullptr,0,0) >= 0;
    }
    bool Update(const unsigned char* data, ULONG size) {
        return BCryptHashData(hash_,const_cast<PUCHAR>(data),size,0) >= 0;
    }
    bool Finish(std::string& output) {
        std::array<unsigned char,32> bytes{};
        if (BCryptFinishHash(hash_,bytes.data(),static_cast<ULONG>(bytes.size()),0) < 0) return false;
        static constexpr char Digits[] = "0123456789ABCDEF";
        output.clear(); output.reserve(64);
        for (const auto byte : bytes) {
            output.push_back(Digits[byte >> 4]); output.push_back(Digits[byte & 15]);
        }
        return true;
    }
private:
    BCRYPT_ALG_HANDLE algorithm_ = nullptr;
    BCRYPT_HASH_HANDLE hash_ = nullptr;
    std::vector<unsigned char> storage_;
};
bool HashFile(const wchar_t* path, std::string& output,
    std::uint64_t expectedBytes=cfgprobe::ExecutableBytes) {
    Handle file(CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
        nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if (file.value == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER length{};
    if (!GetFileSizeEx(file.value,&length) || length.QuadPart < 0 ||
        static_cast<std::uint64_t>(length.QuadPart) != expectedBytes) return false;
    Sha256 hash;
    if (!hash.Initialize()) return false;
    std::array<unsigned char,65536> buffer{};
    for (;;) {
        DWORD read = 0;
        if (!ReadFile(file.value,buffer.data(),static_cast<DWORD>(buffer.size()),&read,nullptr)) return false;
        if (!read) break;
        if (!hash.Update(buffer.data(),read)) return false;
    }
    return hash.Finish(output);
}
bool Pointer(std::uintptr_t value) {
    return cfgprobe::ObjectPointer(value);
}
class Reader {
public:
    enum class Budget {Snapshot,InventoryMetadata,FrontendPreparation};
    static constexpr unsigned FrontendPreparationLimit=32768;
    explicit Reader(HANDLE process,unsigned cap=256,Budget budget=Budget::Snapshot)
        : process_(process),cap_(cap&&cap<=(budget==Budget::FrontendPreparation?FrontendPreparationLimit:
            budget==Budget::InventoryMetadata?16384u:4096u)?cap:0) {}
    unsigned Limit()const noexcept{return cap_;}
    bool ReadBytes(std::uintptr_t address, void* output, std::size_t bytes) {
        if (reads>=cap_) { budgetHit=true; return false; }
        if (!output || !bytes || bytes>65536) return false;
        ++reads;
        SIZE_T obtained=0;
        if (address<0x10000 || address>0x00007FFFFFFFFFFF-bytes+1) return false;
        const bool success=ReadProcessMemory(process_,reinterpret_cast<LPCVOID>(address),output,bytes,&obtained)!=0;
        if (!success || obtained!=bytes) {
            if (!failureAddress) { failureAddress=address; failureError=success?ERROR_PARTIAL_COPY:GetLastError(); }
            return false;
        }
        return true;
    }
    template<class T> bool Read(std::uintptr_t address, T& output) {
        T value{};
        // Fields such as byte Stats[7] are not pointer-aligned. Range-check the
        // read address separately; all object pointers still require alignment.
        if (!ReadBytes(address,&value,sizeof(value))) return false;
        output = value;
        return true;
    }
    unsigned reads = 0;
    bool budgetHit=false;
    std::uintptr_t failureAddress=0;
    DWORD failureError=0;
private:
    HANDLE process_;
    unsigned cap_;
};
struct LocalPlayers { std::uintptr_t data = 0; std::int32_t count = 0, capacity = 0; };
static_assert(sizeof(LocalPlayers) == 16, "x64 array ABI");
bool ThreadBelongsToProcess(DWORD threadId,DWORD processId) {
    if (!threadId || !processId) return false;
    Handle thread(OpenThread(THREAD_QUERY_LIMITED_INFORMATION,FALSE,threadId));
    return thread.value && GetProcessIdOfThread(thread.value)==processId;
}
bool TargetInExecutableImage(HANDLE process,std::uintptr_t module,std::uintptr_t target) {
    MEMORY_BASIC_INFORMATION region{};
    if (VirtualQueryEx(process,reinterpret_cast<LPCVOID>(target),&region,sizeof(region))!=sizeof(region) ||
        region.State!=MEM_COMMIT || region.Type!=MEM_IMAGE ||
        reinterpret_cast<std::uintptr_t>(region.AllocationBase)!=module || (region.Protect&PAGE_GUARD)) return false;
    const DWORD protection=region.Protect&0xFF;
    return protection==PAGE_EXECUTE || protection==PAGE_EXECUTE_READ ||
        protection==PAGE_EXECUTE_READWRITE || protection==PAGE_EXECUTE_WRITECOPY;
}
bool SelfTest() {
    Sha256 hash;
    std::string digest;
    constexpr unsigned char abc[]{'a','b','c'};
    if (!hash.Initialize() || !hash.Update(abc,sizeof(abc)) || !hash.Finish(digest) ||
        digest != "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD") return false;
    alignas(8) std::uintptr_t fixture = 0x12345678;
    std::uintptr_t copy = 0;
    Reader local(GetCurrentProcess());
    alignas(8) std::array<unsigned char,16> fieldFixture{};
    fieldFixture[4] = 7;
    std::array<unsigned char,7> fields{};
    const bool fieldsPass=local.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy) && copy == fixture &&
        !local.Read(0,copy) && !Pointer(0xFFFFFFFFFFFFFFFFull) &&
        local.Read(reinterpret_cast<std::uintptr_t>(fieldFixture.data()+4),fields) && fields[0] == 7;
    local.reads=255;
    copy=0;
    const bool finalRead=local.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy) &&
        copy==fixture && local.reads==256;
    // Exercise the exact production constructor that the inventory capture
    // uses. Its larger one-shot budget is explicit, not a wider legacy cap.
    Reader metadata(GetCurrentProcess(),16384,Reader::Budget::InventoryMetadata);
    Reader legacyOversized(GetCurrentProcess(),16384);
    const bool metadataFirst=metadata.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&copy==fixture&&metadata.reads==1&&!metadata.budgetHit;
    metadata.reads=16383;
    const bool metadataLast=metadata.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&copy==fixture&&metadata.reads==16384;
    copy=0x0/*CFG3.5 address withheld*/;
    const bool metadataLimit=!metadata.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&metadata.budgetHit&&copy==0x0/*CFG3.5 address withheld*/&&metadata.reads==16384;
    const bool legacyLimit=!legacyOversized.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&!legacyOversized.reads&&legacyOversized.budgetHit;
    std::printf("InventoryMetadataReader_FirstLastHardLimit=%s LegacyOversizedBudgetRefused=%s\n",metadataFirst&&metadataLast&&metadataLimit?"PASS":"FAIL",legacyLimit?"PASS":"FAIL");
    Reader frontend(GetCurrentProcess(),Reader::FrontendPreparationLimit,Reader::Budget::FrontendPreparation);
    Reader inventoryOversized(GetCurrentProcess(),Reader::FrontendPreparationLimit,Reader::Budget::InventoryMetadata);
    const bool frontendFirst=frontend.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&copy==fixture&&frontend.Limit()==32768;
    frontend.reads=frontend.Limit()-1;
    const bool frontendLast=frontend.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&copy==fixture&&frontend.reads==32768;
    copy=0x0/*CFG3.5 address withheld*/;
    const bool frontendLimit=!frontend.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&frontend.budgetHit&&copy==0x0/*CFG3.5 address withheld*/&&frontend.reads==32768;
    const bool inventoryLimit=!inventoryOversized.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy)&&!inventoryOversized.reads&&inventoryOversized.budgetHit;
    std::printf("FrontendPreparationReader_FirstLastHardLimit=%s InventoryLimitUnchanged=%s SnapshotDefaultLimit=%u\n",
        frontendFirst&&frontendLast&&frontendLimit?"PASS":"FAIL",inventoryLimit?"PASS":"FAIL",local.Limit());
    copy=0x0/*CFG3.5 address withheld*/;
    return fieldsPass && finalRead && metadataFirst && metadataLast && metadataLimit && legacyLimit &&
        frontendFirst&&frontendLast&&frontendLimit&&inventoryLimit&&ThreadBelongsToProcess(GetCurrentThreadId(),GetCurrentProcessId()) &&
        !ThreadBelongsToProcess(GetCurrentThreadId(),0) &&
        TargetInExecutableImage(GetCurrentProcess(),reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)),
            reinterpret_cast<std::uintptr_t>(&SelfTest)) &&
        !TargetInExecutableImage(GetCurrentProcess(),0,reinterpret_cast<std::uintptr_t>(&SelfTest)) &&
        !local.Read(reinterpret_cast<std::uintptr_t>(&fixture),copy) &&
        copy==0x0/*CFG3.5 address withheld*/ && local.reads==256; // Hard budget: no read and no output mutation.
}
bool FindProcess(const wchar_t* expectedPath, DWORD& pid) {
    Handle enumeration(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0));
    if (enumeration.value == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry{}; entry.dwSize = sizeof(entry);
    if (!Process32FirstW(enumeration.value,&entry)) return false;
    unsigned matches = 0;
    do {
        if (_wcsicmp(entry.szExeFile,L"SummerCamp-Win64-Shipping.exe") != 0) continue;
        Handle query(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,entry.th32ProcessID));
        if (!query.value) continue;
        wchar_t path[32768]{}; DWORD size = 32768;
        if (QueryFullProcessImageNameW(query.value,0,path,&size) && _wcsicmp(path,expectedPath)==0) {
            pid = entry.th32ProcessID; ++matches;
        }
    } while (Process32NextW(enumeration.value,&entry));
    if (matches > 1) std::puts("More than one CFG game process found; close extras before testing.");
    return matches == 1;
}
void PrintPointer(const char* name, std::uintptr_t value) {
    std::printf("%s=0x%016llX\n",name,static_cast<unsigned long long>(value));
}
int Snapshot(HANDLE process, const wchar_t* expectedPath, bool loadingTestStage=false,
    bool observeLifetime=false,bool observeDispatch=false,bool observeAi=false,bool observePrimary=false,
    bool observeFeatures=false,bool allowHostedReadOnly=false) {
    (void)observePrimary;
    (void)observeFeatures;
    observeLifetime=observeLifetime || observeDispatch || observeAi || observeFeatures;
    wchar_t processPath[32768]{}; DWORD pathSize = 32768;
    if (!QueryFullProcessImageNameW(process,0,processPath,&pathSize) || _wcsicmp(processPath,expectedPath)!=0) {
        std::puts("Process identity changed during discovery; no memory accessed.");
        return 3;
    }
    std::array<HMODULE,64> modules{};
    DWORD bytes = 0;
    if (!EnumProcessModulesEx(process,modules.data(),static_cast<DWORD>(sizeof(modules)),&bytes,LIST_MODULES_64BIT) ||
        bytes < sizeof(HMODULE)) {
        std::printf("Module query denied/unavailable (Windows error %lu). No escalation attempted.\n",GetLastError());
        return 4;
    }
    const auto module = reinterpret_cast<std::uintptr_t>(modules[0]);
    wchar_t modulePath[32768]{};
    if (!GetModuleFileNameExW(process,modules[0],modulePath,32768) || _wcsicmp(modulePath,expectedPath)!=0) {
        std::puts("Main executable module not confirmed; no candidate reads attempted.");
        return 3;
    }
    Reader reader(process);
    IMAGE_DOS_HEADER dos{};
    IMAGE_NT_HEADERS64 nt{};
    if (!reader.Read(module,dos) || dos.e_magic!=IMAGE_DOS_SIGNATURE ||
        dos.e_lfanew < 64 || dos.e_lfanew > 4096 ||
        !reader.Read(module+static_cast<std::uintptr_t>(dos.e_lfanew),nt) ||
        nt.Signature!=IMAGE_NT_SIGNATURE || nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 ||
        nt.FileHeader.TimeDateStamp!=cfgprobe::ExecutableTimestamp || nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
        nt.OptionalHeader.SizeOfImage!=cfgprobe::ExecutableImageBytes) {
        std::puts("Loaded image headers do not match the inventoried CFG build; candidate reads refused.");
        return 3;
    }
    std::uintptr_t engine = 0, viewport = 0, world = 0, netDriver = 0, gameMode = 0, gameState = 0;
    if (!reader.Read(module+cfgprobe::EngineGlobalCandidate,engine) || !Pointer(engine) ||
        !reader.Read(engine+cfgprobe::EngineGameViewport,viewport) || !Pointer(viewport) ||
        !reader.Read(viewport+cfgprobe::ViewportWorld,world) || !Pointer(world) ||
        !reader.Read(world+cfgprobe::WorldNetDriver,netDriver)) {
        std::puts("Candidate engine/world path unavailable. No fallback or repeated scanning attempted.");
        return 5;
    }
    if (!cfgcapture::SnapshotWorldAllowed(netDriver!=0,allowHostedReadOnly)) {
        std::puts("Networked world detected; snapshot refused. Use a native offline match only.");
        return 5;
    }
    if(netDriver)std::puts("NetworkedWorld=read-only comprehensive diagnostics explicitly enabled; no target writes, injections, or native calls.");
    if (!reader.Read(world+cfgprobe::WorldGameMode,gameMode) || !Pointer(gameMode) ||
        !reader.Read(world+cfgprobe::WorldGameState,gameState) || !Pointer(gameState)) {
        std::puts("No initialized local gameplay world detected. Start an offline match first.");
        return 5;
    }
    std::uintptr_t instance = 0, player = 0, controller = 0;
    LocalPlayers players{};
    if (!reader.Read(world+cfgprobe::WorldGameInstance,instance) || !Pointer(instance) ||
        !reader.Read(instance+cfgprobe::GameInstanceLocalPlayers,players) || !Pointer(players.data) ||
        players.count < 1 || players.count > 8 || players.capacity < players.count || players.capacity > 64 ||
        !reader.Read(players.data,player) || !Pointer(player) ||
        !reader.Read(player+cfgprobe::PlayerController,controller) || !Pointer(controller)) {
        std::puts("Local-player/controller path unavailable; no retries or ownership changes attempted.");
        return 5;
    }
    std::uintptr_t controllerPlayer = 0, acknowledged = 0, pawn = 0, camera = 0, spectator = 0;
    if (!reader.Read(controller+cfgprobe::ControllerPlayer,controllerPlayer) || controllerPlayer != player ||
        !reader.Read(controller+cfgprobe::ControllerAcknowledgedPawn,acknowledged) ||
        !reader.Read(controller+cfgprobe::ControllerPawn,pawn) ||
        !reader.Read(controller+cfgprobe::ControllerCameraManager,camera) ||
        !reader.Read(controller+cfgprobe::ControllerSpectatorPawn,spectator)) {
        std::puts("Controller ownership/readback check failed. Nothing changed.");
        return 5;
    }
    if (!Pointer(pawn) || acknowledged != pawn) {
        std::puts("Local pawn is not initialized/acknowledged; identity snapshot discarded.");
        return 5;
    }
    cfgprobe::lifetime::ArrayHeader objectArrayHeader;
    std::array<cfgprobe::lifetime::SlotObservation,4> objectSlots{};
    const std::array<std::uintptr_t,4> lifetimeObjects{{world,gameMode,controller,pawn}};
    if (observeLifetime && !cfgprobe::lifetime::ObserveBatch(reader,
        module+cfgprobe::lifetime::ObjectArrayRva,lifetimeObjects,objectArrayHeader,objectSlots)) {
        std::puts("ObjectSlotIdentity=unresolved\nNoSerialAllocationOrNativeCalls=yes"); return 5;
    }
    std::array<std::uintptr_t,cfgprobe::NativeClassCount> reflectionCaches{}, constructorCaches{}, nativeClasses{};
    for (std::size_t i=0; i<cfgprobe::NativeClasses.size(); ++i) {
        if (!reader.Read(module+cfgprobe::NativeClasses[i].reflectionCacheRva,reflectionCaches[i]) ||
            !reader.Read(module+cfgprobe::NativeClasses[i].constructorCacheRva,constructorCaches[i])) {
            std::puts("Native class cache read unavailable; no factory calls or fallback."); return 5;
        }
        nativeClasses[i] = cfgprobe::MatchingClassCache(reflectionCaches[i],constructorCaches[i]);
    }
    cfgprobe::Ancestry modeAncestry, controllerAncestry, pawnAncestry;
    if (!cfgprobe::CaptureAncestry(reader,gameMode,modeAncestry) ||
        !cfgprobe::CaptureAncestry(reader,controller,controllerAncestry) ||
        !cfgprobe::CaptureAncestry(reader,pawn,pawnAncestry)) {
        std::puts("Native ancestry unavailable, cyclic or over bound; discarded."); return 5;
    }
    const char* modeName = nullptr;
    unsigned modeMatches = 0;
    for (const auto mode : {cfgprobe::OfflineBotsMode,cfgprobe::SandboxMode,cfgprobe::ChallengesMode}) {
        if (modeAncestry.Contains(nativeClasses[mode])) { modeName=cfgprobe::NativeClasses[mode].name; ++modeMatches; }
    }
    const bool counselor = pawnAncestry.Contains(nativeClasses[cfgprobe::Counselor]);
    const bool killer = pawnAncestry.Contains(nativeClasses[cfgprobe::Killer]);
    // The comprehensive switch is explicitly read-only and is also used for
    // hosted captures. A listen-host Hunt world is not one of the three offline
    // modes above, but its exact class-cache pair is already pinned by the
    // hosted authority profile.
    std::uintptr_t huntReflection=0,huntConstructor=0;
    const auto huntMode=allowHostedReadOnly&&netDriver&&
        reader.Read(module+cfghosted::disk35::HuntReflection,huntReflection)&&
        reader.Read(module+cfghosted::disk35::HuntConstructor,huntConstructor)?
        cfgprobe::MatchingClassCache(huntReflection,huntConstructor):0;
    if(!modeMatches&&huntMode&&modeAncestry.Contains(huntMode)){
        modeName="SCGameMode_Hunt";modeMatches=1;
    }
    std::uintptr_t featurePawn=pawn;
    bool vehicle=false;
#if defined(CFG_FEATURE_SNAPSHOT)
    std::uintptr_t vehicleReflection=0,vehicleConstructor=0;
    cfgfeature::DriverObservation driverObservation{};
    if(observeFeatures && !counselor && !killer &&
        reader.Read(module+cfgfeature::disk::Classes[cfgfeature::disk::Vehicle].reflection,vehicleReflection) &&
        reader.Read(module+cfgfeature::disk::Classes[cfgfeature::disk::Vehicle].constructor,vehicleConstructor)) {
        const auto vehicleClass=cfgprobe::MatchingClassCache(vehicleReflection,vehicleConstructor);
        vehicle=pawnAncestry.Contains(vehicleClass);
        if(vehicle) {
            if(!cfgfeature::ObserveDriver(reader,objectArrayHeader,pawn,vehicleClass,
                nativeClasses[cfgprobe::Counselor],driverObservation)) {
                std::puts("VehicleDriverIdentity=REFUSED\nNoPossessionOrGameplayChanges=yes");return 5;
            }
            featurePawn=driverObservation.driver;
        }
    }
#endif
    if (modeMatches != 1 || !controllerAncestry.Contains(nativeClasses[cfgprobe::PlayerControllerClass]) ||
        (!vehicle && (!pawnAncestry.Contains(nativeClasses[cfgprobe::Character]) || counselor == killer))) {
        std::printf("NativeIdentityRefusal ModeMatches=%u Mode=%s ControllerPlayerController=%u PawnCharacter=%u Counselor=%u Killer=%u Vehicle=%u HostedReadOnly=%u\n",
            modeMatches,modeName?modeName:"unmatched",controllerAncestry.Contains(nativeClasses[cfgprobe::PlayerControllerClass])?1u:0u,
            pawnAncestry.Contains(nativeClasses[cfgprobe::Character])?1u:0u,counselor?1u:0u,killer?1u:0u,vehicle?1u:0u,allowHostedReadOnly?1u:0u);
        std::puts("Native mode/controller/pawn identity unresolved. No gameplay fields read.");
        for (std::size_t i=0; i<nativeClasses.size(); ++i) {
            std::printf("Cache_%s=%s\n",cfgprobe::NativeClasses[i].name,nativeClasses[i] ? "matching" : "unresolved");
        }
        return 5;
    }
    std::array<unsigned char,7> counselorStats{};
    if (observeAi && modeAncestry.Contains(nativeClasses[cfgprobe::ChallengesMode])) {
        std::puts("AIResourceSnapshot=SKIPPED_CHALLENGES_MODE\nNoResourceReadsAttempted=yes");
        observeAi=false; // This section still records the validated roster and common read-only context.
    }
    std::int32_t trapCount = 0;
    if (((counselor||vehicle) && !reader.Read(featurePawn+cfgprobe::CounselorStats,counselorStats)) ||
        (killer && !reader.Read(pawn+cfgprobe::KillerTrapCount,trapCount))) {
        std::puts("Identified pawn field read unavailable; discarded."); return 5;
    }
    cfgprobe::dispatch::Observation dispatchObservation;
    Reader callbackReader(process,1024);
    cfgprobe::dispatch::diagnostic::Registry callbackRegistry,callbackRegistryAgain;
#if defined(CFG_FEATURE_SNAPSHOT)
    cfgfeature::Report featureReport{};
    unsigned featureReads=0;
    if(observeFeatures) {
        Reader featureReader(process,4096);
        cfgfeature::Capture<Reader> capture(featureReader,module,objectArrayHeader);
        if(!capture.Run(gameState,featurePawn,counselor||vehicle,featureReport)) {
            std::printf("FeatureSnapshot=REFUSED\nFeatureReadCalls=%u\nFeatureReadCap=4096\nBudgetHit=%s\nFirstReadWin32Error=%lu\nNoWritesOrNativeCalls=yes\n",
                featureReader.reads,featureReader.budgetHit?"yes":"no",featureReader.failureError);
            return 5;
        }
        featureReads=featureReader.reads;
    }
#endif
    cfgspawn::ResourceReport aiResources;
    cfgspawn::RosterDiagnostic aiRoster;
    cfgspawn::ModeTreeObservation modeTreeObservation{};
    unsigned aiReads=0;
    if (observeAi) {
        Reader aiReader(process,4096);
        cfgspawn::ResourceProgress progress;
        if (!cfgspawn::CaptureResources(aiReader,module,objectArrayHeader,pawn,counselor?1u:2u,
            gameState,nativeClasses[cfgprobe::Counselor],nativeClasses[cfgprobe::Killer],aiResources,&progress,&aiRoster)) {
            std::printf("AIResourceSnapshot=REFUSED\nAIReadCalls=%u\nAIReadCap=4096\nAIReadBudgetHit=%s\nAIResourceStage=%s\nRosterIndex=%d\nFirstReadFailureAddress=0x%llX\nFirstReadWin32Error=%lu\nNoNativeCallsOrWrites=yes\n",
                aiReader.reads,aiReader.budgetHit?"yes":"no",progress.stage,progress.rosterIndex,
                static_cast<unsigned long long>(aiReader.failureAddress),aiReader.failureError);
            return 5;
        }
        if (!cfgspawn::ObserveModeCounselorTree(aiReader,objectArrayHeader,gameMode,
                aiResources.classes[cfgspawn::Tree],modeTreeObservation)) {
            std::printf("AIResourceSnapshot=REFUSED\nAIReadCalls=%u\nAIReadCap=4096\nAIReadBudgetHit=%s\nAIResourceStage=mode_counselor_tree\nRosterIndex=-1\nFirstReadFailureAddress=0x%llX\nFirstReadWin32Error=%lu\nNoNativeCallsOrWrites=yes\n",
                aiReader.reads,aiReader.budgetHit?"yes":"no",
                static_cast<unsigned long long>(aiReader.failureAddress),aiReader.failureError);
            return 5;
        }
        aiReads=aiReader.reads;
    }
#if defined(CFG_PRIMARY_RESOURCES)
    cfgprimary::Report primaryResources{};
    unsigned primaryReads=0;
    if (observePrimary) {
        Reader primaryReader(process,1024);
        if (!observeAi || !modeAncestry.Contains(nativeClasses[cfgprobe::SandboxMode]) || !counselor ||
            !cfgprimary::Capture(primaryReader,module,objectArrayHeader,gameMode,gameState,
                nativeClasses[cfgprobe::SandboxMode],pawn,nativeClasses[cfgprobe::Counselor],
                nativeClasses[cfgprobe::Killer],aiResources.classes[cfgspawn::PlayerState],
                aiResources.classes[cfgspawn::Tree],primaryResources)) {
            std::printf("PrimaryJasonResources=REFUSED\nPrimaryReadCalls=%u\nPrimaryReadCap=1024\nNoNativeCallsOrWrites=yes\n",primaryReader.reads);
            return 5;
        }
        primaryReads=primaryReader.reads;
    }
#endif
    if (observeDispatch && (!objectSlots[3].HasAssignedSerial() ||
        !cfgprobe::dispatch::Observe(reader,module,pawn,dispatchObservation) ||
        !ThreadBelongsToProcess(dispatchObservation.threadId,GetProcessId(process)) ||
        !TargetInExecutableImage(process,module,dispatchObservation.target))) {
        std::puts("DispatchObservation=unresolved\nSafeCallbackVerified=no\nRuntimeCallsApproved=no\nNoRetry=yes"); return 5;
    }
    if(observeDispatch&&!cfgprobe::dispatch::diagnostic::ObserveRegistry(callbackReader,module,callbackRegistry)) {
        std::printf("CallbackRegistrySnapshot=REFUSED\nCallbackRegistryReadCalls=%u\nCallbackRegistryReadCap=1024\nNoNativeCallsOrWrites=yes\n",
            callbackReader.reads);
        return 5;
    }
    std::uintptr_t engineAgain = 0, viewportAgain = 0, worldAgain = 0, instanceAgain = 0;
    std::uintptr_t playerAgain = 0, controllerAgain = 0, ownerAgain = 0, netAgain = 0;
    LocalPlayers playersAgain{};
    if (!reader.Read(module+cfgprobe::EngineGlobalCandidate,engineAgain) || engineAgain != engine ||
        !reader.Read(engine+cfgprobe::EngineGameViewport,viewportAgain) || viewportAgain != viewport ||
        !reader.Read(viewport+cfgprobe::ViewportWorld,worldAgain) || worldAgain != world ||
        !reader.Read(world+cfgprobe::WorldGameInstance,instanceAgain) || instanceAgain != instance ||
        !reader.Read(instance+cfgprobe::GameInstanceLocalPlayers,playersAgain) ||
            playersAgain.data != players.data || playersAgain.count != players.count ||
            playersAgain.capacity != players.capacity ||
        !reader.Read(players.data,playerAgain) || playerAgain != player ||
        !reader.Read(player+cfgprobe::PlayerController,controllerAgain) || controllerAgain != controller ||
        !reader.Read(controller+cfgprobe::ControllerPlayer,ownerAgain) || ownerAgain != player ||
        !reader.Read(world+cfgprobe::WorldNetDriver,netAgain) || netAgain != netDriver) {
        std::puts("World/controller changed during snapshot. Discarded; rerun only after the match settles.");
        return 5;
    }
    if (!cfgprobe::StableAncestry(reader,gameMode,modeAncestry) ||
        !cfgprobe::StableAncestry(reader,controller,controllerAncestry) ||
        !cfgprobe::StableAncestry(reader,pawn,pawnAncestry)) {
        std::puts("Native object class/ancestry changed during snapshot; discarded."); return 5;
    }
    for (std::size_t i=0; i<nativeClasses.size(); ++i) {
        std::uintptr_t reflectionAgain=0, constructorAgain=0;
        if (!reader.Read(module+cfgprobe::NativeClasses[i].reflectionCacheRva,reflectionAgain) ||
            reflectionAgain != reflectionCaches[i] ||
            !reader.Read(module+cfgprobe::NativeClasses[i].constructorCacheRva,constructorAgain) ||
            constructorAgain != constructorCaches[i]) {
            std::puts("Class caches changed during snapshot; discarded."); return 5;
        }
    }
    std::uintptr_t modeAgain=0, acknowledgedAgain=0, pawnAgain=0;
    if (!reader.Read(world+cfgprobe::WorldGameMode,modeAgain) || modeAgain != gameMode ||
        !reader.Read(controller+cfgprobe::ControllerAcknowledgedPawn,acknowledgedAgain) || acknowledgedAgain != pawn ||
        !reader.Read(controller+cfgprobe::ControllerPawn,pawnAgain) || pawnAgain != pawn) {
        std::puts("Pawn or game mode changed during identity snapshot; discarded."); return 5;
    }
    if (observeDispatch && (!cfgprobe::dispatch::Revalidate(reader,module,pawn,dispatchObservation) ||
        !ThreadBelongsToProcess(dispatchObservation.threadId,GetProcessId(process)) ||
        !TargetInExecutableImage(process,module,dispatchObservation.target))) {
        std::puts("DispatchObservation=changed_or_unreadable\nSnapshotDiscarded=yes\nNoRetry=yes"); return 5;
    }
    if(observeDispatch&&!cfgprobe::dispatch::diagnostic::RevalidateRegistry(
        callbackReader,module,callbackRegistry,&callbackRegistryAgain)) {
        std::printf("CallbackRegistrySnapshot=changed_or_unreadable\nCallbackRegistryReadCalls=%u\nSnapshotDiscarded=yes\nNoRetry=yes\nNoNativeCallsOrWrites=yes\n",
            callbackReader.reads);
        return 5;
    }
    bool objectArrayHeaderChanged=false;
    if (observeLifetime && !cfgprobe::lifetime::RevalidateSlotsCurrentHeader(reader,
        module+cfgprobe::lifetime::ObjectArrayRva,objectArrayHeader,objectSlots,&objectArrayHeaderChanged)) {
        std::puts("ObjectSlotIdentity=changed_or_unreadable\nSnapshotDiscarded=yes\nNoRetry=yes"); return 5;
    }
    if(observeLifetime)std::printf("ObjectArrayHeaderChangedDuringSample=%s\n",
        objectArrayHeaderChanged?"yes":"no");
#if defined(CFG_FEATURE_SNAPSHOT)
    if(vehicle) {
        std::uintptr_t a=0,b=0;
        cfgfeature::DriverObservation fresh{};
        if(!reader.Read(module+cfgfeature::disk::Classes[cfgfeature::disk::Vehicle].reflection,a) || a!=vehicleReflection ||
            !reader.Read(module+cfgfeature::disk::Classes[cfgfeature::disk::Vehicle].constructor,b) || b!=vehicleConstructor ||
            !cfgfeature::ObserveDriver(reader,objectArrayHeader,pawn,a,nativeClasses[cfgprobe::Counselor],fresh) ||
            !(fresh.vehicleSlot==driverObservation.vehicleSlot) || !(fresh.driverSlot==driverObservation.driverSlot)) {
            std::puts("VehicleDriverIdentity=CHANGED\nSnapshotDiscarded=yes\nNoPossessionOrGameplayChanges=yes");return 5;
        }
    }
#endif
    if (observeAi) {
        std::uintptr_t stateAgain=0;
        if (!reader.Read(world+cfgprobe::WorldGameState,stateAgain) || stateAgain!=gameState) {
            std::puts("AIResourceWorldStateChanged=yes\nSnapshotDiscarded=yes"); return 5;
        }
    }
    std::puts("CFG OFFLINE IDENTITY SNAPSHOT -- READ ONLY, NOT A PLAYABLE MOD");
    std::printf("NativeMode=%s\nNativePawnBase=%s\n",modeName,
        vehicle ? "SCDriveableVehicle" : counselor ? "SCCounselorCharacter" : "SCKillerCharacter");
    if(vehicle) {
        PrintPointer("ControlledVehicle",pawn);PrintPointer("HumanCounselorDriver",featurePawn);
        std::puts("VehicleDriverIdentity=sampled_positive_and_rechecked\nVehiclePossessionChanged=no");
    }
    if (counselor||vehicle) {
        std::printf("CounselorStats_Strength_Speed_Intelligence_Stamina_Luck_Composure_Stealth=");
        for (std::size_t i=0; i<counselorStats.size(); ++i)
            std::printf("%s%u",i ? "," : "",static_cast<unsigned>(counselorStats[i]));
        std::puts("");
    } else std::printf("KillerTrapCount=%d\n",trapCount);
    std::puts("FieldValuesAreDiagnosticOnly=yes");
#if defined(CFG_FEATURE_SNAPSHOT)
    if(observeFeatures) {
        cfgfeature::Print(featureReport,featureReads);
        if(counselor&&!vehicle){
            Reader inventoryReader(process,16384,Reader::Budget::InventoryMetadata);
            const auto stable=cfginventory::capture::Run(inventoryReader,module,objectArrayHeader,featurePawn,featureReport);
            std::printf("InventoryMetadataReadCalls=%u ReadCap=16384 BudgetHit=%s MetadataOnly=yes\n",inventoryReader.reads,inventoryReader.budgetHit?"yes":"no");
            if(!stable||inventoryReader.budgetHit){
                std::puts("InventoryMetadataUnavailable=yes; existing feature samples are diagnostic only");
                return 5; // Do not present a partial inventory capture as success.
            }
        }
    }
#endif
#if defined(CFG_PRIMARY_RESOURCES)
    if (observePrimary) {
        std::printf("PrimaryJasonResources=CAPTURED\nPrimaryReadCalls=%u\nPrimaryReadCap=1024\nCurrentKillerAndOwnerReferencesEmpty=%s\nKillerClassCatalogueCount=%d\n",
            primaryReads,primaryResources.EmptyPrimary()?"yes":"no",primaryResources.catalogue.count);
        PrintPointer("CurrentKiller",primaryResources.killer);
        PrintPointer("CurrentKillerPlayerState",primaryResources.killerState);
        PrintPointer("ModeOwnedCounselorTree",primaryResources.counselorTree);
        const auto printSoft=[](const char* label,const cfgprimary::SoftClassBytes& value) {
            std::printf("%s=",label);
            for (const auto byte:value) std::printf("%02X",static_cast<unsigned>(byte));
            std::puts("");
        };
        printSoft("SelectedKillerOpaque40",primaryResources.selected);
        for (std::int32_t i=0;i<primaryResources.catalogue.count;++i) {
            char label[64]{}; std::snprintf(label,sizeof(label),"KillerCatalogue_%d_Opaque40",i);
            printSoft(label,primaryResources.entries[static_cast<unsigned>(i)]);
        }
        std::puts("OpaquePacketsAreNotResolvedClasses=yes\nPrimarySpawnReady=no\nNoFixedZombieIndexOrStockTreeOffset=yes");
    }
#endif
    if (observeAi) {
        std::printf("AIResourceSnapshot=CAPTURED\nAIReadCalls=%u\nNativeAIClassMask=%u\nPlayerStates=%u\nBotPlayerStates=%u\nLinkedCounselorBots=%u\nLinkedKillerBots=%u\n",
            aiReads,aiResources.classMask,aiResources.playerStates,aiResources.botStates,aiResources.counselorBots,aiResources.killerBots);
        PrintPointer("NativeSpawnAIFromClass",aiResources.spawnFunction);
        PrintPointer("NativeAISpawnHelperDefault",aiResources.helperDefault);
        std::printf("PlayerArrayObserved=%u\n",aiResources.playerArrayObserved);
        for (unsigned i=0;i<cfgspawn::Classes.size();++i)
            PrintPointer(cfgspawn::Classes[i].name,aiResources.classes[i]);
        const auto printTemplate=[](const char* label,const cfgspawn::TemplateObservation& item) {
            std::printf("%s_Pawn=0x%016llX\n%s_PawnClass=0x%016llX\n%s_Default=0x%016llX\n%s_Kind=%u\n%s_DefaultAIKind=%u\n%s_DefaultTreeTyped=%u\n%s_InstanceTreeTyped=%u\n%s_BrainTyped=%u\n%s_PawnSerial=%d\n",
                label,static_cast<unsigned long long>(item.pawn),label,static_cast<unsigned long long>(item.pawnClass),
                label,static_cast<unsigned long long>(item.defaults),label,item.kind,label,item.aiKind,label,item.defaultTreeTyped,
                label,item.instanceTreeTyped,label,item.brainTyped,label,item.pawnSlot.serial);
            std::printf("%s_CurrentControllerAIKind=%u\n",label,item.controllerAIKind);
            std::printf("%s_DefaultAI=0x%016llX\n%s_DefaultTree=0x%016llX\n%s_InstanceTree=0x%016llX\n%s_Controller=0x%016llX\n%s_Brain=0x%016llX\n",
                label,static_cast<unsigned long long>(item.defaultAI),label,static_cast<unsigned long long>(item.defaultTree),
                label,static_cast<unsigned long long>(item.instanceTree),label,static_cast<unsigned long long>(item.controller),
                label,static_cast<unsigned long long>(item.brain));
        };
        printTemplate("LocalTemplate",aiResources.local);
        printTemplate("CounselorTemplate",aiResources.counselor);
        for(unsigned i=0;i<aiRoster.count;++i){const auto& row=aiRoster.rows[i];
            std::printf("NativeRosterRow=%u State=0x%llX Controller=0x%llX Pawn=0x%llX BotFlag=%u Kind=%u Serials=%d,%d,%d\n",
                i,static_cast<unsigned long long>(row.state),static_cast<unsigned long long>(row.controller),
                static_cast<unsigned long long>(row.pawn),row.botFlag,row.kind,row.stateSlot.serial,row.controllerSlot.serial,row.pawnSlot.serial);
        }
        std::puts("RosterRowsUseExistingValidatedReads=yes\nRosterRowsAreDiagnosticNotNativeCallAuthority=yes");
        PrintPointer("ModeOwnedCounselorTree",modeTreeObservation.tree);
        std::printf("ModeOwnedCounselorTreeTyped=%s\nModeOwnedCounselorTreeSerial=%d\n",
            modeTreeObservation.typed?"yes":"no",modeTreeObservation.slot.serial);
        std::puts("AIKind_0=none_1=native_counselor_2=native_killer_3=base_AI_4=unknown\nAssetPointersAreSessionObservationsOnly=yes\nAssetSerialZeroIsNotLifetimeProof=yes\nNativeSpawnOrPossessionCalls=0\nLiveGameplayBuildReady=no");
    }
    if (observeLifetime) {
        constexpr std::array<const char*,4> labels{{"World","GameMode","Controller","Pawn"}};
        unsigned assigned=0;
        for (std::size_t i=0; i<objectSlots.size(); ++i) {
            const auto& slot=objectSlots[i]; assigned+=slot.HasAssignedSerial() ? 1u:0u;
            std::printf("%s_ObjectIndex=%d\n%s_ObjectSerial=%d\n%s_SerialObservation=%s\n",
                labels[i],slot.index,labels[i],slot.serial,labels[i],
                slot.HasAssignedSerial() ? "positive_stable_across_samples":"unassigned_pointer_index_agreement_only");
        }
        std::printf("ObjectSlotIdentityAcrossSamples=PASS\nAllocationSerialAssignedCount=%u\n",assigned);
        std::puts("AllocationSerialSampledOnly=yes\nObjectLifetimePinned=no\nGarbageCollectionEligibilityVerified=no\nGameThreadDispatchVerified=no\nNoSerialAllocationOrNativeCalls=yes");
    } else std::puts("AllocationSerialVerified=no");
    if (observeDispatch) {
        std::printf("NativeThreadFlag=%u\nNativeThreadId=%lu\nThreadBelongsToCFGAcrossSamples=yes\n",
            static_cast<unsigned>(dispatchObservation.flag),static_cast<unsigned long>(dispatchObservation.threadId));
        PrintPointer("PawnVtable",dispatchObservation.vtable);
        PrintPointer("PawnReflectedEventTarget",dispatchObservation.target);
        std::printf("NativeActorDispatchTargetMatch=%s\n",
            dispatchObservation.target==module+cfgprobe::dispatch::NativeActorDispatcherRva ? "yes":"no_possible_derived_override");
        std::printf("CallbackRegistrySnapshot=CAPTURED\nCallbackRegistryRva=0x%08llX\nCallbackRegistryEndpointMatch=yes\nCallbackRegistryCount=%d\nCallbackRegistryCountFirstSample=%d\nCallbackRegistryCountSecondSample=%d\nCallbackRegistryActiveCountFirstSample=%u\nCallbackRegistryActiveCountSecondSample=%u\nCallbackDispatchDepthFirstSample=%u\nCallbackDispatchDepthSecondSample=%u\nCallbackRegistryReadCalls=%u\nCallbackRegistryReadCap=1024\n",
            static_cast<unsigned long long>(cfgprobe::dispatch::diagnostic::RegistryRva),callbackRegistry.count,
            callbackRegistry.count,callbackRegistryAgain.count,callbackRegistry.ActiveCount(),
            callbackRegistryAgain.ActiveCount(),callbackRegistry.dispatchDepth,
            callbackRegistryAgain.dispatchDepth,callbackReader.reads);
        for(std::int32_t i=0;i<callbackRegistry.count;++i) {
            const auto& item=callbackRegistry.callbacks[static_cast<std::size_t>(i)];
            std::printf("Callback_%d_Enabled=%u\n",i,item.enabled);
            if(!item.enabled)continue;
            char label[64]{};std::snprintf(label,sizeof(label),"Callback_%d_Object",i);PrintPointer(label,item.object);
            std::snprintf(label,sizeof(label),"Callback_%d_Vtable",i);PrintPointer(label,item.vtable);
            std::snprintf(label,sizeof(label),"Callback_%d_Slot68Target",i);PrintPointer(label,item.target);
            std::printf("Callback_%d_TargetInCFGExecutableImage=%s\n",i,
                TargetInExecutableImage(process,module,item.target)?"yes":"no_or_external_module");
        }
        std::puts("CallbackRegistryEndpointMatch=yes\nCallbackRegistryQuiescenceVerified=no\nCallbackRegistryNoIntermediateInvocationVerified=no\nCallbackTargetsAreSessionObservationsOnly=yes\nCallbackEffectsVerified=no\nRuntimeCallsApproved=no");
        std::puts("DispatchObservationAcrossSamples=PASS\nTargetInCFGExecutableImageAcrossSamples=yes\nSafeCallbackVerified=no\nRuntimeCallsApproved=no\nGameplayReady=no");
    }
    if (observeLifetime && observeDispatch && observeAi)
        std::puts("CaptureBundle=AIResources+ObjectLifetime+Dispatch\nCaptureBundleSameProcess=yes\nCaptureBundleSampling=SequentialNotAtomic");
    PrintPointer("ModuleBase",module); PrintPointer("EngineCandidate",engine);
    PrintPointer("GameViewport",viewport); PrintPointer("World",world);
    PrintPointer("GameMode",gameMode); PrintPointer("GameState",gameState);
    PrintPointer("GameInstance",instance); PrintPointer("LocalPlayer",player);
    PrintPointer("Controller",controller); PrintPointer("AcknowledgedPawn",acknowledged);
    PrintPointer("ControllerPawn",pawn); PrintPointer("CameraManager",camera);
    PrintPointer("SpectatorPawn",spectator);
    std::printf("LocalPlayerCount=%d\nAcknowledgedPawnMatchesControllerPawn=%s\nReadCalls=%u\n",
        players.count,acknowledged==pawn ? "yes" : "no",reader.reads);
    if (loadingTestStage)
        std::puts("SnapshotRuntimeCompatibilityApproved=no\nSnapshotMemoryWrites=0\nSnapshotInjection=none\nSnapshotPolling=none");
    else std::puts("RuntimeCompatibilityApproved=no\nMemoryWrites=0\nInjection=none\nPolling=none");
    return 0;
}
#include "OfflineLoadTest.hpp"
#include "ProcessAccessContext.hpp"
}

int wmain(int argc, wchar_t** argv) {
    if (argc==2 && std::wcscmp(argv[1],L"--access-context-self-test")==0) {
        const bool bounds=AccessContextBoundsSelfTest();
        std::printf("TokenMetadataBoundsSelfTest=%s\n",bounds ? "PASS":"FAIL");
        return bounds ? InspectAccessContext(GetCurrentProcess()):1;
    }
    if (argc == 2 && std::wcscmp(argv[1],L"--loader-self-test") == 0) {
        return LoaderSelfTest();
    }
    const bool loadTest=argc==6 && std::wcscmp(argv[1],L"--offline-load-test")==0 &&
        std::wcscmp(argv[2],L"--exe")==0 && std::wcscmp(argv[4],L"--dll")==0;
    const bool inspectLoader=argc==4 && std::wcscmp(argv[1],L"--inspect-loader-read-only")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectAccess=argc==4 && std::wcscmp(argv[1],L"--inspect-access-context")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectLoadingAccess=argc==4 && std::wcscmp(argv[1],L"--inspect-loading-access")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectLifetime=argc==4 && std::wcscmp(argv[1],L"--offline-lifetime-snapshot")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectDispatch=argc==4 && std::wcscmp(argv[1],L"--offline-dispatch-snapshot")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectAi=argc==4 && std::wcscmp(argv[1],L"--offline-ai-resources")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
    const bool inspectSafety=argc==4 && std::wcscmp(argv[1],L"--offline-safety-capture")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
#if defined(CFG_FEATURE_SNAPSHOT)
    const bool inspectFeatures=argc==4 && std::wcscmp(argv[1],L"--offline-feature-snapshot")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
#else
    constexpr bool inspectFeatures=false;
#endif
#if defined(CFG_PRIMARY_RESOURCES)
    const bool inspectPrimary=argc==4 && std::wcscmp(argv[1],L"--offline-primary-resources")==0 &&
        std::wcscmp(argv[2],L"--exe")==0;
#else
    constexpr bool inspectPrimary=false;
#endif
    if (argc == 2 && std::wcscmp(argv[1],L"--self-test") == 0) {
        const bool pass = SelfTest();
        std::printf("CFG snapshot self-tests: %s. Only this test process was read.\n",pass ? "PASS" : "FAIL");
        return pass ? 0 : 1;
    }
    if (!loadTest && !inspectLoader && !inspectAccess && !inspectLoadingAccess && !inspectLifetime && !inspectDispatch && !inspectAi && !inspectSafety && !inspectPrimary && !inspectFeatures && (argc != 4 || std::wcscmp(argv[1],L"--offline-snapshot") != 0 || std::wcscmp(argv[2],L"--exe") != 0)) {
        std::puts("Usage: CfgOfflineSnapshot.exe --offline-snapshot|--offline-safety-capture --exe <exact CFG shipping EXE path>");
        std::puts("Read-only, one-shot native offline-match diagnostic. Never inject this or run as administrator.");
        return 1;
    }
    Handle bootstrapFile;
    if (loadTest && !ValidateBootstrap(argv[5],bootstrapFile)) return 6;
    std::string digest;
    if (!HashFile(argv[3],digest) || digest != cfgprobe::ExecutableSha256) {
        std::puts("CFG executable identity check failed. No process memory accessed.");
        return 3;
    }
    DWORD pid = 0;
    if (!FindProcess(argv[3],pid)) {
        std::puts("Exactly one matching CFG game process is required. No game launched or changed.");
        return 2;
    }
    if (inspectAccess || inspectLoadingAccess) {
        Handle query(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid));
        if (!query.value) {
            std::printf("LimitedProcessQueryError=%lu\nNoLoadingRetry=yes\n",GetLastError()); return 11;
        }
        wchar_t path[32768]{}; DWORD length=32768;
        if (!QueryFullProcessImageNameW(query.value,0,path,&length) || _wcsicmp(path,argv[3])!=0) {
            std::puts("TargetPathRecheck=FAIL\nNoLoadingRetry=yes"); return 11;
        }
        std::printf("Verified CFG executable SHA256=%s\nPID=%lu\n",digest.c_str(),pid);
        const int context=InspectAccessContext(query.value);
        if (!inspectLoadingAccess || context!=0) return context;
        // Permission check only: acquire and immediately close one exact-rights
        // handle. No read, allocation, write, remote thread or loading call is
        // reachable from this branch, even if access is granted.
        const DWORD required=PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|
            PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE;
        std::puts("Stage=permission_handle_check_only\nLoadingRightsRequested=yes");
        Handle rights(OpenProcess(required,FALSE,pid));
        const DWORD error=rights.value ? ERROR_SUCCESS:GetLastError();
        std::printf("LoadingAccessOpen=%s\nLoadingAccessWindowsError=%lu\nRequestedAccessMask=0x%08lX\n",
            rights.value ? "GRANTED":"DENIED",error,required);
        std::puts("LoadingAccessCheckOnly=yes\nNoAllocationWriteThreadOrDllLoad=yes\nNoPrivilegeOrSecurityChanges=yes");
        return rights.value ? 0:12;
    }
    const DWORD access=PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|
        (loadTest ? PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE : 0);
    Handle process(OpenProcess(access,FALSE,pid));
    if (!process.value) {
        const DWORD error=GetLastError();
        std::printf("ProcessAccessDenied=yes\nRequestedAccessMask=0x%08lX\nLoadingRightsRequested=%s\nWindowsError=%lu\n",
            access,loadTest ? "yes":"no",error);
        std::puts("Process access refused before allocation, write or thread. Stop; do not disable protection or elevate.");
        return 4;
    }
    std::printf("Verified CFG executable SHA256=%s\nPID=%lu\n",digest.c_str(),pid);
    if (inspectLoader) {
        std::puts("LOADER VERIFICATION INSPECTION ONLY -- NO ALLOCATION, WRITE OR THREAD");
        const int gate=Snapshot(process.value,argv[3]);
        if (gate!=0) return gate;
        std::uintptr_t inspectedStart=0;
        const bool verified=ResolveLoadLibrary(process.value,inspectedStart,true);
        std::printf("ReadOnlyLoaderVerification=%s\nNoLoadAttempted=yes\n",verified?"PASS":"FAIL");
        return verified?0:7;
    }
    if (loadTest) return LoadOfflineBootstrap(process.value,argv[3],argv[5]);
    return Snapshot(process.value,argv[3],false,inspectLifetime||inspectPrimary||inspectSafety,
        inspectDispatch||inspectPrimary||inspectSafety,inspectAi||inspectPrimary||inspectSafety,inspectPrimary,inspectFeatures);
}
