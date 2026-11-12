#include "UrinationActionObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
using namespace cfgurination;
struct Reader:OwnedByteReader {unsigned cap=1536;
    template<class T>bool Read(std::uintptr_t at,T& out){return reads<cap&&OwnedByteReader::Read(at,out);}
};
struct Host {bool current=true;unsigned calls=0,failAt=0;
    bool Current(const LoadIdentity& expected){++calls;return current&&calls!=failAt&&expected.world==0x700000&&
        expected.controller==0x800000&&expected.worldSerial==5&&expected.controllerSerial==6&&
        expected.thread==9&&expected.uuid==0x18E030&&expected.generation==1;}
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,Action=0x200000,Path=0x300000,Latent=0x400000,
        Handle=0x500000,Control=0x600000;
    Reader r;Host host;LoadRequest request;
    LoadIdentity identity{0x700000,0x800000,5,6,9,0x18E030,1};
    ActionOwner owner{Module,9,{17,6}};
    Fixture(){
        constexpr std::uintptr_t table=0x900000,chunk=0x910000,entry=0x920000;
        r.Set(Module+cfgnames::disk::Global,table);r.Set(table,chunk);
        r.Set(table+cfgnames::disk::ElementCount,std::array<std::int32_t,2>{2,1});
        r.Set(chunk+8,entry);r.Set(entry,std::uint32_t{});
        std::array<char,128> name{};for(unsigned i=0;i<ExpectedAsset.size();++i)name[i]=static_cast<char>(ExpectedAsset[i]);
        r.Set(entry+cfgnames::disk::EntryText,name);
        const NativePath path{{1,0},{}};
        r.Set(Path,path);r.Set(Action+disk::ActionPath,path);
        r.Set(Latent,NativeLatent{-1,identity.uuid,{},identity.controller});
        r.Set(Action,Module+disk::ConstructorVtable);
        r.Set(Action+disk::ActionExecution,cfgnames::Words{});
        r.Set(Action+disk::ActionLinkage,std::int32_t{-1});r.Set(Action+disk::ActionCallback,owner.controller);
        CHECK(request.Prepare(identity,100,true,true)&&request.Enter(true));r.reads=0;
    }
    bool Constructor(std::uintptr_t returned=Action,std::uintptr_t caller=Module+disk::ConstructorReturn){
        ActionObservation observation(r,host,owner);return observation.Constructor(request,Action,returned,Path,Latent,caller);
    }
    void Waiting(){CHECK(Constructor());CHECK(request.RequestReturned(true,true));
        r.Set(Action,Module+disk::ActionVtable);r.Set(Action+disk::ActionHandle,NativeHandle{Handle,Control});
        r.Set(Handle+disk::HandleComplete,std::uint8_t{1});r.Set(Handle+disk::HandleCancelled,std::uint8_t{});
        r.Set(Action+disk::ActionDelegate,std::array<std::uint64_t,2>{});r.reads=0;host.calls=0;}
    bool Completion(std::uintptr_t action=Action,std::uintptr_t delegate=Action+disk::ActionDelegate,
                    std::uintptr_t caller=Module+disk::DelegateReturn){
        ActionObservation observation(r,host,owner);return observation.Completion(request,action,delegate,caller);
    }
};
}
int main(){unsigned peak=0;
    {Fixture f;CHECK(f.Constructor());CHECK(f.request.Action()==f.Action);CHECK(!f.Constructor());peak=f.r.reads;}
    for(unsigned fault=0;fault<13;++fault){Fixture f;
        if(fault==0)f.owner.thread=10;
        if(fault==1)f.host.current=false;
        if(fault==2)f.r.Set(f.Latent,NativeLatent{-1,f.identity.uuid+1,{},f.identity.controller});
        if(fault==3)f.r.Set(f.Latent,NativeLatent{0,f.identity.uuid,{},f.identity.controller});
        if(fault==4)f.r.Set(f.Latent,NativeLatent{-1,f.identity.uuid,{},f.identity.controller+8});
        if(fault==5)f.r.Set(f.Action,f.Module+disk::ActionVtable); // final table isn't installed until constructor returns
        if(fault==6)f.r.Set(f.Path,NativePath{{1,1},{}});
        if(fault==7)f.r.Set(f.Path,NativePath{{1,0},{0xA00000,1,1}});
        if(fault==8)f.r.Set(f.Action+disk::ActionCallback,NativeWeak{17,7});
        if(fault==9)f.host.failAt=2;
        if(fault==10)f.r.cap=8;
        if(fault==11)f.r.changeAt=f.Action+disk::ActionPath; // coherent input changes mid-observation
        if(fault==11){f.r.changeSlot=f.Action;f.r.newPointer=f.Module+disk::ActionVtable;}
        if(fault==12)f.r.Set(f.Action+disk::ActionExecution,cfgnames::Words{1,0});
        CHECK(!f.Constructor());CHECK(!f.request.Action()&&f.request.Stage()==LoadStage::Requesting);peak=(std::max)(peak,f.r.reads);
    }
    {Fixture f;CHECK(!f.Constructor(f.Action+8));CHECK(f.r.reads==0);
        CHECK(!f.Constructor(f.Action,f.Module+disk::ConstructorReturn+1));CHECK(f.r.reads==0);}
    {Fixture f;f.Waiting();CHECK(f.Completion());CHECK(f.request.Stage()==LoadStage::Waiting);
        ActionObservation observation(f.r,f.host,f.owner);
        CHECK(!observation.Publishing(f.request,f.Action,f.Action+disk::ActionDelegate,f.Module+disk::DelegateReturn));
        CHECK(f.request.BeginCompletion(f.Action,9,1,200,true,true,true,false));
        CHECK(!f.Completion());CHECK(observation.Publishing(f.request,f.Action,f.Action+disk::ActionDelegate,f.Module+disk::DelegateReturn));
        CHECK(f.request.Complete(true));CHECK(!observation.Publishing(f.request,f.Action,f.Action+disk::ActionDelegate,f.Module+disk::DelegateReturn));
        peak=(std::max)(peak,f.r.reads);}
    // An empty FName is the verified native no-dispatch branch; a default weak
    // receiver representation must not be guessed as an object pointer.
    {Fixture f;f.Waiting();f.r.Set(f.Action+disk::ActionDelegate,std::array<std::uint64_t,2>{0xFFFFFFFF,0});CHECK(f.Completion());}
    for(unsigned fault=0;fault<15;++fault){Fixture f;f.Waiting();
        if(fault==0)f.host.current=false;
        if(fault==1)f.owner.thread=10;
        if(fault==2)f.r.Set(f.Action,f.Module+disk::ConstructorVtable);
        if(fault==3)f.r.Set(f.Action+disk::ActionCallback,NativeWeak{18,6});
        if(fault==4)f.r.Set(f.Action+disk::ActionLinkage,std::int32_t{});
        if(fault==5)f.r.Set(f.Handle+disk::HandleComplete,std::uint8_t{});
        if(fault==6)f.r.Set(f.Handle+disk::HandleCancelled,std::uint8_t{1});
        if(fault==7)f.r.Set(f.Action+disk::ActionDelegate,std::array<std::uint64_t,2>{0,1});
        if(fault==8)f.r.Set(f.Action+disk::ActionHandle,NativeHandle{f.Handle,0});
        if(fault==9)f.host.failAt=2;
        if(fault==10)f.r.cap=4;
        if(fault==11)f.r.Set(f.Action+disk::ActionPath,NativePath{{1,1},{}});
        if(fault==12){f.r.changeAt=f.Handle+disk::HandleComplete;f.r.changeSlot=f.Action+disk::ActionHandle;f.r.newPointer=f.Control;}
        if(fault==13)f.request.ObserveDestroyed(f.Action);
        if(fault==14)f.request.Expire(30100,true);
        CHECK(!f.Completion());CHECK(f.request.Stage()!=LoadStage::Ready);peak=(std::max)(peak,f.r.reads);
    }
    {Fixture f;f.Waiting();CHECK(!f.Completion(f.Action+8));CHECK(f.r.reads==0);
        CHECK(!f.Completion(f.Action,f.Action+disk::ActionDelegate+8));CHECK(f.r.reads==0);
        CHECK(!f.Completion(f.Action,f.Action+disk::ActionDelegate,f.Module+disk::DelegateReturn+1));CHECK(f.r.reads==0);}
    std::printf("Exact urination native-action observations: %u failures, peak%u/1536; constructor/final tables, copied path/latent weak owner, native no-dispatch delegate, stable complete/not-cancelled handle. No game/native/publication authority.\n",failures,peak);
    return failures?1:0;
}
