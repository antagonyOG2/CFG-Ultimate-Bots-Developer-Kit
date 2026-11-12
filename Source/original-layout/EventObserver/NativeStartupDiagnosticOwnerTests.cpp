#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "NativeOfflineOwnerObservation.hpp"
// Fixture's historical macro cannot alter the production lookup gate already
// compiled above. No native retention or lookup scope is admitted here.
#define CFG_OFFLINE_OWNER_FIXTURE_ONLY
#include "NativeOfflineOwnerObservationTests.cpp"
int main(){unsigned failures=0;
    const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
    {Fixture fixture;cfgmenu::promotion::Context retained,observed;unsigned stage=0;
        check(o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,retained,nullptr,&stage,o::Purpose::DiagnosticReadOnly),"fixture independently typed");
        const auto baseValidator=[](StageReader& r,void* context){return o::Current(r,*static_cast<cfgmenu::promotion::Context*>(context),42,nullptr,o::Purpose::DiagnosticReadOnly);};
        const auto boundedValidator=[](o::Budget<StageReader>& r,void* context){return o::Current(r,*static_cast<cfgmenu::promotion::Context*>(context),42,nullptr,o::Purpose::DiagnosticReadOnly);};
        {cfgprimary::ScopedLookupEligibility<StageReader> native(fixture.Module,&retained,baseValidator,true);
            const auto before=fixture.r.reads;
            check(native.Entered()&&!o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,observed,nullptr,&stage),"PID16812 native reader alone cannot authorize budget view");
            check(stage==10&&fixture.r.reads-before==8,"exact captured initial refusal reproduced");
            {cfgprimary::ScopedLookupEligibility<o::Budget<StageReader>> bounded(fixture.Module,&retained,boundedValidator,true);
                check(bounded.Entered()&&o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,observed,nullptr,&stage)&&stage==0,"bounded typed validator admits owned runtime observer");
                fixture.r.Set(fixture.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
                check(!o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,observed),"serialization veto preserved in bounded scope");
                fixture.r.Set(fixture.Module+cfgprimary::SerializationFlag,std::uint8_t{0});
                fixture.r.Set(fixture.Module+cfgprimary::GarbageCollectionState,std::uint32_t{1});
                check(!o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,observed),"GC veto preserved in bounded scope");
                fixture.r.Set(fixture.Module+cfgprimary::GarbageCollectionState,std::uint32_t{0});
                o::Budget<StageReader> exhausted(fixture.r);exhausted.reads=o::ReadLimit;
                check(!o::LookupSafe(exhausted,fixture.Module),"bounded validator cannot bypass read cap");
            }
            check(!o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,observed),"budget capability expires independently");
        }
        check(!o::Current(fixture.r,retained,42),"no leaked native reader capability");
    }
    for(unsigned fault=0;fault<6;++fault){Fixture fixture;cfgmenu::promotion::Context owner;unsigned stage=99;
        check(!o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,owner),"runtime stays gated without native retention scope");
        if(fault==1)fixture.r.Set(fixture.Module+cfgprimary::GarbageCollectionState,std::uint32_t{1});
        if(fault==2)fixture.r.Set(fixture.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
        if(fault==3)fixture.r.Set(fixture.Registry+24*3+16,int{0});
        if(fault==4)fixture.r.Set(fixture.World+cfgprobe::WorldNetDriver,fixture.Mode);
        if(fault==5)fixture.r.Set(fixture.Controller+cfgprobe::ControllerPlayer,fixture.Mode);
        const bool observed=o::Observe(fixture.r,fixture.source,fixture.Mode,42,true,owner,nullptr,&stage,o::Purpose::DiagnosticReadOnly);
        check(observed==(fault==0),"read-only observation preserves GC/identity/offline/ownership vetoes");
        if(!fault){check(!o::Current(fixture.r,owner,42),"diagnostic snapshot never becomes runtime authority");
            check(o::Current(fixture.r,owner,42,nullptr,o::Purpose::DiagnosticReadOnly),"same metadata can be diagnosed without native calls");}
    }
    std::printf("Read-only startup owner vs retained runtime admission: %u failures; owned memory only.\n",failures);return failures?1:0;
}
