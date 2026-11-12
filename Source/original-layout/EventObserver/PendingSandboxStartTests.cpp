#define CFGMENU_CATALOGUE_FIXTURE_ONLY
#include "NativeFrontendCataloguesTests.cpp"
#include "PendingSandboxStart.hpp"
using namespace cfgmenu;
struct RouteFixture:CatalogueFixture {
    RouteFixture(){Name(OfflineMode,0,u"ModeDef_OfflineBots_C");Name(Sandbox,1,u"ModeDef_Sandbox_C");
        Text(ModeDefaults+selectiondisk::Alias,Alias,u"oflb");Text(SandboxDefaults+selectiondisk::Alias,SandboxAlias,u"snbx");}
};
struct RouteHost {
    RouteFixture& f;sandboxstart::Route& route;active::Context context{};
    unsigned writes=0,forwards=0,bindings=0;bool armed=true,code=true,refuse=false,fault=false;
    bool afterWriteCode=false,afterWriteOwner=false,changeMap=false,forwardFault=false,reenter=false,travelTearsDown=false;
    bool nestedConsumed=false;
    bool Armed(const active::Context& c){return armed&&c==context;}
    bool OwnedStartBindings(const active::Context& c){++bindings;return code&&c==context;}
    sandboxstart::WriteResult ChangePendingMode(const active::Context& c,std::uintptr_t old,std::uintptr_t next){
        ++writes;std::uintptr_t observed=0;
        if(refuse||!active::Current(f.catalogueReader,c,c.thread)||
            !f.reader.Read(c.mode+selectiondisk::PendingOfflineMode,observed)||observed!=old)return sandboxstart::WriteResult::Refused;
        if(fault)return sandboxstart::WriteResult::Indeterminate;
        f.reader.Set(c.mode+selectiondisk::PendingOfflineMode,next);
        if(writes==1){if(afterWriteCode)code=false;if(afterWriteOwner)f.reader.Set(c.controller+execdisk::ControllerHUD,RouteFixture::Settings);
            if(changeMap)f.reader.Set(c.mode+selectiondisk::PendingOfflineMap,RouteFixture::MapA);}
        return sandboxstart::WriteResult::Changed;
    }
    bool ForwardOriginalStart(const active::Context& c){
        ++forwards;
        if(reenter){const auto reads=f.reader.reads;unsigned consumed=999;
            nestedConsumed=route.Start(f.catalogueReader,*this,c,true,&consumed)==sandboxstart::Result::Consumed&&consumed==0&&f.reader.reads==reads;}
        if(travelTearsDown)f.reader.Set(c.controller+execdisk::ControllerHUD,RouteFixture::Settings);
        return !forwardFault;
    }
};
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned switched=0;switched<2;++switched){RouteFixture f;sandboxstart::Route route;RouteHost host{f,route};
        check(active::Observe(f.reader,Fixture::Module,host.context),"native OfflineBots owner");
        if(switched)f.reader.Set(f.Mode+selectiondisk::PendingOfflineMode,f.Sandbox);
        const auto result=route.Start(f.catalogueReader,host,host.context,true,nullptr,true,sandboxstart::Target::NativeOfflineBots);
        std::uintptr_t pending=0;
        check(result==sandboxstart::Result::Forwarded&&host.forwards==1&&host.writes==switched&&
            f.reader.Read(f.Mode+selectiondisk::PendingOfflineMode,pending)&&pending==f.OfflineMode,"explicit native OfflineBots target preserved/switched exactly once");}
    for(unsigned strict=0;strict<2;++strict){RouteFixture f;sandboxstart::Route route;RouteHost host{f,route};
        check(active::Observe(f.reader,Fixture::Module,host.context),"strict native owner");
        if(strict)f.reader.Set(f.Mode+selectiondisk::PendingOfflineMode,f.Sandbox);
        const auto result=route.Start(f.catalogueReader,host,host.context,true,nullptr,false);
        check(result==(strict?sandboxstart::Result::Forwarded:sandboxstart::Result::Refused),"strict route accepts only native Sandbox");
        check(host.writes==0&&host.forwards==strict,"strict route never switches pending mode or repairs human possession");
    }
    for(unsigned fault=0;fault<17;++fault){RouteFixture f;sandboxstart::Route route;RouteHost host{f,route};
        check(active::Observe(f.reader,Fixture::Module,host.context),"current owner fixture");
        if(fault==1)host.armed=false;if(fault==2)host.code=false;
        if(fault==3)f.Text(f.SandboxDefaults+selectiondisk::Alias,f.SandboxAlias,u"sandbox");
        if(fault==4)f.reader.Set(f.SandboxMaps,f.MapA);
        if(fault==5)f.Name(f.OfflineMode,0,u"ModeDef_Hunt_C");
        if(fault==6)host.refuse=true;if(fault==7)host.fault=true;
        if(fault==8)host.afterWriteCode=true;if(fault==9)host.afterWriteOwner=true;
        if(fault==10)host.changeMap=true;if(fault==11)host.forwardFault=true;
        if(fault==12)host.reenter=true;if(fault==13)host.travelTearsDown=true;
        if(fault==15){f.reader.changeAt=f.SandboxMaps;f.reader.changeSlot=f.Mode+selectiondisk::PendingOfflineMode;f.reader.newPointer=f.Sandbox;}
        if(fault==16)f.reader.Set(f.Mode+selectiondisk::PendingOfflineMode,f.Sandbox);
        unsigned reads=0;const auto result=route.Start(f.catalogueReader,host,host.context,fault!=14,&reads);
        const bool forwarded=fault==0||fault==12||fault==13||fault==16;
        const bool indeterminate=fault==7||fault==9||fault==10||fault==11;
        check(result==(forwarded?sandboxstart::Result::Forwarded:indeterminate?sandboxstart::Result::Indeterminate:sandboxstart::Result::Refused),"guarded native route outcome");
        check(reads<=sandboxstart::ReadLimit,"bounded Start-only work");
        check(host.forwards==(forwarded||fault==11?1u:0u),"original Start exactly once only after valid swap");
        if(fault==8){std::uintptr_t mode=0;check(host.writes==2&&f.reader.Read(f.Mode+selectiondisk::PendingOfflineMode,mode)&&mode==f.OfflineMode,"definite pre-dispatch refusal restores only current owned pending field");}
        if(fault==9||fault==10||fault==11||fault==13)check(host.writes==1,"never restore after owner/selection change or possible travel");
        if(fault==12)check(host.nestedConsumed,"nested Start consumed with zero reads/native writes");
        if(fault==16)check(host.writes==0&&host.forwards==1,"already-native Sandbox template forwarded without any mode/possession writes");
        const auto before=f.reader.reads,oldWrites=host.writes,oldForwards=host.forwards;unsigned duplicateReads=999;
        const auto duplicate=route.Start(f.catalogueReader,host,host.context,true,&duplicateReads);
        check((duplicate==sandboxstart::Result::Consumed||duplicate==sandboxstart::Result::Indeterminate)&&duplicateReads==0&&
            f.reader.reads==before&&host.writes==oldWrites&&host.forwards==oldForwards,"duplicate refuses retry/fallthrough without maintenance");
        if(!fault)std::printf("Sandbox Start route reads=%u writes=%u forwards=%u\n",reads,host.writes,host.forwards);
    }
    std::printf("CFG pending-mode/native Start route tests: %s (%u failures). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
