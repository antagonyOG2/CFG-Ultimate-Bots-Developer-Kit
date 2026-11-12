#pragma once
#include "../RuntimeProbe/NativeMapSelectionObservation.hpp"
namespace cfgmenu::humanjason {
// A routing hint only. Native rows/Start still require the full active owner proof.
struct Top {
 std::uintptr_t world=0,hud=0,stack=0,widget=0,type=0,mode=0,pending=0;
 active::StackArray array{};
 bool operator==(const Top& b)const{return world==b.world&&hud==b.hud&&stack==b.stack&&widget==b.widget&&type==b.type&&mode==b.mode&&pending==b.pending&&array==b.array;}
};
template<class R>bool SampleTop(R& r,const active::Context& source,std::uintptr_t receiver,Top& out){
 out={};if(receiver!=source.hud||!source.hud||!source.viewport)return false;
 Top t;t.hud=receiver;std::uintptr_t engine=0,viewport=0,net=1;
 if(!r.Read(source.module+cfgprobe::EngineGlobalCandidate,engine)||engine!=source.engine||
    !r.Read(engine+cfgprobe::EngineGameViewport,viewport)||viewport!=source.viewport||
    !r.Read(viewport+cfgprobe::ViewportWorld,t.world)||t.world!=source.world||
    !r.Read(t.world+cfgprobe::WorldNetDriver,net)||net||
    !r.Read(t.world+cfgprobe::WorldGameMode,t.mode)||t.mode!=source.mode||
    !r.Read(receiver+execdisk::HUDStack,t.stack)||!cfgprobe::ObjectPointer(t.stack)||
    !r.Read(t.stack+execdisk::StackArray,t.array)||!t.array.Sane()||
    !r.Read(t.array.data+static_cast<std::uintptr_t>(t.array.count-1)*execdisk::RecordBytes+execdisk::RecordWidget,t.widget)||!cfgprobe::ObjectPointer(t.widget)||
    !r.Read(t.widget+cfgprobe::ObjectClass,t.type)||!cfgprobe::ObjectPointer(t.type)||
    !r.Read(t.mode+selectiondisk::PendingOfflineMode,t.pending))return false;
 out=t;return true;
}
template<class R>bool ObserveTop(R& r,const active::Context& source,std::uintptr_t receiver,Top& out){
 out={};Top a,b;if(!SampleTop(r,source,receiver,a)||!SampleTop(r,source,receiver,b)||!(a==b))return false;out=b;return true;
}
}
