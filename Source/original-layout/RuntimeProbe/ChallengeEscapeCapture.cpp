// Separate read-only diagnostic. It does not change the shipped mod package.
#define CFG_MAP_CAPTURE_ENTRY CfgExistingMapCaptureEntry
#include "SandboxMapRouteCapture.cpp"
#undef CFG_MAP_CAPTURE_ENTRY
#include "RadarWorldBindings.generated.hpp"
#include "KillBindings.generated.hpp"
#include "../EventObserver/NativeChallengeEscapeActors.hpp"
namespace {
struct EscapeList {
 std::uintptr_t data=0;int count=0,capacity=0;
 bool Sane(int maximum)const{return count>=0&&count<=maximum&&capacity>=count&&capacity<=maximum&&(count==0||cfgprobe::ObjectPointer(data));}
 bool operator==(const EscapeList& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
constexpr std::array<std::u16string_view,24> EscapeWanted{{u"EscapeVolume",u"ExitPath",u"Seats",u"CounselorInSeat",u"VehicleMovement",
 u"RootComponent",u"Controller",u"Pawn",u"CharacterMovement",u"NetConnection",u"bHasEscaped",u"bIsDying",u"CurrentSeat",
 u"bInVehicle",u"bIsGrabbed",u"EscapeSpline",u"SphereRadius",u"RelativeLocation",u"RelativeRotation",u"Dossier",
 u"bLeavingSeat",u"bIsDriverSeat",u"ParentVehicle",u"CurrentSeat"}};
bool EscapeProperties(Reader& r,std::uintptr_t base,std::uintptr_t object,unsigned nestedDepth=0){
 cfgreflection::Observation<Reader> reflection(r,base);std::uintptr_t type=0;
 if(!r.Read(object+cfgprobe::ObjectClass,type))return false;
 std::array<std::uintptr_t,32> chain{};unsigned depth=0;
 while(type){
  if(depth==chain.size())return false;for(unsigned i=0;i<depth;++i)if(chain[i]==type)return false;chain[depth++]=type;
  std::uintptr_t head=0;if(!r.Read(type+cfgreflection::disk::Children,head))return false;
  std::array<std::uintptr_t,512> fields{},links{};unsigned count=0;
  for(auto child=head;child;){
   if(count==fields.size())return false;for(unsigned i=0;i<count;++i)if(fields[i]==child)return false;
   std::uintptr_t next=0;if(!r.Read(child+cfgreflection::disk::Next,next))return false;
   cfgnames::Observation<Reader> names(r,base);cfgnames::Text name;
   if(!names.Object(child,name))return false;
   fields[count]=child;links[count++]=next;
   if(std::find(EscapeWanted.begin(),EscapeWanted.end(),name.View())!=EscapeWanted.end()){
    cfgreflection::Field field;cfgreflection::Property property,fresh;
    if(!reflection.ReadField(child,type,field)||!cfgmenu::capture::RawProperty(r,field,property)||
       property.offset<0||property.offset>65520||property.dimension<1||property.dimension>8||property.elementBytes<1||property.elementBytes>16)return false;
    Property("EscapeCandidateProperty",property);
    if(field.kind.View()==u"BoolProperty"){
     std::array<unsigned char,4> layout{};if(!r.Read(field.object+0x70,layout))return false;
     std::printf(" EscapeBooleanLayout Field=0x%llX FieldSize=%u ByteOffset=%u ByteMask=%u FieldMask=%u NativeCallApproved=no\n",
      static_cast<unsigned long long>(field.object),layout[0],layout[1],layout[2],layout[3]);
    }
    const auto bytes=static_cast<unsigned>(property.elementBytes*property.dimension);
    if(bytes>128)return false;std::array<unsigned char,128> value{},again{};
    const auto at=object+static_cast<unsigned>(property.offset);
    if(!r.ReadBytes(at,value.data(),bytes)||!cfgmenu::capture::RawProperty(r,field,fresh)||!cfgmenu::capture::SameProperty(property,fresh)||
       !r.ReadBytes(at,again.data(),bytes)||value!=again)return false;
    std::printf(" EscapeCandidateValue Object=0x%llX Property=0x%llX Hex=",static_cast<unsigned long long>(object),static_cast<unsigned long long>(child));
    for(unsigned i=0;i<bytes;++i)std::printf("%02X",value[i]);std::puts(" NativeCallApproved=no");
    // Nested ownership is diagnostic only: zero serials are reported, never
    // allocated or converted into a retained weak/native-call lease.
    if(field.kind.View()==u"ObjectProperty"&&bytes==8){std::uintptr_t nested=0;std::memcpy(&nested,value.data(),8);
     if(nested){std::uintptr_t owner=0,klass=0;cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot;
      if(!cfgprobe::lifetime::ReadArrayHeader(r,base+cfgprobe::lifetime::ObjectArrayRva,registry)||
         !cfgprobe::lifetime::ReadSlot(r,registry,nested,slot)||!r.Read(nested+0x20,owner)||!r.Read(nested+cfgprobe::ObjectClass,klass))return false;
      cfgnames::Text nestedName;if(!names.Object(klass,nestedName))return false;
      std::printf(" EscapeNested Object=0x%llX Outer=0x%llX Serial=%d Class=",static_cast<unsigned long long>(nested),static_cast<unsigned long long>(owner),slot.serial);Name(nestedName);std::puts(" NativeCallApproved=no");
      if(!nestedDepth&&(field.label.View()==u"EscapeVolume"||field.label.View()==u"Controller"||field.label.View()==u"VehicleMovement"||
          field.label.View()==u"CharacterMovement"||field.label.View()==u"RootComponent"))
       if(!EscapeProperties(r,base,nested,1))return false;
     }
    }
    if(!nestedDepth&&field.kind.View()==u"ArrayProperty"&&field.label.View()==u"Seats"){
     EscapeList seats;std::uintptr_t inner=0;
     if(bytes!=16||!r.Read(at,seats)||!seats.Sane(8)||!seats.count||!r.Read(field.object+0x70,inner))return false;
     std::printf(" EscapeSeatsInner Property=0x%llX Inner=0x%llX Count=%d NativeCallApproved=no\n",
      static_cast<unsigned long long>(field.object),static_cast<unsigned long long>(inner),seats.count);
     for(int i=0;i<seats.count;++i){std::uintptr_t seat=0,owner=0,klass=0;cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot;
      if(!r.Read(seats.data+8ULL*static_cast<unsigned>(i),seat)||
       !cfgprobe::lifetime::ReadArrayHeader(r,base+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgprobe::lifetime::ReadSlot(r,registry,seat,slot)||!r.Read(seat+0x20,owner)||!r.Read(seat+cfgprobe::ObjectClass,klass))return false;
      cfgnames::Text seatName;if(!names.Object(klass,seatName))return false;
      std::printf(" EscapeSeat Index=%d Object=0x%llX Outer=0x%llX Serial=%d Class=",i,
       static_cast<unsigned long long>(seat),static_cast<unsigned long long>(owner),slot.serial);Name(seatName);std::puts(" NativeCallApproved=no");
      if(!EscapeProperties(r,base,seat,1))return false;
     }
     EscapeList freshSeats;if(!r.Read(at,freshSeats)||!(freshSeats==seats))return false;
    }
   }
   child=next;
  }
  cfgnames::Observation<Reader> classNames(r,base);cfgnames::Text classLabel;
  if(!classNames.Object(type,classLabel))return false;
  std::printf(" EscapeDeclaredFields Class=");Name(classLabel);std::printf(" Count=%u DiagnosticOnly=yes\n",count);
  std::uintptr_t again=0;if(!r.Read(type+cfgreflection::disk::Children,again)||again!=head)return false;
  for(unsigned i=0;i<count;++i)if(!r.Read(fields[i]+cfgreflection::disk::Next,again)||again!=links[i])return false;
  if(!r.Read(type+cfgprobe::ClassSuper,type))return false;
 }
 return true;
}
int EscapeCapture(HANDLE process,std::uintptr_t base,Reader& r,bool boundaryOnly=false){
 // Reuse the existing native Challenge scope proof; no generic OfflineBots
 // acceptance, no registry search, no factories or arbitrary process paths.
 Context context;if(!ReadContext(r,base,context))return 5;
 std::uintptr_t mode=0,type=0,native=0,ctor=0,state=0;
 if(!r.Read(context.world+cfgprobe::WorldGameMode,mode)||!r.Read(mode+cfgprobe::ObjectClass,type)||
    !r.Read(base+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].reflectionCacheRva,native)||
    !r.Read(base+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].constructorCacheRva,ctor)||!cfgprobe::MatchingClassCache(native,ctor)||type!=native||
    !r.Read(context.world+cfgprobe::WorldGameState,state))return 5;
 cfgnames::Observation<Reader> names(r,base);cfgnames::Words phase{};cfgnames::Text label;
 if(!r.Read(mode+0x3F8,phase)||!names.Decode(phase,label)||label.View()!=u"InProgress"||
    !r.Read(state+cfgprobe::ObjectClass,type)||!names.Object(type,label)||label.View()!=u"SCGameState_SPChallenges")return 5;
 cfgprobe::lifetime::ArrayHeader registry;std::array<cfgprobe::lifetime::SlotObservation,3> owners{};
 const std::array<std::uintptr_t,3> objects{{context.world,mode,state}};
 if(!cfgprobe::lifetime::ObserveBatch(r,base+cfgprobe::lifetime::ObjectArrayRva,objects,registry,owners))return 5;
 for(const auto& owner:owners)if(!owner.HasAssignedSerial())return 5;
 // Capture the primary controller's inherited NetConnection metadata too.
 // Earlier family-only snapshots sampled AI controllers, which have no such
 // property and cannot establish this final standalone admission binding.
 std::uintptr_t instance=0,players=0,player=0,controller=0;EscapeList locals;
 cfgprobe::lifetime::SlotObservation controllerSlot;
 if(!r.Read(context.world+cfgprobe::WorldGameInstance,instance)||
    !r.Read(instance+cfgprobe::GameInstanceLocalPlayers,locals)||!locals.Sane(4)||locals.count!=1||
    !r.Read(locals.data,player)||!r.Read(player+cfgprobe::PlayerController,controller)||
    !cfgprobe::lifetime::ReadSlot(r,registry,controller,controllerSlot))return 5;
 players=locals.data;
 std::printf(" EscapePrimaryController Object=0x%llX Serial=%d NativeCallApproved=no\n",
  static_cast<unsigned long long>(controller),controllerSlot.serial);
 if(!boundaryOnly&&!EscapeProperties(r,base,controller,1))return 6;
 // Diagnostic-only witness: no source admission, action engine, or native calls.
 cfgchallengeescape::scope::Witness witness;witness.generation=1;witness.registry=registry;
 std::uintptr_t pawn=0,ps=0;
 const auto actorCurrent=[&]{return cfgprobe::lifetime::RevalidateBatch(r,base+cfgprobe::lifetime::ObjectArrayRva,registry,owners);};
 if(!r.Read(controller+cfgprobe::ControllerPawn,pawn)||!r.Read(controller+0x388,ps))return 6;
 const std::array<std::uintptr_t,6> diagnosticOwners{{context.world,mode,controller,pawn,state,ps}};
 for(unsigned i=0;i<diagnosticOwners.size();++i)
  if(!cfgprobe::lifetime::ReadSlot(r,registry,diagnosticOwners[i],witness.owners[i]))return 6;
 std::array<std::uintptr_t,3> families{};
 constexpr std::array<std::uintptr_t,3> caches{{0x304A998,0x304A7E8,0x3049C58}};
 for(unsigned i=0;i<caches.size();++i){if(!r.Read(base+caches[i],families[i])||!r.Read(base+caches[i]+8,ctor)||!cfgprobe::MatchingClassCache(families[i],ctor))return 5;}
 EscapeList levels;if(!r.Read(context.world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(128)||!levels.count)return 5;
 unsigned examined=0,censusFailures=0,boundaryFunctions=0,boundaryDispatchers=0;std::array<unsigned,3> counts{};std::array<bool,3> dumped{};
 std::uintptr_t actorNative=0;if(!r.Read(base+cfgchallengeescape::disk::Actor,actorNative))return 6;
 std::vector<std::uintptr_t> sceneObjects;
 for(int l=0;l<levels.count;++l){
  std::uintptr_t level=0;EscapeList actors;
  if(!r.Read(levels.data+8ULL*static_cast<unsigned>(l),level)||!r.Read(level+cfgradarworld::disk::LevelActors,actors)||!actors.Sane(30000))return 6;
  cfgprobe::lifetime::SlotObservation levelSlot;std::uintptr_t levelType=0,defaults=0,owningWorld=0;
  std::uint32_t objectFlags=0,internalFlags=0;
  if(!cfgprobe::lifetime::ReadSlot(r,registry,level,levelSlot)||!r.Read(level+cfgprobe::ObjectClass,levelType)||
     !r.Read(levelType+cfgspawn::ClassDefaultObject,defaults)||!r.Read(level+8,objectFlags)||
     !r.Read(registry.data+24ULL*static_cast<unsigned>(levelSlot.index)+8,internalFlags)||!r.Read(level+cfgkill::disk::OwningWorld,owningWorld))return 6;
  std::printf(" EscapeLevel Object=0x%llX Serial=%d Type=0x%llX Default=0x%llX OwningWorld=0x%llX ObjectFlags=%08X InternalFlags=%08X NativeCallApproved=no\n",
   static_cast<unsigned long long>(level),levelSlot.serial,static_cast<unsigned long long>(levelType),
   static_cast<unsigned long long>(defaults),static_cast<unsigned long long>(owningWorld),objectFlags,internalFlags);
  cfgchallengeescape::actors::LevelLinkProof levelProof;
  const bool levelAccepted=cfgchallengeescape::actors::ObserveLevelLink(r,base,witness,level,levelProof);
  std::printf(" EscapeLevelProductionProof Accepted=%s DiagnosticOnly=yes\n",levelAccepted?"yes":"no");
  for(int a=0;a<actors.count;++a){
   if(++examined>16000)return 6;std::uintptr_t actor=0,klass=0,outer=0;
   const auto cell=actors.data+8ULL*static_cast<unsigned>(a);
   if(!r.Read(cell,actor))return 6;if(!actor)continue;
   if(!r.Read(actor+cfgprobe::ObjectClass,klass)||!cfgprobe::ObjectPointer(klass))return 6;
   unsigned rejected=0;cfgprobe::lifetime::SlotObservation censusSlot;cfgprobe::Ancestry ancestry;
   std::uint32_t censusFlags=0,internal=0;std::uint8_t role=0;std::uintptr_t cellOuter=0;
   if(std::find(sceneObjects.begin(),sceneObjects.end(),actor)!=sceneObjects.end())rejected=1;
   else if(!cfgprobe::lifetime::ReadSlot(r,registry,actor,censusSlot))rejected=2;
   else if(!cfgprobe::CaptureAncestry(r,actor,ancestry)||!ancestry.Contains(actorNative))rejected=3;
   else if(!r.Read(actor+0x110,role)||!r.Read(actor+8,censusFlags)||(censusFlags&0x30u))rejected=4;
   else if(!r.Read(actor+0x20,cellOuter)||cellOuter!=level||!cfgprobe::StableAncestry(r,actor,ancestry))rejected=5;
   else if(!r.Read(registry.data+24ULL*static_cast<unsigned>(censusSlot.index)+8,internal)||(internal&0x30000000u))rejected=6;
   const bool actionCandidate=ancestry.Contains(families[0])||ancestry.Contains(families[1])||ancestry.Contains(families[2]);
   if(!rejected&&actionCandidate&&censusSlot.HasAssignedSerial()&&role!=3)rejected=7;
   if(rejected){++censusFailures;if(censusFailures<=8){cfgnames::Text actorName;if(!names.Object(actor,actorName))return 6;
    std::printf(" EscapeCensusRejected LevelIndex=%d ActorIndex=%d Object=0x%llX Reason=%u Serial=%d Role=%u Flags=%08X Internal=%08X Name=",
     l,a,static_cast<unsigned long long>(actor),rejected,censusSlot.serial,role,censusFlags,internal);Name(actorName);std::puts(" DiagnosticOnly=yes");}}
   sceneObjects.push_back(actor);
   if(boundaryOnly){cfgnames::Text classLabel;if(!names.Object(klass,classLabel))return 6;
    if(classLabel.View()==u"SinglePlayer_Dispatcher_C"){
     if(++boundaryDispatchers!=1||rejected)return 6;
     std::uintptr_t head=0,node=0;std::array<std::uintptr_t,512> walked{},links{};unsigned fields=0;
     if(!r.Read(klass+cfgreflection::disk::Children,head))return 6;node=head;
     constexpr std::array<std::u16string_view,3> wanted{{u"SP_Level_Boundary_Inside",u"SP_Level_Boundary_Outside",u"Restart_Mission_Out_Of_Bounds"}};
     std::array<bool,3> found{};
     while(node){if(fields==walked.size()||!cfgprobe::ObjectPointer(node))return 6;
      for(unsigned i=0;i<fields;++i)if(walked[i]==node)return 6;
      std::uintptr_t next=0;cfgnames::Text name;if(!r.Read(node+cfgreflection::disk::Next,next)||!names.Object(node,name))return 6;
      walked[fields]=node;links[fields++]=next;
      for(unsigned i=0;i<wanted.size();++i)if(name.View()==wanted[i]){if(found[i])return 6;
       cfgreflection::Observation<Reader> reflection(r,base);cfgreflection::Field field;
       auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
       if(!reflection.ReadField(node,klass,field)||field.kind.View()!=u"Function"||
          !cfgmenu::capture::KnownFunction(r,base,field,wanted[i],*evidence))return 6;
       std::printf("BoundaryDispatcher Object=0x%llX Class=0x%llX FunctionIndex=%u DiagnosticOnly=yes\n",static_cast<unsigned long long>(actor),static_cast<unsigned long long>(klass),i);
       Function(*evidence);found[i]=true;++boundaryFunctions;
      }
      node=next;
     }
     if(!cfgchallengeescape::actors::Same(r,klass+cfgreflection::disk::Children,head))return 7;
     for(unsigned i=0;i<fields;++i)if(!cfgchallengeescape::actors::Same(r,walked[i]+cfgreflection::disk::Next,links[i]))return 7;
    }
    continue;
   }
   for(unsigned kind=0;kind<families.size();++kind){
    if(!cfgmenu::maps::Subclass(r,klass,families[kind]))continue;
    if(++counts[kind]>48)return 6;cfgprobe::lifetime::SlotObservation slot,fresh;
    if(!cfgprobe::lifetime::ReadSlot(r,registry,actor,slot)||!r.Read(actor+0x20,outer)||outer!=level||!names.Object(actor,label))return 6;
    std::printf("EscapeActor Kind=%u Object=0x%llX Level=0x%llX Serial=%d Name=",kind,static_cast<unsigned long long>(actor),static_cast<unsigned long long>(level),slot.serial);Name(label);std::puts(" DiagnosticOnly=yes");
    cfgchallengeescape::actors::Membership member;
    const bool membership=cfgchallengeescape::actors::ObserveMembership(r,base,witness,actor,member);
    bool familyProof=false,ownership=false,members=false;
    if(kind==0){cfgchallengeescape::actors::VolumeProof proof;
     familyProof=cfgchallengeescape::actors::ObserveVolume(r,base,witness,actorCurrent,actor,proof);
    }else if(kind==2){cfgchallengeescape::actors::CounselorProof proof;cfgchallengeescape::actors::CounselorOwnershipProof own;
     familyProof=cfgchallengeescape::actors::ObserveCounselor(r,base,witness,actorCurrent,actor,proof);
     ownership=cfgchallengeescape::actors::ObserveCounselorOwnership(r,base,witness,actorCurrent,actor,own);
     cfgchallengeescape::actors::NativeProof nativeProof;std::array<cfgchallengeescape::actors::Member,8> fields{};
     members=cfgchallengeescape::actors::ObserveNative(r,base,witness,actor,cfgchallengeescape::disk::SCCounselorCharacter,
      cfgchallengeescape::disk::SCCounselorCharacterCtor,nativeProof)&&
      cfgchallengeescape::actors::ObserveMembers(r,base,nativeProof,cfgchallengeescape::actors::CounselorSpecs,fields);
    }
    std::printf(" EscapeProductionProof Kind=%u Membership=%s Family=%s Ownership=%s Members=%s Reads=%u DiagnosticOnly=yes\n",
     kind,membership?"yes":"no",familyProof?"yes":"no",ownership?"yes":"no",members?"yes":"no",r.reads);
    if(!dumped[kind]){if(!EscapeProperties(r,base,actor))return 6;dumped[kind]=true;}
    std::uintptr_t again=0;if(!r.Read(cell,again)||again!=actor||!cfgprobe::lifetime::ReadSlot(r,registry,actor,fresh)||!(fresh==slot))return 7;
   }
  }
  EscapeList again;if(!r.Read(level+cfgradarworld::disk::LevelActors,again)||!(again==actors))return 7;
 }
 Context fresh;EscapeList freshLevels;std::uintptr_t again=0;cfgnames::Words freshPhase{};
 if(!ReadContext(r,base,fresh)||!(fresh==context)||!r.Read(context.world+cfgradarworld::disk::WorldLevels,freshLevels)||!(freshLevels==levels)||
    !r.Read(context.world+cfgprobe::WorldGameMode,again)||again!=mode||!r.Read(context.world+cfgprobe::WorldGameState,again)||again!=state||
    !r.Read(base+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].reflectionCacheRva,again)||again!=native||
    !r.Read(base+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].constructorCacheRva,again)||again!=native||
    !r.Read(mode+cfgprobe::ObjectClass,again)||again!=native||!r.Read(mode+0x3F8,freshPhase)||!(freshPhase==phase)||
    !cfgprobe::lifetime::RevalidateBatch(r,base+cfgprobe::lifetime::ObjectArrayRva,registry,owners)||
    !r.Read(players,again)||again!=player||!r.Read(player+cfgprobe::PlayerController,again)||again!=controller||
    !ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
 for(unsigned i=0;i<caches.size();++i)if(!r.Read(base+caches[i],again)||again!=families[i]||!r.Read(base+caches[i]+8,again)||again!=families[i])return 7;
 std::printf("EscapeDiagnosticStable=yes Examined=%u Volumes=%u Vehicles=%u Counselors=%u CensusFailures=%u\n",examined,counts[0],counts[1],counts[2],censusFailures);
 if(boundaryOnly){std::printf("BoundaryDispatchers=%u BoundaryFunctions=%u BoundaryDisableImplemented=no NoScriptWrites=yes\n",boundaryDispatchers,boundaryFunctions);
  return boundaryDispatchers==1&&boundaryFunctions==3?0:8;}
 return counts[0]&&counts[1]&&counts[2]?0:8;
}
}
int wmain(int argc,wchar_t** argv){
 if(argc==2&&!std::wcscmp(argv[1],L"--self-test")){
  unsigned failures=0;
  if(EscapeList{0,1,1}.Sane(128)||EscapeList{0,-1,1}.Sane(128)||EscapeList{0x100000,129,129}.Sane(128)||
     EscapeList{0x100000,1,0}.Sane(128)||!EscapeList{0x100000,1,1}.Sane(128)||!EscapeList{}.Sane(128))++failures;
  std::printf("Challenge escape read-only capture bounds: %u failures. No game access.\n",failures);return failures?1:0;
 }
 const bool boundaryOnly=argc==2&&!std::wcscmp(argv[1],L"--capture-boundary");
 if(argc!=2||(!boundaryOnly&&std::wcscmp(argv[1],L"--capture"))){std::puts("Use --capture or --capture-boundary (paused current native Challenge), or --self-test. Never launches or modifies game.");return 1;}
 Handle file(CreateFileW(Path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
 if(!Hash(file.value))return 3;const DWORD pid=Find();if(!pid){std::puts("Exactly one fixed-path CFG required; no launch.");return 2;}
 Handle process(OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid));if(!process.value)return 4;
 Reader r(process.value);r.cap=524288;std::uintptr_t base=0;if(!Module(process.value,r,base))return 3;
 std::printf("READONLY CHALLENGE ESCAPE CAPTURE PID=%lu ReadLimit=%u RegistryDiscoveryPasses=0\n",pid,r.cap);
 const auto result=EscapeCapture(process.value,base,r,boundaryOnly);
 std::printf("Result=%d Reads=%u BudgetHit=%s NoWritesHooksLoadsOrNativeCalls=yes PreventionImplemented=no\n",result,r.reads,r.exhausted?"yes":"no");return result;
}
