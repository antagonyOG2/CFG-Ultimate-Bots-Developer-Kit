#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/OfflineEntryRouteObservation.hpp"
#include "PickerPrecreationFixture.hpp"
int main(int argc,char** argv){using namespace cfgmenu;unsigned checks=0,failures=0;
    const auto check=[&](bool value,const char* why){++checks;if(!value){++failures;std::printf("FAIL %s\n",why);}};
    const auto text=PickerPreparedFixture::File(argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Offline-Entry-15131-14303.txt");
    auto resource=std::make_unique<entrysettings::Resources>();auto transition=std::make_unique<capture::DiagnosticFunctionEvidence>();
    entryroute::Binding b;b.entry.source=resource.get();
    check(PickerPreparedFixture::Parse(text,"ExecuteUbergraph_OfflinePlayMenuWidget",resource->entry),"captured full entry graph");
    check(PickerPreparedFixture::Parse(text,"OnPlayTransitionOut",*transition),"captured original transition wrapper");
    check(PickerPreparedFixture::Parse(text,NativeFixture::Narrow(entry::SandboxClick),b.entry.sandboxClick),"actual sandbox click wrapper");
    if(!resource->entry.script.count||transition->fieldsCount!=1)return 1;
    // Inert captured pointers remain bytecode data, never dereferenced.
    const auto fieldObject=[&](const char* name){const std::regex expression(std::string(" Parameter=0x([A-F0-9]+) Name=")+name+" Type=");std::smatch match;
        return std::regex_search(text,match,expression)?static_cast<std::uintptr_t>(std::stoull(match[1].str(),nullptr,16)):0;};
    b.entry.context.module=transition->nativeExecutor-startdisk::ScriptExecutor;
    const std::regex ownerRe("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=OnPlayTransitionOut ");std::smatch owner;
    check(std::regex_search(text,owner,ownerRe),"captured exact wrapper owner");if(owner.size()!=3)return 1;
    b.entry.context.type=std::stoull(owner[2].str(),nullptr,16);transition->function.object=std::stoull(owner[1].str(),nullptr,16);transition->function.owner=b.entry.context.type;
    const auto graphRe=std::regex("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=ExecuteUbergraph_OfflinePlayMenuWidget ");std::smatch graphMatch;
    check(std::regex_search(text,graphMatch,graphRe),"captured exact graph owner");if(graphMatch.size()!=3)return 1;
    resource->entry.function.object=std::stoull(graphMatch[1].str(),nullptr,16);resource->entry.function.owner=b.entry.context.type;
    b.functions[2].slot=startbinding::Record(*transition,{});std::copy_n(transition->bytes.begin(),40,b.functions[2].bytes.begin());
    b.parameter=transition->fields[0];b.parameter.field.object=fieldObject("TransitioningTo");b.parameter.field.owner=transition->function.object;
    b.persistent.field.object=fieldObject("K2Node_Event_TransitioningTo");b.persistent.field.owner=resource->entry.function.object;
    b.persistent.field.label=b.parameter.field.label;
    const auto label=[](cfgnames::Text& out,std::u16string_view value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());};
    label(b.persistent.field.label,u"K2Node_Event_TransitioningTo");label(b.persistent.field.kind,u"ClassProperty");b.persistent.elementBytes=8;b.persistent.offset=40;b.persistent.dimension=1;b.persistent.flags=0x0008001040000200ULL;
    resource->push=entrysettings::Pointer(resource->entry,0xDC);b.entry.sandboxMap=entrysettings::Pointer(resource->entry,0xE5);
    b.push=entrysettings::Pointer(resource->entry,0xA4);b.valid=entrysettings::Pointer(resource->entry,0x79);
    check(entryroute::Shape(b),"actual input-to-persistent-local-to-native-PushMenu flow");
    const auto original=resource->entry.bytes;
    for(const auto at:{0x78U,0x81U,0x8BU,0xA3U,0xACU,0xB5U,0xB6U,0xC2U,0xDBU,0xE4U,0xEDU,0x3AFU}){
        resource->entry.bytes[at]^=1;check(!entryroute::Shape(b),"altered consumer opcode refused");resource->entry.bytes=original;}
    for(unsigned at=0;at<40;++at){auto altered=b;altered.functions[2].bytes[at]^=1;check(!entryroute::Shape(altered),"every wrapper byte guarded");}
    for(unsigned fault=0;fault<8;++fault){auto altered=b;if(fault==0)altered.parameter.offset=8;if(fault==1)altered.parameter.flags^=1;
        if(fault==2)altered.persistent.offset=48;if(fault==3)altered.persistent.field.object+=8;if(fault==4)altered.entry.sandboxMap+=8;
        if(fault==5)altered.push+=8;if(fault==6)altered.functions[2].slot.fast.graph=0x100000;if(fault==7)altered.entry.source=nullptr;
        check(!entryroute::Shape(altered),"typed owner/parameter/reference failure refused");}
    // Instantiate the bounded preparation/frame projection paths with owned
    // rejecting memory, ensuring absent metadata cannot produce a binding.
    NativeFixture empty;OptionsReader rejectingReader{empty.fixture.r};entryroute::Binding rejected;unsigned reads=0;
    check(!entryroute::Prepare(rejectingReader,entry::Binding{},rejected,&reads),"empty source preparation refused without game access");
    entryroute::Invocation invocation;check(!entryroute::Borrowed(rejectingReader,rejected,{},0,0,0,invocation),"empty callback frame refused");
    std::printf("CFG exact OfflinePlay entry destination dataflow: %s (%u checks; no writes/calls/game access).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
