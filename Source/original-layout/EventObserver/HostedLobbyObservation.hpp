#pragma once
#include "HostedLobbyReport.hpp"
#include "HostedListenAuthorityObservation.hpp"
#include "CounselorPracticeObservation.hpp"
#include "../RuntimeProbe/HostedLobby35.generated.hpp"
namespace cfghostedlobby {
namespace disk=cfghosted::lobbydisk;
using Slot=cfgprobe::lifetime::SlotObservation;
struct Context {
    std::uintptr_t module=0,engine=0,viewport=0,world=0,mode=0,gameState=0,instance=0,player=0,controller=0,hud=0,stack=0,menu=0,type=0;
    std::uint32_t thread=0;cfgprobe::lifetime::ArrayHeader registry{};
    cfgmenu::active::StackArray records{};std::array<Slot,6> identities{};
    bool hostFlag=false,listenHost=false;
};
template<class R>bool Typed(R& r,const Context& c,std::uintptr_t object,std::uintptr_t cache,Slot& out){
    return cfgpracticeobs::Clear(r,c.registry,object,out)&&cfgpracticeobs::Typed(r,c.registry,c.module,object,cache);
}
template<std::size_t N>void Copy(std::array<char,N>& target,std::u16string_view source){
    target={};for(std::size_t i=0;i<source.size()&&i+1<N;++i)target[i]=source[i]>0&&source[i]<128?static_cast<char>(source[i]):'?';
}
template<class R>bool Sample(R& r,std::uintptr_t module,std::uint32_t thread,Context& out,Failure& failure,ControllerTrace* diagnostic=nullptr){
    out={};Context c;c.module=module;c.thread=thread;std::uint8_t flag=0;std::uint32_t nativeThread=0;
    ControllerTrace trace;struct TraceExit{ControllerTrace& trace;ControllerTrace* out;~TraceExit(){if(out)*out=trace;}}traceExit{trace,diagnostic};
    const auto ptr=[&](std::uintptr_t at,std::uintptr_t& value){return r.Read(at,value)&&cfgprobe::ObjectPointer(value);};
    failure=Failure::Thread;
    if(!module||!thread||!r.Read(module+cfgprobe::dispatch::ThreadFlagRva,flag)||flag!=1||
       !r.Read(module+cfgprobe::dispatch::ThreadIdRva,nativeThread)||nativeThread!=thread)return false;
    failure=Failure::Engine;
    if(!ptr(module+cfgprobe::EngineGlobalCandidate,c.engine)||!ptr(c.engine+cfgprobe::EngineGameViewport,c.viewport)||!ptr(c.viewport+cfgprobe::ViewportWorld,c.world)||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,c.registry))return false;
    failure=Failure::World;Slot unused;
    if(!ptr(c.world+cfgprobe::WorldGameMode,c.mode)||!Typed(r,c,c.mode,disk::ModeReflection,c.identities[0])||
       !ptr(c.world+cfgprobe::WorldGameInstance,c.instance))return false;
    cfghosted::listen35::LocalPlayers players{};
    failure=Failure::Controller;
    trace.stage=1;
    if(!r.Read(c.instance+cfgprobe::GameInstanceLocalPlayers,players))return false;
    trace.count=players.count;trace.capacity=players.capacity;
    if(players.count<1||players.count>16||players.capacity<players.count||players.capacity>16||!cfgprobe::ObjectPointer(players.data))return false;
    trace.stage=2;
    if(!ptr(players.data,c.player)||!ptr(c.player+cfghosted::disk35::PlayerController,c.controller))return false;
    trace.player=c.player;trace.controller=c.controller;trace.stage=3;
    cfgnames::Observation<R> names(r,module);cfgnames::Text label;
    if(r.Read(c.controller+cfgprobe::ObjectClass,trace.type)&&names.Object(trace.type,label))Copy(trace.className,label.View());
    (void)r.Read(module+disk::ControllerReflection,trace.base);
    // Named native APlayerController cache: ObjectIs walks the entire bounded
    // class chain. Lobby/ILL controllers need not derive from SCPlayerController.
    if(!Typed(r,c,c.controller,disk::ControllerReflection,c.identities[1]))return false;
    trace.stage=4;
    if(!r.Read(c.controller+cfghosted::disk35::ControllerPlayer,trace.backlink)||trace.backlink!=c.player)return false;
    trace.stage=5;
    failure=Failure::Hud;
    if(!ptr(c.controller+cfgmenu::execdisk::ControllerHUD,c.hud)||!Typed(r,c,c.hud,disk::HudReflection,c.identities[2])||
       !cfgobserver::Same(r,c.hud+0x368,c.controller))return false;
    failure=Failure::Stack;
    if(!ptr(c.hud+disk::HudStack,c.stack)||!cfgpracticeobs::Clear(r,c.registry,c.stack,c.identities[3])||
       !cfgobserver::Same(r,c.stack+0x20,c.hud)||!r.Read(c.stack+disk::StackArray,c.records)||!c.records.Sane()||
       !ptr(c.records.data+disk::RecordBytes*static_cast<std::uintptr_t>(c.records.count-1)+disk::RecordWidget,c.menu))return false;
    failure=Failure::Menu;
    // Public lobbies may use a different current page. A generic UserWidget
    // is diagnostic-only, still reached through the typed lobby HUD stack.
    if((!Typed(r,c,c.menu,disk::MenuReflection,c.identities[4])&&
        !Typed(r,c,c.menu,cfgpractice::disk::UserWidgetReflection,c.identities[4]))||!ptr(c.menu+cfgprobe::ObjectClass,c.type)||
       !cfgbundle::Code(r,module,disk::GetCurrentMenu,disk::GetCurrentMenuCode))return false;
    // Native ILLLobbyWidget::IsLobbyHost -> owning controller's ILLPlayerState
    // -> IsHost; the named bool setter proves453&1. Read-only hint, not a lease.
    std::uintptr_t state=0;std::uint8_t host=0,modeRole=0,controllerRole=0;Slot stateSlot,driverSlot;
    if(r.Read(c.controller+cfgescape::disk::PlayerState,state)&&Typed(r,c,state,disk::PlayerStateReflection,stateSlot)&&
       cfgobserver::Same(r,state+0x88,c.controller)&&r.Read(state+disk::HostByte,host))c.hostFlag=(host&1)!=0;
    std::uintptr_t driver=0,connection=1;
    // Donor host authority is World+GameMode+GameState. Keep the typed lobby
    // classes, authority roles and listen driver reciprocal links as proofs.
    if(ptr(c.world+cfgprobe::WorldGameState,c.gameState)&&Typed(r,c,c.gameState,disk::StateReflection,c.identities[5])&&
       cfghosted::native::WorldOwned(r,c.gameState,c.world)&&
       r.Read(c.world+cfgprobe::WorldNetDriver,driver)&&Typed(r,c,driver,cfghosted::disk35::NetDriverReflection,driverSlot)&&
       cfgobserver::Same(r,driver+cfghosted::disk35::DriverWorld,c.world)&&r.Read(driver+cfghosted::disk35::ServerConnection,connection)&&!connection&&
       r.Read(c.mode+cfghosted::disk35::ActorRole,modeRole)&&modeRole==3&&r.Read(c.controller+cfghosted::disk35::ActorRole,controllerRole)&&controllerRole==3)c.listenHost=true;
    // No native calls/writes follow this observation. A serial0 UI child is
    // accepted only through its twice-current native strong stack/tree path.
    (void)unused;failure=Failure::None;out=c;return true;
}
inline bool Same(const Context& a,const Context& b){return a.module==b.module&&a.engine==b.engine&&a.viewport==b.viewport&&a.world==b.world&&a.mode==b.mode&&a.gameState==b.gameState&&
    a.instance==b.instance&&a.player==b.player&&a.controller==b.controller&&a.hud==b.hud&&a.stack==b.stack&&a.menu==b.menu&&a.type==b.type&&
    a.thread==b.thread&&a.records==b.records&&a.identities==b.identities&&a.listenHost==b.listenHost;}
template<class R>bool Observe(R& r,std::uintptr_t module,std::uint32_t thread,Context& out,Failure& failure,ControllerTrace* diagnostic=nullptr){
    Context a,b;if(!Sample(r,module,thread,a,failure,diagnostic)||!Sample(r,module,thread,b,failure,diagnostic))return false;
    if(!Same(a,b)){failure=Failure::Changed;return false;}out=b;return true;
}
// Pure memory decoder of the already built FText's display-string leaf. It
// never runs FText::Rebuild or a delegate. Unknown/indirect getters refuse.
template<class R>bool ReadText(R& r,std::uintptr_t at,std::array<char16_t,128>& out,unsigned count){
    if(count>out.size())return false;for(unsigned i=0;i<count;++i)if(!r.Read(at+2ULL*i,out[i]))return false;return true;
}
template<class R>bool AuthoredText(R& r,const Context& c,std::uintptr_t object,std::array<char,72>& out){
    out={};Slot slot;std::uintptr_t data=0,table=0,getter=0,string=0;
    if(!Typed(r,c,object,disk::TextBlockReflection,slot)||!cfgbundle::Code(r,c.module,disk::TextToString,disk::TextToStringCode)||
       !r.Read(object+disk::AuthoredText,data)||!cfgprobe::ObjectPointer(data)||!r.Read(data,table)||
       !r.Read(table+0x10,getter)||getter<c.module||getter-c.module>=cfgprobe::ExecutableImageBytes)return false;
    std::array<std::uint8_t,5> code{};
    if(!r.Read(getter,code)||code[0]!=0x48||(code[1]!=0x8B&&code[1]!=0x8D)||code[2]!=0x41||code[4]!=0xC3||code[3]<8||code[3]>0x78)return false;
    if(code[1]==0x8B){if(!r.Read(data+code[3],string))return false;}else string=data+code[3];
    struct String {std::uintptr_t data=0;int count=0,capacity=0;bool operator==(const String& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}} first{},again{};
    std::array<char16_t,128> text{};
    if(!r.Read(string,first)||first.count<1||first.count>128||first.capacity<first.count||first.capacity>1024||!cfgprobe::ObjectPointer(first.data)||
       !ReadText(r,first.data,text,static_cast<unsigned>(first.count))||text[first.count-1]||
       !r.Read(string,again)||!(again==first)||!cfgobserver::Same(r,object+disk::AuthoredText,data)||!cfgobserver::Same(r,data,table))return false;
    Copy(out,std::u16string_view(text.data(),first.count-1));return true;
}
}
