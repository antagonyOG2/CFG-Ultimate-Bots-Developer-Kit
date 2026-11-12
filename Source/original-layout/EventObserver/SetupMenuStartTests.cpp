#define main ExistingRowTests
#include "NativeSetupRowsTests.cpp"
#undef main
#include "SetupMenuStartRuntime.hpp"
#include "NativeSettingsMenuOptions.hpp"
struct StartHost : Host {
    cfgmenu::MenuCatalogueEvidence catalogue{};unsigned travels=0,selected=0;
    bool nativeOkay=true,travelOkay=true,weatherOkay=true;
    bool Index(std::uintptr_t,unsigned& out){out=selected;return !invalidIndex;}
    bool Weather(unsigned index,std::uint64_t& identity){identity=1000+index;return weatherOkay;}
    bool NativeChoices(cfgmenu::Session&,std::uint64_t){return nativeOkay;}
    bool Catalogue(cfgmenu::MenuCatalogueEvidence& out){out=catalogue;return true;}
    bool TravelSandbox(const cfgmenu::MenuCatalogueEvidence&){++travels;return travelOkay;}
};
#ifndef CFGMENU_START_FIXTURE_ONLY
int main(){unsigned failures=0;const auto check=[&](bool value,const char* name){if(!value){++failures;std::printf("FAIL %s\n",name);}};
    for(unsigned selection=0;selection<6;++selection){cfgmenu::Session session;cfgmenu::Capabilities caps;caps.automaticJason=true;
        cfgmenu::settingsoptions::EnableExpandedLoadout(caps);session.Open(100,cfgmenu::Mode::Counselor,caps,9);
        const auto generation=session.Generation();session.Pick(100,generation,1,2,3);
        cfgmenu::GameplayHandoff handoff(session);cfgmenu::rows::Runtime rows;cfgmenu::SetupStart start(session,handoff,rows);
        struct ExpandedHost:StartHost {std::uint64_t generation=0;bool Current(std::uint64_t g){return current&&g==generation;}} host;
        host.generation=generation;host.selected=selection;host.catalogue={100,generation,44,0x500000,1,2,3,8,{},true,true};host.catalogue.selectedJason[16]=9;
        const auto spec=cfgmenu::settingsoptions::ExpandedStartingInventory();
        check(rows.Begin(generation,&spec,1)&&start.Claim(100,generation),"expanded inventory native row");
        for(unsigned i=0;i<4;++i)rows.Step(host,i*25);
        check(start.Start(host)==cfgmenu::StartDisposition::TravelSubmitted,"expanded inventory starts native travel");
        cfgmenu::MenuCatalogueEvidence evidence;cfgmenu::Selection chosen;std::uint64_t id=0;
        check(handoff.InspectArmed(evidence,chosen,id)&&chosen.inventory==(selection==1?5u:selection),"expanded inventory resolves Random exactly once");
    }
    for(unsigned selection=0;selection<4;++selection)for(unsigned fault=0;fault<4;++fault){
        cfgmenu::Session session;cfgmenu::Capabilities caps;caps.automaticJason=true;
        if(fault!=3)cfgmenu::settingsoptions::EnableRestrictedInventory(caps);
        session.Open(100,cfgmenu::Mode::Counselor,caps,9);const auto generation=session.Generation();session.Pick(100,generation,1,2,3);
        cfgmenu::GameplayHandoff handoff(session);cfgmenu::rows::Runtime rows;cfgmenu::SetupStart start(session,handoff,rows);
        struct InventoryHost:StartHost {std::uint64_t generation=0;bool Current(std::uint64_t g){return current&&g==generation;}} host;
        host.generation=generation;host.selected=selection;host.catalogue={100,generation,44,0x500000,1,2,3,8,{},true,true};host.catalogue.selectedJason[16]=9;
        auto spec=cfgmenu::settingsoptions::StartingInventory();if(fault==1)spec.labels[1]=u"Random";
        check(rows.Begin(generation,&spec,1)&&start.Claim(100,generation),"restricted inventory row claim");
        for(unsigned i=0;i<4;++i)rows.Step(host,i*25);
        if(fault==2)host.catalogue.cfgMetadata=false;
        check(start.Start(host)==(fault?cfgmenu::StartDisposition::Refused:cfgmenu::StartDisposition::TravelSubmitted),"restricted schema and capability gate at Start");
        if(!fault){cfgmenu::MenuCatalogueEvidence evidence;cfgmenu::Selection chosen;std::uint64_t id=0;
            check(handoff.InspectArmed(evidence,chosen,id)&&chosen.inventoryEnabled&&chosen.inventory==std::array<unsigned,4>{{0,2,3,5}}[selection],"native row maps names to released base indexes");
            check(start.Start(host)==cfgmenu::StartDisposition::ConsumedDuplicate&&host.travels==1,"restricted inventory captured once");
        }else{auto original=session;cfgmenu::Selection chosen;std::uint64_t id=0;
            check(!host.travels&&original.Commit(100,generation)&&original.InspectCommitted(100,generation,chosen,id)&&chosen.inventory==0,"inventory refusal leaves draft atomic");}
    }
    for(unsigned fault=0;fault<2;++fault){
        cfgmenu::Session session;cfgmenu::Capabilities caps;caps.automaticJason=true;caps.verified[1]=true;
        check(session.Open(100,cfgmenu::Mode::Counselor,caps,9),"status/count session");
        const auto generation=session.Generation();check(session.PickNativeCustomization(100,generation,1,2),"native customization preserved");
        cfgmenu::GameplayHandoff handoff(session);cfgmenu::rows::Runtime rows;cfgmenu::SetupStart start(session,handoff,rows);
        struct StatusHost : StartHost {std::uint64_t generation=0;unsigned bad=0,weatherCalls=0;
            bool Current(std::uint64_t g){return current&&g==generation;}
            bool Index(std::uintptr_t widget,unsigned& value){value=widget==0x100000?4:bad;return true;}
            bool Weather(unsigned,std::uint64_t&){++weatherCalls;return false;}
        } host;host.generation=generation;host.bad=fault;
        host.catalogue={100,generation,44,0x500000,1,2,0,8,{},true,true,true};host.catalogue.selectedJason[16]=9;
        cfgmenu::rows::Spec specs[2];specs[0].id=cfgmenu::RowId::Count;specs[0].title=u"Counselors";specs[0].count=7;
        for(unsigned i=0;i<7;++i)specs[0].labels[i]=u"Counselor count";
        specs[1].id=cfgmenu::RowId::Weather;specs[1].title=u"Offline Bots - Play as a Counselor";
        specs[1].count=1;specs[1].labels[0]=u"AI Jason - Automatic";specs[1].informationOnly=true;specs[1].preserveNativeValue=true;
        check(rows.Begin(generation,specs,2)&&start.Claim(100,generation),"native status layout claim");
        for(unsigned i=0;i<10;++i)rows.Step(host,i*25);
        check(start.Start(host)==(fault?cfgmenu::StartDisposition::Refused:cfgmenu::StartDisposition::TravelSubmitted),"status choice validated without fake weather setting");
        check(!host.weatherCalls&&host.travels==(fault?0u:1u),"status row adds no weather API/duplicate travel");
        if(!fault){cfgmenu::MenuCatalogueEvidence evidence;cfgmenu::Selection selection;std::uint64_t id=0;
            check(handoff.InspectArmed(evidence,selection,id)&&selection.counselorCount==5&&selection.preserveNativeCounselor,"count and normal human handoff unchanged");}
    }
    {cfgmenu::Session s;cfgmenu::Capabilities caps;caps.automaticJason=true;
        check(s.Open(100,cfgmenu::Mode::Counselor,caps,9),"selecting native menu before cold choices");
        cfgmenu::GameplayHandoff handoff(s);cfgmenu::rows::Runtime rows;cfgmenu::SetupStart start(s,handoff,rows);
        struct ColdHost : StartHost {std::uint64_t gen=0;bool Current(std::uint64_t g){return current&&g==gen;}} h;
        h.gen=s.Generation();
        check(!start.Claim(100,h.gen)&&start.ClaimPreparing(100,h.gen),"explicit guard-only selecting claim, not ordinary ready claim");
        check(start.Start(h)==cfgmenu::StartDisposition::ConsumedWaiting&&start.Start(h)==cfgmenu::StartDisposition::ConsumedWaiting&&
            !h.travels&&handoff.State()==cfgmenu::HandoffState::Empty&&s.Current()==cfgmenu::Stage::Selecting,
            "early Start during paced cold preparation stays waiting without travel/default choice/handoff");
        check(s.PickNativeCustomization(100,h.gen,1,2)&&start.Start(h)==cfgmenu::StartDisposition::ConsumedWaiting,
            "native choices can subsequently advance while unbuilt rows still block Start");
        h.current=false;check(start.Start(h)==cfgmenu::StartDisposition::Refused&&!h.travels,"lost owner refuses early Start");
    }
    for(unsigned fault=0;fault<8;++fault){
        cfgmenu::Session session;cfgmenu::Capabilities capabilities;capabilities.verified.fill(true);capabilities.automaticJason=true;
        check(session.Open(100,cfgmenu::Mode::Counselor,capabilities,9),"open counselor setup");
        const auto generation=session.Generation();check(session.Pick(100,generation,1,2,3),"native catalogue selections");
        cfgmenu::GameplayHandoff handoff(session);cfgmenu::rows::Runtime rows;cfgmenu::SetupStart start(session,handoff,rows);
        // Owned row fixture normally requires generation19; start-specific host
        // supplies the real Session generation through its overridden Current.
        struct ScopedHost : StartHost {std::uint64_t generation=0;bool Current(std::uint64_t g){return current&&g==generation;}} scoped;
        scoped.generation=generation;
        scoped.catalogue={100,generation,44,0x500000,1,2,3,8,{},true,true};scoped.catalogue.selectedJason[16]=9;
        cfgmenu::rows::Spec spec;spec.id=cfgmenu::RowId::Duration;spec.title=u"Match Length";spec.labels[0]=u"30 minutes";
        spec.labels[1]=u"60 minutes";spec.labels[2]=u"90 minutes";spec.count=3;spec.selected=1;scoped.selected=2;
        check(start.Start(scoped)==cfgmenu::StartDisposition::NativeRoute,"unclaimed stock route untouched");
        check(rows.Begin(generation,&spec,1)&&start.Claim(100,generation),"claim native setup");
        check(start.Start(scoped)==cfgmenu::StartDisposition::ConsumedWaiting&&!scoped.travels,"early Start cannot fall through to human Jason");
        for(unsigned i=0;i<6;++i)rows.Step(scoped,i*25);
        if(fault==1)scoped.nativeOkay=false;
        if(fault==2)scoped.invalidIndex=true;
        if(fault==3)scoped.catalogue.cfgMetadata=false;
        if(fault==4)scoped.catalogue.generation++;
        if(fault==5)scoped.travelOkay=false;
        if(fault==6)scoped.selected=3;
        if(fault==7)scoped.current=false;
        const auto disposition=start.Start(scoped);
        check(disposition==(fault?cfgmenu::StartDisposition::Refused:cfgmenu::StartDisposition::TravelSubmitted),"all values and catalogue checked before travel");
        check(scoped.travels==(fault==0||fault==5?1u:0u),"one travel only");
        if(fault!=0&&fault!=5){
            check(session.Current()==cfgmenu::Stage::Settings,"refusal leaves the editable draft intact");
            auto original=session;cfgmenu::Selection selection;std::uint64_t submission=0;
            check(original.Commit(100,generation)&&original.InspectCommitted(100,generation,selection,submission)&&selection.duration==1,
                "no partial row changes on catalogue/selection refusal");
        }
        if(!fault){
            check(start.Start(scoped)==cfgmenu::StartDisposition::ConsumedDuplicate&&scoped.travels==1,"duplicate wrapper never forwards native Jason travel");
            check(handoff.State()==cfgmenu::HandoffState::Armed,"menu handoff armed exactly once");
        }
        if(fault==5)check(handoff.State()==cfgmenu::HandoffState::Empty&&scoped.travels==1,"ambiguous travel cancels spawn association");
    }
    std::printf("CFG native-row Start/configured-AI handoff/duplicate-regression tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
#endif
