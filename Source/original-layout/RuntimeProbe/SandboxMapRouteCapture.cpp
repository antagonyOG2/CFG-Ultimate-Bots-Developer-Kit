// Reuse fixed-path/hash/read-only primitives; never dispatch legacy capture.
#define wmain CfgLegacyFrontendCaptureEntry
#include "FrontendMenuCapture.cpp"
#undef wmain
#include "MenuPush.generated.hpp"
#include "MenuPanelChildrenObservation.hpp"
#include "OfflineEntrySettingsObservation.hpp"
#include "NativePickerCapturePolicy.hpp"
#include "MenuGraphSnapshot.hpp"
#include "MenuStackParentObservation.hpp"
#include "JasonPickerObservation.hpp"
#include "WorldMapBindings.generated.hpp"
#include "ChallengesCapturePolicy.hpp"
#include "Compatibility/CfgBuildProfile.hpp"
#include "ConvergenceBindings.generated.hpp"
namespace {
constexpr unsigned MapRouteReadLimit=32768,MapRouteFunctionLimit=16;
bool ActualSandboxMapScreen(std::u16string_view name){return name==u"PickSandboxMapMenuWidget_C";}
bool ActualOfflineEntryScreen(std::u16string_view name){return name==u"OfflinePlayMenuWidget_C";}
template<std::size_t Fields,std::size_t Bytes>void ReferenceCandidates(Reader&,std::uintptr_t,const cfgmenu::capture::FunctionEvidenceStorage<Fields,Bytes>&);
bool ChallengeOwnerCapture(Reader& r,std::uintptr_t base,std::uintptr_t owner,unsigned& functions,
    cfgreflection::Property* retained=nullptr,std::u16string_view wanted={},bool allFunctions=false){
    // Diagnostic ONLY: unknown asset schemas are not native call authority.
    // Walk one positively sourced class, never the global object registry.
    cfgreflection::Observation<Reader> reflection(r,base);std::uintptr_t head=0;
    auto fields=std::make_unique<std::array<cfgreflection::Field,512>>();unsigned count=0;
    if(!r.Read(owner+cfgreflection::disk::Children,head))return false;
    for(auto at=head;at;){
        if(count==fields->size())return false;
        for(unsigned i=0;i<count;++i)if((*fields)[i].object==at)return false;
        auto& field=(*fields)[count++];if(!reflection.ReadField(at,owner,field))return false;
        if(field.kind.View()!=u"Function"){
            cfgreflection::Property property;if(!cfgmenu::capture::RawProperty(r,field,property))return false;
            Property("ChallengeOwnedProperty",property);
            if(retained&&property.field.label.View()==wanted)*retained=property;
        }else if(allFunctions||cfgmenu::challengecapture::Function(field.label.View())){
            if(functions>=32)return false;
            auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
            if(!cfgmenu::capture::KnownFunction(r,base,field,field.label.View(),*evidence))return false;
            Function(*evidence);ReferenceCandidates(r,base,*evidence);++functions;
            // The actual lobby graph's captured 0x7C8 native BeginChallenge
            // operand is identified by its live typed owner, not the old PID.
            if(field.label.View()==u"ExecuteUbergraph_SP_LobbyWidget"&&evidence->script.count>0x7D0&&evidence->bytes[0x7C8]==0xF5){
                std::uintptr_t target=0,native=0;std::memcpy(&target,evidence->bytes.data()+0x7C9,8);cfgreflection::Field consumer;
                if(!cfgprobe::ObjectPointer(target)||!r.Read(target+0x20,native)||!cfgprobe::ObjectPointer(native)||!reflection.ReadField(target,native,consumer)||
                   consumer.label.View()!=u"BeginChallenge"||consumer.kind.View()!=u"Function")return false;
                auto nativeEvidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
                if(!cfgmenu::capture::KnownFunction(r,base,consumer,u"BeginChallenge",*nativeEvidence))return false;
                std::puts("ActualNativeBeginChallengeSchema=observed DiagnosticOnly=yes");Function(*nativeEvidence);
            }
        }
        at=field.next;
    }
    std::uintptr_t again=0;if(!r.Read(owner+cfgreflection::disk::Children,again)||again!=head)return false;
    for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& saved=(*fields)[i];
        if(!reflection.ReadField(saved.object,owner,fresh)||fresh.type!=saved.type||fresh.next!=saved.next||!(fresh.name==saved.name))return false;}
    std::printf("ChallengeOwnedFields=%u OwnedFunctions=%u ClassHeadStable=yes\n",count,functions);return true;
}
bool ChallengeObjectMetadata(Reader& r,std::uintptr_t base,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t object,unsigned& functions,cfgreflection::Property* retained=nullptr,std::u16string_view wanted={},bool diagnosticUnassigned=false){
    std::uintptr_t type=0,again=0;cfgprobe::lifetime::SlotObservation identity,fresh;
    if(!cfgprobe::ObjectPointer(object)||!r.Read(object+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type)||
       !cfgprobe::lifetime::ReadSlot(r,registry,object,identity))return false;
    std::printf("ChallengeMetadataObject=0x%llX Type=0x%llX Serial=%d\n",
        static_cast<unsigned long long>(object),static_cast<unsigned long long>(type),identity.serial);
    if(!identity.HasAssignedSerial()){
        std::puts("ChallengeMetadataSerialUnassigned=yes NativeCallAuthority=no SerialAllocatorCalled=no");
        if(!diagnosticUnassigned)return false;
    }
    return ChallengeOwnerCapture(r,base,type,functions,retained,wanted)&&
        r.Read(object+cfgprobe::ObjectClass,again)&&again==type&&
        cfgprobe::lifetime::ReadSlot(r,registry,object,fresh)&&fresh==identity;
}
bool NamedOffset(Reader& r,std::uintptr_t base,std::uintptr_t type,std::u16string_view label,std::u16string_view kind,int bytes,unsigned& offset){
    cfgreflection::Observation<Reader> reflection(r,base);cfgreflection::Field field;cfgreflection::Property property;
    if(!reflection.Find(type,label,field)||!cfgmenu::capture::RawProperty(r,field,property)||field.kind.View()!=kind||
       property.dimension!=1||property.elementBytes!=bytes||property.offset<0||property.offset>65520||property.offset%8)return false;
    Property("RouteNamedField",property);offset=static_cast<unsigned>(property.offset);return true;
}
template<std::size_t Fields,std::size_t Bytes> void ReferenceCandidates(Reader& r,std::uintptr_t base,const cfgmenu::capture::FunctionEvidenceStorage<Fields,Bytes>& evidence){
    cfgnames::Observation<Reader> names(r,base);unsigned seen=0;
    // Bounded diagnostic candidates only, NOT an instruction decoder or hook
    // approval. Resolve embedded direct-call/name operands while process lives.
    for(unsigned i=0;i<static_cast<unsigned>(evidence.script.count)&&seen<64;++i){
        const auto* b=evidence.bytes.data()+i;const unsigned remaining=static_cast<unsigned>(evidence.script.count)-i;
        if(b[0]==0xEE&&remaining>=13){std::int32_t index=0,copy=0,number=0;
            std::memcpy(&index,b+1,4);std::memcpy(&copy,b+5,4);std::memcpy(&number,b+9,4);
            cfgnames::Text text;if(index!=copy||index<0||number<0||!names.Decode({index,number},text))continue;
            std::printf("ScriptNameCandidate Offset=0x%X Index=%d Number=%d Name=",i,index,number);Name(text);std::puts(" CallApproved=no");++seen;
        }else if((b[0]==0xF5||b[0]==0x89||b[0]==0xC6)&&remaining>=9){std::uintptr_t object=0;std::memcpy(&object,b+1,8);
            cfgnames::Text text,kind;std::uintptr_t type=0;
            if(!cfgprobe::ObjectPointer(object)||!r.Read(object+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type)||
               !names.Object(object,text)||!names.Object(type,kind))continue;
            std::printf("ScriptObjectCandidate Offset=0x%X Opcode=%02X Object=0x%llX Name=",i,b[0],static_cast<unsigned long long>(object));Name(text);
            std::printf(" Kind=");Name(kind);std::puts(" CallApproved=no");++seen;
        }
    }
}
bool EntryOwnerCapture(Reader& r,const cfgmenu::active::Context& c,std::uintptr_t object,std::uintptr_t owner,unsigned& functions){
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgnames::Observation<Reader> names(r,c.module);
    cfgnames::Text name;std::uintptr_t head=0;std::int32_t storage=0;
    if(!names.Object(owner,name)||!r.Read(owner+cfgreflection::disk::Children,head)||
       !r.Read(owner+cfgreflection::disk::StorageBytes,storage)||storage<0||storage>65536)return false;
    std::printf("EntryOwnedClass=0x%llX Object=0x%llX Name=",static_cast<unsigned long long>(owner),static_cast<unsigned long long>(object));Name(name);std::puts("");
    std::array<cfgreflection::Field,128> fields{};unsigned count=0;
    for(auto child=head;child;){
        if(count==fields.size())return false;for(unsigned i=0;i<count;++i)if(fields[i].object==child)return false;
        auto& field=fields[count++];if(!reflection.ReadField(child,owner,field))return false;
        std::printf("EntryOwnedField=0x%llX Name=",static_cast<unsigned long long>(child));Name(field.label);std::printf(" Kind=");Name(field.kind);std::puts("");
        if(field.kind.View()==u"Function"){
            if(functions==64)return false;auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
            if(!cfgmenu::capture::KnownFunction(r,c.module,field,field.label.View(),*evidence))return false;
            Function(*evidence);ReferenceCandidates(r,c.module,*evidence);cfgmenu::startbinding::FastCall fast;
            if(!cfgmenu::startbinding::ReadFastCall(r,field.object,fast))return false;
            std::printf("FastGraph=0x%llX FastEntry=%d\n",static_cast<unsigned long long>(fast.graph),fast.entry);++functions;
        }else if(object&&(field.kind.View()==u"ClassProperty"||field.kind.View()==u"ObjectProperty")){
            cfgreflection::Property property;std::uintptr_t value=0;
            if(!cfgmenu::capture::RawProperty(r,field,property)||property.dimension!=1||property.elementBytes!=8||
               property.offset%8||property.offset>storage-8||!r.Read(object+static_cast<unsigned>(property.offset),value))return false;
            Property("EntryPointerField",property);std::printf("EntryPointerValue=0x%llX",static_cast<unsigned long long>(value));
            if(value){std::uintptr_t type=0;cfgnames::Text valueName,typeName;cfgprobe::lifetime::SlotObservation slot;
                if(!r.Read(value+cfgprobe::ObjectClass,type)||!names.Object(value,valueName)||!names.Object(type,typeName)||
                   !cfgprobe::lifetime::ReadSlot(r,c.registry,value,slot))return false;
                std::printf(" Name=");Name(valueName);std::printf(" Class=");Name(typeName);
                std::printf(" Serial=%d SerialAssigned=%s",slot.serial,slot.HasAssignedSerial()?"yes":"no");
            }
            std::puts(" CallApproved=no");
            std::uintptr_t again=0;if(!r.Read(object+static_cast<unsigned>(property.offset),again)||again!=value)return false;
        }
        child=field.next;
    }
    std::uintptr_t final=0;if(!r.Read(owner+cfgreflection::disk::Children,final)||final!=head)return false;
    for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];
        if(!reflection.ReadField(old.object,owner,fresh)||fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return false;
    }
    return true;
}
int OfflineEntryCapture(HANDLE process,std::uintptr_t base,Reader& r){
    cfgmenu::active::Context c;if(!cfgmenu::active::Observe(r,base,c))return 5;
    cfgnames::Observation<Reader> names(r,base);cfgnames::Text label;
    if(!names.Object(c.type,label)||!ActualOfflineEntryScreen(label.View())){
        std::puts("OfflineEntry=REFUSED_ACTUAL_SCREEN_MISMATCH NoWrites=yes");return 5;
    }
    std::printf("ActiveMenuClass=");Name(label);std::puts("");
    auto resources=std::make_unique<cfgmenu::entrysettings::Resources>();unsigned reads=0,stage=0;
    const bool loaded=cfgmenu::entrysettings::Observe(r,c,c.thread,true,*resources,&reads,&stage);
    std::printf("AuthoredSettingsChainObserved=%s Stage=%u ReadCalls=%u NativeCallsApproved=no\n",loaded?"yes":"no",stage,reads);
    if(loaded){
        std::printf("AuthoredMapClass=0x%llX Serial=%d AuthoredSettingsClass=0x%llX Serial=%d\n",
            static_cast<unsigned long long>(resources->map),resources->mapIdentity.serial,
            static_cast<unsigned long long>(resources->settings),resources->settingsIdentity.serial);
        Property("AuthoredPendingSettingsField",resources->pending);
    }
    // Capture actual entry members/click wrappers even if native cache warming
    // hasn't completed. Failure of the resource proof is not an absent button.
    unsigned functions=0;if(!EntryOwnerCapture(r,c,c.widget,c.type,functions))return 6;
    cfgreflection::Observation<Reader> reflection(r,base);std::uintptr_t head=0;std::int32_t storage=0;
    if(!r.Read(c.type+cfgreflection::disk::Children,head)||!r.Read(c.type+cfgreflection::disk::StorageBytes,storage)||storage<8||storage>65536)return 6;
    std::array<std::uintptr_t,16> types{};unsigned typeCount=0,fieldCount=0;
    for(auto child=head;child;){
        cfgreflection::Field field;cfgreflection::Property property;
        if(++fieldCount>128||!reflection.ReadField(child,c.type,field))return 6;child=field.next;
        if(field.kind.View()!=u"ObjectProperty")continue;
        std::uintptr_t value=0,type=0;
        if(!cfgmenu::capture::RawProperty(r,field,property)||property.dimension!=1||property.elementBytes!=8||property.offset%8||property.offset>storage-8||
           !r.Read(c.widget+static_cast<unsigned>(property.offset),value))return 6;
        if(!value)continue;if(!r.Read(value+cfgprobe::ObjectClass,type)||!names.Object(type,label))return 6;
        // Actual native entry evidence now identifies the original button class.
        // Chat/action-bar/header graphs do not resolve the counselor entry and
        // must not consume its bounded capture budget.
        if(label.View()!=u"WideMenuTextButton_C")continue;
        bool seen=false;for(unsigned i=0;i<typeCount;++i)seen|=types[i]==type;if(seen)continue;
        if(typeCount==types.size())return 6;types[typeCount++]=type;
        if(!EntryOwnerCapture(r,c,value,type,functions))return 6;
    }
    if(!cfgmenu::active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    std::printf("EntryOwnedFunctions=%u EntryDirectWidgetTypes=%u CaptureSampleStable=yes NoGlobalObjectScan=yes\n",functions,typeCount);
    std::puts("DedicatedCounselorClickInstalled=no FullDonorMenuParity=no GameplayTestReady=no");return 0;
}
#include "NativePickerCaptureRuntime.inl"
#include "NativeMenuRootCapture.inl"
int JasonRouteReferencesCapture(HANDLE process,std::uintptr_t base,Reader& r){
    using namespace cfgmenu;active::Context c;if(!active::Observe(r,base,c))return 5;
    cfgnames::Observation<Reader> names(r,base);cfgnames::Text label;
    if(!names.Object(c.type,label)||label.View()!=u"Jason_Select_Widget_C"){
        std::puts("JasonRouteReferencesRefused Stage=ActualJasonScreen");return 5;}
    cfgreflection::Observation<Reader> reflection(r,base);cfgreflection::Field field;
    auto graph=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
    if(!reflection.Find(c.type,u"ExecuteUbergraph_Jason_Select_Widget",field)||!capture::Graph(r,base,field,*graph,diagnostic)){
        std::printf("JasonRouteReferencesRefused Stage=%s\n",diagnostic.stage);return 6;}
    std::array<jasonpicker::Reference,3> refs;unsigned reads=0;
    if(!jasonpicker::LateGraphReferences(r,c,*graph,c.thread,true,refs,&reads)){
        std::printf("JasonRouteReferencesRefused Stage=NativeLateReferences Reads=%u\n",reads);return 6;}
    std::printf("JasonRouteSource Class=0x%llX Widget=0x%llX WidgetSerial=%d Graph=0x%llX ScriptBytes=%d StorageBytes=%d\n",
        static_cast<unsigned long long>(c.type),static_cast<unsigned long long>(c.widget),c.identities[2].serial,
        static_cast<unsigned long long>(field.object),graph->script.count,graph->storageBytes);
    for(unsigned i=0;i<refs.size();++i){const auto& ref=refs[i];
        std::printf("JasonLateNativeClass ConstantOffset=0x%X Class=0x%llX Name=",jasonpicker::DestinationConstants[i],static_cast<unsigned long long>(ref.type));Name(ref.name);
        std::printf(" Serial=%d DestinationKind=%u ScriptAndNativeTransitionRevalidated=yes CallApproved=no\n",ref.identity.serial,static_cast<unsigned>(ref.destination));}
    if(!active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    std::puts("JasonLateRouteCaptureStable=yes NoRepeatedCosmeticOrSelectedRowCapture=yes NoLoadOrCallAuthority=yes");return 0;
}
bool MapButtonCapture(Reader& r,const cfgmenu::active::Context& c){
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgnames::Observation<Reader> names(r,c.module);
    std::uintptr_t panel=0,panelType=0,firstSlot=0,slotType=0;unsigned slotsOffset=0,parentOffset=0,contentOffset=0;
    cfgmenu::panelcapture::Array header;
    if(!reflection.ReadObjectMember(c.widget,c.type,u"MapListVerticalBox",panel)||!panel||
       !r.Read(panel+cfgprobe::ObjectClass,panelType)||!NamedOffset(r,c.module,panelType,u"Slots",u"ArrayProperty",16,slotsOffset)||
       !r.Read(panel+slotsOffset,header)||!header.Valid()||!r.Read(header.data,firstSlot)||!cfgprobe::ObjectPointer(firstSlot)||
       !r.Read(firstSlot+cfgprobe::ObjectClass,slotType)||!NamedOffset(r,c.module,slotType,u"Parent",u"ObjectProperty",8,parentOffset)||
       !NamedOffset(r,c.module,slotType,u"Content",u"ObjectProperty",8,contentOffset))return false;
    cfgmenu::panelcapture::Snapshot snapshot;
    if(!cfgmenu::panelcapture::Observe(r,panel,slotsOffset,parentOffset,contentOffset,snapshot))return false;
    std::printf("ActualMapButtonCount=%d ReservedSlotCapacity=%d NoReservedCapacityReads=yes\n",snapshot.slots.count,snapshot.slots.capacity);
    std::array<std::uintptr_t,8> types{};unsigned typeCount=0,totalFunctions=0;
    for(int index=0;index<snapshot.slots.count;++index){const auto& button=snapshot.children[static_cast<unsigned>(index)];cfgnames::Text name;
        if(!names.Object(button.type,name))return false;
        std::printf("ActualMapButton Index=%d Widget=0x%llX Type=0x%llX Class=",index,static_cast<unsigned long long>(button.widget),static_cast<unsigned long long>(button.type));Name(name);std::puts("");
        bool done=false;for(unsigned i=0;i<typeCount;++i)if(types[i]==button.type)done=true;if(done)continue;
        auto type=button.type;std::array<std::uintptr_t,8> parents{};unsigned parentCount=0;
        while(type){
            if(parentCount==parents.size())return false;for(unsigned i=0;i<parentCount;++i)if(parents[i]==type)return false;
            parents[parentCount++]=type;if(!names.Object(type,name))return false;
            // Native parent bindings already have named disk inventory. Capture
            // the cooked Blueprint handler/parent graphs, not all engine APIs.
            if(name.View().size()<2||name.View().substr(name.View().size()-2)!=u"_C")break;
            bool already=false;for(unsigned i=0;i<typeCount;++i)if(types[i]==type)already=true;if(already)break;
            if(typeCount==types.size())return false;types[typeCount++]=type;
            std::printf("MapButtonBlueprintOwner=0x%llX Name=",static_cast<unsigned long long>(type));Name(name);std::puts("");
            std::uintptr_t head=0,child=0,parent=0;if(!r.Read(type+cfgreflection::disk::Children,head)||!r.Read(type+cfgprobe::ClassSuper,parent))return false;child=head;
            std::array<cfgreflection::Field,128> fields{};unsigned fieldCount=0;
            while(child){if(fieldCount==fields.size())return false;for(unsigned i=0;i<fieldCount;++i)if(fields[i].object==child)return false;
                auto& field=fields[fieldCount++];if(!reflection.ReadField(child,type,field))return false;
                std::printf("MapButtonField=0x%llX Name=",static_cast<unsigned long long>(child));Name(field.label);std::printf(" Kind=");Name(field.kind);std::puts("");
                if(field.kind.View()==u"Function"){
                    if(totalFunctions==32)return false;auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
                    if(!cfgmenu::capture::KnownFunction(r,c.module,field,field.label.View(),*evidence))return false;
                    Function(*evidence);cfgmenu::startbinding::FastCall fast;if(!cfgmenu::startbinding::ReadFastCall(r,field.object,fast))return false;
                    std::printf("FastGraph=0x%llX FastEntry=%d\n",static_cast<unsigned long long>(fast.graph),fast.entry);ReferenceCandidates(r,c.module,*evidence);++totalFunctions;
                }else if(field.kind.View()==u"ClassProperty"||field.kind.View()==u"ObjectProperty"){
                    cfgreflection::Property property;std::int32_t storage=0;std::uintptr_t value=0;
                    if(!cfgmenu::capture::RawProperty(r,field,property)||!r.Read(button.type+cfgreflection::disk::StorageBytes,storage)||
                       property.dimension!=1||property.elementBytes!=8||property.offset<0||property.offset%8||storage<8||property.offset>storage-8||
                       !r.Read(button.widget+static_cast<unsigned>(property.offset),value))return false;
                    Property("MapButtonPointerField",property);
                    std::printf("MapButtonPointerValue=0x%llX",static_cast<unsigned long long>(value));
                    if(value){cfgnames::Text text;if(!cfgprobe::ObjectPointer(value)||!names.Object(value,text))return false;std::printf(" Name=");Name(text);}std::puts("");
                }
                child=field.next;
            }
            std::uintptr_t again=0;if(!r.Read(type+cfgreflection::disk::Children,again)||again!=head||!r.Read(type+cfgprobe::ClassSuper,again)||again!=parent)return false;
            for(unsigned i=0;i<fieldCount;++i){cfgreflection::Field fresh;if(!reflection.ReadField(fields[i].object,type,fresh)||fresh.type!=fields[i].type||fresh.next!=fields[i].next||!(fresh.name==fields[i].name))return false;}
            type=parent;
        }
    }
    cfgmenu::panelcapture::Snapshot fresh;if(!cfgmenu::panelcapture::Observe(r,panel,slotsOffset,parentOffset,contentOffset,fresh)||!(fresh.slots==snapshot.slots))return false;
    for(int i=0;i<snapshot.slots.count;++i)if(!(fresh.children[static_cast<unsigned>(i)]==snapshot.children[static_cast<unsigned>(i)]))return false;
    std::printf("ActualMapButtonTypes=%u ActualMapButtonFunctions=%u CurrentPanelStable=yes\n",typeCount,totalFunctions);return totalFunctions!=0;
}
int ChallengeControlsCapture(HANDLE process,std::uintptr_t base,Reader& r){
    cfgmenu::active::Context c;cfgnames::Text label;cfgnames::Observation<Reader> names(r,base);
    cfgreflection::Observation<Reader> reflection(r,base);std::uintptr_t donor=0,donorType=0,slot=0,slotType=0,panel=0,panelType=0;
    unsigned parentOffset=0,contentOffset=0,slotsOffset=0;
    if(!cfgmenu::active::Observe(r,base,c)||!names.Object(c.type,label)||label.View()!=u"SPChallengesSettingsMenuWidget_C"||
       !reflection.ReadObjectMember(c.widget,c.type,u"DifficultyComboBox",donor)||!cfgprobe::ObjectPointer(donor)||
       !r.Read(donor+cfgprobe::ObjectClass,donorType)||!names.Object(donorType,label)||label.View()!=u"SettingsOptionComboBoxWidget_C"||
       !reflection.ReadObjectMember(donor,donorType,u"Slot",slot)||!cfgprobe::ObjectPointer(slot)||!r.Read(slot+cfgprobe::ObjectClass,slotType)||
       !NamedOffset(r,base,slotType,u"Parent",u"ObjectProperty",8,parentOffset)||!r.Read(slot+parentOffset,panel)||!cfgprobe::ObjectPointer(panel)||
       !NamedOffset(r,base,slotType,u"Content",u"ObjectProperty",8,contentOffset)||!r.Read(panel+cfgprobe::ObjectClass,panelType)||
       !NamedOffset(r,base,panelType,u"Slots",u"ArrayProperty",16,slotsOffset))return 5;
    cfgmenu::panelcapture::Snapshot snapshot;if(!cfgmenu::panelcapture::Observe(r,panel,slotsOffset,parentOffset,contentOffset,snapshot))return 6;
    unsigned functions=0;if(!ChallengeOwnerCapture(r,base,donorType,functions,nullptr,{},true))return 6;
    cfgreflection::Field indexField;cfgreflection::Property index;
    if(!reflection.Find(donorType,u"CurrentOptionIndex",indexField)||!cfgmenu::capture::RawProperty(r,indexField,index)||
       index.field.kind.View()!=u"IntProperty"||index.elementBytes!=4||index.dimension!=1||index.offset<0||index.offset>65520)return 6;
    std::printf("ChallengeControlsRows=%d AuthoredFocusRow=0x%llX DiagnosticOnly=yes\n",snapshot.slots.count,static_cast<unsigned long long>(donor));
    for(int i=0;i<snapshot.slots.count;++i){const auto& row=snapshot.children[static_cast<unsigned>(i)];
        if(row.type!=donorType)continue;int value=-1,again=-1;
        if(!r.Read(row.widget+static_cast<unsigned>(index.offset),value)||!r.Read(row.widget+static_cast<unsigned>(index.offset),again)||value!=again)return 7;
        std::printf("ChallengeControlsRow=%d Widget=0x%llX Selected=%d Authored=%s\n",i,static_cast<unsigned long long>(row.widget),value,row.widget==donor?"yes":"no");
        // Capture named cooked child pointers, never guessed button offsets or
        // a native input action. Rows may have unassigned serials: no lease/call.
        std::uintptr_t child=0;if(!r.Read(donorType+cfgreflection::disk::Children,child))return 6;
        for(unsigned n=0;child&&n<128;++n){cfgreflection::Field field;
            if(!reflection.ReadField(child,donorType,field))return 6;
            if(field.kind.View()==u"ObjectProperty"){
                cfgreflection::Property property;std::uintptr_t object=0,repeat=0;
                if(!cfgmenu::capture::RawProperty(r,field,property)||property.dimension!=1||property.elementBytes!=8||
                   property.offset<0||property.offset>65520||!r.Read(row.widget+static_cast<unsigned>(property.offset),object)||
                   !r.Read(row.widget+static_cast<unsigned>(property.offset),repeat)||repeat!=object)return 6;
                std::printf("ChallengeControlsChild Field=");Name(field.label);std::printf(" Object=0x%llX NoInputOrCallAuthority=yes\n",static_cast<unsigned long long>(object));
            }
            child=field.next;
        }
        if(child)return 6;
    }
    cfgmenu::panelcapture::Snapshot fresh;
    if(!cfgmenu::panelcapture::Observe(r,panel,slotsOffset,parentOffset,contentOffset,fresh)||!(fresh.slots==snapshot.slots)||
       !cfgmenu::active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    for(int i=0;i<snapshot.slots.count;++i)if(!(fresh.children[static_cast<unsigned>(i)]==snapshot.children[static_cast<unsigned>(i)]))return 7;
    std::puts("ChallengeControlsSampleStable=yes NoWritesNoLoadsNoNativeInput=yes");return 0;
}
int ChallengesCapture(HANDLE process,std::uintptr_t base,Reader& r,bool mission,bool stateOnly=false,bool pawnOnly=false,std::uintptr_t loadedPage=0){
    unsigned functions=0;
    if(!mission){
        cfgmenu::active::Context c;cfgnames::Text screen;cfgnames::Observation<Reader> names(r,base);
        if(!cfgmenu::active::Observe(r,base,c)||!names.Object(c.type,screen)||!cfgmenu::challengecapture::Screen(screen.View())){
            std::puts("ChallengesCapture=REFUSED_ACTUAL_SCREEN NoLoadOrStartAttempted=yes");return 5;}
        std::printf("ActualChallengesScreen=");Name(screen);std::puts("");
        std::printf("ChallengesCurrentContext World=0x%llX Mode=0x%llX Controller=0x%llX HUD=0x%llX Stack=0x%llX Widget=0x%llX Type=0x%llX Depth=%d Capacity=%d Thread=%u\n",
            static_cast<unsigned long long>(c.world),static_cast<unsigned long long>(c.mode),static_cast<unsigned long long>(c.controller),
            static_cast<unsigned long long>(c.hud),static_cast<unsigned long long>(c.stack),static_cast<unsigned long long>(c.widget),
            static_cast<unsigned long long>(c.type),c.array.count,c.array.capacity,c.thread);
        for(int i=0;i<c.array.count;++i){std::uintptr_t widget=0,type=0,recordClass=0;cfgnames::Text label;cfgprobe::lifetime::SlotObservation slot;
            if(!r.Read(c.array.data+static_cast<unsigned>(i)*cfgmenu::execdisk::RecordBytes+cfgmenu::parentdisk::RecordClass,recordClass)||
               !cfgprobe::ObjectPointer(recordClass)||!names.Object(recordClass,label))return 6;
            std::printf("ChallengesStackClass%d Type=0x%llX Class=",i,static_cast<unsigned long long>(recordClass));Name(label);std::puts(" DiagnosticOnly=yes");
            if(!r.Read(c.array.data+static_cast<unsigned>(i)*cfgmenu::execdisk::RecordBytes+cfgmenu::execdisk::RecordWidget,widget))return 6;
            std::printf("ChallengesStackRecord%d Widget=0x%llX",i,static_cast<unsigned long long>(widget));
            if(widget){if(!cfgprobe::ObjectPointer(widget)||!r.Read(widget+cfgprobe::ObjectClass,type)||!names.Object(type,label)||
                    !cfgprobe::lifetime::ReadSlot(r,c.registry,widget,slot))return 6;
                std::printf(" Type=0x%llX Serial=%d Class=",static_cast<unsigned long long>(type),slot.serial);Name(label);}
            std::puts(" DiagnosticOnly=yes");
        }
        if(!cfgmenu::active::Current(r,c,c.thread))return 7;
        if(loadedPage){
            // Address is supplied from the separately hash-verified installed
            // resolver's report. Diagnostic only, never call/creation authority.
            std::uintptr_t type=0,menu=0,ctor=0,outer=0,freshOuter=0;cfgnames::Text pageName,kind,package;
            cfgprobe::lifetime::SlotObservation before,after;
            if(screen.View()!=u"SP_LobbyWidget_C"||!cfgprobe::ObjectPointer(loadedPage)||!names.Object(loadedPage,pageName)||
               pageName.View()!=u"SPChallengesSettingsMenuWidget_C"||!r.Read(loadedPage+cfgprobe::ObjectClass,type)||!names.Object(type,kind)||
               kind.View()!=u"WidgetBlueprintGeneratedClass"||!r.Read(base+0x304CFF0,menu)||!r.Read(base+0x304CFF8,ctor)||
               !cfgprobe::MatchingClassCache(menu,ctor)||!cfgmenu::maps::Subclass(r,loadedPage,menu)||
               !cfgprobe::lifetime::ReadSlot(r,c.registry,loadedPage,before)||!r.Read(loadedPage+0x20,outer)||!names.Object(outer,package)||
               package.View()!=u"/Game/UI/Menu/FrontEnd/OfflinePlay/SPChallengesSettingsMenuWidget")return 5;
            std::printf("ResolverLoadedPageClass=0x%llX Serial=%d Package=",static_cast<unsigned long long>(loadedPage),before.serial);Name(package);std::puts(" DiagnosticOnly=yes");
            if(!ChallengeOwnerCapture(r,base,loadedPage,functions,nullptr,{},true)||!functions||
               !cfgprobe::lifetime::ReadSlot(r,c.registry,loadedPage,after)||!(after==before)||!r.Read(loadedPage+0x20,freshOuter)||freshOuter!=outer||
               !cfgmenu::active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
            std::puts("LoadedChallengesPageSchemaStable=yes NoPageCreatedOrPushed=yes NoNativeCallAuthority=yes");return 0;
        }
        {
            const auto& binding=cfgconvergence::disk::Conv_StringToName;
            std::uintptr_t owner=0,ctor=0,function=0,again=0;
            if(!r.Read(base+binding.ownerReflection,owner)||!r.Read(base+binding.ownerConstructor,ctor)||!r.Read(base+binding.cache,function))return 6;
            std::printf("ChallengeConverterCaches Owner=0x%llX Constructor=0x%llX Function=0x%llX NativeCallApproved=no\n",
                static_cast<unsigned long long>(owner),static_cast<unsigned long long>(ctor),static_cast<unsigned long long>(function));
            if(cfgprobe::MatchingClassCache(owner,ctor)&&cfgprobe::ObjectPointer(function)){
                cfgreflection::Observation<Reader> reflection(r,base);cfgreflection::Field field;
                auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
                if(!reflection.ReadField(function,owner,field)||!cfgmenu::capture::KnownFunction(r,base,field,u"Conv_StringToName",*evidence))return 6;
                Function(*evidence);
            }
            if(!r.Read(base+binding.ownerReflection,again)||again!=owner||!r.Read(base+binding.ownerConstructor,again)||again!=ctor||
               !r.Read(base+binding.cache,again)||again!=function)return 7;
        }
        if(!ChallengeOwnerCapture(r,base,c.type,functions)||!functions||!cfgmenu::active::Current(r,c,c.thread))return 6;
    }else{
        // One current native Challenge world, not a relaxed OfflineBots gate.
        std::uintptr_t engine=0,viewport=0,world=0,mode=0,instance=0,players=0,player=0,controller=0,pawn=0,net=1;
        int count=0,capacity=0;const auto pointer=[&](std::uintptr_t at,std::uintptr_t& value){return r.Read(at,value)&&cfgprobe::ObjectPointer(value);};
        if(!pointer(base+cfgprobe::EngineGlobalCandidate,engine)||!pointer(engine+cfgprobe::EngineGameViewport,viewport)||
           !pointer(viewport+cfgprobe::ViewportWorld,world)||!r.Read(world+cfgprobe::WorldNetDriver,net)||net||
           !pointer(world+cfgprobe::WorldGameMode,mode)||!pointer(world+cfgprobe::WorldGameInstance,instance)||
           !pointer(instance+cfgprobe::GameInstanceLocalPlayers,players)||!r.Read(instance+cfgprobe::GameInstanceLocalPlayers+8,count)||count!=1||
           !r.Read(instance+cfgprobe::GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||!pointer(players,player)||
           !pointer(player+cfgprobe::PlayerController,controller)||!pointer(controller+cfgprobe::ControllerPawn,pawn))return 5;
        cfgprobe::lifetime::ArrayHeader registry;std::array<cfgprobe::lifetime::SlotObservation,4> identities{};
        const std::array<std::uintptr_t,4> objects{{world,mode,controller,pawn}};
        if(!cfgprobe::lifetime::ReadArrayHeader(r,base+cfgprobe::lifetime::ObjectArrayRva,registry))return 5;
        for(unsigned i=0;i<objects.size();++i)if(!cfgprobe::lifetime::ReadSlot(r,registry,objects[i],identities[i])||!identities[i].HasAssignedSerial())return 5;
        std::array<std::uintptr_t,3> native{},types{};const std::array<cfgprobe::NativeClass,3> kinds{{cfgprobe::ChallengesMode,cfgprobe::PlayerControllerClass,cfgprobe::Killer}};
        const std::array<std::uintptr_t,3> instances{{mode,controller,pawn}};
        for(unsigned i=0;i<kinds.size();++i){std::uintptr_t ctor=0;const auto& d=cfgprobe::NativeClasses[kinds[i]];
            if(!r.Read(base+d.reflectionCacheRva,native[i])||!r.Read(base+d.constructorCacheRva,ctor)||!cfgprobe::MatchingClassCache(native[i],ctor)||
               !r.Read(instances[i]+cfgprobe::ObjectClass,types[i])||!cfgmenu::maps::Subclass(r,types[i],native[i]))return 5;}
        if(!stateOnly&&!pawnOnly&&!ChallengeOwnerCapture(r,base,native[0],functions))return 6;
        // Primary Jason property names/offsets are evidence only. No consumer
        // is enabled by this capture, and no actor baseline is modified.
        if(!stateOnly&&!ChallengeOwnerCapture(r,base,native[2],functions))return 6;
        if(stateOnly){
            std::uintptr_t state=0,dossier=0,again=0;cfgreflection::Property dossierField,activeChallengeField;
            if(!pointer(world+cfgprobe::WorldGameState,state)||
               !ChallengeObjectMetadata(r,base,registry,state,functions,&dossierField,u"Dossier"))return 6;
            if(dossierField.field.object){
                if(dossierField.field.kind.View()!=u"ObjectProperty"||dossierField.elementBytes!=8||dossierField.dimension!=1||
                   dossierField.offset<0||dossierField.offset%8||!r.Read(state+static_cast<unsigned>(dossierField.offset),dossier))return 6;
                std::printf("ActualDossierValue=0x%llX Unset=%s\n",static_cast<unsigned long long>(dossier),dossier?"no":"yes");
                if(dossier&&(!cfgprobe::ObjectPointer(dossier)||
                   !ChallengeObjectMetadata(r,base,registry,dossier,functions,&activeChallengeField,u"ActiveChallengeClass",true)||
                   !r.Read(state+static_cast<unsigned>(dossierField.offset),again)||again!=dossier))return 6;
                if(activeChallengeField.field.object)Property("ActualChallengeIdentityField",activeChallengeField);
            }else std::puts("DossierNotDeclaredOnActualStateClass=yes InheritedSchemaStillRequired=yes");
            std::uintptr_t stateBase=0,stateCtor=0,stateType=0;
            if(!r.Read(base+cfgcompat::cfg35::objects::SCGameStateReflection,stateBase)||
               !r.Read(base+cfgcompat::cfg35::objects::SCGameStateConstructor,stateCtor)||!cfgprobe::MatchingClassCache(stateBase,stateCtor)||
               !r.Read(state+cfgprobe::ObjectClass,stateType)||!cfgmenu::maps::Subclass(r,stateType,stateBase)||
               !ChallengeOwnerCapture(r,base,stateBase,functions)||!r.Read(base+cfgcompat::cfg35::objects::SCGameStateReflection,again)||again!=stateBase||
               !r.Read(base+cfgcompat::cfg35::objects::SCGameStateConstructor,again)||again!=stateBase)return 6;
            if(!r.Read(world+cfgprobe::WorldGameState,again)||again!=state)return 7;
            if(dossierField.field.object&&(!r.Read(state+static_cast<unsigned>(dossierField.offset),again)||again!=dossier))return 7;
        }
        if(pawnOnly){
            // Existing CFG native movement member is diagnostic input only;
            // actual class/serial/reciprocal ownership must also agree.
            std::uintptr_t movement=0,owner=0,again=0;
            if(!pointer(pawn+0x3D0,movement)||!r.Read(movement+0x20,owner)||owner!=pawn||
               !ChallengeObjectMetadata(r,base,registry,movement,functions)||
               !r.Read(pawn+0x3D0,again)||again!=movement||!r.Read(movement+0x20,again)||again!=pawn)return 6;
        }
        std::uintptr_t again=0;
        if(!r.Read(viewport+cfgprobe::ViewportWorld,again)||again!=world||!r.Read(world+cfgprobe::WorldNetDriver,again)||again||
           !r.Read(world+cfgprobe::WorldGameMode,again)||again!=mode||!r.Read(player+cfgprobe::PlayerController,again)||again!=controller||
           !r.Read(controller+cfgprobe::ControllerPawn,again)||again!=pawn)return 7;
        for(unsigned i=0;i<objects.size();++i){cfgprobe::lifetime::SlotObservation fresh;
            if(!cfgprobe::lifetime::ReadSlot(r,registry,objects[i],fresh)||!(fresh==identities[i]))return 7;}
        for(unsigned i=0;i<kinds.size();++i){std::uintptr_t ctor=0;const auto& d=cfgprobe::NativeClasses[kinds[i]];
            if(!r.Read(base+d.reflectionCacheRva,again)||again!=native[i]||!r.Read(base+d.constructorCacheRva,ctor)||ctor!=native[i]||
               !r.Read(instances[i]+cfgprobe::ObjectClass,again)||again!=types[i]||!cfgmenu::maps::Subclass(r,types[i],native[i]))return 7;}
        std::puts("ActualNativeChallengeWorld=current_standalone_typed_positive_slots DiagnosticOnly=yes");
    }
    if(!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    std::puts("ChallengesCaptureSampleStable=yes ClassLoading=none ModifiersImplemented=no");return 0;
}
int MapRouteCapture(HANDLE process,std::uintptr_t base,Reader& r){
    cfgmenu::active::Context c;
    if(!cfgmenu::active::Observe(r,base,c))return 5;
    cfgnames::Observation<Reader> names(r,base);cfgnames::Text screen,modeName;
    std::printf("ActiveMenuClass=");if(!names.Object(c.type,screen))return 5;Name(screen);std::puts("");
    // This is ONLY a read-only capture of a real active map menu, not acceptance
    // of its unknown class/handler as a callable UI or gameplay binding.
    std::uintptr_t frontend=0,frontendCtor=0,frontendType=0,menu=0,menuCtor=0,mode=0,settings=0,map=0;
    using namespace cfgmenu::selectiondisk;
    if(!ActualSandboxMapScreen(screen.View())){
        std::puts("SandboxMapRoute=REFUSED_ACTUAL_SCREEN_MISMATCH NoWrites=yes");return 5;
    }
    if(!r.Read(base+SCGame_MenuClass,frontend)||!r.Read(base+SCGame_MenuConstructor,frontendCtor)||
       !r.Read(c.mode+cfgprobe::ObjectClass,frontendType)){
        std::puts("SandboxMapRoute=REFUSED_NATIVE_OWNER_READ NoWrites=yes");return 5;
    }
    const bool frontendOwner=cfgprobe::MatchingClassCache(frontend,frontendCtor)&&cfgmenu::maps::Subclass(r,frontendType,frontend);
    std::printf("NativeFrontendOwnerFamilyObserved=%s NativeCallApproved=no\n",frontendOwner?"yes":"no");
    if(frontendOwner&&(!r.Read(c.mode+PendingOfflineMode,mode)||!r.Read(c.mode+PendingOfflineMap,map)||
       !r.Read(c.mode+PendingOfflineSettingsWidget,settings))){
        std::puts("SandboxMapRoute=REFUSED_PENDING_FIELD_READ NoWrites=yes");return 5;
    }
    if(frontendOwner){std::printf("PendingFieldsObserved=yes PendingMode=0x%llX PendingMap=0x%llX PendingSettingsClass=0x%llX\n",
        static_cast<unsigned long long>(mode),static_cast<unsigned long long>(map),static_cast<unsigned long long>(settings));
    std::printf("PendingModeName=");
    if(!mode)std::puts("<unset>");else if(names.Object(mode,modeName)){Name(modeName);std::puts("");}
    else std::puts("<unreadable> DiagnosticNotCallApproval=yes");
    }else std::puts("PendingFieldsObserved=no NativeOwnerUnproven=yes NotReportedAsUnset=yes");
    // A map picker may not commit pending mode/settings until its selection
    // callback. Observing that callback must not depend on assuming its result.
    std::puts("PendingModeDoesNotGateReadOnlyHandlerCapture=yes");
    cfgreflection::Observation<Reader> reflection(r,base);
    // Known native field descriptors give names to heap operands in captured
    // scripts; no retained old-process pointer is dereferenced or reused.
    if(frontendOwner)for(const auto* label:{u"PendingOfflineMap",u"PendingOfflineMode",u"PendingOfflineSettingsWidget"}){
        cfgreflection::Field field;cfgreflection::Property property;
        if(!reflection.Find(frontend,label,field)||!cfgmenu::capture::RawProperty(r,field,property))return 6;
        Property("NativeFrontendField",property);
    }
    std::array<cfgreflection::Field,128> fields{};unsigned count=0;std::uintptr_t head=0,child=0;
    if(!r.Read(c.type+cfgreflection::disk::Children,head))return 6;child=head;
    while(child){
        if(count==fields.size())return 6;
        for(unsigned i=0;i<count;++i)if(fields[i].object==child)return 6;
        auto& f=fields[count++];if(!reflection.ReadField(child,c.type,f))return 6;
        std::printf("OwnedField=0x%llX Kind=",static_cast<unsigned long long>(child));Name(f.kind);
        std::printf(" Name=");Name(f.label);std::puts("");child=f.next;
    }
    unsigned functions=0;
    for(unsigned i=0;i<count;++i){const auto& field=fields[i];if(field.kind.View()!=u"Function")continue;
        if(functions==MapRouteFunctionLimit){std::puts("OwnedFunctionLimit=refused NoPartialSuccess=yes");return 6;}
        auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
        if(!cfgmenu::capture::KnownFunction(r,base,field,field.label.View(),*evidence))return 6;
        Function(*evidence);ReferenceCandidates(r,base,*evidence);cfgmenu::startbinding::FastCall fast;
        if(!cfgmenu::startbinding::ReadFastCall(r,field.object,fast))return 6;
        std::printf("FastGraph=0x%llX FastEntry=%d\n",static_cast<unsigned long long>(fast.graph),fast.entry);++functions;
    }
    if(!MapButtonCapture(r,c)){std::puts("ActualMapButtonCapture=REFUSED NoPartialSuccess=yes");return 8;}
    std::uintptr_t push=0;
    if(!r.Read(base+cfgmenu::pushdisk::MenuClass,menu)||!r.Read(base+cfgmenu::pushdisk::MenuConstructor,menuCtor)||
       !r.Read(base+cfgmenu::pushdisk::FunctionCache,push))return 6;
    const bool pushOwner=cfgprobe::MatchingClassCache(menu,menuCtor)&&cfgmenu::maps::Subclass(r,c.type,menu);
    std::printf("NativePushOwnerFamilyObserved=%s\n",pushOwner?"yes":"no");
    if(push&&pushOwner){cfgreflection::Field field;auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
        if(!reflection.ReadField(push,menu,field)||!cfgmenu::capture::KnownFunction(r,base,field,u"PushMenu",*evidence))return 6;
        Function(*evidence);std::puts("PushMenuCacheObserved=yes NativeCallApproved=no");
    }else std::puts("PushMenuMetadataUnavailable=yes NoFactoryCalled=yes HandlerCaptureIndependent=yes");
    std::uintptr_t again=0;
    if(!functions||!r.Read(c.type+cfgreflection::disk::Children,again)||again!=head)return 7;
    for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];
        if(!reflection.ReadField(old.object,c.type,fresh)||fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return 7;
    }
    if(!cfgmenu::active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    if(frontendOwner&&(!r.Read(c.mode+PendingOfflineMode,again)||again!=mode||
       !r.Read(c.mode+PendingOfflineMap,again)||again!=map||
       !r.Read(c.mode+PendingOfflineSettingsWidget,again)||again!=settings))return 7;
    std::printf("OwnedMapFunctions=%u CaptureSampleStable=yes NoGlobalObjectScan=yes\n",functions);
    return 0;
}
}
#ifndef CFG_MAP_CAPTURE_ENTRY
#define CFG_MAP_CAPTURE_ENTRY wmain
#endif
int CFG_MAP_CAPTURE_ENTRY(int argc,wchar_t** argv){
    if(argc==2&&!std::wcscmp(argv[1],L"--self-test")){
        if(CfgLegacyFrontendCaptureEntry(argc,argv))return 1;
        Reader r(GetCurrentProcess());r.cap=2;alignas(8) std::uintptr_t value=42,observed=0;
        const auto at=reinterpret_cast<std::uintptr_t>(&value);
        const bool okay=ActualSandboxMapScreen(u"PickSandboxMapMenuWidget_C")&&
            !ActualSandboxMapScreen(u"PickOfflineBotsMapMenuWidget_C")&&!ActualSandboxMapScreen(u"OfflineBotsSettingsMenuWidget_C")&&
            !ActualSandboxMapScreen(u"PauseMenuWidget_C")&&ActualOfflineEntryScreen(u"OfflinePlayMenuWidget_C")&&
            !ActualOfflineEntryScreen(u"OfflineBotsSettingsMenuWidget_C")&&!ActualOfflineEntryScreen(u"PickSandboxMapMenuWidget_C")&&
            cfgmenu::pickercapture::Screen(cfgmenu::pickercapture::Kind::Counselor,u"Counselor_Menu_C")&&
            cfgmenu::pickercapture::Screen(cfgmenu::pickercapture::Kind::Jason,u"Jason_Select_Widget_C")&&
            !cfgmenu::pickercapture::Screen(cfgmenu::pickercapture::Kind::Jason,u"OfflineBotsSettingsMenuWidget_C")&&
            !cfgmenu::pickercapture::Function(u"ChatNotification")&&cfgmenu::pickercapture::Function(u"OnClick_Accept")&&
            cfgmenu::challengecapture::Screen(u"SP_LobbyWidget_C")&&cfgmenu::challengecapture::Screen(u"SPChallengesSettingsMenuWidget_C")&&
            !cfgmenu::challengecapture::Screen(u"OfflineBotsSettingsMenuWidget_C")&&!cfgmenu::challengecapture::Screen(u"PrivateLobbyWidget_C")&&
            cfgmenu::challengecapture::Function(u"OnStartPressed")&&cfgmenu::challengecapture::Function(u"BeginChallenge")&&
            !cfgmenu::challengecapture::Function(u"OnJoinSession")&&
            r.Read(at,observed)&&observed==42&&r.Read(at,observed)&&
            !r.Read(at,observed)&&observed==42&&r.reads==2&&r.exhausted;
        std::printf("Actual Sandbox screen/readonly cap regression: %s. Pending mode not required for diagnostic; no call approval.\n",okay?"PASS":"FAIL");
        return okay?0:1;
    }
    const bool entry=argc==2&&!std::wcscmp(argv[1],L"--capture-offline-entry");
    const bool counselor=argc==2&&!std::wcscmp(argv[1],L"--capture-counselor-picker");
    const bool jason=argc==2&&!std::wcscmp(argv[1],L"--capture-jason-picker");
    const bool root=argc==2&&!std::wcscmp(argv[1],L"--capture-menu-roots");
    const bool route=argc==2&&!std::wcscmp(argv[1],L"--capture-jason-route-references");
    const bool challenges=argc==2&&!std::wcscmp(argv[1],L"--capture-challenges");
    const bool challengeControls=argc==2&&!std::wcscmp(argv[1],L"--capture-challenges-controls");
    const bool challengeMission=argc==2&&!std::wcscmp(argv[1],L"--capture-challenges-mission");
    const bool challengeState=argc==2&&!std::wcscmp(argv[1],L"--capture-challenges-state");
    const bool challengePawn=argc==2&&!std::wcscmp(argv[1],L"--capture-challenges-pawn");
    const bool challengePage=argc==3&&!std::wcscmp(argv[1],L"--capture-challenges-page");std::uintptr_t loadedPage=0;
    if(challengePage){wchar_t* end=nullptr;loadedPage=static_cast<std::uintptr_t>(std::wcstoull(argv[2],&end,16));
        if(!end||*end||!cfgprobe::ObjectPointer(loadedPage))return 1;}
    if(!entry&&!counselor&&!jason&&!root&&!route&&!challenges&&!challengeControls&&!challengeMission&&!challengeState&&!challengePawn&&!challengePage&&(argc!=2||std::wcscmp(argv[1],L"--capture-map-route"))){std::puts("Use --capture-map-route, --capture-offline-entry, --capture-counselor-picker, --capture-jason-picker, --capture-menu-roots, --capture-jason-route-references, --capture-challenges, --capture-challenges-controls, --capture-challenges-mission, --capture-challenges-state, --capture-challenges-pawn, --capture-challenges-page <resolver-class-address> or --self-test. Read-only, fixed CFG path, no game launch.");return 1;}
    Handle file(CreateFileW(Path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if(!Hash(file.value))return 3;const DWORD pid=Find();if(!pid){std::puts("Exactly one fixed-path CFG required; nothing launched.");return 2;}
    Handle process(OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid));
    if(!process.value){std::puts("Read access refused; no escalation or bypass.");return 4;}
    Reader r(process.value);r.cap=MapRouteReadLimit;std::uintptr_t base=0;if(!Module(process.value,r,base))return 3;
    std::printf("READONLY %s HANDLER CAPTURE PID=%lu ReadLimit=%u RegistryDiscoveryPasses=0\n",challenges?"CHALLENGES FRONTEND":challengeState?"CHALLENGES STATE":challengePawn?"CHALLENGES PAWN":challengeMission?"CHALLENGES MISSION":entry?"OFFLINE PLAY ENTRY":counselor?"COUNSELOR PICKER":jason?"JASON PICKER":root?"NATIVE MENU ROOTS":route?"JASON LATE ROUTE REFERENCES":"SANDBOX MAP",pid,r.cap);
    const auto result=challengeControls?ChallengeControlsCapture(process.value,base,r):(challenges||challengeMission||challengeState||challengePawn||challengePage)?ChallengesCapture(process.value,base,r,challengeMission||challengeState||challengePawn,challengeState,challengePawn,loadedPage):route?JasonRouteReferencesCapture(process.value,base,r):root?MenuRootCapture(process.value,base,r):entry?OfflineEntryCapture(process.value,base,r):(counselor||jason)?PickerCapture(process.value,base,r,counselor?cfgmenu::pickercapture::Kind::Counselor:cfgmenu::pickercapture::Kind::Jason):MapRouteCapture(process.value,base,r);
    std::printf("Result=%d ReadCalls=%u BudgetHit=%s\nNoWritesHooksLoadsOrNativeCalls=yes\nGameplayReady=no\n",result,r.reads,r.exhausted?"yes":"no");return result;
}
