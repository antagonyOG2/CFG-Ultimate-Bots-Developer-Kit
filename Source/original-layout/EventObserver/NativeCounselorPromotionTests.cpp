#define main ExistingActiveOwnerTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "NativeCounselorPromotion.hpp"
#include <functional>
using cfgmenu::promotion::Packet;
struct PromotionFixture:Fixture {
    static constexpr std::uintptr_t State=0x400000,Selected=0x410000,Property=0x420000,Vtable=0x430000;
    static constexpr std::uintptr_t ModeType=0x440000,ControllerType=0x450000,StateType=0x460000,CounselorType=0x470000,KillerType=0x480000;
    cfgmenu::promotion::Context c{};
    PromotionFixture(){
        using namespace cfgprobe;using namespace cfgmenu::birthdisk;
        c.module=Module;c.engine=Engine;c.viewport=Viewport;c.world=World;c.gameInstance=Instance;c.localPlayers=Players;c.player=Player;
        c.mode=Mode;c.controller=Controller;c.playerState=State;c.selectedClass=Selected;c.softProperty=Property;
        c.modeVtable=Vtable;c.restartGate=Module+0x10000;c.getterGate=Module+0x11000;c.thread=42;
        c.exactImageVerified=true;c.pickedResolutionVerified=true;c.registry={Registry,4};
        std::array<std::uintptr_t,4> actors{{World,Mode,Controller,State}};
        for(unsigned i=0;i<actors.size();++i){c.identities[i]={actors[i],static_cast<int>(i),static_cast<int>(100+i)};
            reader.Set(actors[i]+lifetime::ObjectIndexOffset,static_cast<int>(i));reader.Set(Registry+24*i,actors[i]);reader.Set(Registry+24*i+16,static_cast<int>(100+i));}
        reader.Set(Controller+ControllerPlayer,Player);reader.Set(Controller+ControllerPawn,std::uintptr_t{0});reader.Set(Controller+ControllerAcknowledgedPawn,std::uintptr_t{0});
        reader.Set(Controller+0x388,State);reader.Set(State+0x88,Controller);reader.Set(State+SpawnedCharacterClass,std::uintptr_t{0});
        reader.Set(State+0x394,std::uint32_t{0});reader.Set(State+0x110,static_cast<unsigned char>(3));
        reader.Set(Mode,Vtable);reader.Set(Vtable+RestartSlot,c.restartGate);reader.Set(Vtable+GetterSlot,c.getterGate);
        const std::array<std::uintptr_t,5> types{{ModeType,ControllerType,StateType,CounselorType,KillerType}};
        const std::array<std::uintptr_t,5> caches{{SCGameMode_SandboxClass,SCPlayerController_SandboxClass,SCPlayerStateClass,SCCounselorCharacterClass,SCKillerCharacterClass}};
        for(unsigned i=0;i<types.size();++i){reader.Set(Module+caches[i],types[i]);reader.Set(Module+caches[i]+8,types[i]);reader.Set(types[i]+ClassSuper,std::uintptr_t{0});reader.Set(types[i]+0xF8,std::uintptr_t{0});}
        reader.Set(Mode+ObjectClass,ModeType);reader.Set(Controller+ObjectClass,ControllerType);reader.Set(State+ObjectClass,StateType);reader.Set(Selected+ClassSuper,CounselorType);
        reader.Set(Property,Module+cfgmenu::softdisk::PropertyVtable);reader.Set(Property+0x30,int{1});reader.Set(Property+0x34,int{40});
        reader.Set(Property+0x20,StateType);reader.Set(Property+0x44,int{0x748});
        const std::uint64_t path=0x1234;std::memcpy(c.picked.bytes.data()+16,&path,8);
        reader.Set(State+PickedCounselorClass,c.picked);reader.Set(State+ActiveCharacterClass,Packet{});
    }
};
struct FakeCalls {
    PromotionFixture& f;unsigned copies=0,destroys=0,consumes=0,restarts=0,gets=0;
    unsigned fault=0;bool faulted=false,owned=false,born=false;DWORD thread=42;
    std::function<void()> inCopy,inConsume,inGet,inRestart;
    explicit FakeCalls(PromotionFixture& fixture):f(fixture){}
    DWORD Thread()const{return thread;}
    bool Faulted()const{return faulted;}
    bool Ready(const cfgmenu::promotion::Context&){return fault!=1;}
    bool Copy(const cfgmenu::promotion::Context&,Packet& p){++copies;if(fault==2){faulted=true;return false;}
        p=f.c.picked;owned=true;if(inCopy)inCopy();return true;}
    bool Destroy(const cfgmenu::promotion::Context&,Packet& p){++destroys;
        if(fault==3)return false;if(fault==4){faulted=true;return false;}
        if(!owned){faulted=true;return false;}owned=false;p={};return true;}
    bool Consume(const cfgmenu::promotion::Context&,Packet& p){++consumes;
        if(fault==5){faulted=true;return false;}if(!owned){faulted=true;return false;}
        f.reader.Set(f.State+cfgmenu::birthdisk::ActiveCharacterClass,p);owned=false;p={};if(inConsume)inConsume();return true;}
    bool Get(const cfgmenu::promotion::Context&,std::uintptr_t& type){++gets;if(inGet)inGet();
        if(fault==6){faulted=true;return false;}type=fault==7?f.KillerType:f.Selected;return true;}
    bool Restart(const cfgmenu::promotion::Context&){++restarts;if(inRestart)inRestart();
        if(fault==8){faulted=true;return false;}return true;}
};
static unsigned abiCopies=0,abiConsumes=0,abiDestroys=0;
static void* __fastcall TestCopy(void* dst,const void* src){
    ++abiCopies;std::memcpy(dst,src,40);auto* p=static_cast<Packet*>(dst);
    const wchar_t text[]=L"owned";void* s=HeapAlloc(GetProcessHeap(),0,sizeof(text));
    if(!s)RaiseException(0xE0123401,0,0,nullptr);std::memcpy(s,text,sizeof(text));std::memcpy(p->bytes.data()+24,&s,8);return dst;
}
static void __fastcall TestConsume(void* state,void* value){
    ++abiConsumes;if(!state)RaiseException(0xE0123402,0,0,nullptr);void* str=nullptr;
    std::memcpy(&str,static_cast<Packet*>(value)->bytes.data()+24,8);HeapFree(GetProcessHeap(),0,str);
}
static void __fastcall TestDestroy(void*,void* value){++abiDestroys;void* str=nullptr;std::memcpy(&str,static_cast<Packet*>(value)->bytes.data()+24,8);HeapFree(GetProcessHeap(),0,str);}
static std::uintptr_t __fastcall TestGetter(void* a,void* b){return reinterpret_cast<std::uintptr_t>(a)^reinterpret_cast<std::uintptr_t>(b);}
static void __fastcall TestFault(void*,void*){RaiseException(0xE0123403,0,0,nullptr);}
int main(){
    using namespace cfgmenu::promotion;unsigned failures=0,checks=0;
    const auto check=[&](bool okay,const char* name){++checks;if(!okay){++failures;std::printf("FAIL %s\n",name);}};
    for(unsigned bad=0;bad<26;++bad){PromotionFixture f;FakeCalls native(f);NativeCounselorPromotion<OwnedByteReader,FakeCalls> adapter(f.reader,native);
        if(bad==1)f.c.exactImageVerified=false;
        if(bad==2)f.c.pickedResolutionVerified=false;
        if(bad==3)native.thread=41;
        if(bad==4)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(bad==5)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(bad==6)f.reader.Set(f.State+cfgmenu::birthdisk::SpawnedCharacterClass,f.KillerType);
        if(bad==7)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.Controller);
        if(bad==8)f.reader.Set(f.State+0x88,f.Player);
        if(bad==9)f.reader.Set(f.Mode+cfgprobe::ObjectClass,f.ControllerType);
        if(bad==10)f.reader.Set(f.Selected+cfgprobe::ClassSuper,f.KillerType);
        if(bad==11)f.reader.Set(f.Registry+3*24+16,int{0});
        if(bad==12)f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,Packet{});
        if(bad==13)f.reader.Set(f.Property+0x34,int{8});
        if(bad==14)f.reader.Set(f.State+0x110,static_cast<unsigned char>(2));
        if(bad==15)f.reader.Set(f.State+0x394,std::uint32_t{4});
        if(bad==16)f.reader.Set(f.Vtable+cfgmenu::birthdisk::GetterSlot,f.Module+cfgmenu::birthdisk::Getter);
        if(bad==17)f.reader.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
        if(bad==18)f.reader.Set(f.Module+cfgmenu::birthdisk::SCCounselorCharacterConstructor,f.KillerType);
        if(bad==19)native.fault=1;
        if(bad==20)f.reader.Set(f.Property+0x20,f.ControllerType);
        if(bad==21)f.reader.Set(f.Property+0x44,int{0x848});
        if(bad==22)f.reader.Set(f.Selected+cfgprobe::ClassSuper,f.Selected);
        if(bad==23)f.reader.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{5});
        if(bad==24)f.reader.Set(f.ModeType+0xF8,f.Mode);
        if(bad==25)f.reader.Set(f.StateType+0xF8,f.State);
        native.inRestart=[&](){const auto value=adapter.PawnClass(f.Mode,f.Controller);native.born=value.type==f.Selected;};
        const auto result=adapter.Restart(f.c);
        check(result==(bad?Result::Refused:Result::Forwarded),"context/class/field/gate/authority refusal");
        check(bad?(!native.copies&&!native.consumes&&!native.restarts):native.born,"no native mutation on invalid context");
        check(native.destroys==0,"no destructor after consuming setter");
        Packet saved;f.reader.Read(f.State+cfgmenu::birthdisk::PickedCounselorClass,saved);
        check(bad==12||saved.bytes==f.c.picked.bytes,"picked field never written");
        check(f.reader.reads<3000,"bounded owner-chain reads");
        check(adapter.Restart(f.c)==Result::Refused&&native.restarts<=1,"one-shot; no restart retry");
    }
    for(unsigned scenario=0;scenario<14;++scenario){PromotionFixture f;FakeCalls native(f);NativeCounselorPromotion<OwnedByteReader,FakeCalls> a(f.reader,native);
        native.inRestart=[&](){auto g=a.PawnClass(f.Mode,f.Controller);native.born=g.type==f.Selected;};
        if(scenario==0||scenario==1||scenario==2)native.inCopy=[&](){f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);};
        if(scenario==1)native.fault=3;
        if(scenario==2)native.fault=4;
        if(scenario==3)native.fault=2;
        if(scenario==4)native.fault=5;
        if(scenario==5)native.fault=6;
        if(scenario==6)native.fault=7;
        if(scenario==7)native.fault=8;
        if(scenario==8)native.inConsume=[&](){check(a.State()==Phase::Preparing,"guard armed before broadcast");auto g=a.PawnClass(f.Mode,f.Controller);check(g.handled&&!g.type,"broadcast reentry cannot birth");};
        if(scenario==9)native.inGet=[&](){auto g=a.PawnClass(f.Mode,f.Controller);check(g.handled&&!g.type,"recursive getter fails closed");};
        if(scenario==10)native.inGet=[&](){f.reader.Set(f.Registry+3*24+16,int{999});};
        if(scenario==11)native.inRestart=[](){};
        if(scenario==12)native.inConsume=[&](){check(a.Restart(f.c)==Result::Refused,"setter-triggered Restart reentry refuses");};
        if(scenario==13)native.inRestart=[&](){
            const auto unrelated=a.PawnClass(f.Mode,f.Player);check(!unrelated.handled,"unrelated getter remains host-forwarded");
            for(unsigned i=0;i<5;++i){const auto g=a.PawnClass(f.Mode,f.Controller);check(g.handled&&(i<4?g.type==f.Selected:!g.type),"finite getter budget");}};
        const auto result=a.Restart(f.c);
        const std::array<Result,14> expected{{Result::Refused,Result::CleanupFailed,Result::Indeterminate,Result::Indeterminate,
            Result::Indeterminate,Result::Indeterminate,Result::SpawnDenied,Result::Indeterminate,Result::PromotedNoBirth,Result::SpawnDenied,
            Result::SpawnDenied,Result::SpawnDenied,Result::PromotedNoBirth,Result::SpawnDenied}};
        check(result==expected[scenario],"explicit refusal/cleanup/fault outcomes");
        check(native.destroys==(scenario<3?1u:0u),"cleanup only before ownership transfer");
        check(native.consumes<=1&&native.restarts<=1,"no repeated consume/restart on fault");
        if(scenario<5||scenario==8||scenario==12)check(!native.restarts,"no forwarding after preparation failure");
        if(scenario==6)check(!native.born,"killer getter result never forwarded");
        if(scenario==9)check(!native.born,"outer getter also refuses after recursive entry");
        if(scenario==10)check(!native.born,"replacement during native getter refuses");
    }
    // Actual Windows x64 calls against our own compiled functions and heap only.
    Packet source,owned;check(detail::Copy(reinterpret_cast<std::uintptr_t>(&TestCopy),&owned,&source),"actual native copy ABI");
    check(detail::Two(reinterpret_cast<std::uintptr_t>(&TestConsume),1,&owned),"actual consuming ABI");owned={};
    check(abiCopies==1&&abiConsumes==1&&!abiDestroys,"consumed input not destroyed twice");
    check(detail::Copy(reinterpret_cast<std::uintptr_t>(&TestCopy),&owned,&source),"second fresh owned copy");
    check(detail::Two(reinterpret_cast<std::uintptr_t>(&TestDestroy),1,&owned),"pre-transfer abort destructor ABI");owned={};
    std::uintptr_t out=0;check(detail::Get(reinterpret_cast<std::uintptr_t>(&TestGetter),0x10000,0x20000,out)&&out==0x30000,"getter register/return ABI");
    check(!detail::Two(reinterpret_cast<std::uintptr_t>(&TestFault),1,&owned),"SEH native failure contained");
    PromotionFixture f;WindowsCalls real(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)),GetCurrentThreadId(),false);
    check(!real.Ready(f.c),"real Windows adapter refuses non-CFG test process");
    std::array<unsigned char,16> nativePrefix{};CurrentProcessReader current;
    const auto nativeAddress=reinterpret_cast<std::uintptr_t>(&TestGetter),image=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    check(current.Read(nativeAddress,nativePrefix)&&detail::Code(image,nativeAddress,nativePrefix),"actual current-process executable-image code validator");
    nativePrefix[0]^=1;check(!detail::Code(image,nativeAddress,nativePrefix),"exact code mismatch refuses");
    check(!detail::Code(image,reinterpret_cast<std::uintptr_t>(&owned),owned.bytes),"non-image non-executable packet cannot be a native target");
    std::printf("CFG counselor promotion: %s (%u checks, %u failures). Owned fixtures/heap only; no CFG access or hooks.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
