// Replay the current birth member with a synthetic UObject table and native boundary.
#include <Windows.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include "SpawnAssetObservation.hpp"
#include "HostedAdoption.hpp"
#include "HostedListenAuthorityObservation.hpp"
#include "../Core/HostedCounselorBirth.hpp"
#include "../RuntimeProbe/HostedBirth35.generated.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../RuntimeProbe/HostedCounselorBrainBindings.generated.hpp"

namespace cfgbundle {
template<class R,class T> bool Code(R&,std::uintptr_t,std::uintptr_t,const T&){return true;}
bool NativeFrame(unsigned bytes,unsigned expected,unsigned storage){return bytes==expected&&storage==16;}
}
namespace {
using Slot=cfgprobe::lifetime::SlotObservation;
using namespace cfgprobe::lifetime;
constexpr std::uintptr_t Data=0x21000000, World=0x22000000, Mode=0x23000000, GameState=0x24000000,
    Local=0x25000000, Human=0x26000000, Bot=0x27000000, Pawn=0x28000000, State=0x29000000,
    Tree=0x30000000, TreeType=0x31000000, AiType=0x32000000, Owner=0x33000000, Function=0x34000000;
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    unsigned reads=0;
    template<class T> void Put(std::uintptr_t at,const T& value){
        static_assert(sizeof(T)<=16);auto& bytes=memory[at];bytes={};std::memcpy(bytes.data(),&value,sizeof(T));
    }
    template<class T> bool Read(std::uintptr_t at,T& value){
        ++reads;const auto found=memory.find(at);if(found==memory.end())return false;
        std::memcpy(&value,found->second.data(),sizeof(T));return true;
    }
};
struct Trace {
    std::uintptr_t pawn=Pawn,controller=Bot,brain=0,tree=0;
    unsigned samples=0,last=0,first=0,reads=0,startAttempts=0,startReturned=0,running=2,paused=2;
};
std::array<Trace,1> hostedBots;
struct {std::uintptr_t module=0,world=World,mode=Mode,controller=Local,pawn=Human;} plan;
struct {std::uintptr_t gameState=GameState;} hostedTx;
struct {
    std::uintptr_t pawn=Pawn;
    Slot pawnSlot{Pawn,7,101};
    struct {Slot controller{Bot,6,102},state{State,8,103};bool stateSerialAssigned=true;} born;
} hostedCounselor;
using Dispatch=void(__fastcall*)(void*,void*,void*);
unsigned dispatches=0;
bool nativeReturn=true,nativeFault=false,correctInputs=false;
void __fastcall Native(void* receiver,void* function,void* packet){
    ++dispatches;
    std::uintptr_t tree=0;std::memcpy(&tree,packet,sizeof(tree));
    correctInputs=reinterpret_cast<std::uintptr_t>(receiver)==Bot&&
        reinterpret_cast<std::uintptr_t>(function)==Function&&tree==Tree;
    if(nativeFault)RaiseException(0xE1234567,0,0,nullptr);
    static_cast<unsigned char*>(packet)[8]=nativeReturn?1:0;
}
Reader* factoryMemory=nullptr;
bool replaceTreeAtFactory=false;
}
namespace cfgmenu::population::detail {
bool GetClass(std::uintptr_t,std::uintptr_t& value){
    value=Function;factoryMemory->Put(plan.module+cfghostedbrain::disk::RunBehaviorTree.cache,value);
    if(replaceTreeAtFactory)factoryMemory->Put(Mode+cfghosted::birth35::CounselorBehaviorTree,std::uintptr_t{});
    return true;
}
}
namespace {
struct Fixture {
    Reader r_;
    struct {ArrayHeader objectArray{Data,32};} context;
    unsigned calls_=0;
    bool exception_=false,window=true,finish=true,uncertain=false,treeRooted=false;
    bool replaceTreeAtPrepare=false,entered=false;
    Fixture(){
        hostedBots={};hostedCounselor={};dispatches=0;nativeReturn=true;nativeFault=false;
        correctInputs=false;replaceTreeAtFactory=false;factoryMemory=&r_;
        plan.module=reinterpret_cast<std::uintptr_t>(&Native)-cfgspawn::GenericDispatcherRva;
        Add({World,0,1});Add({Mode,1,2});Add({GameState,2,3});Add({Local,3,4});Add({Human,4,5});
        Add({Tree,5,0});Add(hostedCounselor.born.controller);Add(hostedCounselor.pawnSlot);
        Add(hostedCounselor.born.state);Add({Owner,9,0});Add({Function,10,0});
        r_.Put(Bot+cfgprobe::ControllerPawn,Pawn);
        r_.Put(Pawn+cfgescape::disk::Controller,Bot);
        r_.Put(Pawn+cfgescape::disk::PlayerState,State);
        r_.Put(Bot+cfgescape::disk::PlayerState,State);
        r_.Put(State+0x88,Bot);
        r_.Put(Mode+cfghosted::birth35::CounselorBehaviorTree,Tree);
        r_.Put(Bot+0x418,std::uintptr_t{});
        const auto& ai=cfgcompat::cfg35::ai::Classes[2];
        const auto& tree=cfgcompat::cfg35::ai::Classes[4];
        r_.Put(plan.module+ai.reflection,AiType);r_.Put(plan.module+ai.constructor,AiType);
        r_.Put(plan.module+tree.reflection,TreeType);r_.Put(plan.module+tree.constructor,TreeType);
        r_.Put(Bot+cfgprobe::ObjectClass,AiType);r_.Put(AiType+cfgprobe::ClassSuper,Owner); r_.Put(Owner+cfgprobe::ClassSuper,std::uintptr_t{});
        r_.Put(Tree+cfgprobe::ObjectClass,TreeType);r_.Put(TreeType+cfgprobe::ClassSuper,std::uintptr_t{});
        const auto& binding=cfghostedbrain::disk::RunBehaviorTree;
        r_.Put(plan.module+binding.ownerReflection,Owner);r_.Put(plan.module+binding.ownerConstructor,Owner);
        r_.Put(plan.module+binding.cache,Function);
        r_.Put(Function+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(binding.nativeBytes));
        r_.Put(Function+0x20,Owner);
        r_.Put(Function+0xB0,plan.module+cfghostedbrain::disk::RunBehaviorTreeExecutor);
    }
    void Add(const Slot& slot){
        r_.Put(slot.object+ObjectIndexOffset,slot.index);
        const auto at=Data+static_cast<std::uintptr_t>(slot.index)*ItemBytes;
        r_.Put(at,slot.object);r_.Put(at+8,std::uint32_t{});r_.Put(at+ItemSerialOffset,slot.serial);
    }
    bool HostedWorld(){return true;}
    bool HostedInWorld(std::uintptr_t object){return object==Pawn||object==Bot||object==State;}
    bool HostedReadRoster(std::uintptr_t state,std::int32_t& count,bool& contains){
        count=1;contains=state==State;return true;
    }
    bool HostedPromoteCounselorState(){return false;} // fixture birth tuple already has positive identities
    template<std::size_t N> bool HostedPrepare(const std::array<std::uintptr_t,N>& roots,unsigned){
        if(!window||entered)return false;
        // Exercise the production root-value slot predicate. Lease acquisition is simulated.
        for(const auto root:roots){
            Slot current{};
            if(root&&!cfgspawn::ObjectSlot(r_,context.objectArray,root,current))return false;
            if(root==Tree)treeRooted=current.serial==0;
        }
        entered=true;
        if(replaceTreeAtPrepare)r_.Put(Mode+cfghosted::birth35::CounselorBehaviorTree,std::uintptr_t{});
        return true;
    }
    bool RetainBundleReturnedObject(std::uintptr_t object){const bool ok=entered&&object==Function;entered=false;return ok;}
    bool FinishBundleNative(){const bool ok=entered&&finish;entered=false;if(!ok)BundleRetentionFailed();return ok;}
    void BundleRetentionFailed(){entered=false;uncertain=true;}
#include "HostedCounselorBirthBrainMembers.hpp"
};
}
int main(){
    unsigned checks=0,failures=0;
    const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s (refusal=%u)\n",label,hostedBots[0].last);}};
    {Fixture f;check(f.HostedCounselorBirthBrain()&&f.treeRooted&&correctInputs&&dispatches==1&&
         hostedBots[0].startReturned==1,"zero-serial owned tree dispatches");
     check(!f.HostedCounselorBirthBrain()&&dispatches==1,"successful start not replayed");}
    {Fixture f;f.r_.Put(Tree+cfgprobe::ObjectClass,Owner);
     check(!f.HostedCounselorBirthBrain()&&dispatches==0,"wrong tree type");}
    {Fixture f;f.r_.Put(plan.module+cfgcompat::cfg35::ai::Classes[4].constructor,Owner);
     check(!f.HostedCounselorBirthBrain()&&dispatches==0,"changed class cache");}
    {Fixture f;f.r_.Put(Data+5*ItemBytes,Owner);
     check(!f.HostedCounselorBirthBrain()&&dispatches==0,"recycled tree slot");}
    {Fixture f;f.window=false;check(!f.HostedCounselorBirthBrain()&&!hostedBots[0].startAttempts&&
         dispatches==0,"retention refused before entry");}
    {Fixture f;f.replaceTreeAtPrepare=true;
     check(!f.HostedCounselorBirthBrain()&&dispatches==0&&!f.entered&&!f.uncertain,"tree changed during retention, window closed");}
    {Fixture f;f.r_.Put(plan.module+cfghostedbrain::disk::RunBehaviorTree.cache,std::uintptr_t{});
     replaceTreeAtFactory=true;
     check(!f.HostedCounselorBirthBrain()&&dispatches==0,"tree changed during factory reentry");}
    {Fixture f;nativeReturn=false;
     check(!f.HostedCounselorBirthBrain()&&!f.HostedCounselorBirthBrain()&&dispatches==1,"false return not replayed");}
    {Fixture f;nativeFault=true;
     check(!f.HostedCounselorBirthBrain()&&!f.HostedCounselorBirthBrain()&&dispatches==1&&f.uncertain,"fault not replayed");}
    {Fixture f;f.finish=false;
     check(!f.HostedCounselorBirthBrain()&&!f.HostedCounselorBirthBrain()&&dispatches==1&&f.uncertain,"uncertain finish not replayed");}
    std::printf("H225 counselor birth member: %u checks, %u failures.\n",checks,failures);
    return failures?1:0;
}

