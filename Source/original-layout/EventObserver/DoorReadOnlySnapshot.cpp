#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <cstdlib>
#include "../Core/NativeSpawnParameters.hpp"
#include "SelectedDoorObservation.hpp"
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
struct DoorSnapshotReader {
 HANDLE process;unsigned reads=0;
 template<class T>bool Read(std::uintptr_t at,T& value){
  if(++reads>1024||!cfgprobe::lifetime::Extent(at,sizeof(T)))return false;
  SIZE_T done=0;T next{};
  if(!ReadProcessMemory(process,reinterpret_cast<const void*>(at),&next,sizeof(next),&done)||done!=sizeof(next))return false;
  value=next;return true;
 }
};
int main(int argc,char** argv){
 if(argc!=3&&argc!=4)return 2;
 const DWORD pid=static_cast<DWORD>(std::strtoul(argv[1],nullptr,10));
 const auto pawn=static_cast<std::uintptr_t>(std::strtoull(argv[2],nullptr,16));
 HANDLE process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
 if(!process)return 3;
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);
 std::uintptr_t module=0;unsigned modules=0;
 if(snapshot!=INVALID_HANDLE_VALUE){if(Module32FirstW(snapshot,&entry))do{
  ++modules;
  if(_wcsicmp(entry.szModule,L"SummerCamp-Win64-Shipping.exe")==0){module=reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);break;}
 }while(Module32NextW(snapshot,&entry));CloseHandle(snapshot);}
 std::printf("ModuleEnumeration Entries=%u Error=%lu Module=%llX\n",modules,GetLastError(),static_cast<unsigned long long>(module));
 // Optional ASLR base is an observation from the current pinned main harness,
 // never a guessed RVA. Validate the local target's PE header before using it.
 if(!module&&argc==4)module=static_cast<std::uintptr_t>(std::strtoull(argv[3],nullptr,16));
 DoorSnapshotReader r{process};cfgprobe::lifetime::ArrayHeader objects,fresh;
 IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
 const bool image=module&&r.Read(module,dos)&&dos.e_magic==IMAGE_DOS_SIGNATURE&&dos.e_lfanew>0&&dos.e_lfanew<1048576&&
  r.Read(module+static_cast<unsigned>(dos.e_lfanew),nt)&&nt.Signature==IMAGE_NT_SIGNATURE&&nt.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64;
 cfgprobe::lifetime::SlotObservation pawnSlot,pawnAgain;
 std::uintptr_t doorClass=0,componentClass=0;
 const bool scope=image&&cfgprimary::LookupVetoClearForObservation(r,module)&&
  cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,objects)&&
  cfgspawn::ObjectSlot(r,objects,pawn,pawnSlot)&&pawnSlot.HasAssignedSerial()&&
  r.Read(module+cfgdoor::disk::SCDoorReflection,doorClass)&&cfgobserver::Same(r,module+cfgdoor::disk::SCDoorConstructor,doorClass)&&
  r.Read(module+cfgdoor::disk::ILLInteractableComponentReflection,componentClass)&&
  cfgobserver::Same(r,module+cfgdoor::disk::ILLInteractableComponentConstructor,componentClass);
 std::printf("DoorReadOnlyScope=%u PawnSerial=%d DoorClass=%llX ComponentClass=%llX\n",scope?1u:0u,pawnSlot.serial,
  static_cast<unsigned long long>(doorClass),static_cast<unsigned long long>(componentClass));
 if(scope){
  std::uintptr_t manager=0,best=0,fallback=0;cfgnames::Observation<DoorSnapshotReader> names(r,module);
  if(r.Read(pawn+cfghunter::disk::Manager,manager)&&r.Read(manager+cfgdoor::disk::Best,best)&&r.Read(manager+cfgdoor::disk::Fallback,fallback)){
   std::printf("DoorSelection Manager=%llX Best=%llX Fallback=%llX\n",static_cast<unsigned long long>(manager),static_cast<unsigned long long>(best),static_cast<unsigned long long>(fallback));
   for(const auto component:{best,fallback}){
    if(!component)continue;std::uintptr_t owner=0,outer=0;cfgprobe::lifetime::SlotObservation c{},d{};cfgnames::Text cn{},dn{};
    const bool cslot=cfgspawn::ObjectSlot(r,objects,component,c);const bool ctype=cslot&&cfgspawn::ObjectIs(r,component,componentClass);
    const bool ownerRead=r.Read(component+cfgdoor::disk::ComponentOwner,owner)&&r.Read(component+0x20,outer);
    const bool dslot=ownerRead&&cfgspawn::ObjectSlot(r,objects,owner,d);const bool dtype=dslot&&cfgspawn::ObjectIs(r,owner,doorClass);
    std::uint8_t open=255,locked=255;if(dtype){r.Read(owner+cfgdoor::disk::Open,open);r.Read(owner+cfgdoor::breakdisk::Locked,locked);}
    const bool cnamed=names.Object(component,cn),dnamed=owner&&names.Object(owner,dn);
    std::printf("DoorCandidate Component=%llX Slot=%u Serial=%d Typed=%u Owner=%llX Outer=%llX DoorSlot=%u DoorSerial=%d DoorTyped=%u Open=%u Locked=%u Name=",
     static_cast<unsigned long long>(component),cslot?1u:0u,c.serial,ctype?1u:0u,static_cast<unsigned long long>(owner),static_cast<unsigned long long>(outer),dslot?1u:0u,d.serial,dtype?1u:0u,open,locked);
    if(cnamed)for(const auto x:cn.View())std::putchar(x<128?static_cast<char>(x):'?');std::printf(" DoorName=");
    if(dnamed)for(const auto x:dn.View())std::putchar(x<128?static_cast<char>(x):'?');std::putchar('\n');
   }
  }
 }
 cfgdoor::Selected selected;cfgdoor::BreakState state;std::uint8_t attackable=255;
 const bool observed=scope&&cfgdoor::ObserveSelected(r,objects,pawn,doorClass,componentClass,selected)&&
  cfgdoor::ObserveBreak(r,objects,pawn,selected,doorClass,state)&&
  r.Read(selected.component+cfgdoor::breakdisk::CanAttackToInteract,attackable)&&
  cfgspawn::ObjectSlot(r,objects,pawn,pawnAgain)&&pawnAgain==pawnSlot&&
  cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,fresh)&&fresh==objects&&
  cfgprimary::LookupVetoClearForObservation(r,module);
 std::printf("ReadOnlyDoorSnapshot PID=%lu Pawn=%llX Verified=%u Reads=%u NoWritesOrNativeCalls=yes\n",pid,static_cast<unsigned long long>(pawn),observed?1u:0u,r.reads);
 if(observed)std::printf("SelectedDoor=%llX Component=%llX Locked=%u Open=%u Destroyed=%u Attackable=%u Wants=%u BreakTarget=%llX DoorKiller=%llX NativeSelection=%u\n",
  static_cast<unsigned long long>(selected.door),static_cast<unsigned long long>(selected.component),state.locked,state.open,state.destroyed,attackable,state.wants,
  static_cast<unsigned long long>(state.target),static_cast<unsigned long long>(state.killer),cfgdoor::NativeSelectionMatches(r,selected)?1u:0u);
 CloseHandle(process);return observed?0:4;
}
