#define CFGMENU_CATALOGUE_FIXTURE_ONLY
#include "NativeFrontendCataloguesTests.cpp"
#include "NativePendingModeAccess.hpp"
struct MemberFixture:CatalogueFixture {
    static constexpr std::uintptr_t Pending=0x900000,AliasField=0x910000,ClassKind=0x920000,StringKind=0x930000;
    MemberFixture(){using namespace cfgmenu;
        reader.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{8,1}});
        Name(Pending,2,u"PendingOfflineMode");Name(AliasField,3,u"Alias");
        Name(ClassKind,4,u"ClassProperty");Name(StringKind,5,u"StrProperty");
        reader.Set(FrontendType+cfgreflection::disk::Children,Pending);
        reader.Set(ModeDefinition+cfgreflection::disk::Children,AliasField);
        const auto property=[&](std::uintptr_t at,std::uintptr_t owner,std::uintptr_t kind,int bytes,int offset){
            reader.Set(at+cfgprobe::ObjectClass,kind);reader.Set(at+0x20,owner);reader.Set(at+cfgreflection::disk::Next,std::uintptr_t{0});
            reader.Set(at+cfgreflection::disk::PropertyDimensions,1);reader.Set(at+cfgreflection::disk::PropertyElementSize,bytes);
            reader.Set(at+cfgreflection::disk::PropertyFlags,std::uint64_t{4});reader.Set(at+cfgreflection::disk::PropertyOffset,offset);
        };
        property(Pending,FrontendType,ClassKind,8,selectiondisk::PendingOfflineMode);
        property(AliasField,ModeDefinition,StringKind,16,selectiondisk::Alias);
        reader.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,5);reader.Set(Mode+cfgprobe::lifetime::ObjectIndexOffset,4);
        reader.Set(Registry+24*4,Mode);reader.Set(Registry+24*4+16,104);
    }
};
// Compile the actual Windows adapter, not just a substitute writer interface.
template class cfgmenu::sandboxstart::NativePendingModeAccess<CatalogueReader>;
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    {MemberFixture f;active::Context context;
        f.reader.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,static_cast<std::uint32_t>(GetCurrentThreadId()));
        f.reader.Set(f.Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x001C001040000204});
        check(active::Observe(f.reader,f.Module,context),"actual owned Windows thread context");
        cfgreflection::Property mode;
        check(pendingmember::PropertyAt(f.catalogueReader,context,f.Pending,f.FrontendType,u"PendingOfflineMode",u"ClassProperty",8,0x520,mode),"Windows adapter authored mode proof");
        f.reader.Set(f.FrontendType+cfgreflection::disk::Children,std::uintptr_t{0});
        sandboxstart::NativePendingModeAccess<CatalogueReader> generic(f.catalogueReader),authored(f.catalogueReader);
        check(!generic.Prepare(context,true),"actual Windows generic adapter retains native discovery requirement");
        check(authored.PrepareWithAuthoredMode(context,true,mode),"actual Windows authored adapter prepares without native child discovery");
        check(!authored.PrepareWithAuthoredMode(context,true,mode),"actual Windows adapter refuses repeated preparation");
        // Preparation reads owned fixture memory only. Do not invoke the
        // Windows pointer exchange on these synthetic fixture addresses.
    }
    for(unsigned fault=0;fault<17;++fault){MemberFixture f;active::Context context;
        f.reader.Set(f.Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x001C001040000204});
        check(active::Observe(f.reader,Fixture::Module,context),"authored-mode fixture context");
        cfgreflection::Property mode;
        check(pendingmember::PropertyAt(f.catalogueReader,context,f.Pending,f.FrontendType,u"PendingOfflineMode",u"ClassProperty",8,0x520,mode),"saved exact authored-mode descriptor");
        f.reader.Set(f.FrontendType+cfgreflection::disk::Children,std::uintptr_t{0});
        pendingmember::Binding binding;unsigned reads=0;
        check(!pendingmember::Observe(f.catalogueReader,context,42,true,binding),"generic path still requires native child discovery");
        check(pendingmember::ObserveWithAuthoredMode(f.catalogueReader,context,42,true,mode,binding,&reads)&&binding.authored,
            "authored-mode path works with empty native Children");
        if(fault==1)f.reader.Set(f.Pending+cfgreflection::disk::PropertyOffset,0x528);
        if(fault==2)f.reader.Set(f.Pending+cfgreflection::disk::PropertyDimensions,2);
        if(fault==3)f.reader.Set(f.Pending+cfgreflection::disk::PropertyElementSize,16);
        if(fault==4)f.reader.Set(f.Pending+cfgprobe::ObjectClass,f.StringKind);
        if(fault==5)f.reader.Set(f.Pending+0x20,f.ModeDefinition);
        if(fault==6)f.reader.Set(f.Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0014000000000004});
        if(fault==7)f.reader.Set(f.AliasField+cfgreflection::disk::PropertyOffset,0x58);
        if(fault==8)f.reader.Set(f.ModeDefinition+cfgreflection::disk::Children,std::uintptr_t{0});
        if(fault==9)f.reader.Set(f.Module+selectiondisk::SCGame_MenuConstructor,f.ModeDefinition);
        if(fault==10)f.reader.Set(f.Registry+24*4+16,105);
        if(fault==11)f.reader.Set(f.Registry+24*4+16,0);
        if(fault==12)f.reader.Set(f.Pending+cfgreflection::disk::Next,f.AliasField);
        if(fault==13)f.Name(f.Pending,2,u"PendingOfflineSettingsWidget");
        if(fault==14)f.reader.Set(f.Mode+cfgprobe::ObjectClass,f.Sandbox);
        if(fault==15)f.reader.Set(f.Registry+24*3+16,999);
        check(pendingmember::Current(f.catalogueReader,binding,fault==16?41u:42u,&reads)==(fault==0),
            "authored retained mode refuses changed schema/name/link/owner/current context");
        check(reads<=pendingmember::ReadLimit,"authored Start path remains bounded");
        pendingmember::Binding denied;
        if(fault&&fault!=10)check(!pendingmember::ObserveWithAuthoredMode(f.catalogueReader,context,fault==16?41u:42u,true,mode,denied)&&!denied.authored,
            "stale authored descriptor cannot establish new binding");
    }
    for(unsigned fault=0;fault<17;++fault){MemberFixture f;active::Context context;
        check(active::Observe(f.reader,Fixture::Module,context),"active frontend fixture");
        pendingmember::Binding binding;unsigned reads=0;
        check(pendingmember::Observe(f.catalogueReader,context,42,true,binding,&reads),"concrete named native instance schema");
        if(!fault)std::printf("Pending mode property observation reads=%u\n",reads);
        if(fault==1)f.reader.Set(f.Pending+cfgreflection::disk::PropertyOffset,0x4D0);
        if(fault==2)f.reader.Set(f.Pending+cfgreflection::disk::PropertyDimensions,2);
        if(fault==3)f.reader.Set(f.Pending+cfgreflection::disk::PropertyElementSize,40);
        if(fault==4)f.reader.Set(f.Pending+cfgprobe::ObjectClass,f.StringKind);
        if(fault==5)f.reader.Set(f.Pending+0x20,f.ModeDefinition);
        if(fault==6)f.reader.Set(f.Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{8});
        if(fault==7)f.reader.Set(f.AliasField+cfgreflection::disk::PropertyOffset,0x58);
        if(fault==8)f.reader.Set(f.AliasField+cfgreflection::disk::PropertyElementSize,8);
        if(fault==9)f.reader.Set(f.Module+selectiondisk::SCGame_MenuConstructor,f.ModeDefinition);
        if(fault==10)f.reader.Set(f.Registry+24*4+16,105);
        if(fault==11)f.reader.Set(f.Registry+24*4+16,0);
        if(fault==12)f.reader.Set(f.FrontendType+cfgreflection::disk::Children,std::uintptr_t{0});
        if(fault==13)f.reader.Set(f.Pending+cfgreflection::disk::Next,f.AliasField);
        if(fault==14)f.reader.Set(f.Mode+cfgprobe::ObjectClass,f.Sandbox);
        if(fault==15)f.reader.Set(f.Registry+24*3+16,999);
        check(pendingmember::Current(f.catalogueReader,binding,fault==16?41u:42u,&reads)==(fault==0),
            "only unchanged schema/live owner/positive mode serial authorizes one instance pointer");
        check(reads<=pendingmember::ReadLimit,"Start-only bounded work");
    }
    {MemberFixture f;active::Context c;active::Observe(f.reader,Fixture::Module,c);pendingmember::Binding out;unsigned reads=999;
        check(!pendingmember::Observe(f.catalogueReader,c,42,false,out,&reads)&&!reads&&!out.context.widget,"unknown image zero reads/no partial authority");
        sandboxstart::NativePendingModeAccess<CatalogueReader> access(f.catalogueReader);
        const auto before=f.reader.reads;
        check(access.ChangePendingMode(c,f.OfflineMode,f.Sandbox)==sandboxstart::WriteResult::Refused&&before==f.reader.reads,
            "unprepared Windows writer refuses without memory access");}
    std::printf("CFG concrete pending-mode member/Windows writer: %s (%u failures). No game access/writes.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
