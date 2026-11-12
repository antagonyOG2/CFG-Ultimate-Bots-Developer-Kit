#define CFG_H27_LOBBY_INPUT_FIXTURE_ONLY
#include "H27LobbyInputTests.cpp"
#undef CFG_H27_LOBBY_INPUT_FIXTURE_ONLY

int main(){
    using F=NativeFixture;using M=MenuFixture;using Scope=cfgmenu::rows::HostedLobbyRetentionScope;
    NativeFixture f;memory=&f;lobbyCurrent=callbackComplete=true;windowHeld=false;
    const auto thread=GetCurrentThreadId();
    f.fixture.Name(F::Panel,"CustomServerSettings");f.fixture.Name(F::PanelType,"VerticalBox");
    f.fixture.r.Set(M::Base+cfgpractice::disk::PanelWidgetReflection,F::RemoveOwner);
    f.fixture.r.Set(M::Base+cfgpractice::disk::PanelWidgetConstructor,F::RemoveOwner);
    f.fixture.r.Set(M::Base+cfgpractice::disk::PanelSlotReflection,F::SlotType);
    f.fixture.r.Set(M::Base+cfgpractice::disk::PanelSlotConstructor,F::SlotType);
    f.fixture.r.Set(M::Base+cfgprobe::dispatch::ThreadIdRva,thread);
    f.fixture.r.Set(F::Panel+cfgprobe::ObjectClass,F::PanelType);
    f.fixture.r.Set(F::Panel+cfgprobe::lifetime::ObjectIndexOffset,-1);
    f.fixture.r.Set(F::Panel+cfgpractice::disk::Slots,cfgpracticeobs::WidgetNode::List{F::PanelSlots,0,64});
    f.fixture.r.Set(F::PanelType+cfgprobe::ClassSuper,F::RemoveOwner);
    f.fixture.r.Set(F::RemoveOwner+cfgprobe::ClassSuper,std::uintptr_t{});
    const auto ensure=[&](std::uintptr_t value,int serial){int index=-1;std::uintptr_t existing=0;
        f.fixture.r.Read(value+cfgprobe::lifetime::ObjectIndexOffset,index);
        if(!(index>=0&&f.fixture.r.Read(F::Registry+24ULL*static_cast<unsigned>(index),existing)&&existing==value))f.Register(value,serial);
        f.fixture.r.Read(value+cfgprobe::lifetime::ObjectIndexOffset,index);
        f.fixture.r.Set(value+8,std::uint32_t{});f.fixture.r.Set(F::Registry+24ULL*static_cast<unsigned>(index)+8,std::uint32_t{});};
    for(auto [object,serial]:std::array<std::pair<std::uintptr_t,int>,13>{{
        {F::World,100},{F::Mode,101},{F::Controller,102},{F::HUD,103},{F::Stack,104},{M::Widget,105},
        {M::Donor,106},{F::Panel,0},{F::PanelType,305},{F::RemoveOwner,306},{M::DonorType,307},{F::TextType,308},{F::SlotType,309}}})ensure(object,serial);
    ensure(f.addFunction,0);ensure(f.textFunction,0);ensure(f.removeFunction,0);
    ensure(F::Panel,0);ensure(F::PanelType,305);ensure(F::RemoveOwner,306);ensure(M::DonorType,307);ensure(F::TextType,308);ensure(F::SlotType,309);
    cfghostedlobby::Discovery discovery;auto& c=discovery.context;c.module=M::Base;c.thread=thread;c.world=F::World;c.mode=F::Mode;
    c.gameState=F::World;c.controller=F::Controller;c.hud=F::HUD;c.stack=F::Stack;c.menu=M::Widget;
    cfghostedlobby::RowLayout layout{F::World,M::Widget,M::DonorType,F::Panel,M::Donor,1,221};
    HostedLobbyRowReceiptStore receipts{};
    const auto addRow=[&](unsigned i,std::uintptr_t& row,std::uintptr_t& slot,std::uintptr_t& title,std::uintptr_t& value){
        const auto base=std::uintptr_t{0x1800000}+0x10000ULL*i;row=base;slot=base+0x1000;title=base+0x2000;value=base+0x3000;
        f.fixture.r.Set(row+cfgprobe::ObjectClass,M::DonorType);f.fixture.r.Set(row+0x188,title);f.fixture.r.Set(row+0x190,value);
        f.fixture.r.Set(title+cfgprobe::ObjectClass,F::TextType);f.fixture.r.Set(value+cfgprobe::ObjectClass,F::TextType);
        f.fixture.r.Set(slot+cfgprobe::ObjectClass,F::SlotType);
        ensure(row,500+static_cast<int>(i));ensure(title,0);ensure(value,0);
        cfgmenu::reflection::AddParameters packet{};packet.content=row;packet.result=slot;
        ownedPacket=&packet;frameBytes=sizeof(packet);
        HostedLobbyRootProvider provider(discovery,layout,F::Mode,0x810000,0x820000,&receipts);Scope scope(M::Base,provider.Hooks(),provider.Admitted());
        if(!scope.Entered()||!Scope::Inputs(M::Base,F::Panel,f.addFunction,&packet)||!Scope::EnterNative(M::Base))return false;
        f.fixture.r.Set(slot+0x180,F::Panel);f.fixture.r.Set(slot+0x188,row);f.fixture.r.Set(slot+cfgpractice::disk::Content,row);
        cfgmenu::panelcapture::Array list{};if(!f.fixture.r.Read(F::Panel+cfgpractice::disk::Slots,list))return false;
        f.fixture.r.Set(list.data+8ULL*static_cast<unsigned>(list.count),slot);++list.count;f.fixture.r.Set(F::Panel+cfgpractice::disk::Slots,list);
        ensure(slot,0);
        if(!Scope::Outputs(M::Base,f.addFunction,&packet)){
            std::printf("ADD row=%u entered=%d provider=%u/%u value=%p index=%d serial=%d flags=%08X/%08X scopeFailure=%d slot=%p result=%p\n",i,scope.Entered(),provider.Stage(),provider.Failure().reason,reinterpret_cast<void*>(provider.Failure().value),provider.Failure().slot.index,provider.Failure().slot.serial,provider.Failure().internal,provider.Failure().flags,Scope::InputFailure(),reinterpret_cast<void*>(slot),reinterpret_cast<void*>(packet.result));return false;}
        return receipts.rows[i].row==row&&receipts.rows[i].slot==slot;
    };
    const auto textField=[&](std::uintptr_t target){cfgmenu::reflection::OpaqueText packet{};ownedPacket=&packet;frameBytes=sizeof(packet);
        HostedLobbyRootProvider provider(discovery,layout,F::Mode,0x810000,0x820000,&receipts);Scope scope(M::Base,provider.Hooks(),provider.Admitted());
        const bool entered=scope.Entered();const bool input=entered&&Scope::Inputs(M::Base,target,f.textFunction,&packet);
        const bool native=input&&Scope::EnterNative(M::Base);const bool output=native&&Scope::Outputs(M::Base,f.textFunction,&packet);
        return output;
    };
    struct RemovePacket{std::uintptr_t content=0;std::uint8_t result=0;std::array<std::uint8_t,7> padding{};};
    const auto removeRow=[&](unsigned i,std::uintptr_t row,std::uintptr_t slot){RemovePacket packet{};packet.content=row;ownedPacket=&packet;frameBytes=sizeof(packet);
        HostedLobbyRootProvider provider(discovery,layout,F::Mode,0x810000,0x820000,&receipts);Scope scope(M::Base,provider.Hooks(),provider.Admitted());
        const bool entered=scope.Entered();const bool input=entered&&Scope::Inputs(M::Base,F::Panel,f.removeFunction,&packet);
        const bool native=input&&Scope::EnterNative(M::Base);
        if(!native){std::printf("REMOVE row=%u entered=%d input=%d provider=%u/%u role=%u value=%p serial=%d scope=%d\n",i,entered,input,provider.Stage(),provider.Failure().reason,provider.Role(),reinterpret_cast<void*>(provider.Failure().value),provider.Failure().slot.serial,Scope::InputFailure());return false;}
        cfgmenu::panelcapture::Array list{};if(!f.fixture.r.Read(F::Panel+cfgpractice::disk::Slots,list))return false;unsigned out=0;
        for(int n=0;n<list.count;++n){std::uintptr_t item=0;if(!f.fixture.r.Read(list.data+8ULL*static_cast<unsigned>(n),item))return false;if(item!=slot)f.fixture.r.Set(list.data+8ULL*out++,item);}
        list.count=static_cast<int>(out);f.fixture.r.Set(F::Panel+cfgpractice::disk::Slots,list);f.fixture.r.Set(slot+0x180,std::uintptr_t{});
        packet.result=1;const bool output=Scope::Outputs(M::Base,f.removeFunction,&packet)&&!receipts.rows[i].row&&!receipts.rows[i].slot;
        if(!output)std::printf("REMOVE out row=%u provider=%u/%u scope=%d receipt=%p/%p\n",i,provider.Stage(),provider.Failure().reason,Scope::InputFailure(),reinterpret_cast<void*>(receipts.rows[i].row),reinterpret_cast<void*>(receipts.rows[i].slot));return output;
    };
    unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL H221 %s\n",label);}};
    std::array<std::uintptr_t,3> rows{},slots{},titles{},values{};
    for(unsigned i=0;i<3;++i)check(addRow(i,rows[i],slots[i],titles[i],values[i]),"fresh provider adds and records exact attached row receipt");
    for(unsigned i=0;i<3;++i){check(textField(titles[i]),"fresh provider admits exact attached TitleText");check(textField(values[i]),"fresh provider admits exact attached ValueText");}
    const auto foreign=std::uintptr_t{0x1B00000};f.fixture.r.Set(foreign+cfgprobe::ObjectClass,F::TextType);ensure(foreign,600);
    check(!textField(foreign),"foreign TextBlock is rejected");
    f.fixture.r.Set(rows[0]+0x188,foreign);check(!textField(titles[0]),"changed TitleText member is rejected");f.fixture.r.Set(rows[0]+0x188,titles[0]);
    cfgmenu::panelcapture::Array list{};f.fixture.r.Read(F::Panel+cfgpractice::disk::Slots,list);
    f.fixture.r.Set(list.data,slots[0]);list.count=1;f.fixture.r.Set(F::Panel+cfgpractice::disk::Slots,list);
    check(!textField(titles[1]),"detached row receipt is rejected");
    // Restore all three forward edges, then remove each row through a fresh provider.
    list.count=3;for(unsigned i=0;i<3;++i)f.fixture.r.Set(list.data+8ULL*i,slots[i]);f.fixture.r.Set(F::Panel+cfgpractice::disk::Slots,list);
    for(unsigned i=0;i<3;++i)check(removeRow(i,rows[i],slots[i]),"pre-detach close clears only the removed row receipt");
    check(!windowHeld&&hostedLobbyRootBank.Healthy(),"all split provider collector windows close cleanly");
    memory=nullptr;ownedPacket=nullptr;
    std::printf("H221 split hosted row receipts: %u checks, %u failures; actual provider, fresh callback providers, no game/native calls.\n",checks,failures);
    return failures?1:0;
}
