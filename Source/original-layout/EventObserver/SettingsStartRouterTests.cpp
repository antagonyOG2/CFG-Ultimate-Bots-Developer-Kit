#define CFGMENU_START_FIXTURE_ONLY
#include "SetupMenuStartTests.cpp"
#undef CFGMENU_START_FIXTURE_ONLY
#include "SettingsStartRouter.hpp"
#include "NativeGraphEntryBoundary.hpp"
#include <functional>
struct StartEvidence {
    cfgmenu::active::Context context;
    cfgmenu::capture::DiagnosticFunctionEvidence start,construct,graph;
    cfgmenu::startbinding::FastCall startFast,constructFast;
    StartEvidence(){
        context.module=0x100000;context.widget=0x200000;context.type=0x300000;context.thread=42;
        context.identities[2]={context.widget,1,123};
        const auto wrapper=[&](auto& f,std::uintptr_t object,std::u16string_view label,std::uint32_t flags,std::int32_t entry){
            f.function.object=object;f.function.owner=context.type;PutText(f.function.label,label);PutText(f.function.kind,u"Function");
            f.flags=flags;f.nativeExecutor=context.module+cfgmenu::startdisk::ScriptExecutor;f.script={object+0x1000,22,22};
            f.bytes[0]=0x90;for(unsigned i=1;i<=12;++i)f.bytes[i]=static_cast<unsigned char>(i);
            f.bytes[13]=0x5A;std::memcpy(f.bytes.data()+14,&entry,4);
            f.bytes[18]=0x97;f.bytes[19]=0xA5;f.bytes[20]=0x39;f.bytes[21]=0xE5;
        };
        wrapper(start,0x400000,u"OnClicked_Start",0x0C000000,cfgmenu::startdisk::StartEntry);
        wrapper(construct,0x410000,u"Construct",0x08020808,cfgmenu::startdisk::ConstructEntry);
        graph.function.object=0x420000;graph.function.owner=context.type;
        PutText(graph.function.label,u"ExecuteUbergraph_OfflineBotsSettingsMenuWidget");PutText(graph.function.kind,u"Function");
        graph.nativeExecutor=context.module+cfgmenu::startdisk::ScriptExecutor;graph.parameterBytes=4;graph.storageBytes=cfgmenu::startbinding::shape::StorageBytes;
        graph.destructorLink=0x430000;graph.script={0x500000,cfgmenu::startbinding::shape::GraphBytes,cfgmenu::startbinding::shape::GraphBytes};graph.fieldsCount=1;
        auto& p=graph.fields[0];p.field.object=0x440000;p.field.owner=graph.function.object;
        PutText(p.field.label,u"EntryPoint");PutText(p.field.kind,u"IntProperty");p.elementBytes=4;p.dimension=1;p.flags=0x0008001040000294ULL;
        for(const auto entry:{cfgmenu::startdisk::StartEntry,cfgmenu::startdisk::ConstructEntry}){
            graph.bytes[entry]=0x0A;const std::uint32_t destination=entry==cfgmenu::startdisk::StartEntry?cfgmenu::startbinding::shape::StartDestination:cfgmenu::startbinding::shape::ConstructDestination;
            std::memcpy(graph.bytes.data()+entry+1,&destination,4);
        }
        startFast={graph.function.object,cfgmenu::startdisk::StartEntry};constructFast={graph.function.object,cfgmenu::startdisk::ConstructEntry};
    }
    bool Validate(cfgmenu::startbinding::Binding& out)const{
        return cfgmenu::startbinding::Validate(context,start,construct,graph,startFast,constructFast,out);
    }
};
struct RoutingHost : StartHost {
    std::uint64_t generation=0;bool claimed=true,bindingOkay=true;unsigned validations=0;
    std::function<void()> reenter;
    bool Claimed()const{return claimed;}
    bool Current(std::uint64_t g){return current&&g==generation;}
    bool CurrentBinding(const cfgmenu::startbinding::Binding&,cfgmenu::startroute::SlotKind){++validations;return bindingOkay;}
    bool TravelSandbox(const cfgmenu::MenuCatalogueEvidence&){++travels;if(reenter)reenter();return travelOkay;}
};
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;using namespace cfgmenu::startroute;
    auto evidence=std::make_unique<StartEvidence>();startbinding::Binding binding;
    check(evidence->Validate(binding),"exact owned Start/Construct/graph schema");
    check(HasStartGraphAlias(binding),"graph-only adapter requires the exact native Start fast alias");
    for(unsigned fault=0;fault<20;++fault){auto changed=std::make_unique<StartEvidence>(*evidence);startbinding::Binding refused;
        switch(fault){
        case 0:changed->context.identities[2].serial=0;break;
        case 1:changed->start.function.owner++;break;
        case 2:changed->start.flags^=1;break;
        case 3:changed->start.script.count=21;break;
        case 4:changed->start.bytes[14]^=1;break;
        case 5:changed->construct.bytes[1]^=1;break;
        case 6:changed->construct.nativeExecutor++;break;
        case 7:changed->graph.storageBytes=4;break;
        case 8:changed->graph.parameterBytes=937;break;
        case 9:changed->graph.bytes[startdisk::StartEntry]=0x90;break;
        case 10:changed->graph.bytes[startdisk::ConstructEntry+1]^=1;break;
        case 11:changed->graph.fields[0].flags^=1;break;
        case 12:changed->graph.fields[0].offset=4;break;
        case 13:changed->graph.fieldsCount=2;changed->graph.fields[1]=changed->graph.fields[0];break;
        case 14:changed->startFast.entry=2506;break; // Decimal is NOT the captured hexadecimal entry.
        case 15:changed->startFast.graph=0x450000;break;
        case 16:changed->constructFast.entry=startdisk::StartEntry;break;
        case 17:changed->graph.destructorLink=0;break;
        case 18:changed->graph.fieldsCount=129;break;
        case 19:changed->graph.function.object=changed->start.function.object;break;
        }
        check(!changed->Validate(refused)&&!refused.context.widget,"faulted metadata never publishes binding");
    }
    {auto ordinary=std::make_unique<StartEvidence>(*evidence);ordinary->startFast={0,123};ordinary->constructFast={0,456};
        startbinding::Binding out;check(ordinary->Validate(out)&&!out.start.fast.graph,"zero fast pointer does not invent an alias");
        check(!HasStartGraphAlias(out),"valid zero-alias diagnostic metadata never authorizes graph-only live publication");
        out.start.fast.graph=out.graph.field.object;out.start.fast.entry=2506;
        check(!HasStartGraphAlias(out),"wrong entry cannot authorize graph-only live publication");
        out.start.fast.entry=startdisk::StartEntry;out.start.fast.graph+=8;
        check(!HasStartGraphAlias(out),"different graph cannot authorize graph-only live publication");}
    for(unsigned fast=0;fast<2;++fast){
        auto ordinary=std::make_unique<StartEvidence>(*evidence);if(!fast)ordinary->startFast={0,0};
        startbinding::Binding out;check(ordinary->Validate(out),"both actual native entry variants validated");
        unsigned claims=0,forwards=0,travels=0;
        const auto originalGraph=[&](){++travels;};
        const auto graph=[&](){NativeEntryBoundary(out,SlotKind::Graph,[&](){++claims;originalGraph();return Result::Consumed;},[&](){++forwards;originalGraph();});};
        if(fast)graph(); // ProcessEvent substitutes the fast graph before invocation.
        else NativeEntryBoundary(out,SlotKind::Wrapper,[&](){++claims;++travels;return Result::Consumed;},[&](){++forwards;graph();});
        check(claims==1&&travels==1&&!forwards,"actual zero-alias wrapper / fast-graph route claimed exactly once");
        check(StartBoundary(out)==(fast?SlotKind::Graph:SlotKind::Wrapper),"metadata chooses reachable native entry");
        unsigned unrelatedClaims=0,unrelatedForwards=0;
        NativeEntryBoundary(out,fast?SlotKind::Wrapper:SlotKind::Graph,[&](){++unrelatedClaims;return Result::Consumed;},[&](){++unrelatedForwards;});
        check(!unrelatedClaims&&unrelatedForwards==1,"other boundary forwards without claiming nested original graph");
    }
    for(unsigned fault=0;fault<16;++fault){
        Session session;Capabilities capabilities;capabilities.verified.fill(true);capabilities.automaticJason=true;
        check(session.Open(100,Mode::Counselor,capabilities,9)&&session.Pick(100,session.Generation(),1,2,3),"selected counselor session");
        GameplayHandoff handoff(session);rows::Runtime rows;SetupStart start(session,handoff,rows);RoutingHost host;
        host.generation=session.Generation();host.catalogue={100,host.generation,44,0x500000,1,2,3,8,{},true,true};host.catalogue.selectedJason[16]=9;
        rows::Spec spec;spec.id=RowId::Duration;spec.title=u"Match Length";spec.labels[0]=u"30 minutes";spec.count=1;
        check(rows.Begin(host.generation,&spec,1)&&start.Claim(100,host.generation),"claim setup");
        if(fault!=3)for(unsigned i=0;i<6;++i)rows.Step(host,i*25);
        Reader reader;constexpr std::uintptr_t frame=0x600000,locals=0x610000;
        const auto makeFrame=[&](SlotKind kind,std::int32_t entry){const auto& slot=kind==SlotKind::Wrapper?binding.start:binding.graph;
            reader.Set(frame+startdisk::FrameNode,slot.field.object);reader.Set(frame+startdisk::FrameObject,binding.context.widget);
            reader.Set(frame+startdisk::FrameCode,slot.script.data);reader.Set(frame+startdisk::FrameCurrentNative,slot.field.object);
            reader.Set(frame+startdisk::FrameLocals,locals);reader.Set(locals,entry);
        };
        auto kind=fault==4?SlotKind::Wrapper:SlotKind::Graph;makeFrame(kind,startdisk::StartEntry);
        auto receiver=binding.context.widget;std::uint32_t thread=binding.context.thread;
        if(fault==0)receiver+=8;
        if(fault==1)thread++;
        if(fault==2)host.claimed=false;
        if(fault==5)reader.Set(locals,startdisk::ConstructEntry);
        if(fault==6)host.bindingOkay=false;
        if(fault==7)reader.Set(frame+startdisk::FrameNode,std::uintptr_t{0x450000});
        if(fault==8)reader.Set(frame+startdisk::FrameObject,std::uintptr_t{0x200008});
        if(fault==9)reader.Set(frame+startdisk::FrameCode,binding.graph.script.data+1);
        if(fault==10)reader.Set(frame+startdisk::FrameCurrentNative,binding.start.field.object);
        if(fault==11)reader.Set(frame+startdisk::FrameLocals,std::uintptr_t{0});
        if(fault==12)host.travelOkay=false;
        Router router(binding,start);unsigned forwards=0;Result nested=Result::Forwarded;
        if(fault==13)host.reenter=[&]{nested=router.Dispatch(reader,host,SlotKind::Graph,thread,receiver,frame,[&]{++forwards;});};
        unsigned nativeWrappers=0;Result result;
        if(fault==15){
            host.reenter=[&]{nested=router.Dispatch(reader,host,SlotKind::Graph,thread,receiver,frame,[&]{++forwards;});};
            GraphEntryBoundary(SlotKind::Wrapper,[&](){++forwards;return Result::Refused;},[&](){
                ++nativeWrappers;
                result=GraphEntryBoundary(SlotKind::Graph,[&](){return router.Dispatch(reader,host,kind,thread,receiver,frame,[&]{++forwards;});},[&](){++forwards;});
            });
            check(nativeWrappers==1&&host.travels==1&&nested==Result::Consumed&&!forwards,
                "native wrapper reaches one claimed graph; only genuine reentrant Start consumed");
        }else result=router.Dispatch(reader,host,kind,thread,receiver,frame,[&]{++forwards;});
        const bool unrelated=fault<3||fault==5,refused=fault>=6&&fault<=12;
        check(result==(unrelated?Result::Forwarded:refused?Result::Refused:Result::Consumed),"route disposition");
        check(forwards==(unrelated?1u:0u),"unrelated forwarded once; all owned Start consumed");
        check(host.travels==((fault==4||fault>=12)?1u:0u),"only validated ready Start travels once");
        if(fault<3)check(reader.reads==0&&!host.validations,"unrelated callbacks do zero reads/calls");
        if(fault==13)check(nested==Result::Consumed&&host.validations==1,"dual-slot reentry consumed before another transaction");
        if(fault==14){makeFrame(SlotKind::Wrapper,startdisk::StartEntry);
            check(router.Dispatch(reader,host,SlotKind::Wrapper,thread,receiver,frame,[&]{++forwards;})==Result::Consumed&&
                host.travels==1&&!forwards,"graph arrival followed by wrapper cannot spawn human Jason");}
        std::uintptr_t code=0;check(reader.Read(frame+startdisk::FrameCode,code)&&code==(fault==9?binding.graph.script.data+1:
            kind==SlotKind::Wrapper||fault==14?binding.start.script.data:binding.graph.script.data),"router never steps or changes VM frame");
    }
    for(unsigned alias=0;alias<2;++alias)for(unsigned slot=0;slot<2;++slot){
        startbinding::Binding b;b.graph.field.object=0x440000;
        if(alias)b.start.fast={b.graph.field.object,startdisk::StartEntry};
        unsigned claims=0,forwards=0;
        const auto result=ClaimedNativeEntryBoundary(b,slot?SlotKind::Graph:SlotKind::Wrapper,false,
            [&](){++claims;return Result::Refused;},[&](){++forwards;});
        check(result==Result::Consumed&&!claims&&!forwards,"new or warming counselor settings cannot launch stock Jason through either Start alias");
    }
    std::printf("CFG class-local Start fast/ordinary/construct/reentrant/duplicate router: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
