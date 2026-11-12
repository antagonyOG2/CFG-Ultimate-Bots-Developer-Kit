#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "NativeCounselorConstruct.hpp"
#include <fstream>
#include <regex>
struct ConstructHost {
    bool owned=true,current=true,needs=true,lost=false,foreign=false;unsigned writes=0,widens=0,restores=0;
    std::uintptr_t value=0x100000;cfgmenu::counselorpicker::CastResult result=cfgmenu::counselorpicker::CastResult::Changed;
    bool OwnedConstruct(){return owned;}
    bool Current(bool widened){return current&&!lost&&value==(widened?0x200000:0x100000);}
    bool NeedsWidening(){return needs;}
    cfgmenu::counselorpicker::CastResult Widen(){++widens;if(result==cfgmenu::counselorpicker::CastResult::Changed){value=0x200000;++writes;}return result;}
    cfgmenu::counselorpicker::CastResult Restore(){++restores;if(foreign)return cfgmenu::counselorpicker::CastResult::Refused;value=0x100000;++writes;return cfgmenu::counselorpicker::CastResult::Changed;}
};
int main(int argc,char** argv){using namespace cfgmenu;using namespace counselorpicker;unsigned failures=0,checks=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const char* capture=argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Native-Picker-Chain-3243-25117-Counselor.txt";
    std::ifstream input(capture,std::ios::binary);const std::string text((std::istreambuf_iterator<char>(input)),{});
    std::smatch match;const std::regex graphExpression("Function=0x([0-9A-F]+) Owner=0x([0-9A-F]+)[^\\r\\n]+Name=ExecuteUbergraph_Counselor_Menu Flags=[^\\r\\n]+NativeExecutor=0x([0-9A-F]+)[^\\r\\n]*[\\s\\S]*?ScriptHexBegin\\r?\\n([0-9A-F\\r\\n]+)\\r?\\nScriptHexEnd");
    check(std::regex_search(text,match,graphExpression),"complete captured native graph fixture");
    if(match.size()!=5)return 1;
    auto graph=std::make_unique<capture::GraphSnapshot>();graph->function.object=std::stoull(match[1].str(),nullptr,16);graph->function.owner=std::stoull(match[2].str(),nullptr,16);
    const auto module=std::stoull(match[3].str(),nullptr,16)-startdisk::ScriptExecutor;
    const auto label=[&](cfgnames::Text& out,std::u16string_view value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());};
    label(graph->function.label,u"ExecuteUbergraph_Counselor_Menu");label(graph->function.kind,u"Function");
    graph->flags=0x00800000;graph->parameterBytes=4;graph->storageBytes=2762;graph->script.count=GraphBytes;graph->destructorLink=0x2229444CD00;graph->nativeExecutor=module+startdisk::ScriptExecutor;
    auto hex=match[4].str();hex.erase(std::remove(hex.begin(),hex.end(),'\r'),hex.end());hex.erase(std::remove(hex.begin(),hex.end(),'\n'),hex.end());
    check(hex.size()==2*GraphBytes,"captured graph full extent");for(unsigned i=0;i<GraphBytes;++i)graph->bytes[i]=static_cast<unsigned char>(std::stoul(hex.substr(2*i,2),nullptr,16));
    check(GraphShape(*graph,graph->function.owner,module),"actual CFG graph class/entries/cast/typed operand shape");
    check(graph->bytes[CastOpcodeOffset]==0x0F&&Pointer(*graph,CastOperandOffset)!=0,"CFG cast0F1045/operand1046, not donor2E");
    for(unsigned fault=0;fault<8;++fault){auto changed=*graph;
        if(fault==0)--changed.script.count;if(fault==1)++changed.storageBytes;if(fault==2)changed.bytes[CastOpcodeOffset]=0x2E;
        if(fault==3)changed.bytes[0x1E30]=0x2E;if(fault==4)changed.bytes[0x1095]=0x76;
        if(fault==5)changed.bytes[AcceptEntry]=0x0A;if(fault==6)++changed.bytes[0x229E];if(fault==7)changed.bytes[0x1064]=0x76;
        check(!GraphShape(changed,graph->function.owner,module),"changed schema/alias/operand refuse");}
    for(unsigned fault=0;fault<8;++fault){ConstructHost host;ScopedConstruct scope;unsigned forwards=0;
        if(fault==1)host.owned=false;if(fault==2)host.current=false;if(fault==3)host.needs=false;
        if(fault==4)host.result=CastResult::Refused;if(fault==5)host.result=CastResult::Indeterminate;
        check(scope.Run(host,[&]()noexcept{++forwards;if(fault==6){host.lost=true;}if(fault==7){host.foreign=true;host.value=0x300000;}return true;}),"native original always forwarded once");
        check(forwards==1,"exact one original native forward");
        if(fault==0)check(host.writes==2&&host.value==0x100000&&scope.State()==ConstructState::Returned,"one operand widened and restored after native return");
        if(fault>=1&&fault<=3)check(host.writes==0,"unrelated/invalid/already-lobby callback unchanged");
        if(fault>=4&&fault<=5)check(host.writes==0&&host.restores==0,"uncertain publication no blind rollback");
        if(fault>=6)check(host.restores==0&&scope.State()==ConstructState::Indeterminate,"lost owner/foreign operand no rollback permission");
    }
    ConstructHost nested;ScopedConstruct recursion;unsigned forwards=0;
    check(recursion.Run(nested,[&]()noexcept{++forwards;return recursion.Run(nested,[&]()noexcept{++forwards;return true;});}),"native wrapper to graph recursion forwarded");
    check(nested.widens==1&&nested.restores==1&&forwards==2,"outer Construct lease alone mutates/restores");
    Binding binding;binding.context.widget=0x100000;binding.context.thread=42;binding.construct.field.object=0x110000;binding.construct.script.data=0x120000;
    binding.graph.field.object=0x130000;binding.graph.script.data=0x140000;
    constexpr std::uintptr_t frame=0x150000,locals=0x160000;MenuFixture fixture;OptionsReader reader{fixture.r};Callback kind=Callback::Other;
    fixture.r.Set(frame+startdisk::FrameNode,binding.construct.field.object);fixture.r.Set(frame+startdisk::FrameObject,binding.context.widget);
    fixture.r.Set(frame+startdisk::FrameCode,binding.construct.script.data);fixture.r.Set(frame+startdisk::FrameCurrentNative,binding.construct.field.object);
    check(Frame(reader,binding,binding.context.widget,frame,42,kind)&&kind==Callback::Construct,"borrowed native Construct wrapper frame");
    check(!Frame(reader,binding,binding.context.widget,frame,43,kind)&&!Frame(reader,binding,binding.context.widget+16,frame,42,kind),"foreign frame receiver/thread refuses");
    fixture.r.Set(frame+startdisk::FrameNode,binding.graph.field.object);fixture.r.Set(frame+startdisk::FrameCode,binding.graph.script.data);
    fixture.r.Set(frame+startdisk::FrameCurrentNative,binding.graph.field.object);fixture.r.Set(frame+startdisk::FrameLocals,locals);
    for(const auto entry:{ConstructEntry,AcceptEntry,TransitionEntry,0x123}){fixture.r.Set(locals,entry);
        check(Frame(reader,binding,binding.context.widget,frame,42,kind)==(entry!=0x123),"graph callback entry belongs to exact captured route");}
    NativeCastHost<OptionsReader> foreignModule(reader,binding,binding.context.widget,frame);
    check(foreignModule.Widen()==CastResult::Refused&&foreignModule.Restore()==CastResult::Refused,"native writer refuses foreign module and nonexistent lease");
    Binding refused;unsigned reads=0,stage=0;
    check(!Observe(reader,binding.context,42,false,refused,&reads,&stage)&&!refused.context.widget,"unverified module cannot produce a runtime binding");
    std::printf("CFG counselor captured graph/borrowed frame/scoped Construct cast: %s (%u checks, %u failures). Owned fixtures; no game access or gameplay approval.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
