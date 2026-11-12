#include "../RuntimeProbe/SpectatorRuntime.hpp"
#include "../RuntimeProbe/SpectatorNativeBindings.generated.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
using namespace cfgspectator;
struct Access {
    Snapshot snapshot;unsigned publications=0,restores=0,selections=0,abandons=0;
    bool observe=true,pose=true,publish=true,restore=true;Vector base{100,100,100};
    float yaw=0;Rotation look{-10,90,0};bool hasLook=true;CameraFrame frame{};
    Access(){
        snapshot.world={1,1};snapshot.controller={2,1};snapshot.spectator={3,1};
        snapshot.offline=snapshot.localOwned=snapshot.gameThread=snapshot.inProgress=true;
        snapshot.humanTerminal=snapshot.spectatorPawn=true;
        snapshot.count=2;snapshot.counselors[0]={{10,1},{11,1},true};
        snapshot.counselors[1]={{20,1},{21,1},true};snapshot.jason={{30,1},{31,1},true};
        snapshot.observedState=snapshot.counselors[0].state;
    }
    bool Observe(Snapshot& s){s=snapshot;return observe;}
    bool ReadPose(const Snapshot&,const Target&,Vector& p,float& y,Rotation& r,bool& h){p=base;y=yaw;r=look;h=hasLook;return pose;}
    bool Publish(const Snapshot&,const Target& t,const CameraFrame& f,bool selecting){
        if(!publish)return false;++publications;if(selecting){++selections;snapshot.observedState=t.state;}frame=f;return true;
    }
    bool RestoreCounselor(const Snapshot&,const Target& t){if(!restore)return false;++restores;snapshot.observedState=t.state;return true;}
    void Abandon(){++abandons;if(snapshot.observedState==snapshot.jason.state)snapshot.observedState={};}
};
int main(){
    for(unsigned count=1;count<=2;++count){Access a;Runtime r;a.snapshot.count=count;
     r.Step(a,100,true);assert(r.Following()&&a.selections==1);
     a.snapshot.observedState=a.snapshot.counselors[0].state;a.snapshot.cycleEvent=1;
     r.Step(a,200,false);assert(!r.Following()&&a.selections==1&&r.report.result==Result::Observing);
     r.Step(a,250,false);assert(!r.Following()&&a.selections==1);
     if(count==2){a.snapshot.observedState=a.snapshot.counselors[1].state;a.snapshot.cycleEvent=2;
         r.Step(a,300,false);assert(!r.Following()&&a.selections==1);}
     a.snapshot.observedState=a.snapshot.counselors[0].state;a.snapshot.cycleEvent=count+1;
     r.Step(a,400,false);assert(r.Following()&&a.selections==2);}
    {Access a;auto& s=a.snapshot;
     s.nativeSelection=s.counselors[0].pawn;ResolveNativeSelection(s);assert(s.observedState==s.counselors[0].state);
     s.nativeSelection=s.jason.pawn;ResolveNativeSelection(s);assert(s.observedState==s.jason.state);
     s.nativeSelection=s.jason.state;ResolveNativeSelection(s);assert(!s.observedState);
     s.nativeSelection={s.jason.pawn.object,0};ResolveNativeSelection(s);assert(!s.observedState);
     s.nativeSelection={99,1};ResolveNativeSelection(s);assert(!s.observedState);
     s.nativeSelection=s.jason.pawn;s.jason.living=false;ResolveNativeSelection(s);assert(!s.observedState);}
    {Access a;Runtime r;auto& s=a.snapshot;
     s.nativeSelection=s.counselors[0].pawn;ResolveNativeSelection(s);r.Step(a,100,false);
     s.nativeSelection=s.counselors[1].pawn;ResolveNativeSelection(s);r.Step(a,150,false);
     s.nativeSelection=s.counselors[0].pawn;ResolveNativeSelection(s);r.Step(a,200,false);
     assert(r.Following()&&a.selections==1);
     s.nativeSelection=s.jason.pawn;
     for(unsigned i=0;i<100;++i){ResolveNativeSelection(s);r.Step(a,220+20*i,false);}
     assert(r.Following()&&a.selections==1&&a.abandons==0&&a.publications==101);}
    {Access a;Runtime r;r.Step(a,100,false);assert(a.publications==0);
     a.snapshot.observedState=a.snapshot.counselors[1].state;r.Step(a,150,false);assert(!r.Following());
     a.snapshot.observedState=a.snapshot.counselors[0].state;r.Step(a,200,false);
     assert(r.Following()&&a.publications==1&&a.selections==1&&a.frame.location.z>192);
     r.Step(a,201,false);assert(a.publications==1);r.Step(a,216,false);assert(a.publications==2&&a.selections==1);
     a.snapshot.observedState=a.snapshot.counselors[1].state;r.Step(a,250,false);assert(!r.Following()&&a.restores==0);}
    {Access a;Runtime r;r.Step(a,100,true);assert(r.Following());r.Step(a,120,true);assert(a.restores==0);
     r.Step(a,140,false);r.Step(a,160,true);assert(!r.Following()&&a.restores==1&&a.snapshot.observedState==a.snapshot.counselors[0].state);}
    {Access a;Runtime r;a.snapshot.count=1;r.Step(a,100,false);assert(!r.Following());
     a.snapshot.cycleEvent=1;r.Step(a,150,false);assert(r.Following()&&a.publications==1);}
    {Access a;Runtime r;r.Step(a,100,false);a.snapshot.observedState=a.snapshot.counselors[1].state;r.Step(a,150,false);
     a.observe=false;r.Step(a,175,false);assert(a.publications==0&&!r.Following());
     a.observe=true;a.snapshot.observedState=a.snapshot.counselors[0].state;r.Step(a,200,false);
     assert(r.Following()&&a.selections==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.count=0;r.Step(a,120,false);assert(!r.Following()&&a.publications==1&&a.restores==0&&a.abandons==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.paused=true;r.Step(a,120,false);assert(!r.Following()&&a.publications==1&&a.abandons==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.inProgress=false;r.Step(a,120,false);assert(!r.Following()&&a.publications==1&&a.abandons==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.jason.pawn.serial=2;r.Step(a,120,false);assert(!r.Following()&&a.restores==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.controller.serial=2;a.snapshot.observedState=a.snapshot.counselors[0].state;
     r.Step(a,120,false);assert(!r.Following()&&a.publications==1&&a.abandons==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.counselors[0]=a.snapshot.counselors[1];a.snapshot.count=1;
     r.Step(a,120,false);r.Step(a,140,true);assert(a.restores==1&&!r.Following()&&a.snapshot.observedState==a.snapshot.counselors[0].state);}
    {Access a;Runtime r;r.Step(a,100,true);a.observe=false;r.Step(a,120,false);r.Step(a,140,false);
     assert(!r.Following()&&a.abandons==1&&a.publications==1&&a.restores==0);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.jason.living=false;r.Step(a,120,false);
     assert(!r.Following()&&a.abandons==1&&a.publications==1&&a.restores==0);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.jason.pawn.serial=2;a.snapshot.count=1;r.Step(a,120,false);
     assert(!r.Following()&&a.abandons==0&&a.restores==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.publish=false;r.Step(a,120,false);
     assert(!r.Following()&&a.abandons==1&&a.publications==1);}
    {Access a;Runtime r;r.Step(a,100,true);a.restore=false;r.Step(a,120,false);r.Step(a,140,true);
     assert(!r.Following()&&a.abandons==1&&a.restores==0);}
    {Access a;Runtime r;a.snapshot.counselors[0].living=false;r.Step(a,100,true);assert(a.publications==0);}
    {Access a;Runtime r;a.snapshot.counselors[1]=a.snapshot.counselors[0];r.Step(a,100,true);assert(a.publications==0);}
    {Access a;Runtime r;a.snapshot.observedState={99,1};r.Step(a,100,false);
     assert(a.restores==0&&a.publications==0&&r.report.decision==4);}
    {Access a;Runtime r;r.Step(a,100,false);a.snapshot.observedState=a.snapshot.counselors[1].state;r.Step(a,150,false);
     a.snapshot.observedState={};r.Step(a,200,false);assert(a.restores==0&&a.publications==0);
     a.snapshot.observedState={99,1};r.Step(a,250,false);assert(a.restores==0&&a.publications==0);
     a.snapshot.observedState=a.snapshot.counselors[0].state;r.Step(a,300,false);
     assert(r.Following()&&a.selections==1&&r.report.decision==9);}
    {Access a;Runtime r;r.Step(a,100,true);a.snapshot.observedState={99,1};r.Step(a,150,false);
     assert(!r.Following()&&a.restores==0&&a.abandons==1&&a.snapshot.observedState.object==99);}
    for(unsigned flag=0;flag<8;++flag){Access a;Runtime r;
        switch(flag){case 0:a.snapshot.offline=false;break;case 1:a.snapshot.localOwned=false;break;
        case 2:a.snapshot.gameThread=false;break;case 3:a.snapshot.humanTerminal=false;break;
        case 4:a.snapshot.spectatorPawn=false;break;case 5:a.snapshot.jason.living=false;break;
        case 6:a.snapshot.world.serial=0;break;default:a.snapshot.count=17;break;}
        r.Step(a,100,true);assert(a.publications==0&&a.restores==0);}
    {Access a;Runtime r;a.publish=false;r.Step(a,100,true);assert(!r.Following()&&r.report.refusals==1);}
    {Access a;Runtime r;a.base.x=std::numeric_limits<float>::quiet_NaN();r.Step(a,100,true);assert(!r.Following()&&a.publications==0);}
    {Orbit o;CameraFrame f;assert(o.Build({0,0,0},0,{},false,f));
     assert(o.Build({10,0,0},0,{100,0,30},true,f));assert(f.rotation.pitch==-2&&f.rotation.roll==0);
     assert(std::fabs(o.focus.x-1.4f)<.001f);assert(o.Build({2000,0,0},0,{-100,0,0},true,f));
     assert(o.focus.x==2000&&f.rotation.pitch==-55);assert(!o.Build({1000001,0,0},0,{},false,f));
     assert(!o.Build({},0,{0,std::numeric_limits<float>::infinity(),0},true,f));}
    static_assert(disk::ClientSetSpectatorCamera.nativeBytes==sizeof(CameraFrame),"binding frame");
    static_assert(disk::SetViewTargetWithBlend.nativeBytes==21&&offsetof(RestoreFrame,lockOutgoing)==20,"restore logical native extent");
    static_assert(disk::SetViewTargetWithBlend.storageBytes==sizeof(RestoreFrame),"restore padded storage");
    static_assert(disk::IsGamePaused.nativeBytes==sizeof(PauseFrame),"pause binding frame");
    static_assert(disk::GetControlRotation.nativeBytes==sizeof(Rotation),"look binding frame");
    std::puts("CFG spectator policy/runtime tests PASS (offline mock; no game process access)");
}
