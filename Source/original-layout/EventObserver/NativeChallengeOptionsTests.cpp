#define main ExistingOfflineOwnerTests
#include "NativeOfflineOwnerObservationTests.cpp"
#undef main
#include "NativeChallengeOptions.hpp"
struct ChallengeReader:StageReader{
    unsigned count=0,cap=8192;
    template<class T>bool Read(std::uintptr_t at,T& out){return count++<cap&&StageReader::Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t length){if(count++>=cap)return false;auto* p=static_cast<unsigned char*>(out);
        for(std::size_t i=0;i<length;++i)if(!OwnedByteReader::Read(at+i,p[i]))return false;return true;}
};
struct ChallengeWriter{ChallengeReader& r;unsigned writes=0;bool fail=false;
    template<class T>bool Write(std::uintptr_t at,const T& old,const T& value){T actual{};
        if(fail||!r.Read(at,actual)||actual!=old)return false;r.Set(at,value);++writes;return true;}
};
struct ChallengeFixture:Fixture{
    static constexpr std::uintptr_t State=0x700000,SCState=0x710000,Dossier=0x720000,DossierType=0x730000,
        Challenge=0x740000,ChallengePackage=0x750000,WorldPackage=0x760000,Move=0x770000,MoveType=0x780000;
    unsigned id=4;std::uintptr_t field=0x1000000;std::map<std::uintptr_t,std::uintptr_t> heads;
    void Label(std::uintptr_t object,const char* text){Name(id,text);r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(id++),0});}
    void Add(std::uintptr_t owner,const char* name,const char* kind,int offset,int dim,int size){
        const auto at=field;field+=0x1000;const auto klass=field;field+=0x1000;
        Label(at,name);Label(klass,kind);r.Set(at+cfgprobe::ObjectClass,klass);r.Set(at+0x20,owner);
        r.Set(at+cfgreflection::disk::Next,heads[owner]);heads[owner]=at;r.Set(owner+cfgreflection::disk::Children,at);
        r.Set(at+cfgreflection::disk::PropertyDimensions,dim);r.Set(at+cfgreflection::disk::PropertyElementSize,size);
        r.Set(at+cfgreflection::disk::PropertyOffset,offset);r.Set(at+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000214ULL});
    }
    ChallengeFixture(){using namespace cfgprobe;Birth();r.Set(Selected+ClassSuper,Killer);
        r.Set(ModeType+ClassSuper,std::uintptr_t{});r.Set(ControllerType+ClassSuper,std::uintptr_t{});
        r.Set(Module+NativeClasses[ChallengesMode].reflectionCacheRva,ModeType);
        r.Set(Module+NativeClasses[ChallengesMode].constructorCacheRva,ModeType);
        r.Set(Module+NativeClasses[PlayerControllerClass].reflectionCacheRva,ControllerType);
        r.Set(Module+NativeClasses[PlayerControllerClass].constructorCacheRva,ControllerType);
        r.Set(Mode+0x110,std::uint8_t{3});r.Set(Pawn+0x110,std::uint8_t{3});
        r.Set(World+WorldGameState,State);r.Set(State+ObjectClass,SCState);
        r.Set(Module+cfgcompat::cfg35::objects::SCGameStateReflection,SCState);
        r.Set(Module+cfgcompat::cfg35::objects::SCGameStateConstructor,SCState);
        r.Set(SCState+ClassSuper,std::uintptr_t{});r.Set(SCState+0xF8,SCState+0x2000);
        Label(SCState,"SCGameState_SPChallenges");r.Set(State+0x420,Pawn);r.Set(State+0x490,PS);
        r.Set(State+lifetime::ObjectIndexOffset,6);r.Set(Module+lifetime::ObjectArrayRva+28,int{7});
        r.Set(Registry+24*6,State);r.Set(Registry+24*6+16,int{106});
        r.Set(State+0x5B8,Dossier);r.Set(Dossier+ObjectClass,DossierType);r.Set(DossierType+ClassSuper,std::uintptr_t{});
        r.Set(Dossier+0x58,Challenge);Label(Challenge,"ChallengeC01_BrokenDown_C");r.Set(Challenge+0x20,ChallengePackage);
        Label(ChallengePackage,"/Game/Blueprints/MapRegistry/Challenges/ChallengeC01_BrokenDown");
        r.Set(World+0x20,WorldPackage);Label(WorldPackage,"/Game/Maps/Single_Player/SPC01_BrokenDown");
        Name(id,"InProgress");r.Set(Mode+0x3F8,cfgnames::Words{static_cast<int>(id++),0});
        r.Set(Pawn+0x3D0,Move);r.Set(Move+ObjectClass,MoveType);r.Set(Move+0x20,Pawn);
        r.Set(Module+cfgcompat::cfg35::objects::SCCharacterMovementReflection,MoveType);r.Set(Module+cfgcompat::cfg35::objects::SCCharacterMovementConstructor,MoveType);r.Set(MoveType+ClassSuper,std::uintptr_t{});r.Set(MoveType+0xF8,MoveType+0x2000);
        r.Set(Selected+cfgreflection::disk::Children,std::uintptr_t{});
        Add(SCState,"Dossier","ObjectProperty",0x5B8,1,8);Add(SCState,"RemainingTime","IntProperty",0x3FC,1,4);
        Add(DossierType,"ActiveChallengeClass","ClassProperty",0x58,1,8);
        Add(Killer,"NumKnives","IntProperty",0x169C,1,4);Add(Killer,"AbilityRechargeTime","FloatProperty",0x129C,5,4);
        Add(Killer,"StealthNoiseScale","FloatProperty",0x18E4,1,4);Add(Killer,"StealthRadiusScale","FloatProperty",0x18E8,1,4);
        Add(Killer,"AntiStealthNoiseScale","FloatProperty",0x18EC,1,4);Add(Killer,"AntiStealthRadiusScale","FloatProperty",0x18F0,1,4);
        Add(MoveType,"MaxRunSpeed","FloatProperty",0x724,1,4);Add(MoveType,"MaxSprintSpeed","FloatProperty",0x720,1,4);
        r.Set(Pawn+0x169C,int{5});r.Set(Pawn+0x129C,std::array<float,5>{{20,40,60,80,100}});
        r.Set(Pawn+0x18E4,std::array<float,4>{{1,2,3,4}});r.Set(Move+0x720,std::array<float,2>{{600,400}});r.Set(State+0x3FC,int{600});
        r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
    }
    void Phase(const char* phase){Name(id,phase);r.Set(Mode+0x3F8,cfgnames::Words{static_cast<int>(id++),0});r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});}
    cfgmenu::active::Context Menu(){cfgmenu::active::Context c;c.module=Module;c.engine=Engine;c.viewport=Viewport;c.world=source.frontendWorld;
        c.player=Player;c.gameInstance=Instance;c.thread=42;return c;}
};
#ifndef CFG_CHALLENGE_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0,peak=0;const auto check=[&](bool ok,const char* message){++checks;if(!ok){++failures;std::printf("FAIL %s\n",message);}};
    using namespace cfgmenu::challengeoptions;
    for(unsigned fault=0;fault<12;++fault){ChallengeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;ChallengeWriter writer{r};Runtime runtime;
        if(fault==1)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
        if(fault==2)r.Set(f.Module+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].reflectionCacheRva,f.Killer);
        if(fault==3)r.Set(f.Pawn+0x3A0,f.Player);
        if(fault==4)r.Set(f.State+0x420,f.PS);
        if(fault==5)r.Set(f.Pawn+0x110,std::uint8_t{1});
        if(fault==6)r.Set(f.Registry+24*5+16,int{0});
        if(fault==7)r.Set(f.Module+0x03198110,std::uint32_t{1});
        if(fault==8)r.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
        if(fault==9)r.Set(f.Mode+0x110,std::uint8_t{1});
        if(fault==10)r.Set(f.Dossier+0x58,std::uintptr_t{});
        if(fault==11)writer.fail=true;
        check(runtime.Arm(f.Menu(),{true,true,true,true,true},0),"submit five donor options");
        runtime.Step(r,f.Pawn,42,100,writer);peak=(std::max)(peak,r.count);
        if(fault){check(writer.writes==0,"wrong scope/authority/owner refuses all writes");continue;}
        check(runtime.bound&&runtime.failure==0&&writer.writes==5,"production provider applies all five to possessed primary Jason");
        std::array<float,5> recharge{};r.Read(f.Pawn+0x129C,recharge);check(recharge==std::array<float,5>{{10,20,30,40,100}},"four-slot donor recharge preserves CFG Endure");
        std::array<float,2> speed{};r.Read(f.Move+0x720,speed);check(speed==std::array<float,2>{{1200,800}},"native run/sprint x2");
        int knives=0,left=0;r.Read(f.Pawn+0x169C,knives);r.Read(f.State+0x3FC,left);check(knives==99&&left==-1,"refill99 and no-timeout sentinel");
        r.count=0;runtime.Step(r,f.Pawn,42,200,writer);check(writer.writes==5,"no repeated multiplication/write on same baseline");
        r.Set(f.Pawn+0x169C,int{98});r.count=0;runtime.Step(r,f.Pawn,42,300,writer);check(writer.writes==6,"refill only after native throw consumes knife");
        f.Phase("WaitingPostMatch");r.Set(f.Mode+0x3F8,f.r.bytes.empty()?cfgnames::Words{}:cfgnames::Words{static_cast<int>(f.id-1),0});
        // Copy the newly created name storage; no live process or allocator.
        for(const auto& item:f.r.bytes)if(item.first>=Fixture::Names&&item.first<0x600000)r.bytes[item.first]=item.second;
        r.count=0;runtime.Step(r,f.Pawn,42,400,writer);check(runtime.retired&&!runtime.Armed(),"native postmatch retires option ownership");
        r.Read(f.State+0x3FC,left);check(left==-1,"never restore shared postmatch sentinel");
        r.Read(f.Pawn+0x129C,recharge);check(recharge==std::array<float,5>{{20,40,60,80,100}},"restore unchanged owned recharge baseline");
    }
    {ChallengeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;ChallengeWriter writer{r};Runtime runtime;
        check(runtime.Arm(f.Menu(),{},0)&&!runtime.Armed(),"all Off leaves native challenge untouched");runtime.Step(r,f.Pawn,42,100,writer);
        check(!r.count&&!writer.writes,"all Off zero gameplay reads/writes");}
    check(StockClass(u"ChallengeC10_SnuggleByTheFire_C")&&!StockClass(u"ModeDef_OfflineBots_C"),"exact ten stock mission whitelist");
    {ChallengeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;ChallengeWriter writer{r};Runtime runtime;
        check(runtime.Arm(f.Menu(),{true,false,false,false,false},0),"mixed-mode original challenge armed");
        runtime.Step(r,f.Pawn,42,100,writer);const auto writes=writer.writes;
        check(runtime.Armed()&&!runtime.Arm(f.Menu(),{},200),"missed departure callback reproduces stale armed submission refusal");
        cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation current;
        check(cfgprobe::lifetime::ReadArrayHeader(r,f.Module+cfgprobe::lifetime::ObjectArrayRva,array)&&
         cfgprobe::lifetime::ReadSlot(r,array,f.World,current),"observe original exact positive world");
        check(!runtime.Departed(current)&&runtime.Armed(),"same live world cannot be discarded by old menu callback");
        auto menuWorld=current;++menuWorld.serial; // New incarnation, including recycled address.
        check(runtime.Departed(menuWorld)&&!runtime.Armed()&&writer.writes==writes,"fresh different world retires per-actor choices without restoring old actors");
        check(runtime.Arm(f.Menu(),{},300)&&!runtime.Armed(),"next explicit all-Off submission inherits no earlier modifier");
        r.count=0;runtime.Step(r,f.Pawn,42,400,writer);check(!r.count&&writer.writes==writes,"retired all-Off generation makes no old-world read or write");}
    {ChallengeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;ChallengeWriter writer{r};Runtime runtime;
        check(runtime.Arm(f.Menu(),{true,false,false,false,false},0),"first challenge submission");runtime.Step(r,f.Pawn,42,100,writer);
        r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);r.Set(f.Controller+cfgprobe::ControllerPawn,std::uintptr_t{});r.count=0;
        check(!runtime.Callback(r,f.Controller,200)&&runtime.retired&&!runtime.Armed()&&writer.writes==1,"menu return without pawn callback retires without old writes");
        check(runtime.Arm(f.Menu(),{},200),"next menu submission does not inherit stale armed state");}
    {ChallengeFixture f;Runtime runtime;
        check(runtime.Arm(f.Menu(),{true,false,false,false,false},0)&&runtime.Armed(),"pending settings submission armed");
        check(runtime.CancelPending()&&!runtime.Armed()&&!runtime.bound,"Back cancels unbound choices without gameplay writes");
        check(runtime.Arm(f.Menu(),{},200),"Back cancellation permits next explicit submission");}
    {ChallengeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;ChallengeWriter writer{r};Runtime runtime;
        check(runtime.Arm(f.Menu(),{true,false,false,false,false},0),"owned active modifiers armed");runtime.Step(r,f.Pawn,42,100,writer);
        check(!runtime.CancelPending()&&runtime.Armed()&&runtime.bound&&writer.writes==1,"old Back callback cannot clear active gameplay modifier ownership");}
    std::printf("Challenge native provider: %u checks, %u failures, peak%u/8192; synthetic owned memory only.\n",checks,failures,peak);return failures?1:0;
}
#endif
