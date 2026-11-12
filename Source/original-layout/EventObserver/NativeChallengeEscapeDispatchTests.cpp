#include "NativeChallengeEscapeDispatch.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <type_traits>
namespace d=cfgchallengeescape::dispatch;
constexpr std::uintptr_t Module=0x100000000, Object=0x200000, Function=0x300000, LeaseAddress=0x400000;
struct Source {
    std::map<std::uintptr_t,unsigned char> memory;
    unsigned reads=0,cap=d::ReadLimit;
    std::function<void(Source&,std::uintptr_t)> onRead;
    template<class T> void Set(std::uintptr_t at,const T& value) {
        const auto* bytes=reinterpret_cast<const unsigned char*>(&value);
        for(std::size_t i=0;i<sizeof(value);++i) memory[at+i]=bytes[i];
    }
    unsigned Remaining() const { return reads<cap ? cap-reads : 0; }
    bool ReadBytes(std::uintptr_t at,void* output,std::size_t bytes) {
        if(!Remaining()) return false; ++reads;
        if(onRead) onRead(*this,at);
        auto* out=static_cast<unsigned char*>(output);
        for(std::size_t i=0;i<bytes;++i) {
            const auto it=memory.find(at+i); if(it==memory.end()) return false; out[i]=it->second;
        }
        return true;
    }
    Source() {
        Set(Module+cfgprobe::dispatch::ThreadIdRva,std::uint32_t{42});
        Set(Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{1});
        Set(Module+cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix);
        Set(LeaseAddress,std::uint32_t{7});
    }
};
using Reader=d::CallbackReader<Source>;
struct Platform {
    std::uint32_t thread=42;
    std::uintptr_t module=::Module, factoryResult=Function;
    unsigned invokes=0,factories=0;
    bool nativeOkay=true;
    std::function<void()> duringNative;
    std::uint32_t Thread() const noexcept { return thread; }
    std::uintptr_t Module() const noexcept { return module; }
    bool Invoke(std::uintptr_t address,std::uintptr_t object,std::uintptr_t function,void* packet) {
        ++invokes; if(duringNative) duringNative();
        return nativeOkay && address==module+cfgspawn::GenericDispatcherRva && object==Object && function==Function && packet;
    }
    bool Factory(std::uintptr_t address,std::uintptr_t& output) {
        ++factories; if(duringNative) duringNative(); output=factoryResult;
        return nativeOkay && address>=module && address-module<d::ImageBytes;
    }
};
struct Lease {
    bool* permitted=nullptr;
    std::function<void()>* onLease=nullptr;
    bool operator()(Reader& reader) {
        if(onLease && *onLease) (*onLease)();
        std::uint32_t generation=0;
        return permitted && *permitted && reader.Read(LeaseAddress,generation) && generation==7;
    }
};
using Engine=d::Engine<Reader,Platform,Lease>;
struct Fixture {
    Source source;
    Reader reader{source};
    Platform platform;
    bool permitted=true;
    std::function<void()> onLease;
    Lease lease{&permitted,&onLease};
    Engine engine{reader,platform,Module,42,true,lease};
    std::array<unsigned char,32> packet{};
    bool Invoke() { return engine.Invoke(Object,Function,packet.data()); }
};
static_assert(!std::is_copy_constructible_v<Reader> && !std::is_move_constructible_v<Reader>,"callback reader cannot duplicate its counter");
static_assert(!std::is_copy_constructible_v<Engine> && !std::is_move_constructible_v<Engine>,"callback engine cannot duplicate its counter");
#if defined(_WIN32)
using Native=d::NativeEngine<Reader,Lease>;
static_assert(!std::is_copy_constructible_v<Native> && !std::is_move_constructible_v<Native>,"native engine is non-copyable");
// Instantiate all actual adapter methods at compile time. Tests never create a
// native engine, install hooks, launch processes or dispatch native game calls.
template class d::NativeEngine<Reader,Lease>;
#endif
int main() {
    unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* label) { ++checks; if(!okay) { ++failures; std::printf("FAIL %s\n",label); } };
    cfgdispatch::enabled=false; cfgdispatch::owned={};
    { Fixture f; check(f.Invoke() && f.engine.Calls()==1 && f.platform.invokes==1,"exact callback guard admits owned fake invocation");
      check(f.reader.Reads()==7 && f.source.reads==7 && f.reader.Remaining()==d::ReadLimit-7,"scope and both dispatch proofs share exact callback reader accounting"); }
    for(unsigned fault=0;fault<9;++fault) {
        Fixture f;
        switch(fault) {
          case 0:f.platform.thread=41;break;
          case 1:f.platform.module+=0x1000;break;
          case 2:f.source.Set(Module+cfgprobe::dispatch::ThreadIdRva,std::uint32_t{41});break;
          case 3:f.source.Set(Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{0});break;
          case 4:f.source.Set(Module+cfgspawn::GenericDispatcherRva,std::array<unsigned char,19>{});break;
          case 5:f.permitted=false;break;
          case 6:f.source.Set(LeaseAddress,std::uint32_t{8});break;
          case 7:f.source.memory.erase(Module+cfgprobe::dispatch::ThreadIdRva);break;
          case 8:f.source.memory.erase(LeaseAddress);break;
        }
        check(!f.Invoke() && !f.engine.Calls() && !f.platform.invokes,"wrong thread/module/code/scope and unreadable guard refuse without a native call");
    }
    for(unsigned fault=0;fault<4;++fault) {
        Source source; Reader reader(source); Platform platform; bool permitted=true; Lease lease{&permitted,nullptr};
        const auto module=fault==1 ? std::uintptr_t{0} : fault==2 ? UINTPTR_MAX : Module;
        const auto thread=fault==3 ? 0U : 42U;
        Engine engine(reader,platform,module,thread,fault!=0,lease); std::array<unsigned char,32> packet{};
        check(!engine.Invoke(Object,Function,packet.data()) && !engine.Preflight(10,100) && !engine.Calls(),"immutable exact pin positive module and callback owner required");
    }
    { Fixture f; check(!f.engine.Invoke(0,Function,f.packet.data()) && !f.engine.Invoke(Object,0,f.packet.data()) &&
        !f.engine.Invoke(Object,Function,nullptr) && !f.reader.Reads(),"invalid object/function/packet refuse before observations"); }
    { Fixture f; unsigned success=0; for(unsigned i=0;i<d::CallLimit;++i) if(f.Invoke()) ++success;
      check(success==512 && f.engine.Calls()==512 && !f.Invoke() && f.platform.invokes==512,"dedicated callback admits exactly512 calls without changing menu dispatcher cap");
      check(!f.engine.Preflight(1,1) && f.engine.RemainingCalls()==0,"spent call budget cannot reset through preflight"); }
    { Fixture f; unsigned success=0; std::uint32_t word=0;
      for(unsigned i=0;i<d::ReadLimit;++i) if(f.reader.Read(LeaseAddress,word)) ++success;
      check(success==524288 && f.reader.Reads()==524288 && f.source.reads==524288 && !f.reader.Read(LeaseAddress,word),"reader enforces fixed aggregate524288 reads");
      check(!f.Invoke() && !f.engine.Preflight(1,1) && !f.platform.invokes && f.reader.Remaining()==0,"exhausted shared reader leaves no guard or reservation authority"); }
    { Fixture f; f.source.cap=20; std::uint32_t word=0;
      for(unsigned i=0;i<5;++i) f.reader.Read(LeaseAddress,word);
      check(f.reader.Reads()==5 && f.reader.Remaining()==15,"callback reader composes actual underlying remaining budget");
      check(!f.engine.Preflight(5,9) && f.reader.Reads()==12 && f.reader.Remaining()==8 && !f.engine.Reserved(),"preflight accounts guard reads BEFORE checking requested read reserve");
      check(f.engine.Preflight(5,1) && f.reader.Remaining()==1 && f.engine.Reserved(),"stricter underlying reader cap cannot be enlarged by reservation");
      check(!f.Invoke() && !f.engine.Calls(),"reservation does not bypass subsequent exhausted guard proof"); }
    { Fixture f; f.source.cap=100; check(f.engine.Preflight(10,90),"read reserve accepted only after fresh guard leaves enough");
      const auto spent=f.reader.Reads(); check(!f.engine.Preflight(11,1) && f.reader.Reads()==spent,"latched transaction cannot replace or enlarge reservation");
      for(unsigned i=0;i<10;++i) check(f.Invoke(),"reserved fake invocation consumes original allowance");
      check(f.engine.Calls()==10 && !f.Invoke() && f.engine.Reserved() && !f.engine.Preflight(1,1),"reservation remains latched after exact calls spent"); }
    for(unsigned invalid=0;invalid<4;++invalid) {
        Fixture f; const unsigned calls=invalid==0 ? 0 : invalid==1 ? 513 : 1;
        const unsigned reads=invalid==2 ? 0 : invalid==3 ? d::ReadLimit+1 : 1;
        check(!f.engine.Preflight(calls,reads) && !f.engine.Reserved() && !f.engine.Calls(),"zero and oversized requests never arm a reservation");
    }
    { Fixture f; for(unsigned i=0;i<510;++i) f.Invoke();
      check(!f.engine.Preflight(3,1) && f.engine.Preflight(2,1),"preflight uses actual unspent native calls");
      check(f.Invoke() && f.Invoke() && !f.Invoke() && f.engine.Calls()==512,"reservation reaches fixed cap without overflow"); }
    { Fixture f; Engine borrowed(f.reader,f.platform,Module,42,true,f.lease);
      check(f.Invoke() && borrowed.Calls()==1 && borrowed.Preflight(2,10) && !f.engine.Preflight(10,1),"engines borrowing one callback share native counts and single reservation");
      check(f.Invoke() && borrowed.Invoke(Object,Function,f.packet.data()) && !f.Invoke() && borrowed.Calls()==3,"borrowed engine cannot multiply reserved call capacity"); }
    { Fixture f; Engine foreign(f.reader,f.platform,Module+0x1000,42,true,f.lease);
      check(!foreign.Invoke(Object,Function,f.packet.data()) && !foreign.Preflight(1,1) && !foreign.Calls(),"shared reader cannot be rebound to another image pin"); }
    { Fixture f; f.platform.duringNative=[&] {
          check(f.engine.Busy() && !f.Invoke() && !f.engine.Preflight(1,1),"native reentry cannot invoke or reserve on current engine");
          std::uintptr_t result=1; check(!f.engine.Factory(Module+0x1000,result) && result==0,"native reentry cannot run a factory");
          Engine borrowed(f.reader,f.platform,Module,42,true,f.lease);
          check(!borrowed.Preflight(1,1) && !borrowed.Invoke(Object,Function,f.packet.data()) && borrowed.Calls()==1,"new nested engine shares native in-progress flag and cannot reserve");
          Reader replacement(f.source); Engine reset(replacement,f.platform,Module,42,true,f.lease);
          check(!reset.Preflight(1,1) && !reset.Invoke(Object,Function,f.packet.data()) && !reset.Calls(),
              "fresh reader inside native callback cannot create independent dispatch allowance");
          std::uint32_t word=0; check(f.reader.Read(LeaseAddress,word) && word==7,"native reentry may perform shared-budget read-only observation");
      };
      check(f.Invoke() && f.engine.Calls()==1 && !f.engine.Busy(),"native reentry does not corrupt outer invocation state"); }
    { Fixture f; f.onLease=[&] {
          check(f.engine.Busy() && !f.engine.Preflight(1,1) && !f.Invoke(),"scope guard lease cannot reserve or dispatch recursively");
          Engine borrowed(f.reader,f.platform,Module,42,true,f.lease);
          check(!borrowed.Preflight(1,1),"scope lease cannot create reservation using a borrowed engine");
          Reader replacement(f.source); Engine reset(replacement,f.platform,Module,42,true,f.lease);
          check(!reset.Preflight(1,1),"fresh reader inside scope guard cannot reserve a reset allowance");
      };
      check(f.Invoke() && f.engine.Calls()==1,"read-only scope lease completes ordinary guard"); }
    for(unsigned stale=0;stale<4;++stale) {
        Fixture f; f.onLease=[&] {
            if(stale==0) f.source.Set(Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{0});
            if(stale==1) f.source.Set(Module+cfgprobe::dispatch::ThreadIdRva,std::uint32_t{43});
            if(stale==2) f.source.Set(Module+cfgspawn::GenericDispatcherRva,std::array<unsigned char,19>{});
            if(stale==3) f.platform.module+=0x1000;
        };
        check(!f.Invoke() && !f.engine.Calls(),"dispatcher thread/code/image changed during lease is rechecked before invocation");
    }
    { Fixture f; f.platform.nativeOkay=false;
      check(!f.Invoke() && f.engine.Faulted() && f.engine.Calls()==1,"native fault poisons shared callback after counting attempted call");
      Engine borrowed(f.reader,f.platform,Module,42,true,f.lease);
      check(!f.Invoke() && !borrowed.Preflight(1,1) && borrowed.Faulted() && f.platform.invokes==1,"faulted engine cannot regain authority through new wrapper"); }
    { Fixture f; std::uintptr_t result=0;
      check(f.engine.Factory(Module+0x1000,result) && result==Function && f.engine.Calls()==1 && f.platform.factories==1,"guarded fake reflection factory shares callback call budget");
      check(!f.engine.Factory(Module-1,result) && !result && !f.engine.Factory(Module+d::ImageBytes,result) && f.engine.Calls()==1,"factory must remain inside pinned image range");
      check(f.engine.Preflight(3,10) && !f.engine.Factory(Module+0x1000,result) && f.platform.factories==1,"factory cannot enter a latched recovery transaction"); }
    { Fixture f; std::uintptr_t result=0; f.platform.factoryResult=0;
      check(!f.engine.Factory(Module+0x1000,result) && !result && f.engine.Calls()==1 && !f.engine.Faulted(),"null factory result is refused without inventing native fault"); }
    { Fixture f; std::uintptr_t result=0; f.platform.nativeOkay=false;
      check(!f.engine.Factory(Module+0x1000,result) && !result && f.engine.Faulted() && f.engine.Calls()==1,"factory SEH failure poisons callback"); }
    { Fixture f; std::uintptr_t result=0; f.platform.duringNative=[&] {
          check(!f.engine.Preflight(1,1) && !f.Invoke(),"factory reentry rejects recovery reservation and invocation");
      };
      check(f.engine.Factory(Module+0x1000,result) && f.engine.Calls()==1,"outer factory remains one bounded native call"); }
    { Fixture f; cfgdispatch::Hook hook;
      hook.module=Module; hook.trampoline=0x500000; hook.relay=0x500014; hook.detour=0x600000;
      hook.entry=cfgspawn::GenericDispatcherPrefix; hook.entry[0]=0xE9;
      hook.continuation={1,2,3}; hook.jump=cfgdispatch::AbsoluteJump(hook.detour);
      f.source.Set(Module+cfgspawn::GenericDispatcherRva,hook.entry);
      f.source.Set(hook.trampoline,hook.continuation); f.source.Set(hook.relay,hook.jump);
      cfgdispatch::owned=hook; cfgdispatch::enabled=true;
      check(f.Invoke() && f.reader.Reads()==11,"existing owned generic hook proof uses same reader without installing another hook");
      f.source.Set(hook.relay,std::array<unsigned char,14>{});
      check(!f.Invoke() && f.engine.Calls()==1,"modified owned relay refuses next callback dispatch");
      cfgdispatch::enabled=false; cfgdispatch::owned={}; }
    { Fixture f; std::uint32_t word=123;
      check(!f.reader.Read(0,word) && word==123 && !f.reader.Reads(),"invalid extent never reads or modifies output");
      check(!f.reader.Read(0x700000,word) && word==123 && f.reader.Reads()==1,"unreadable source spends bounded read but leaves typed output unchanged"); }
    {d::WindowsSource source;d::CallbackReader<d::WindowsSource> reader(source);
     std::uint64_t value=0x123456789ABCDEF0ULL,copy=0;
     check(reader.Read(reinterpret_cast<std::uintptr_t>(&value),copy)&&copy==value,"guarded local source reads owned data without a kernel roundtrip");
     void* denied=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_NOACCESS);
     check(denied!=nullptr,"owned inaccessible page allocated for refusal fixture");
     if(denied){copy=42;check(!reader.Read(reinterpret_cast<std::uintptr_t>(denied),copy)&&copy==42,"SEH refusal preserves typed output on inaccessible local memory");
       VirtualFree(denied,0,MEM_RELEASE);}
     LARGE_INTEGER started{},finished{},frequency{};QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&started);copy=0;
     for(unsigned i=0;i<500000;++i)if(!reader.Read(reinterpret_cast<std::uintptr_t>(&value),copy)){check(false,"finite owned local read benchmark");break;}
     check(copy==value&&reader.Reads()<d::ReadLimit,"guarded local benchmark honors unchanged fixed read cap");
     QueryPerformanceCounter(&finished);
     std::printf("Owned guarded local field-read benchmark500000: %.3f ms; not a game frame benchmark.\n",1000.0*static_cast<double>(finished.QuadPart-started.QuadPart)/static_cast<double>(frequency.QuadPart));
    }
    std::printf("NativeChallengeEscapeDispatch: %u checks, %u failures; callback limits calls=%u reads=%u. Owned fixtures only.\n",checks,failures,d::CallLimit,d::ReadLimit);
    return failures ? 1 : 0;
}
