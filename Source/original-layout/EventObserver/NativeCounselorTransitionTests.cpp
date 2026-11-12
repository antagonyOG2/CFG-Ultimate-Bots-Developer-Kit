#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/NativeCounselorTransition.hpp"
#include <fstream>
#include <regex>
using namespace cfgmenu;
namespace {
void Label(cfgnames::Text& out,std::u16string_view value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());}
void SetProperty(cfgreflection::Property& p,std::uintptr_t owner,std::uintptr_t object,std::u16string_view name,std::u16string_view kind,int bytes,int offset,std::uint64_t flags){
    p={};p.field.owner=owner;p.field.object=object;Label(p.field.label,name);Label(p.field.kind,kind);p.elementBytes=bytes;p.offset=offset;p.dimension=1;p.flags=flags;
}
bool Parse(const std::string& text,const char* name,capture::GraphSnapshot& graph){
    const std::regex re("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=([^ ]+) Flags=([A-F0-9]+) ParameterBytes=([0-9]+) StorageBytes=([0-9]+) ScriptBytes=([0-9]+) NativeExecutor=0x([A-F0-9]+) DestructorLink=0x([A-F0-9]+)[\\s\\S]*?ScriptHexBegin\\r?\\n([A-F0-9\\r\\n]+)\\r?\\nScriptHexEnd");
    for(auto it=std::sregex_iterator(text.begin(),text.end(),re);it!=std::sregex_iterator();++it){const auto& m=*it;if(m[3].str()!=name)continue;
        graph={};graph.function.object=std::stoull(m[1].str(),nullptr,16);graph.function.owner=std::stoull(m[2].str(),nullptr,16);
        const std::string n=name;Label(graph.function.label,std::u16string(n.begin(),n.end()));Label(graph.function.kind,u"Function");
        graph.flags=static_cast<std::uint32_t>(std::stoul(m[4].str(),nullptr,16));graph.parameterBytes=static_cast<std::uint16_t>(std::stoul(m[5].str()));
        graph.storageBytes=std::stoi(m[6].str());graph.script.count=graph.script.capacity=std::stoi(m[7].str());graph.nativeExecutor=std::stoull(m[8].str(),nullptr,16);graph.destructorLink=std::stoull(m[9].str(),nullptr,16);
        auto hex=m[10].str();hex.erase(std::remove(hex.begin(),hex.end(),'\r'),hex.end());hex.erase(std::remove(hex.begin(),hex.end(),'\n'),hex.end());
        if(hex.size()!=2*static_cast<unsigned>(graph.script.count)||static_cast<unsigned>(graph.script.count)>graph.bytes.size())return false;
        for(unsigned i=0;i<static_cast<unsigned>(graph.script.count);++i)graph.bytes[i]=static_cast<unsigned char>(std::stoul(hex.substr(2*i,2),nullptr,16));return true;
    }return false;
}
struct EmptyHost {
    template<class T>bool Read(std::uintptr_t,T&){return false;}
    bool ReadBytes(std::uintptr_t,void*,std::size_t){return false;}
    bool ValidateAcceptedRoute(const counselortransition::Request&,counselortransition::Lease&)noexcept{return false;}
    bool ExchangeParameter(std::uintptr_t,std::uintptr_t,std::uintptr_t)noexcept{return false;}
};
unsigned forwarded=0;void Forward(void*,void*,void*)noexcept{++forwarded;}
}
int main(int argc,char** argv){using namespace counselortransition;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* why){++checks;if(!okay){++failures;std::printf("FAIL %s\n",why);}};
    const char* file=argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Native-Picker-Chain-3243-25117-Counselor.txt";
    std::ifstream input(file,std::ios::binary);const std::string text((std::istreambuf_iterator<char>(input)),{});
    auto p=std::make_unique<pickerprepare::Prepared>();auto wrapper=std::make_unique<capture::GraphSnapshot>();Consumer consumer;
    check(Parse(text,"ExecuteUbergraph_Counselor_Menu",p->graph)&&Parse(text,"OnPlayTransitionOut",*wrapper),"full original captured graph and forty-byte wrapper parsed");
    if(!p->graph.function.object||!wrapper->function.object)return 1;
    p->kind=pickerprepare::Kind::Counselor;p->type=p->graph.function.owner;p->source.counselor=p->type;p->source.source.module=p->graph.nativeExecutor-startdisk::ScriptExecutor;
    std::memcpy(&p->graph.function.name.index,wrapper->bytes.data()+19,4);p->source.transition=counselorpicker::Pointer(p->graph,0x234F);
    auto& t=p->handlers[static_cast<unsigned>(pickerprepare::Role::Transition)];t.present=true;t.entry=0x270F;t.fieldCount=1;
    t.slot.field=wrapper->function;t.slot.flags=wrapper->flags;t.slot.parameters=wrapper->parameterBytes;t.slot.storage=wrapper->storageBytes;t.slot.script=wrapper->script;
    std::copy_n(wrapper->bytes.begin(),40,t.bytes.begin());
    SetProperty(t.fields[0],t.slot.field.object,counselorpicker::Pointer(*wrapper,10),u"TransitioningTo",u"ClassProperty",8,0,0x000C001040000294ULL);
    SetProperty(consumer.persistent,p->graph.function.object,counselorpicker::Pointer(*wrapper,1),u"K2Node_Event_TransitioningTo",u"ClassProperty",8,1248,0x0008001040000200ULL);
    consumer.push=counselorpicker::Pointer(p->graph,0x5B);consumer.valid=counselorpicker::Pointer(p->graph,0x30);
    check(Shape(*p,consumer),"captured source parameter to persistent local to named PushMenu consumer");
    for(unsigned i=0;i<40;++i){t.bytes[i]^=1;check(!Shape(*p,consumer),"every original wrapper byte is authoritative");t.bytes[i]^=1;}
    for(const auto at:{0x2FU,0x30U,0x38U,0x39U,0x42U,0x43U,0x48U,0x5AU,0x5BU,0x63U,0x64U,0x6CU,0x6DU,0x19BU,0x1A5U,
        0x279FU,0x27B2U,0x27BBU,0x27BFU,0x27C0U,0x27C9U,0x27CAU,0x27D6U,0x27D9U,0x234EU,0x2357U,0x2A31U,0x2A3AU}){
        p->graph.bytes[at]^=1;check(!Shape(*p,consumer),"source/branch/latent/consumer mutation refuses");p->graph.bytes[at]^=1;
    }
    ++consumer.persistent.offset;check(!Shape(*p,consumer),"unproved persistent storage offset refuses");--consumer.persistent.offset;
    ++t.fields[0].offset;check(!Shape(*p,consumer),"nonzero input parameter offset refuses");--t.fields[0].offset;
    p->kind=pickerprepare::Kind::Jason;check(!Shape(*p,consumer),"other picker class refuses");p->kind=pickerprepare::Kind::Counselor;

    NativeFixture fixture;auto& f=fixture.fixture;auto& r=f.r;OptionsReader reader{r};active::Context current;
    check(active::Observe(reader,MenuFixture::Base,current),"positive owned world/controller/widget context");
    p->type=current.type;p->source.counselor=p->type;
    const pickerprepare::Receiver receiver{current.widget,current.type,current.identities[2]};
    constexpr std::uintptr_t frame=0x5000000,locals=0x5010000;t.slot.script.data=0x5020000;
    r.Set(frame+startdisk::FrameNode,t.slot.field.object);r.Set(frame+startdisk::FrameObject,current.widget);r.Set(frame+startdisk::FrameCode,t.slot.script.data);
    r.Set(frame+startdisk::FrameCurrentNative,t.slot.field.object);r.Set(frame+startdisk::FrameLocals,locals);r.Set(locals,std::uintptr_t{0});Packet packet;
    check(Borrowed(reader,current,*p,receiver,current.widget,frame,42,packet)&&packet.address==locals&&!packet.value,"only real wrapper input packet is projected");
    for(const auto offset:{startdisk::FrameNode,startdisk::FrameObject,startdisk::FrameCode,startdisk::FrameCurrentNative,startdisk::FrameLocals}){
        std::uintptr_t old=0;r.Read(frame+offset,old);r.Set(frame+offset,std::uintptr_t{0});
        check(!Borrowed(reader,current,*p,receiver,current.widget,frame,42,packet)&&!packet.address,"foreign/absent borrowed frame component refuses");r.Set(frame+offset,old);
    }
    auto wrong=receiver;wrong.identity.serial=0;check(!Borrowed(reader,current,*p,wrong,current.widget,frame,42,packet),"zero serial receiver cannot authorize input");
    check(!Borrowed(reader,current,*p,receiver,current.widget,frame,43,packet),"off-thread callback refuses");
    r.Set(frame+startdisk::FrameNode,p->graph.function.object);check(!Borrowed(reader,current,*p,receiver,current.widget,frame,42,packet),"graph frame is not an event parameter packet");r.Set(frame+startdisk::FrameNode,t.slot.field.object);
    check(Borrowed(reader,current,*p,receiver,current.widget,frame,42,packet),"restored owned fixture accepted");
    Lease lease;lease.generation=lease.acceptedGeneration=1;lease.widget=current.widget;lease.controller=current.controller;lease.sourceClass=p->type;
    lease.targetClass=0x5030000;lease.targetReference=0x5040000;lease.thread=42;lease.widgetIdentity=receiver.identity;lease.controllerIdentity=current.identities[1];
    check(Matches(lease,current,*p,receiver),"exact accepted route and positive identities match");
    auto badLease=lease;badLease.acceptedGeneration=0;check(!Matches(badLease,current,*p,receiver),"Back without accepted generation refuses");
    badLease=lease;++badLease.controllerIdentity.serial;check(!Matches(badLease,current,*p,receiver),"recycled controller identity refuses");
    badLease=lease;++badLease.widget;check(!Matches(badLease,current,*p,receiver),"another counselor receiver refuses");
    badLease=lease;badLease.targetReference=0;check(!Matches(badLease,current,*p,receiver),"target class without retained reference refuses");

    // Independent ownership model: original copies the input into a persistent
    // engine graph frame. The adapter cannot access that destination frame.
    State state;std::uintptr_t inputValue=0,persistent=0,seen=0;unsigned calls=0,exchanges=0;
    const auto exchange=[&](std::uintptr_t at,std::uintptr_t oldValue,std::uintptr_t next)noexcept{++exchanges;if(at!=locals||inputValue!=oldValue)return false;inputValue=next;return true;};
    const auto original=[&]()noexcept{++calls;seen=inputValue;persistent=inputValue;};packet={locals,0};
    check(ForwardBorrowed(state,packet,lease,true,exchange,original)==Result::Redirected&&calls==1&&exchanges==2&&seen==lease.targetClass&&
        persistent==lease.targetClass&&!inputValue&&state.consumedGeneration==1&&!state.busy,"original sees class, graph retains it, caller input restored exactly once");
    calls=exchanges=0;check(ForwardBorrowed(state,packet,lease,true,exchange,original)==Result::OriginalOnly&&calls==1&&!exchanges,"consumed acceptance generation cannot replay");
    state={};calls=exchanges=0;check(ForwardBorrowed(state,packet,lease,false,exchange,original)==Result::OriginalOnly&&calls==1&&!exchanges&&!state.consumedGeneration,"Back null forwards unchanged without accepted lease");
    packet.value=inputValue=0x5050000;calls=exchanges=0;check(ForwardBorrowed(state,packet,lease,true,exchange,original)==Result::OriginalOnly&&calls==1&&!exchanges&&seen==0x5050000,"cosmetic nonnull destination remains unchanged");
    packet.value=0;calls=exchanges=0;check(ForwardBorrowed(state,packet,lease,true,exchange,original)==Result::OriginalOnly&&calls==1&&exchanges==1&&!state.consumedGeneration,"changed parameter between observation and conditional exchange refuses");
    inputValue=0;state={};calls=exchanges=0;
    const auto foreign=[&]()noexcept{original();inputValue=0x5060000;};
    check(ForwardBorrowed(state,packet,lease,true,exchange,foreign)==Result::RestoreConflict&&calls==1&&exchanges==2&&inputValue==0x5060000&&persistent==lease.targetClass&&!state.busy,"foreign input mutation is not rollback authority");
    inputValue=0;state={};calls=exchanges=0;Result nested=Result::MissingOriginal;
    const auto reentrant=[&]()noexcept{original();nested=ForwardBorrowed(state,packet,lease,true,exchange,original);};
    check(ForwardBorrowed(state,packet,lease,true,exchange,reentrant)==Result::Redirected&&nested==Result::OriginalOnly&&calls==2&&exchanges==2,"reentrant callback forwards without a second substitution");
    state={};packet.value=inputValue=0x5050000;calls=exchanges=0;
    check(ForwardBorrowed(state,packet,lease,true,exchange,original,0x5050000)==Result::Redirected&&calls==1&&exchanges==2&&
        inputValue==0x5050000&&persistent==lease.targetClass,"separately proved entry route may supply exact nonnull expected old input");

    // Native reflection is read by exact module caches, no lookup scan.
    const auto nativeFixture=[&](bool push){using namespace counselortransitiondisk;
        const std::uintptr_t owner=push?0x5070000:0x5080000,fn=push?0x5090000:0x50A0000;const unsigned count=push?3:2;
        r.Set(MenuFixture::Base+(push?PushOwner:ValidOwner),owner);r.Set(MenuFixture::Base+(push?PushConstructor:ValidConstructor),owner);r.Set(MenuFixture::Base+(push?PushFunction:ValidFunction),fn);
        f.Field(fn,owner,MenuFixture::FunctionClass,push?"PushMenu":"IsValidClass",0);fixture.Register(fn);r.Set(fn+cfgreflection::disk::Children,fn+0x100);
        r.Set(fn+cfgreflection::disk::FunctionFlags,push?PushFlags:ValidFlags);r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(push?24:9));
        r.Set(fn+cfgreflection::disk::StorageBytes,push?24:16);r.Set(fn+0xB0,MenuFixture::Base+(push?PushExecutor:ValidExecutor));r.Set(fn+0x68,std::uintptr_t{0});r.Set(fn+scriptdisk::Script,capture::ScriptHeader{});
        for(unsigned i=0;i<count;++i){const auto at=fn+0x100+0x100*i,kind=fn+0x1000+0x100*i;const bool ret=i+1==count;
            f.Name(kind,i==0?"ClassProperty":ret&&push?"ObjectProperty":"BoolProperty");f.Field(at,fn,kind,i==0?(push?"MenuClass":"Class"):ret?"ReturnValue":"bSkipTransitionOut",ret?0:at+0x100);
            f.Property(at,i==0||ret&&push?8:1,ret?(push?16:8):i==0?0:8);
            const std::uint64_t flags=i==0?(push?0x001C001040000280ULL:0x0018001040000280ULL):ret?(push?0x0018001040080788ULL:0x0018001040000780ULL):0x0018041040000282ULL;
            r.Set(at+cfgreflection::disk::PropertyFlags,flags);
        }return fn;
    };
    const auto push=nativeFixture(true),valid=nativeFixture(false);
    check(Native(reader,MenuFixture::Base,true,push)&&Native(reader,MenuFixture::Base,false,valid),"native named reflection consumers and exact layouts");
    r.Set(push+0x200+cfgreflection::disk::PropertyOffset,9);check(!Native(reader,MenuFixture::Base,true,push),"skip Boolean shifted layout refuses");r.Set(push+0x200+cfgreflection::disk::PropertyOffset,8);
    r.Set(push+0xB0,MenuFixture::Base+counselortransitiondisk::PushExecutor+1);check(!Native(reader,MenuFixture::Base,true,push),"foreign native PushMenu executor refuses");
    r.Set(push+0xB0,MenuFixture::Base+counselortransitiondisk::PushExecutor);
    for(const auto& op:counselortransitiondisk::Opcodes)r.Set(MenuFixture::Base+op.slot,MenuFixture::Base+op.executor);
    for(const auto& code:counselortransitiondisk::Codes)for(unsigned i=0;i<code.size;++i)r.Set(MenuFixture::Base+code.rva+i,code.bytes[i]);
    r.Set(MenuFixture::Base+startdisk::ScriptExecutor,startdisk::ScriptExecutorCode);
    r.Set(MenuFixture::Base+pickersourcedisk::ObjectSlot,MenuFixture::Base+pickersourcedisk::ObjectExecutor);r.Set(MenuFixture::Base+pickersourcedisk::ObjectExecutor,pickersourcedisk::ObjectCode);
    r.Set(MenuFixture::Base+pickersourcedisk::FinalSlot,MenuFixture::Base+pickersourcedisk::FinalExecutor);r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,pickersourcedisk::FinalCode);
    check(Code(reader,MenuFixture::Base),"all named opcode slots and full native code pins match owned image");
    for(const auto& op:counselortransitiondisk::Opcodes){r.Set(MenuFixture::Base+op.slot,std::uintptr_t{0});check(!Code(reader,MenuFixture::Base),"each foreign opcode slot refuses");r.Set(MenuFixture::Base+op.slot,MenuFixture::Base+op.executor);}
    for(const auto& code:counselortransitiondisk::Codes){r.Set(MenuFixture::Base+code.rva,static_cast<unsigned char>(code.bytes[0]^1));check(!Code(reader,MenuFixture::Base),"each changed native code pin refuses");r.Set(MenuFixture::Base+code.rva,code.bytes[0]);}
    Bounded<OptionsReader> bounded(reader);std::uintptr_t ignored=0;for(unsigned i=0;i<ReadLimit;++i)bounded.Read(1,ignored);check(!bounded.Read(1,ignored)&&bounded.reads==ReadLimit,"read cap never retries beyond explicit budget");
    EmptyHost empty;Consumer refused;unsigned reads=0;check(!Prepare(empty,current,*p,42,refused,&reads)&&reads<=ReadLimit&&!refused.push,"missing native evidence fails closed without producing proof");
    state={};forwarded=0;check(Dispatch(empty,state,current,*p,pickerprepare::Original(*p),consumer,receiver,nullptr,nullptr,nullptr,42,Forward)==Result::OriginalOnly&&forwarded==1,"rejected dispatch still forwards original once noexcept");
    check(Dispatch(empty,state,current,*p,pickerprepare::Original(*p),consumer,receiver,nullptr,nullptr,nullptr,42,nullptr)==Result::MissingOriginal&&forwarded==1,"missing original cannot fabricate forwarding");
    std::printf("CFG counselor transition adapter: %s (%u checks, %u failures). Owned captured fixtures; no game/native calls.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
