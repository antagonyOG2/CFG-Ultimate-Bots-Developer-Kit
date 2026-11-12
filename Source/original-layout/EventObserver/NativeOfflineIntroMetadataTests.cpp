#include "NativeOfflineIntroMetadata.hpp"
#include <cstdio>
#include <functional>
#include "OwnedByteReader.hpp"
namespace m=cfgmenu::intrometadata;
#define CHECK(x) do{if(!(x)){std::printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(false)
template class m::WindowsHost<cfgmenu::promotion::CurrentProcessReader>;
struct Host {
    m::Packet active{},klass{},counselor{},killer{};std::uintptr_t owner=0;
    bool fresh=true;unsigned calls=0,failAt=0;std::function<void()> during;
    Host(){counselor.bytes[16]=1;killer.bytes[16]=2;active=counselor;}
    m::Packet Killer(){return killer;}m::Packet Counselor(){return counselor;}
    std::uintptr_t Human(){return 0x100000;}bool Fresh(){return fresh;}
    bool Read(m::Packet& a,m::Packet& k,std::uintptr_t& o){a=active;k=klass;o=owner;return fresh;}
    bool Step(){++calls;if(during)during();return !failAt||calls!=failAt;}
    bool SetClass(const m::Packet& old,const m::Packet& next){if(!m::SameValue(klass,old)||!Step())return false;klass=next;return fresh;}
    bool SetActive(const m::Packet& old,const m::Packet& next){if(!m::SamePath(active,old)||!Step())return false;active=next;return fresh;}
    bool Publish(std::uintptr_t old,std::uintptr_t next){if(owner!=old||!Step())return false;owner=next;return fresh;}
};
struct SchemaFixture {
    static constexpr std::uintptr_t Module=0x100000,Names=0x200000,Chunk=0x300000,Object=0x500000,
        Type=0x510000,Parent=0x520000,Field=0x530000,Kind=0x540000,Other=0x550000;
    OwnedByteReader r;unsigned names=0;
    void Name(std::uintptr_t object,const char* text){auto index=names++;const auto entry=std::uintptr_t{0x400000}+index*0x100;
        r.Set(Chunk+8*index,entry);r.Set(entry,std::uint32_t{});std::array<unsigned char,128> bytes{};
        for(unsigned i=0;text[i];++i)bytes[i]=static_cast<unsigned char>(text[i]);
        r.Set(entry+cfgnames::disk::EntryText,bytes);r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});}
    SchemaFixture(bool soft){
        r.Set(Module+cfgnames::disk::Global,Names);r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{20,1}});r.Set(Names,Chunk);
        r.Set(Object+cfgprobe::ObjectClass,Type);r.Set(Type+cfgprobe::ClassSuper,Parent);r.Set(Parent+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(Type+cfgreflection::disk::Children,std::uintptr_t{});r.Set(Parent+cfgreflection::disk::Children,Other);
        Name(Kind,soft?"SoftClassProperty":"ObjectProperty");Name(Field,"Target");Name(Other,"Sibling");
        for(const auto field:{Field,Other}){r.Set(field,Module+cfgmenu::softdisk::PropertyVtable);
            r.Set(field+cfgprobe::ObjectClass,Kind);r.Set(field+0x20,Parent);r.Set(field+cfgreflection::disk::Next,field==Other?Field:std::uintptr_t{});
            r.Set(field+0x30,int{1});r.Set(field+0x34,soft?40:8);r.Set(field+0x44,int{0x490});}
    }
};
// Execute the three independently pinned leaf bodies in TEST-OWNED memory at
// their original relative distances. No game process, UObject or engine heap.
// Unmapped allocator/memcpy targets make an accidental allocating path fail.
struct OwnedAssignment {
    unsigned char* base=nullptr;
    OwnedAssignment(){base=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x7C0000,MEM_RESERVE,PAGE_NOACCESS));}
    ~OwnedAssignment(){if(base)VirtualFree(base,0,MEM_RELEASE);}
    template<std::size_t N> bool Place(std::uintptr_t rva,const std::array<unsigned char,N>& bytes){
        if(!base)return false;const auto page=rva&~std::uintptr_t{4095};
        if(!VirtualAlloc(base+page,4096,MEM_COMMIT,PAGE_READWRITE))return false;
        std::memcpy(base+rva,bytes.data(),N);DWORD before=0;
        return VirtualProtect(base+page,4096,PAGE_EXECUTE_READ,&before)&&FlushInstructionCache(GetCurrentProcess(),base+page,4096);
    }
    bool Prepare(){return Place(cfgmenu::softdisk::CopyInternal,cfgmenu::softdisk::CopyInternalCode)&&
        Place(cfgmenu::softdisk::PathCopy,cfgmenu::softdisk::PathCopyCode)&&
        Place(cfgmenu::softdisk::FStringAssign,cfgmenu::softdisk::FStringAssignCode);}
};
int main(){
    {OwnedByteReader reader;m::Context context;cfgmenu::NativeMetadataDiagnostic diagnostic;
     context.diagnostic=&diagnostic;m::WindowsHost<OwnedByteReader> host(reader,context);
     m::Packet active{},klass{},next{};std::uintptr_t owner=0;next.bytes[16]=1;
     CHECK(!host.Read(active,klass,owner)&&diagnostic.operation==1&&diagnostic.step==1&&diagnostic.freshStage==1&&diagnostic.packetMask==0);
     CHECK(!host.SetClass(klass,next)&&diagnostic.operation==2&&diagnostic.freshStage==1);
     CHECK(!host.SetActive(active,next)&&diagnostic.operation==3&&diagnostic.freshStage==1);
     CHECK(!host.Publish(0,0)&&diagnostic.operation==4&&diagnostic.publishStatus==0&&diagnostic.freshStage==1);
     CHECK(reader.reads==0&&diagnostic.observedMask==0);
     std::puts("Metadata diagnostic refusal never grants lookup/write authority: PASS");}
    {OwnedAssignment native;CHECK(native.Prepare());
     CHECK(cfgmenu::intrometadatadisk::AssignValuesVerified);
     CHECK(cfgmenu::intrometadatadisk::AssignValues==cfgmenu::softdisk::CopyInternal);
     const auto fn=reinterpret_cast<std::uintptr_t>(native.base)+cfgmenu::intrometadatadisk::AssignValues;
     struct Guarded{std::uint64_t first=0x1122334455667788ULL;m::Packet value{};std::uint64_t last=0x8877665544332211ULL;} dst;
     m::Packet source{};for(unsigned i=0;i<24;++i)source.bytes[i]=static_cast<unsigned char>(i+1);
     dst.value.bytes[12]=0xAD;const auto original=source;
     CHECK(m::AssignPlain(fn,&dst.value,&source));
     CHECK(std::memcmp(dst.value.bytes.data(),source.bytes.data(),12)==0&&m::SameValue(dst.value,source));
     CHECK(dst.value.bytes[12]==0xAD&&source.bytes==original.bytes);
     CHECK(dst.first==0x1122334455667788ULL&&dst.last==0x8877665544332211ULL);
     const m::Packet empty{};CHECK(m::AssignPlain(fn,&dst.value,&empty));CHECK(m::SameValue(dst.value,empty));
     CHECK(!m::AssignPlain(fn,&dst.value,&dst.value));CHECK(!m::AssignPlain(0,&dst.value,&source));
     CHECK(!m::AssignPlain(fn,nullptr,&source));CHECK(!m::AssignPlain(fn,&dst.value,nullptr));
     for(unsigned i=24;i<40;++i){
         auto invalid=source;invalid.bytes[i]=1;const auto before=dst.value;
         CHECK(!m::AssignPlain(fn,&dst.value,&invalid)&&dst.value.bytes==before.bytes);
         dst.value=invalid;CHECK(!m::AssignPlain(fn,&dst.value,&source)&&dst.value.bytes==invalid.bytes);dst.value={};
     }
     std::puts("Pinned native plain SoftClass ABI/copy/restore/nonallocating boundary: PASS");}
    for(bool soft:{false,true})for(unsigned fault=0;fault<13;++fault){SchemaFixture f(soft);m::SchemaLease lease;
        const auto observe=[&](){return lease.Observe(f.r,f.Module,f.Object,u"Target",0x490,soft);};
        CHECK(observe());const auto initial=f.r.reads;CHECK(observe());const auto retained=f.r.reads-initial;
        CHECK(retained<initial&&retained<32);
        if(!fault){for(unsigned n=0;n<30;++n)CHECK(observe());std::printf("Schema lease %s: initial=%u retained=%u reads\n",soft?"soft":"object",initial,retained);continue;}
        if(fault==1)f.r.Set(f.Field+0x20,f.Type);
        if(fault==2)f.r.Set(f.Field+cfgprobe::ObjectClass,f.Type);
        if(fault==3)f.r.Set(f.Field+cfgnames::disk::ObjectName,cfgnames::Words{0,0});
        if(fault==4)f.r.Set(f.Field+0x30,int{2});
        if(fault==5)f.r.Set(f.Field+0x34,int{16});
        if(fault==6)f.r.Set(f.Field+0x44,int{0x498});
        if(fault==7)f.r.Set(f.Field,f.Type);
        if(fault==8)f.r.Set(f.Object+cfgprobe::ObjectClass,f.Parent);
        if(fault==9)f.r.Set(f.Type+cfgprobe::ClassSuper,std::uintptr_t{});
        if(fault==10)f.r.Set(f.Other+cfgreflection::disk::Next,std::uintptr_t{});
        if(fault==11)f.r.Set(f.Type+cfgreflection::disk::Children,f.Field);
        if(fault==12)f.r.Set(f.Kind+cfgnames::disk::ObjectName,cfgnames::Words{1,0});
        CHECK(!observe());SchemaFixture restored(soft);f.r=restored.r;CHECK(!observe()); // no reacquisition after drift
    }
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());CHECK(h.calls==3&&h.owner==h.Human());CHECK(b.HumanRestartForbidden());
     CHECK(b.Restore());CHECK(h.calls==6&&h.owner==0&&m::SamePath(h.active,h.counselor));CHECK(!b.HumanRestartForbidden());CHECK(b.Restore());CHECK(h.calls==6);}
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());h.owner=0x200000;h.klass.bytes[16]=3;CHECK(b.Restore());
     CHECK(h.calls==4&&h.owner==0x200000&&h.klass.bytes[16]==3&&m::SamePath(h.active,h.counselor));}
    {Host h;h.killer.bytes[24]=1;m::Borrow<Host> b(h);CHECK(!b.Prepare());CHECK(h.calls==0);}
    {Host h;h.klass.bytes[24]=1;m::Borrow<Host> b(h);CHECK(!b.Prepare());CHECK(h.calls==0);}
    {Host h;h.owner=0x200000;m::Borrow<Host> b(h);CHECK(!b.Prepare());CHECK(h.calls==0);}
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());
     h.during=[&](){if(h.calls==4)h.owner=0x200000;}; // native SetActive broadcast transfers ownership, same class
     CHECK(b.Restore());CHECK(h.calls==4&&h.owner==0x200000&&m::SamePath(h.klass,h.killer)&&m::SamePath(h.active,h.counselor));}
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());
     h.during=[&](){if(h.calls==4){h.owner=0x200000;h.klass.bytes[16]=3;}};
     CHECK(b.Restore());CHECK(h.calls==4&&h.owner==0x200000&&h.klass.bytes[16]==3);}
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());h.fresh=false;CHECK(!b.Restore());CHECK(h.calls==3&&b.HumanRestartForbidden());}
    {Host h;m::Borrow<Host> b(h);CHECK(b.Prepare());h.active.bytes[16]=3;CHECK(!b.Restore());CHECK(h.calls==3&&h.active.bytes[16]==3);}
    {Host h;m::Borrow<Host> b(h);h.during=[&](){(void)b.Prepare();};CHECK(!b.Prepare());CHECK(h.calls==1&&b.Status()==m::State::Indeterminate);}
    for(unsigned i=1;i<=3;++i){Host h;h.failAt=i;m::Borrow<Host> b(h);CHECK(!b.Prepare());CHECK(h.calls==i&&b.Status()==m::State::Indeterminate);CHECK(!b.Prepare());CHECK(h.calls==i);}
    std::puts("Native offline intro metadata ownership/restore tests: PASS");return 0;
}
