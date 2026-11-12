#pragma once
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/MenuClassCache.hpp"
#include "../RuntimeProbe/CounselorPickerCast.generated.hpp"
#include "ObjectivePanelObservation.hpp"
#include "ObjectiveAttachmentObservation.hpp"
#include <cstdio>
#include <memory>
#include <cstring>
#include <io.h>
#include <fcntl.h>
namespace cfgobjectivecapture {
inline FILE* OpenExclusive(const wchar_t* path)noexcept{
    HANDLE file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return nullptr;
    const int descriptor=_open_osfhandle(reinterpret_cast<std::intptr_t>(file),_O_WRONLY|_O_TEXT);
    if(descriptor<0){CloseHandle(file);return nullptr;}
    FILE* stream=_fdopen(descriptor,"w");if(!stream)_close(descriptor);return stream;
}
constexpr std::array<std::u16string_view,8> Names{{u"Objective_Boat_Widget_C",u"Objective_Car_Widget_C",u"Objective_GrendelBridgeRepairWidget_C",u"Objective_GrendelPolice_Widget_C",u"Objective_GrendelRadio_Widget_C",u"Objective_Police_Widget_C",u"Objective_Radio_Widget_C",u"ObjectivesWidget_C"}};
inline int Wanted(std::u16string_view name){for(unsigned i=0;i<Names.size();++i)if(name==Names[i])return static_cast<int>(i);return -1;}
class Reader{
    HANDLE process_;ULONGLONG started_=GetTickCount64();
public:
    unsigned reads=0;bool exhausted=false;
    explicit Reader(HANDLE p):process_(p){}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){
        if(reads>=500000||GetTickCount64()-started_>12000){exhausted=true;return false;}++reads;
        if(!out||bytes>65536||!cfgprobe::lifetime::Extent(at,bytes))return false;SIZE_T done=0;
        return ReadProcessMemory(process_,reinterpret_cast<const void*>(at),out,bytes,&done)&&done==bytes;
    }
    template<class T>bool Read(std::uintptr_t at,T& out){T fresh{};if(!ReadBytes(at,&fresh,sizeof(fresh)))return false;out=fresh;return true;}
};
template<class R> void FieldMetadata(R& r,std::uintptr_t module,FILE* output,const cfgreflection::Field& field){
    std::fprintf(output,"Field=%llX Owner=%llX Next=%llX Name=",static_cast<unsigned long long>(field.object),static_cast<unsigned long long>(field.owner),static_cast<unsigned long long>(field.next));
    for(char16_t c:field.label.View())std::fputc(static_cast<char>(c),output);std::fputs(" Kind=",output);
    for(char16_t c:field.kind.View())std::fputc(static_cast<char>(c),output);
    cfgreflection::Property p;if(cfgmenu::capture::RawProperty(r,field,p)){
        std::fprintf(output," Offset=%d ElementBytes=%d Dimension=%d Flags=%llX",p.offset,p.elementBytes,p.dimension,static_cast<unsigned long long>(p.flags));
        if(field.kind.View()==u"ObjectProperty"){
            std::uintptr_t declared=0;cfgnames::Text label;cfgnames::Observation<R> names(r,module);
            if(r.Read(field.object+0x70,declared)&&names.Object(declared,label)){std::fprintf(output," DeclaredClass=%llX:",static_cast<unsigned long long>(declared));for(char16_t c:label.View())std::fputc(static_cast<char>(c),output);}
        }else if(field.kind.View()==u"BoolProperty"){
            std::array<unsigned char,4> layout{};if(r.Read(field.object+0x70,layout))std::fprintf(output," BoolLayoutRaw=%02X%02X%02X%02X",layout[0],layout[1],layout[2],layout[3]);
        }
    }
    std::fputc('\n',output);
}
// Explicit Save only. Never called from a game callback or startup watcher.
template<class R>void Attachment(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t root,FILE* output){
    cfgnames::Observation<R> names(r,module);
    std::array<std::uintptr_t,8> seen{};std::uintptr_t object=root;
    for(unsigned depth=0;object&&depth<seen.size();++depth){
        for(unsigned i=0;i<depth;++i)if(seen[i]==object){std::fputs("ObjectiveOuterCycle=1\n",output);return;}
        seen[depth]=object;cfgprobe::lifetime::SlotObservation slot{},current;
        std::uintptr_t outer=0,again=0,type=0,defaults=0;cfgnames::Text label,kind;
        if(!cfgprobe::lifetime::ReadSlot(r,registry,object,slot)||!r.Read(object+cfgprobe::ObjectClass,type)||
           !r.Read(object+0x20,outer)||!names.Object(object,label)||!names.Object(type,kind)||
           !r.Read(object+0x20,again)||again!=outer||!cfgprobe::lifetime::ReadSlot(r,registry,object,current)||!(current==slot))break;
        const bool defaultKnown=r.Read(type+cfgspawn::ClassDefaultObject,defaults);
        std::fprintf(output,"ObjectiveOuterDepth=%u Object=%llX Serial=%d Outer=%llX CDO=%u DefaultKnown=%u Name=",depth,
            static_cast<unsigned long long>(object),slot.serial,static_cast<unsigned long long>(outer),defaultKnown&&defaults==object?1u:0u,defaultKnown?1u:0u);
        for(char16_t c:label.View())std::fputc(c<128?static_cast<char>(c):'?',output);std::fputs(" Type=",output);
        for(char16_t c:kind.View())std::fputc(c<128?static_cast<char>(c):'?',output);std::fputc('\n',output);object=outer;
    }
    std::uintptr_t widget=0,slot=0,panel=0;using namespace cfgobjectivepanel;
    if(!Code(r,module+disk::NativeParentRva,disk::NativeParentCode)||
       !NativeClass(r,module,disk::Widget,disk::WidgetConstructor,u"Widget",widget)||
       !NativeClass(r,module,disk::PanelSlot,disk::PanelSlotConstructor,u"PanelSlot",slot)||
       !NativeClass(r,module,disk::PanelWidget,disk::PanelWidgetConstructor,u"PanelWidget",panel))return;
    object=root;seen={};
    for(unsigned depth=0;object&&depth<seen.size();++depth){
        for(unsigned i=0;i<depth;++i)if(seen[i]==object){std::fputs("ObjectiveParentCycle=1\n",output);return;}
        seen[depth]=object;cfgobjectiveattachment::Edge edge;
        if(!cfgobjectiveattachment::Parent(r,registry,object,widget,slot,panel,edge)){
            std::fprintf(output,"ObjectiveParentBoundary=%llX Depth=%u NoOwnershipInference=1\n",static_cast<unsigned long long>(object),depth);break;}
        std::fprintf(output,"ObjectiveParentDepth=%u Widget=%llX Slot=%llX Parent=%llX WidgetSerial=%d SlotSerial=%d ParentSerial=%d\n",depth,
            static_cast<unsigned long long>(edge.widget.object),static_cast<unsigned long long>(edge.slot.object),
            static_cast<unsigned long long>(edge.parent.object),edge.widget.serial,edge.slot.serial,edge.parent.serial);
        object=edge.parent.object;
    }
}
inline void Capture(HANDLE process,std::uintptr_t module,FILE* output){
    if(!output)return;Reader r(process);std::uintptr_t engine=0,viewport=0,world=0,mode=0,net=0,type=0;
    cfgnames::Observation<Reader> names(r,module);cfgnames::Text modeName{};
    if(!r.Read(module+cfgprobe::EngineGlobalCandidate,engine)||!engine||!r.Read(engine+cfgprobe::EngineGameViewport,viewport)||!viewport||
       !r.Read(viewport+cfgprobe::ViewportWorld,world)||!world||!r.Read(world+cfgprobe::WorldNetDriver,net)||net||
       !r.Read(world+cfgprobe::WorldGameMode,mode)||!mode||!r.Read(mode+cfgprobe::ObjectClass,type)||!names.Object(type,modeName)||
       (modeName.Base()!=u"SCGameMode_Sandbox"&&modeName.Base()!=u"SCGameMode_OfflineBots")){
        std::fputs("ObjectiveCapture=offline_gate_refused\n",output);return;
    }
    cfgprobe::lifetime::ArrayHeader registry{};
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||registry.count>1048576){std::fputs("ObjectiveCapture=registry_refused\n",output);return;}
    std::fprintf(output,"ReadOnlyObjectiveCapture=1 World=%llX Mode=%llX Module=%llX\nNoWritesOrNativeCalls=1\n",static_cast<unsigned long long>(world),static_cast<unsigned long long>(mode),static_cast<unsigned long long>(module));
    cfgmenu::capture::MenuClassCache cache;std::array<std::uintptr_t,8> found{};unsigned functions=0,totalBytes=0;
    cfgreflection::Observation<Reader> reflection(r,module);
    std::array<std::uintptr_t,256*3> page{};
    for(int i=0;i<registry.count&&!r.exhausted;++i){
        if(i%256==0&&!r.ReadBytes(registry.data+24ULL*i,page.data(),24ULL*(std::min)(256,registry.count-i)))break;
        std::uintptr_t object=page[static_cast<unsigned>(i%256)*3],objectType=0;if(!object||!r.Read(object+cfgprobe::ObjectClass,objectType)||!objectType)continue;
        bool generated=false;if(!cache.Lookup(objectType,generated)){cfgnames::Text kind{};generated=names.Object(objectType,kind)&&(kind.Base()==u"WidgetBlueprintGeneratedClass"||kind.Base()==u"Class");cache.Store(objectType,generated);}
        if(!generated)continue;cfgnames::Text label{};if(!names.Object(object,label))continue;
        if(label.Base()==u"SCGameState"||label.Base()==u"SCGameState_Hunt"){
            cfgprobe::lifetime::SlotObservation nativeSlot{},again{};std::uintptr_t super=0,typeAgain=0;cfgnames::Text labelAgain{};
            if(cfgprobe::lifetime::ReadSlot(r,registry,object,nativeSlot)&&r.Read(object+cfgprobe::ClassSuper,super)&&
               names.Object(object,labelAgain)&&labelAgain.View()==label.View()&&r.Read(object+cfgprobe::ObjectClass,typeAgain)&&typeAgain==objectType&&
               cfgprobe::lifetime::ReadSlot(r,registry,object,again)&&again==nativeSlot){
                std::fprintf(output,"NativeGameStateClass=%llX Type=%llX Super=%llX Serial=%d Name=",static_cast<unsigned long long>(object),static_cast<unsigned long long>(objectType),static_cast<unsigned long long>(super),nativeSlot.serial);
                for(char16_t c:label.View())std::fputc(static_cast<char>(c),output);std::fputc('\n',output);
            }continue;
        }
        const int wanted=Wanted(label.Base());
        if(wanted<0||found[static_cast<unsigned>(wanted)])continue;found[static_cast<unsigned>(wanted)]=object;
        cfgprobe::lifetime::SlotObservation slot{};if(!cfgprobe::lifetime::ReadSlot(r,registry,object,slot))continue;
        std::fprintf(output,"Class=%llX Serial=%d Name=",static_cast<unsigned long long>(object),slot.serial);
        for(char16_t c:Names[static_cast<unsigned>(wanted)])std::fputc(static_cast<char>(c),output);std::fputc('\n',output);
        std::uintptr_t child=0;if(!r.Read(object+cfgreflection::disk::Children,child))continue;
        for(unsigned n=0;child&&n<128&&functions<256&&!r.exhausted;++n){
            cfgreflection::Field field{};if(!reflection.ReadField(child,object,field))break;child=field.next;
            FieldMetadata(r,module,output,field);
            if(field.kind.View()!=u"Function")continue;cfgmenu::capture::ScriptHeader script{};
            std::uintptr_t local=0;if(r.Read(field.object+cfgreflection::disk::Children,local))for(unsigned j=0;local&&j<128&&!r.exhausted;++j){
                cfgreflection::Field parameter;if(!reflection.ReadField(local,field.object,parameter))break;FieldMetadata(r,module,output,parameter);local=parameter.next;
            }
            auto bytes=std::make_unique<std::array<unsigned char,65536>>();
            if(!cfgmenu::capture::Script(r,field.object,script,*bytes)||!script.count||totalBytes+static_cast<unsigned>(script.count)>524288)continue;
            ++functions;totalBytes+=static_cast<unsigned>(script.count);
            std::fprintf(output,"Function=%llX Owner=%llX ScriptData=%llX Bytes=%d Name=",static_cast<unsigned long long>(field.object),static_cast<unsigned long long>(object),static_cast<unsigned long long>(script.data),script.count);
            for(char16_t c:field.label.View())std::fputc(static_cast<char>(c),output);std::fputs("\nScriptHexBegin\n",output);
            for(int b=0;b<script.count;++b){std::fprintf(output,"%02X",(*bytes)[static_cast<unsigned>(b)]);if(b%32==31)std::fputc('\n',output);}std::fputs("\nScriptHexEnd\n",output);
            // Candidate operand evidence only; byte search is NOT a parser or
            // patch authority. Names let offline analysis identify Hunt casts.
            unsigned operands=0;
            for(int b=0;b+9<=script.count&&operands<64;++b)if((*bytes)[static_cast<unsigned>(b)]==cfgmenu::counselorpickerdisk::DynamicCast){
                std::uintptr_t operand=0;std::memcpy(&operand,bytes->data()+b+1,sizeof(operand));cfgnames::Text targetName{};
                if(cfgprobe::ObjectPointer(operand)&&names.Object(operand,targetName)){
                    ++operands;std::fprintf(output,"CastCandidateOffset=%d Class=%llX Name=",b,static_cast<unsigned long long>(operand));
                    for(char16_t c:targetName.View())std::fputc(static_cast<char>(c),output);std::fputc('\n',output);
                }
            }
        }
    }
    unsigned instances=0;cfgobjectivepanel::Binding panelBinding;
    for(int i=0;found[7]&&i<registry.count&&instances<16&&!r.exhausted;++i){
        if(i%256==0&&!r.ReadBytes(registry.data+24ULL*i,page.data(),24ULL*(std::min)(256,registry.count-i)))break;
        const auto object=page[static_cast<unsigned>(i%256)*3];std::uintptr_t objectType=0;
        if(!object||!r.Read(object+cfgprobe::ObjectClass,objectType)||objectType!=found[7])continue;
        ++instances;Attachment(r,module,registry,object,output);cfgobjectivepanel::Snapshot panel;
        bool okay=false;
        for(unsigned warm=0;warm<96&&!r.exhausted;++warm){okay=cfgobjectivepanel::ReadPanel(r,module,object,objectType,panel,&panelBinding);if(okay||!panelBinding.pending)break;}
        std::fprintf(output,"ObjectiveInstance=%llX PanelObserved=%u OwningPlayerNotInvoked=1 WorldOwnershipApproved=0\n",static_cast<unsigned long long>(object),okay?1:0);
        if(!okay)continue;
        std::fprintf(output,"Panel=%llX RootSerial=%d PanelSerial=%d Children=%d\n",static_cast<unsigned long long>(panel.panel),panel.rootIdentity.serial,panel.panelIdentity.serial,panel.slots.count);
        for(int j=0;j<panel.slots.count;++j){const auto& c=panel.children[static_cast<unsigned>(j)];
            std::fprintf(output,"PanelChild=%d Slot=%llX Content=%llX Type=%llX SlotSerial=%d ContentSerial=%d Kind=%u\n",j,static_cast<unsigned long long>(c.slot),static_cast<unsigned long long>(c.widget),static_cast<unsigned long long>(c.type),c.slotIdentity.serial,c.widgetIdentity.serial,static_cast<unsigned>(c.kind));
        }
    }
    std::uintptr_t finalWorld=0,finalMode=0,finalNet=1;const bool stable=r.Read(viewport+cfgprobe::ViewportWorld,finalWorld)&&finalWorld==world&&r.Read(world+cfgprobe::WorldGameMode,finalMode)&&finalMode==mode&&r.Read(world+cfgprobe::WorldNetDriver,finalNet)&&!finalNet;
    std::fprintf(output,"Functions=%u ScriptBytes=%u Reads=%u BudgetHit=%u WorldStable=%u\nDiagnosticOnly=1 SnapshotNotAtomic=1\n",functions,totalBytes,r.reads,r.exhausted?1:0,stable?1:0);
}
}
