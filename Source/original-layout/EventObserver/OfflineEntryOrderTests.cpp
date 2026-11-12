#define CFGMENU_ENTRY_FIXTURE_ONLY
#include "OfflineEntryRuntimeTests.cpp"
#undef CFGMENU_ENTRY_FIXTURE_ONLY
struct OrderHost {
    std::array<std::uintptr_t,cfgmenu::entryorder::Limit> list{};unsigned count=0,calls=0,failCall=UINT32_MAX;bool current=true;
    bool Current(){return current;}
    bool Snapshot(std::array<std::uintptr_t,cfgmenu::entryorder::Limit>& out,unsigned& n){out=list;n=count;return true;}
    bool Remove(std::uintptr_t child){if(++calls==failCall)return false;if(!count||list[count-1]!=child)return false;list[--count]=0;return true;}
    bool Add(std::uintptr_t child){if(++calls==failCall)return false;list[count++]=child;return true;}
};
struct OrderFixture:EntryFixture {
    static constexpr std::uintptr_t Tree=0x3900000,TreeType=0x3910000,Pane=0x3920000,SlotData=0x3930000,
        TreeMember=0x3940000,RootMember=0x3950000,SlotMember=0x3960000,SlotsMember=0x3970000,
        OrderSlotType=0x3980000,OrderParent=0x3990000,OrderContent=0x3991000;
    std::array<std::uintptr_t,6> list{};unsigned count=6,added=0,calls=0,phaseCalls=0,factoryCalls=0;
    std::uintptr_t refusedFactory=0;
    OrderFixture(){using namespace cfgmenu;using M=MenuFixture;auto& r=fixture.r;
        fixture.Name(Sandbox,"SandboxButton");fixture.Name(Jason,"OfflineBotsButton");
        Root(ButtonType);
        r.Set(Tree+cfgprobe::ObjectClass,TreeType);Root(TreeType);r.Set(Pane+cfgprobe::ObjectClass,PanelType);
        ObjectMember(TreeMember,M::Type,"WidgetTree",EntryGraph,0x600);r.Set(M::Type+cfgreflection::disk::Children,TreeMember);r.Set(M::Widget+0x600,Tree);
        ObjectMember(RootMember,TreeType,"RootWidget",0,0x80);r.Set(TreeType+cfgreflection::disk::Children,RootMember);r.Set(Tree+0x80,Pane);
        ObjectMember(SlotMember,ButtonType,"Slot",0,0x28);r.Set(TitleMember+cfgreflection::disk::Next,SlotMember);
        fixture.Field(SlotsMember,PanelType,MenuFixture::ArrayProperty,"Slots",0);fixture.Property(SlotsMember,16,0xF8);
        r.Set(PanelType+cfgreflection::disk::Children,SlotsMember);
        ObjectMember(OrderParent,OrderSlotType,"Parent",OrderContent,0x28);ObjectMember(OrderContent,OrderSlotType,"Content",0,0x30);
        Root(OrderSlotType);r.Set(OrderSlotType+cfgreflection::disk::Children,OrderParent);
        const std::array<const char*,4> names={"ChallengeButton","VC1Button","VC2Button","LazarusButton"};
        list[0]=Jason;list[5]=Sandbox;
        for(unsigned i=0;i<4;++i){const auto widget=std::uintptr_t{0x39A0000}+0x1000*i,field=std::uintptr_t{0x39E0000}+0x1000*i;
            Register(widget,9800+static_cast<int>(i));fixture.Name(widget,names[i]);r.Set(widget+cfgprobe::ObjectClass,ButtonType);list[i+1]=widget;
            ObjectMember(field,M::Type,names[i],i==3?TreeMember:field+0x1000,0x608+static_cast<int>(8*i));r.Set(M::Widget+0x608+8*i,widget);}
        r.Set(M::Type+cfgreflection::disk::Children,std::uintptr_t{0x39E0000});
        Refresh();
    }
    void Refresh(){auto& r=fixture.r;using namespace cfgmenu;
        r.Set(Pane+0xF8,panelcapture::Array{SlotData,static_cast<int>(count),16});
        for(unsigned i=0;i<count;++i){const auto slot=std::uintptr_t{0x3A30000}+0x1000*i;
            r.Set(SlotData+8*i,slot);r.Set(slot+cfgprobe::ObjectClass,OrderSlotType);r.Set(slot+0x28,Pane);r.Set(slot+0x30,list[i]);r.Set(list[i]+0x28,slot);}
    }
    void AddShippingSizedMetadata(){auto& r=fixture.r;using namespace cfgmenu;
        for(unsigned ownerIndex=0;ownerIndex<2;++ownerIndex){const auto owner=ownerIndex?ButtonType:MenuFixture::Type;
            std::uintptr_t old=0;r.Read(owner+cfgreflection::disk::Children,old);
            const auto first=std::uintptr_t{0x5000000}+0x100000*ownerIndex;
            for(unsigned i=0;i<160;++i){const auto field=first+0x100*i;
                const std::string name="K2Node_Captured_Unrelated_Menu_Metadata_Long_Name_"+std::to_string(i);
                ObjectMember(field,owner,name.c_str(),i==159?old:field+0x100,0x8000+static_cast<int>(8*i));
                r.Set(MenuFixture::Widget+0x8000+8*i,std::uintptr_t{0});}
            r.Set(owner+cfgreflection::disk::Children,first);
        }
        r.Set(MenuFixture::Table+cfgnames::disk::ElementCount,std::array<int,2>{2048,1});
        // Designer's UObject name differs from its generated variable name.
        fixture.Name(Sandbox,"HostSandboxMatchButton");
    }
    unsigned Thread()const{return 42;}
    bool BeginPhase(){phaseCalls=0;return true;}
    void Cold(unsigned mask){using namespace cfgmenu::execdisk;auto& r=fixture.r;
        r.Set(MenuFixture::Base+RemoveChild.factory,RemoveChildFactoryCode);
        r.Set(MenuFixture::Base+AddChildToVerticalBox.factory,AddChildToVerticalBoxFactoryCode);
        if(mask&1)r.Set(MenuFixture::Base+RemoveChild.cache,std::uintptr_t{0});
        if(mask&2)r.Set(MenuFixture::Base+AddChildToVerticalBox.cache,std::uintptr_t{0});
    }
    bool Factory(std::uintptr_t address,std::uintptr_t& out){using namespace cfgmenu::execdisk;
        out=0;++factoryCalls;if(++phaseCalls>1||address==refusedFactory)return false;
        const auto* binding=address==MenuFixture::Base+RemoveChild.factory?&RemoveChild:
            address==MenuFixture::Base+AddChildToVerticalBox.factory?&AddChildToVerticalBox:nullptr;
        if(!binding)return false;out=binding==&RemoveChild?removeFunction:addFunction;
        fixture.r.Set(MenuFixture::Base+binding->cache,out);return true;
    }
    bool Invoke(std::uintptr_t target,std::uintptr_t nativeFunction,void* packet){
        if(target!=Pane||++phaseCalls>1)return false;++calls;const auto child=*static_cast<std::uintptr_t*>(packet);
        if(nativeFunction==removeFunction){if(!count||list[count-1]!=child)return false;list[--count]=0;fixture.r.Set(child+0x28,std::uintptr_t{0});
            static_cast<unsigned char*>(packet)[8]=1;Refresh();return true;}
        if(nativeFunction==addFunction){if(count==list.size())return false;list[count++]=child;++added;Refresh();
            static_cast<std::uintptr_t*>(packet)[1]=std::uintptr_t{0x3A30000}+0x1000*(count-1);return true;}
        return false;
    }
};
struct MutatingOrderFixture:OrderFixture {
    bool Invoke(std::uintptr_t target,std::uintptr_t nativeFunction,void* packet){
        const bool result=OrderFixture::Invoke(target,nativeFunction,packet);
        if(calls==1)fixture.r.Set(Tree+0x80,std::uintptr_t{0});
        return result;
    }
};
int main(){using namespace cfgmenu;using namespace entryorder;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* name){++checks;if(!okay){++failures;std::printf("FAIL %s\n",name);}};
    for(unsigned mask=1;mask<=3;++mask){
        for(unsigned fault=0;fault<3;++fault){auto fixture=std::make_unique<OrderFixture>();OptionsReader reader{fixture->fixture.r};
            active::Context context;entry::Binding binding;auto source=std::make_unique<entrysettings::Resources>();
            check(active::Observe(reader,MenuFixture::Base,context)&&entrysettings::Observe(reader,context,42,true,*source)&&
                entry::Observe(reader,*source,binding),"cold ordering source fixture");
            fixture->Cold(mask);const auto original=fixture->list;
            const auto& cold=(mask&1)?execdisk::RemoveChild:execdisk::AddChildToVerticalBox;
            if(fault==1)fixture->fixture.r.Set(MenuFixture::Base+cold.factory,std::uint8_t{0});
            if(fault==2)fixture->refusedFactory=MenuFixture::Base+cold.factory;
            auto orderStorage=std::make_unique<Native<OptionsReader,OrderFixture>>(reader,*fixture,binding);auto& order=*orderStorage;
            for(unsigned i=0;i<80&&!order.Finished();++i){order.Service();check(fixture->phaseCalls<=1,"cold warmup single native call per phase");}
            if(!fault)check(order.Finished()&&order.Status()==static_cast<unsigned>(State::Ready)&&!order.Failure()&&
                fixture->list[1]==OrderFixture::Sandbox&&fixture->calls==10&&fixture->factoryCalls==((mask==3)?2u:1u),
                "cold RemoveChild/AddChild caches warm and finish ordering");
            else check(order.Finished()&&order.Failure()==3&&order.Status()==static_cast<unsigned>(State::Preserved)&&
                fixture->calls==0&&fixture->list==original&&fixture->factoryCalls==(fault==1?0u:1u),
                "bad factory bytes or dispatch preserve the verified original menu without mutation");
        }
    }
    {auto fixture=std::make_unique<OrderFixture>();OptionsReader preparation{fixture->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        check(active::Observe(preparation,MenuFixture::Base,context)&&entrysettings::Observe(preparation,context,42,true,*source)&&entry::Observe(preparation,*source,binding),"shipping-sized metadata source");
        fixture->AddShippingSizedMetadata();StartupBudgetReader reader{{fixture->fixture.r},32768,0,false};
        auto orderStorage=std::make_unique<Native<StartupBudgetReader,OrderFixture>>(reader,*fixture,binding);auto& order=*orderStorage;unsigned maximum=0,phases=0;
        while(!order.Finished()&&phases++<128){reader.reads=0;reader.budgetHit=false;order.Service();maximum=(std::max)(maximum,reader.reads);
            check(!reader.budgetHit&&fixture->phaseCalls<=1,"shipping-sized metadata keeps bounded phase and single native call");}
        std::printf("Shipping-sized menu order maximum phase reads=%u/32768 phases=%u\n",maximum,phases);
        check(order.Status()==static_cast<unsigned>(State::Ready)&&!order.Failure()&&fixture->list[1]==OrderFixture::Sandbox&&maximum<32768,
            "large shipping class and differing designer name finish counselor-second order without budget fallback");
    }
    {auto fixture=std::make_unique<OrderFixture>();OptionsReader preparation{fixture->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        check(active::Observe(preparation,MenuFixture::Base,context)&&entrysettings::Observe(preparation,context,42,true,*source)&&entry::Observe(preparation,*source,binding),"bounded coalesced order source");
        fixture->AddShippingSizedMetadata();fixture->Cold(3);
        StartupBudgetReader reader{{fixture->fixture.r},32768,0,false};auto order=std::make_unique<Native<StartupBudgetReader,OrderFixture>>(reader,*fixture,binding);
        unsigned callbacks=0,peak=0,peakCalls=0;
        while(!order->Finished()&&callbacks++<128){reader.reads=0;reader.budgetHit=false;
            const auto before=fixture->calls+fixture->factoryCalls;order->ServiceSlice();
            const auto calls=fixture->calls+fixture->factoryCalls-before;
            peak=(std::max)(peak,reader.reads);peakCalls=(std::max)(peakCalls,calls);
            check(!reader.budgetHit&&reader.reads<=32768&&calls<=4,"coalesced order shares original read cap and four-call maximum");}
        check(order->Finished()&&!order->Failure()&&order->Status()==static_cast<unsigned>(State::Ready)&&fixture->list[1]==OrderFixture::Sandbox,
            "coalesced production order reaches same donor order");
        check(callbacks<39&&peakCalls>1,"coalescing reduces callbacks rather than inventing HUD events");
        const auto reads=reader.reads,calls=fixture->calls;order->ServiceSlice();
        check(reader.reads==reads&&fixture->calls==calls,"coalesced ready does zero idle work");
        std::printf("Shipping-sized coalesced menu callbacks=%u maxReads=%u/32768 maxNativeCalls=%u/4\n",callbacks,peak,peakCalls);
    }
    {auto fixture=std::make_unique<MutatingOrderFixture>();OptionsReader preparation{fixture->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        check(active::Observe(preparation,MenuFixture::Base,context)&&entrysettings::Observe(preparation,context,42,true,*source)&&entry::Observe(preparation,*source,binding),"mid-batch mutation source");
        StartupBudgetReader reader{{fixture->fixture.r},32768,0,false};
        auto order=std::make_unique<Native<StartupBudgetReader,MutatingOrderFixture>>(reader,*fixture,binding);
        for(unsigned i=0;i<32&&!order->Finished();++i){reader.reads=0;order->ServiceSlice();}
        check(order->Finished()&&order->Failure()&&fixture->calls==1,"lost native ownership between subphases stops before second native call");
    }
    const std::array<std::uintptr_t,Limit> original={0xA0000,0xA1000,0xA2000,0xA3000,0xA4000,0xA5000};
    auto desired=original;desired[1]=original[5];for(unsigned i=2;i<6;++i)desired[i]=original[i-1];
    for(unsigned failure=0;failure<=10;++failure){OrderHost host;host.list=original;host.count=6;host.failCall=failure?failure:UINT32_MAX;
        Transaction transaction;check(transaction.Begin(original,6,original[0],original[5]),"native menu order begin");
        for(unsigned i=0;i<64&&!transaction.Finished();++i){const auto before=host.calls;transaction.Step(host);check(host.calls-before<=1,"one native call per existing phase");}
        check(transaction.Status()==(failure?State::Failed:State::Ready)&&host.list==(failure?original:desired),"successful order or verified original-order rollback");}
    {OrderHost host;host.list=desired;host.count=6;Transaction transaction;
        check(transaction.Begin(desired,6,desired[0],desired[1])&&transaction.Status()==State::Ready&&!host.calls,"already ordered does no native work");}
    {OrderHost host;host.list=original;host.count=6;Transaction transaction;transaction.Begin(original,6,original[0],original[5]);host.current=false;
        check(!transaction.Step(host)&&transaction.Status()==State::Detached&&!host.calls,"changed owner never clears native menu");}
    {auto fixture=std::make_unique<OrderFixture>();OptionsReader reader{fixture->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        unsigned stage=0;const bool active=active::Observe(reader,MenuFixture::Base,context);
        const bool sourceOkay=active&&entrysettings::Observe(reader,context,42,true,*source,nullptr,&stage);
        std::printf("Order fixture active=%u source=%u stage=%u\n",active?1u:0u,sourceOkay?1u:0u,stage);
        const bool entryOkay=sourceOkay&&entry::Observe(reader,*source,binding,&stage);std::printf("Order fixture entry=%u stage=%u\n",entryOkay?1u:0u,stage);
        check(entryOkay,"production ordering current source fixture");
        auto orderStorage=std::make_unique<Native<OptionsReader,OrderFixture>>(reader,*fixture,binding);auto& order=*orderStorage;
        for(unsigned i=0;i<80&&!order.Finished();++i){order.Service();check(fixture->phaseCalls<=1,"production ordering one actual engine call per phase");}
        std::printf("Order native state=%u failure=%u calls=%u count=%u\n",order.Status(),order.Failure(),fixture->calls,fixture->count);
        check(order.Finished()&&order.Status()==static_cast<unsigned>(State::Ready)&&!order.Failure()&&
            fixture->list[0]==OrderFixture::Jason&&fixture->list[1]==OrderFixture::Sandbox&&fixture->calls==10,
            "production native root/member/serial/code/packet path puts counselor second preserving others");
        const auto reads=fixture->fixture.r.reads,calls=fixture->calls;for(unsigned i=0;i<1000;++i)order.Service();
        check(fixture->fixture.r.reads==reads&&fixture->calls==calls,"finished order performs zero recurring reads or calls");
    }
    for(unsigned fault=0;fault<3;++fault){auto fixture=std::make_unique<OrderFixture>();OptionsReader reader{fixture->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        check(active::Observe(reader,MenuFixture::Base,context)&&entrysettings::Observe(reader,context,42,true,*source)&&entry::Observe(reader,*source,binding),"ordering refusal source fixture");
        auto orderStorage=std::make_unique<Native<OptionsReader,OrderFixture>>(reader,*fixture,binding);auto& order=*orderStorage;order.Service();
        if(fault==0)fixture->fixture.r.Set(OrderFixture::Tree+0x80,std::uintptr_t{0});
        if(fault==1)fixture->fixture.r.Set(MenuFixture::Base+execdisk::RemoveChild.executor,std::uint8_t{0});
        if(fault==2)fixture->fixture.r.Set(MenuFixture::Widget+0x608,OrderFixture::Sandbox);
        for(unsigned i=0;i<80&&!order.Finished();++i)order.Service();check(order.Finished()&&order.Failure()!=0&&fixture->calls==0&&fixture->count==6,
            "lost tree/changed native code/changed strong child reference refuses BEFORE menu removal");
        check((order.Status()==static_cast<unsigned>(State::Preserved))==(fault==1),
            "only native warmup failure with unchanged original menu permits preserved fallback");
    }
    std::printf("CFG native Offline Play ordering: %s (%u checks, %u failures; owned memory only).\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
