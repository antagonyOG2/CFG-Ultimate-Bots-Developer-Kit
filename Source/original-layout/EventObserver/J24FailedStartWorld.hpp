#pragma once
#include "PreviousWorldObservation.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include "../Core/J24SpawnRefusal.hpp"
namespace cfgj24 {
enum class WorldRecord {Skipped,Recorded,Failed};
// Recording is diagnostic-only. A failed fresh record deliberately invalidates
// the old Ready world, so the existing load gate refuses instead of reusing it.
template<class R>WorldRecord RecordRootedWorld(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t world,std::uintptr_t packageType,cfgworlddiag::Previous& previous,bool enabled=CFG_J24_FAILED_START_RETIRE){
    if(!enabled||(previous.world.object&&!previous.checked))return WorldRecord::Skipped;
    return cfgworlddiag::Record(r,objects,world,previous,packageType)?WorldRecord::Recorded:WorldRecord::Failed;
}
struct FailedStartWorldCounters {
    std::uint64_t records=0,failures=0,retirements=0,generation=0,initializedGeneration=0;
    unsigned rootsAtRecord=0;
    void Recorded(WorldRecord result,const cfgworlddiag::Previous& previous,unsigned roots){
        if(result==WorldRecord::Skipped)return;
        ++records;rootsAtRecord=roots;
        if(result==WorldRecord::Failed)++failures;else generation=previous.generation;
    }
    void Initialized(const cfgworlddiag::Previous& previous){initializedGeneration=previous.generation;}
    void Retired(std::uint64_t retired){if(retired&&retired==generation&&retired!=initializedGeneration){++retirements;generation=0;}}
};
}
