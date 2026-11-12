#include "NativeMenuRowAccess.hpp"
#include "OwnedByteReader.hpp"
#include "NativeMenuDispatch.hpp"
#include <cstdio>
#include <memory>
struct Reader : OwnedByteReader {
    bool ReadBytes(std::uintptr_t address,void* out,std::size_t count){auto* p=static_cast<unsigned char*>(out);
        for(std::size_t i=0;i<count;++i)if(!Read(address+i,p[i]))return false;return true;}
};
struct Engine {unsigned calls=0;unsigned Thread()const{return 42;}bool Invoke(std::uintptr_t,std::uintptr_t,void*){++calls;return false;}
    bool Factory(std::uintptr_t,std::uintptr_t&){++calls;return false;}};
struct Host {
    unsigned operation=0,failAt=0,appended=0,removed=0,conversions=0,calls=0;bool current=true,invalidIndex=false,removeFailure=false;
    // Each fixture owns a pool. Several fixtures in the same test must not
    // place all process-resident-sized pools on Windows'1MB default stack.
    std::unique_ptr<cfgmenu::reflection::RetainedMenuText> pool=std::make_unique<cfgmenu::reflection::RetainedMenuText>();
    bool Current(std::uint64_t generation){return current&&generation==19;}
    bool Op(){++calls;return ++operation!=failAt;}
    bool Append(std::uintptr_t& widget,bool& attached){widget=0x100000+8*appended++;attached=true;return Op();}
    bool BorrowNativeRow(std::uintptr_t&,bool&,const cfgmenu::rows::Spec&){return false;}
    bool CopyOptions(std::uintptr_t,const cfgmenu::rows::Spec& spec){return Op()&&spec.Valid();}
    bool Text(std::u16string_view label,cfgmenu::reflection::RetainedMenuText::Request& out){
        if(!Op())return false;if(pool->Find(label,out))return true;
        if(!pool->Begin(label,out))return false;++conversions;
        const std::uintptr_t pointers[]{0x400000,0x500000};std::memcpy(out.packet->result.bytes.data(),pointers,16);
        return pool->Complete(out,true);
    }
    bool SetText(std::uintptr_t,std::u16string_view,const cfgmenu::reflection::RetainedMenuText::Request& text){return Op()&&pool->Borrow(text);}
    bool Index(std::uintptr_t,unsigned& value){value=invalidIndex?33:0;return true;}
    bool RemoveOwned(std::uintptr_t){++removed;return !removeFailure;}
};
void PutText(cfgnames::Text& text,std::u16string_view value){text={};std::copy(value.begin(),value.end(),text.data.begin());text.count=text.baseCount=static_cast<unsigned>(value.size());}
template<std::size_t N> cfgmenu::capture::FunctionEvidence Frame(const cfgmenu::execdisk::Binding& b,
    const std::array<cfgmenu::execdisk::Parameter,N>& fields){
    cfgmenu::capture::FunctionEvidence f;f.function.object=0x200000;f.function.owner=0x300000;
    PutText(f.function.label,b.name);PutText(f.function.kind,u"Function");f.flags=b.flags;f.parameterBytes=static_cast<std::uint16_t>(cfgmenu::native::ParameterExtent(b,fields));
    f.storageBytes=static_cast<int>(b.frame);f.nativeExecutor=0x100000+b.executor;f.fieldsCount=N;
    for(unsigned i=0;i<N;++i){auto& p=f.fields[i];const auto& source=fields[i];p.field.object=0x400000+256*i;p.field.owner=f.function.object;
        PutText(p.field.label,source.name);PutText(p.field.kind,cfgmenu::native::Kind(source.kind));
        p.dimension=static_cast<int>(source.dimension);p.offset=static_cast<int>(source.offset);
        p.elementBytes=source.kind==12?1:source.kind==21?16:source.kind==28?24:8;p.flags=cfgmenu::native::RuntimeFlags(b,source);
        if(b.name==u"RemoveChild")p.flags=source.name==u"Content"?0x0018001040080280ULL:0x0018001040000780ULL;}
    return f;
}
template<std::size_t N> unsigned TestFrame(const cfgmenu::execdisk::Binding& b,const std::array<cfgmenu::execdisk::Parameter,N>& fields){
    const auto f=Frame(b,fields);unsigned failures=0;
    const auto matches=[&](const auto& value){return cfgmenu::native::FrameMatches(value,0x100000,0x300000,b,fields);};
    if(!matches(f))++failures;
    for(unsigned bit=0;bit<32;++bit){auto changed=f;changed.flags^=std::uint32_t{1}<<bit;if(matches(changed))++failures;}
    for(unsigned i=0;i<N;++i){for(unsigned bit=0;bit<64;++bit){auto changed=f;changed.fields[i].flags^=std::uint64_t{1}<<bit;if(matches(changed))++failures;}
        auto changed=f;++changed.fields[i].offset;if(matches(changed))++failures;changed=f;changed.fields[i].dimension=2;if(matches(changed))++failures;}
    auto changed=f;++changed.nativeExecutor;if(matches(changed))++failures;changed=f;changed.destructorLink=0x400000;if(matches(changed))++failures;
    changed=f;changed.script.count=1;if(matches(changed))++failures;changed=f;++changed.storageBytes;if(matches(changed))++failures;
    changed=f;++changed.parameterBytes;if(matches(changed))++failures;
    if(f.parameterBytes!=b.frame){changed=f;changed.parameterBytes=static_cast<std::uint16_t>(b.frame);if(matches(changed))++failures;}
    if(N>1){changed=f;changed.fields[1]=changed.fields[0];if(matches(changed))++failures;}
    return failures;
}
int main(){using namespace cfgmenu::rows;unsigned failures=0;
    const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
    failures+=TestFrame(cfgmenu::execdisk::Create,cfgmenu::execdisk::CreateParameters);
    failures+=TestFrame(cfgmenu::execdisk::AddChildToVerticalBox,cfgmenu::execdisk::AddChildToVerticalBoxParameters);
    failures+=TestFrame(cfgmenu::execdisk::SetText,cfgmenu::execdisk::SetTextParameters);
    failures+=TestFrame(cfgmenu::execdisk::Conv_StringToText,cfgmenu::execdisk::Conv_StringToTextParameters);
    failures+=TestFrame(cfgmenu::execdisk::RemoveChild,cfgmenu::execdisk::RemoveChildParameters);
    check(cfgmenu::native::ParameterExtent(cfgmenu::execdisk::RemoveChild,cfgmenu::execdisk::RemoveChildParameters)==9,
        "physical CFG RemoveChild has9 parameter bytes in16 storage bytes");
    failures+=TestFrame(cfgmenu::execdisk::ShowLoadingAndTravel,cfgmenu::execdisk::ShowLoadingAndTravelParameters);
    check(cfgmenu::native::ParameterExtent(cfgmenu::execdisk::ShowLoadingAndTravel,cfgmenu::execdisk::ShowLoadingAndTravelParameters)==49&&
        cfgmenu::execdisk::ShowLoadingAndTravel.frame==56,"physical CFG travel uses49 parameter bytes and56 storage bytes");
    auto overlapping=cfgmenu::execdisk::RemoveChildParameters;overlapping[0].offset=7;
    check(!cfgmenu::native::ParameterExtent(cfgmenu::execdisk::RemoveChild,overlapping),"overlapping generated descriptor refused");
    auto conversion=Frame(cfgmenu::execdisk::Conv_StringToText,cfgmenu::execdisk::Conv_StringToTextParameters);
    for(auto& property:conversion.fields)if(property.field.label.View()==u"InString")PutText(property.field.label,u"inString");
    check(cfgmenu::native::FrameMatches(conversion,0x100000,0x300000,cfgmenu::execdisk::Conv_StringToText,cfgmenu::execdisk::Conv_StringToTextParameters),"actual CFG FString parameter spelling");
    Spec spec;spec.id=cfgmenu::RowId::Duration;spec.title=u"Match Length";spec.labels[0]=u"30 minutes";spec.labels[1]=u"60 minutes";spec.count=2;spec.selected=1;
    for(unsigned fault=0;fault<=12;++fault){Runtime runtime;Host host;host.failAt=fault;
        Spec pair[]{spec,spec};pair[1].id=cfgmenu::RowId::Difficulty;
        check(runtime.Begin(19,pair,2),"begin two rows");
        for(unsigned i=0;i<50;++i){const auto before=host.operation;runtime.Step(host,i*25);check(host.operation-before<=1,"one phase per callback");}
        check(runtime.CurrentState()==(fault?State::Failed:State::Ready),"phase failure rolls back");
        check(!fault||host.removed==host.appended,"only our attached rows removed");
    }
    Runtime runtime;Host host;check(runtime.Begin(19,&spec,1),"native row lifecycle");
    check(runtime.Step(host,100)&&!runtime.Step(host,124)&&!runtime.Step(host,99)&&host.operation==1,"no catch-up or clock-back work");
    for(unsigned i=1;i<=5;++i)check(runtime.Step(host,100+i*25),"next paced phase");
    std::array<unsigned,Capacity> choices;check(runtime.Choices(host,19,choices)&&choices[0]==0,"atomic Start values");
    host.invalidIndex=true;choices.fill(7);check(!runtime.Choices(host,19,choices)&&choices[0]==0,"invalid index clears whole output");
    host.current=false;check(!runtime.Choices(host,19,choices),"changed route rejects Start");
    Runtime detached;Host other;check(detached.Begin(19,&spec,1)&&detached.Step(other,0),"attach before route loss");
    other.current=false;detached.Step(other,25);check(detached.CurrentState()==State::Detached&&!other.removed,"no calls into old tree after Back/travel");
    Runtime own;Host ownHost;std::u16string changing=u"Match Length";auto ownedSpec=spec;ownedSpec.title=changing;
    check(own.Begin(19,&ownedSpec,1),"owned input copied");changing.assign(u"Changed");
    check(own.At(0).spec.title==u"Match Length","caller text lifetime independent");
    auto prior=own.At(0).spec;own.Detached();
    check(own.Begin(20,&prior,1)&&own.At(0).spec.title==u"Match Length"&&own.At(0).spec.labels[1]==u"60 minutes",
        "reopening copies labels before retiring aliased previous storage");
    Runtime retirement;Host retiringHost;check(retirement.Begin(19,&spec,1),"retiring lifecycle starts");
    for(unsigned i=0;i<6;++i)check(retirement.Step(retiringHost,i*25),"retiring fixture builds row");
    check(retirement.Retire()&&retirement.Retire()&&retirement.CurrentState()==State::Retiring,"idempotent close request");
    check(retirement.Step(retiringHost,150)&&retiringHost.removed==1&&retirement.Step(retiringHost,175)&&
        retirement.CurrentState()==State::Empty,"paced cleanup ends Empty, not failed");
    check(retirement.Begin(20,&spec,1),"cleaned cached view reopens");
    cfgmenu::reflection::RetainedMenuText retained;cfgmenu::reflection::RetainedMenuText::Request request,reused;
    check(retained.Begin(u"Match Length",request),"label conversion");const std::uintptr_t pointers[]{0x400000,0x500000};
    std::memcpy(request.packet->result.bytes.data(),pointers,16);check(retained.Complete(request,true),"retain native text");
    for(unsigned i=0;i<1000;++i)check(retained.Find(u"Match Length",reused)&&reused.packet==request.packet&&retained.Used()==1,"repeat menu retains same text without leak growth");
    Reader reader;Engine engine;cfgmenu::rows::NativeAccess<Reader,Engine> access(reader,engine,retained);cfgmenu::active::Context context;
    check(!access.Prepare(context,19),"missing live native owner rejects preparation");
    std::uintptr_t widget=0;bool attached=false;unsigned index=0;
    check(!access.Append(widget,attached)&&!access.CopyOptions(0x400000,spec)&&!access.Text(u"Title",request)&&
        !access.SetText(0x400000,u"TitleText",request)&&!access.Index(0x400000,index)&&!access.RemoveOwned(0x400000)&&!engine.calls,
        "production adapter refuses every engine operation without current ownership");
    check(!access.WarmBinding(cfgmenu::execdisk::RemoveChild,cfgmenu::execdisk::RemoveChildParameters,cfgmenu::execdisk::RemoveChildCode,
        cfgmenu::execdisk::RemoveChildFactoryCode)&&!engine.calls,"missing ownership cannot initialize a native cache");
    NativeDispatch dispatch(0,GetCurrentThreadId(),false);std::uintptr_t result=0;
    check(!dispatch.BeginPhase()&&!dispatch.Invoke(0x100000,0x200000,&result)&&!dispatch.Factory(0x100000,result)&&!dispatch.Calls(),
        "actual Windows dispatcher refuses without exact-image preparation");
    std::printf("CFG native row runtime/ABI/rollback/pacing/input lifetime tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
