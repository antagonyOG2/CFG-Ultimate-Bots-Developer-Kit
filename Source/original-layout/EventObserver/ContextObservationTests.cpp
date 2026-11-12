#include "OfflineContextObservation.hpp"
#include "PrimaryContextObservation.hpp"
#include <cstdio>
#include <cstring>
#include <map>
#include <array>
#include <utility>
namespace {
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    unsigned reads=0,cap=64;
    template<class T> void Put(std::uintptr_t address,const T& value) {
        static_assert(sizeof(T)<=16,"Fixture cell bound");
        auto& bytes=memory[address]; bytes={}; std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& output) {
        if (reads>=cap) return false;
        ++reads;
        const auto found=memory.find(address); if (found==memory.end()) return false;
        T value{}; std::memcpy(&value,found->second.data(),sizeof(value)); output=value; return true;
    }
};
template<class T,class=void>struct PointerSameCallable:std::false_type{};
template<class T>struct PointerSameCallable<T,std::void_t<decltype(cfgobserver::Same(
    std::declval<Reader&>(),std::declval<std::uintptr_t>(),std::declval<T>()))>>:std::true_type{};
enum class ByteFlag:unsigned char {Set=1};
static_assert(PointerSameCallable<std::uintptr_t>::value);
static_assert(PointerSameCallable<const std::uintptr_t&>::value);
static_assert(!PointerSameCallable<std::uint8_t>::value&&!PointerSameCallable<std::uint16_t>::value);
static_assert(!PointerSameCallable<std::uint32_t>::value&&!PointerSameCallable<int>::value);
static_assert(!PointerSameCallable<float>::value&&!PointerSameCallable<double>::value);
static_assert(!PointerSameCallable<bool>::value&&!PointerSameCallable<ByteFlag>::value);
static_assert(!PointerSameCallable<std::nullptr_t>::value);
int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
}
int main() {
    using namespace cfgobserver;
    using namespace cfgprobe;
    Plan p; p.bytes=sizeof(p); p.threadId=77; p.module=0x10000000;
    p.engine=0x20000000; p.viewport=0x20010000; p.world=0x20020000; p.mode=0x20030000;
    p.controller=0x20040000; p.pawn=0x20050000; p.player=0x20060000;
    p.gameInstance=0x20070000; p.localPlayerData=0x20080000;
    p.objectArray={0x30000000,16};
    p.dispatch={1,77,0x20090000,p.module+dispatch::NativeActorDispatcherRva};
    const std::array<std::uintptr_t,4> objects{p.world,p.mode,p.controller,p.pawn};
    for (std::size_t i=0;i<4;++i) p.objects[i]={objects[i],static_cast<std::int32_t>(i),static_cast<std::int32_t>(100+i)};
    Reader r;
    r.Put(p.module+EngineGlobalCandidate,p.engine); r.Put(p.engine+EngineGameViewport,p.viewport);
    r.Put(p.viewport+ViewportWorld,p.world); r.Put(p.world+WorldNetDriver,std::uintptr_t{0});
    r.Put(p.world+WorldGameMode,p.mode); r.Put(p.world+WorldGameInstance,p.gameInstance);
    r.Put(p.gameInstance+GameInstanceLocalPlayers,LocalPlayers{p.localPlayerData,1,1});
    r.Put(p.localPlayerData,p.player); r.Put(p.player+PlayerController,p.controller);
    r.Put(p.controller+ControllerPlayer,p.player); r.Put(p.controller+ControllerAcknowledgedPawn,p.pawn);
    r.Put(p.controller+ControllerPawn,p.pawn);
    r.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,p.objectArray.data);
    r.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,p.objectArray.count);
    for (const auto& slot:p.objects) {
        r.Put(slot.object+lifetime::ObjectIndexOffset,slot.index);
        r.Put(p.objectArray.data+static_cast<std::uintptr_t>(slot.index)*lifetime::ItemBytes,slot.object);
        r.Put(p.objectArray.data+static_cast<std::uintptr_t>(slot.index)*lifetime::ItemBytes+lifetime::ItemSerialOffset,slot.serial);
    }
    r.Put(p.module+dispatch::ThreadFlagRva,p.dispatch.flag); r.Put(p.module+dispatch::ThreadIdRva,p.dispatch.threadId);
    r.memory[p.module+dispatch::ThreadFlagRva][1]=0xE1;
    r.memory[p.module+dispatch::ThreadFlagRva][2]=77; // adjacent thread storage is not zero padding
    r.memory[p.module+dispatch::ThreadIdRva][4]=0xE2; // require exact uint32 thread read
    r.Put(p.pawn,p.dispatch.vtable); r.Put(p.dispatch.vtable+dispatch::EventSlotBytes,p.dispatch.target);
    CHECK(SanePlan(p)); CHECK(ContextUnchanged(r,p)); CHECK(r.reads==30);
    r.reads=0;CHECK(DiagnoseContextFailure(r,p)==ContextFailure::None&&r.reads==30);
    // Every known-field omission fails without adopting a new pointer or retry.
    for (const auto& cell:r.memory) {
        auto absent=r; absent.reads=0; absent.memory.erase(cell.first);
        CHECK(!ContextUnchanged(absent,p)); CHECK(absent.reads<=30);
        absent.reads=0;CHECK(DiagnoseContextFailure(absent,p)!=ContextFailure::None&&absent.reads<=30);
    }
    auto net=r; net.reads=0; net.Put(p.world+WorldNetDriver,std::uintptr_t{0x50000000});
    CHECK(!ContextUnchanged(net,p));net.reads=0;CHECK(DiagnoseContextFailure(net,p)==ContextFailure::WorldNetDriver);
    auto transferred=r; transferred.reads=0;
    transferred.Put(p.controller+ControllerAcknowledgedPawn,std::uintptr_t{0x50000000});
    CHECK(!ContextUnchanged(transferred,p));transferred.reads=0;CHECK(DiagnoseContextFailure(transferred,p)==ContextFailure::AcknowledgedPawn);
    auto reused=r; reused.reads=0;
    reused.Put(p.objectArray.data+3*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{200});
    CHECK(!ContextUnchanged(reused,p));reused.reads=0;CHECK(DiagnoseContextFailure(reused,p)==ContextFailure::ObjectRegistry);
    auto moved=r; moved.reads=0; moved.Put(p.objectArray.data+3*lifetime::ItemBytes,std::uintptr_t{0x50000000});
    CHECK(!ContextUnchanged(moved,p));moved.reads=0;CHECK(DiagnoseContextFailure(moved,p)==ContextFailure::ObjectRegistry);
    auto wrongThread=r; wrongThread.reads=0; wrongThread.Put(p.module+dispatch::ThreadIdRva,std::uint32_t{78});
    CHECK(!ContextUnchanged(wrongThread,p));wrongThread.reads=0;CHECK(DiagnoseContextFailure(wrongThread,p)==ContextFailure::Dispatcher);
    auto split=r; split.reads=0;
    split.Put(p.gameInstance+GameInstanceLocalPlayers,LocalPlayers{p.localPlayerData,2,2});
    CHECK(!ContextUnchanged(split,p));split.reads=0;CHECK(DiagnoseContextFailure(split,p)==ContextFailure::LocalPlayers);
    auto bounded=r; bounded.reads=0; bounded.cap=29;
    CHECK(!ContextUnchanged(bounded,p) && bounded.reads==29);
    // Exact passing production header-refresh path. Normal object allocation
    // after installation must not require a new human possession/handoff.
    auto grew=r; grew.reads=0;
    grew.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,std::int32_t{24});
    Plan refreshed{};
    CHECK(!ContextUnchanged(grew,p)); grew.reads=0;
    CHECK(cfgprimary::RefreshKnownObjectStorage(grew,p,refreshed) && grew.reads==32 &&
        refreshed.objectArray.count==24 && refreshed.pawn==p.pawn && refreshed.controller==p.controller &&
        refreshed.world==p.world && refreshed.objects==p.objects);
    auto relocated=grew; relocated.reads=0; const std::uintptr_t newStorage=0x40000000;
    relocated.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,newStorage);
    for (const auto& slot:p.objects) {
        relocated.Put(newStorage+static_cast<std::uintptr_t>(slot.index)*lifetime::ItemBytes,slot.object);
        relocated.Put(newStorage+static_cast<std::uintptr_t>(slot.index)*lifetime::ItemBytes+lifetime::ItemSerialOffset,slot.serial);
    }
    CHECK(cfgprimary::RefreshKnownObjectStorage(relocated,p,refreshed) && refreshed.objectArray.data==newStorage &&
        refreshed.objects==p.objects && relocated.reads==32);
    auto refreshedReuse=relocated; refreshedReuse.reads=0;
    refreshedReuse.Put(newStorage+3*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});
    CHECK(!cfgprimary::RefreshKnownObjectStorage(refreshedReuse,p,refreshed));
    auto refreshedTransfer=relocated; refreshedTransfer.reads=0;
    refreshedTransfer.Put(p.controller+ControllerPawn,std::uintptr_t{0x50000000});
    CHECK(!cfgprimary::RefreshKnownObjectStorage(refreshedTransfer,p,refreshed));
    auto shortRefresh=relocated; shortRefresh.reads=0; shortRefresh.cap=31;
    CHECK(!cfgprimary::RefreshKnownObjectStorage(shortRefresh,p,refreshed) && shortRefresh.reads==31);
    // Sustained AI world observation neither requires nor adopts the human's
    // possession after death/escape. Changed world/network/serial/thread refuse.
    auto dead=r;dead.reads=0;dead.Put(p.controller+ControllerPawn,std::uintptr_t{});
    dead.Put(p.controller+ControllerAcknowledgedPawn,std::uintptr_t{});
    dead.memory.erase(p.pawn);dead.memory.erase(p.pawn+lifetime::ObjectIndexOffset);
    dead.memory.erase(p.objectArray.data+3*lifetime::ItemBytes);
    CHECK(!ContextUnchanged(dead,p));dead.reads=0;
    CHECK(cfgprimary::RetainedOfflineWorld(dead,p,refreshed)&&refreshed.pawn==p.pawn&&refreshed.controller==p.controller);
    using Retention=cfgprimary::RetentionContext;
    dead.reads=0;CHECK(!cfgprimary::RefreshRetentionContext(dead,p,refreshed,Retention::Human));
    dead.reads=0;CHECK(!cfgprimary::RefreshRetentionContext(dead,p,refreshed,Retention::ScopedWorld)&&!dead.reads);
    bool leaseLive=true;
    const auto validate=[](Reader&,void* state){return *static_cast<bool*>(state);};
    {cfgprimary::ScopedLookupEligibility<Reader> scope(p.module,&leaseLive,validate,true);
     CHECK(scope.Entered());dead.reads=0;
     CHECK(cfgprimary::RefreshRetentionContext(dead,p,refreshed,Retention::ScopedWorld)&&
        refreshed.pawn==p.pawn&&refreshed.controller==p.controller);
     for(unsigned fault=0;fault<4;++fault){auto denied=dead;denied.reads=0;
      if(fault==0)denied.Put(p.viewport+ViewportWorld,std::uintptr_t{0x50000000});
      if(fault==1)denied.Put(p.world+WorldNetDriver,std::uintptr_t{0x50000000});
      if(fault==2)denied.Put(p.module+dispatch::ThreadIdRva,std::uint32_t{78});
      if(fault==3)denied.Put(p.objectArray.data+lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});
      CHECK(!cfgprimary::RefreshRetentionContext(denied,p,refreshed,Retention::ScopedWorld));
     }
     leaseLive=false;dead.reads=0;
     CHECK(!cfgprimary::RefreshRetentionContext(dead,p,refreshed,Retention::ScopedWorld)&&!dead.reads);
     leaseLive=true;auto foreign=p;foreign.module+=0x10000;dead.reads=0;
     CHECK(!cfgprimary::RefreshRetentionContext(dead,foreign,refreshed,Retention::ScopedWorld)&&!dead.reads);
    }
    dead.reads=0;CHECK(!cfgprimary::RefreshRetentionContext(dead,p,refreshed,Retention::ScopedWorld)&&!dead.reads);
    auto changedWorld=dead;changedWorld.reads=0;changedWorld.Put(p.viewport+ViewportWorld,std::uintptr_t{0x50000000});
    CHECK(!cfgprimary::RetainedOfflineWorld(changedWorld,p,refreshed));
    auto changedMode=dead;changedMode.reads=0;changedMode.Put(p.objectArray.data+lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});
    CHECK(!cfgprimary::RetainedOfflineWorld(changedMode,p,refreshed));
    auto online=dead;online.reads=0;online.Put(p.world+WorldNetDriver,std::uintptr_t{0x50000000});
    CHECK(!cfgprimary::RetainedOfflineWorld(online,p,refreshed));
    auto wrongOwner=dead;wrongOwner.reads=0;wrongOwner.Put(p.module+dispatch::ThreadIdRva,std::uint32_t{78});
    CHECK(!cfgprimary::RetainedOfflineWorld(wrongOwner,p,refreshed));
    for (unsigned n=0;n<9;++n) {
        auto invalid=p;
        switch (n) {
        case 0: invalid.magic=0; break; case 1: invalid.version=0; break;
        case 2: invalid.bytes=0; break; case 3: invalid.objects[3].serial=0; break;
        case 4: invalid.objects[3].object=invalid.controller; break;
        case 5: invalid.objects[3].index=invalid.objects[2].index; break;
        case 6: invalid.dispatch.target=invalid.module+1; break;
        case 7: invalid.module=0x00007FFFFFFFFFF8; break;
        default: invalid.threadId=78; break;
        }
        auto fresh=r; fresh.reads=0;
        CHECK(!ContextUnchanged(fresh,invalid) && fresh.reads==0);
    }
    std::printf("CFG fixed offline-context observation tests: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
