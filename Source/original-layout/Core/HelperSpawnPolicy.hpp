#pragma once
#include "NativeSpawnParameters.hpp"
#include "GameplayLimits.hpp"
#include <array>
namespace cfgbundle {
constexpr unsigned HelperCandidates=4,MaximumHelperAttempts=cfglimits::MaximumHelpers*HelperCandidates;
enum class HelperStage : unsigned { Project, Spawn, Complete, Unavailable, Indeterminate };
enum class HelperFailure : unsigned { None, Resources, Projection, Budget, Packet, NullResult, Dispatch, Result, Slots };
inline bool HelpersSettled(unsigned spawned,unsigned requested,HelperStage stage){
    if(spawned>requested)return false;
    // A pre-dispatch exhausted pipeline cannot fulfill a missing request.
    // Native terminal/roster proof is STILL required before ending a match.
    // An allocated but indeterminate result must never be called settled here.
    return stage==HelperStage::Unavailable||(spawned==requested&&
        (stage==HelperStage::Complete||(!requested&&stage==HelperStage::Project)));
}
struct HelperStartup {
    HelperStage stage=HelperStage::Project;
    HelperFailure failure=HelperFailure::None;
    unsigned candidate=0;
    cfgspawn::Vec3 location{},surface{};
    // Reuse the working base spawn policy; these are geometry policy values,
    // not transplanted CFG offsets or independently measured capsule dimensions.
    static constexpr float CapsuleLift=90.f,GroundDelta=120.f;
    bool Active()const{return stage==HelperStage::Project||stage==HelperStage::Spawn;}
    bool AdoptNative(unsigned spawned,unsigned requested){
        if(stage!=HelperStage::Project||candidate||spawned>requested||requested>cfglimits::MaximumHelpers)return false;
        stage=spawned==requested?HelperStage::Complete:HelperStage::Project;return true;
    }
    bool Reinforcement(unsigned spawned,unsigned requested,unsigned limit){
        if(spawned!=requested||requested>=limit||
           (stage!=HelperStage::Complete&&!(requested==0&&stage==HelperStage::Project)))return false;
        *this={};return true;
    }
    void Refuse(HelperFailure why,bool dispatched=false){
        failure=why;stage=dispatched?HelperStage::Indeterminate:HelperStage::Unavailable;
    }
    void Next(HelperFailure why){
        if(!Active())return; // no exhausted/indeterminate pipeline can restart
        failure=why;location={};
        if(++candidate>=HelperCandidates)stage=HelperStage::Unavailable;
        else stage=HelperStage::Project;
    }
    bool Projected(cfgspawn::Vec3 point,cfgspawn::Vec3 projected){
        if(stage!=HelperStage::Project||!cfgspawn::Finite(point)||!cfgspawn::Finite(projected))return false;
        const auto dx=point.x-projected.x,dy=point.y-projected.y;
        if(dx*dx+dy*dy>220.f*220.f||std::fabs((point.z-25.f)-projected.z)>GroundDelta)return false;
        surface=projected;location=projected;location.z+=CapsuleLift;
        stage=HelperStage::Spawn;failure=HelperFailure::None;return true;
    }
    void Confirm(unsigned spawned,unsigned requested){
        if(stage!=HelperStage::Spawn||spawned>requested)return;
        candidate=0;failure=HelperFailure::None;location={};
        stage=spawned>=requested?HelperStage::Complete:HelperStage::Project;
    }
    cfgspawn::Vec3 Candidate(cfgspawn::Vec3 human,unsigned bot)const{
        constexpr std::array<cfgspawn::Vec3,12> offsets{{{700,0,0},{-700,0,0},{0,700,0},{0,-700,0},
            {500,500,0},{-500,500,0},{500,-500,0},{-500,-500,0},{350,0,0},{-350,0,0},{0,350,0},{0,-350,0}}};
        const auto o=offsets[(bot*2+candidate)%offsets.size()];
        return {human.x+o.x,human.y+o.y,human.z-CapsuleLift+25.f};
    }
};
}
