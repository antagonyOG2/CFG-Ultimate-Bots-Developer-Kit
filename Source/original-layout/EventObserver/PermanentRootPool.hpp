#pragma once
#include "GcRegistryProbe.hpp"
namespace cfgretain {
// Fixed resident storage for staged native creation. Each owner is prepared
// once per match; reuse requires verified removal of EVERY published owner.
// The containing pool MUST live in the pinned DLL, not a service stack frame.
template<std::size_t Limit=16>class PermanentRootPool {
    static_assert(Limit>0&&Limit<=32);
    std::array<cfggcretain::Owner,Limit> owners_{};
    std::array<std::array<std::uintptr_t,cfggcretain::Capacity>,Limit> expected_{};
    std::array<cfggcprobe::Report,Limit> reports_{};
    unsigned used_=0;bool poisoned_=false;
    bool RetirementInputsPreserved(unsigned index)const noexcept {
        const auto& owner=owners_[index];unsigned cleared=0;
        if(owner.expected!=expected_[index]||owner.callbacks.load()!=owner.completedCallbacks.load()||owner.destroyRequests.load())return false;
        for(std::size_t j=0;j<cfggcretain::Capacity;++j){
            if(owner.slots[j]==expected_[index][j])continue;
            if(owner.slots[j]||!expected_[index][j])return false;
            ++cleared; // native collector may legitimately clear retired actors
        }
        return owner.clearedSlots.load()==cleared;
    }
public:
    unsigned Count()const noexcept{return used_;}
    bool Poisoned()const noexcept{return poisoned_;}
    std::uintptr_t Address(unsigned i)const noexcept {
        return i<used_?reinterpret_cast<std::uintptr_t>(&owners_[i]):0;
    }
    unsigned References(unsigned i)const noexcept{return i<used_?reports_[i].references:0;}
    std::uintptr_t First(unsigned i)const noexcept{return i<used_?expected_[i][0]:0;}
    const cfggcprobe::Report& LastReport()const noexcept{return reports_[used_?used_-1:0];}
    bool Contains(std::uintptr_t value)const noexcept {
        if(!value||poisoned_)return false;
        for(unsigned i=0;i<used_;++i){
            if(owners_[i].state.load(std::memory_order_acquire)!=cfggcretain::State::Registered)return false;
            for(const auto item:expected_[i])if(item==value)return true;
        }
        return false;
    }
    template<class Access,std::size_t N>bool AddLocked(Access& access,
        const std::array<std::uintptr_t,N>& input,cfggcprobe::Report& registration)noexcept {
        static_assert(N>0&&N<=cfggcretain::Capacity);
        if(poisoned_)return false;
        std::array<std::uintptr_t,cfggcretain::Capacity> fresh{};unsigned count=0;
        for(const auto value:input){
            if(!value||Contains(value))continue;
            bool duplicate=false;for(unsigned j=0;j<count;++j)duplicate|=fresh[j]==value;
            if(!duplicate)fresh[count++]=value;
        }
        if(!count)return used_!=0;
        if(used_==Limit){poisoned_=true;return false;}
        // Reserve permanently BEFORE a native call. An exception or uncertain
        // result can never recycle storage that the engine may have published.
        const auto index=used_++;poisoned_=true;
        cfggcprobe::RegisterPermanentInputsLocked(access,owners_[index],registration,fresh);
        expected_[index]=owners_[index].expected;reports_[index]=registration;
        poisoned_=registration.result!=cfggcprobe::Result::Passed||!registration.permanent;
        return !poisoned_;
    }
    void RecordAccess(const cfggcprobe::Report& registration)noexcept {
        for(unsigned i=0;i<used_;++i)if(registration.owner==Address(i)){reports_[i]=registration;return;}
    }
    // Completed-match retirement only. Caller owns the independently proved
    // collector/registry window throughout; a failed native removal is NEVER
    // replayed and no uncertain storage is recycled.
    template<class Access>bool ReleaseAllLocked(Access& access,cfggcprobe::Report& report)noexcept {
        report={};report.result=cfggcprobe::Result::Refused;report.stage=9;
        if(poisoned_)return false;
        const auto fail=[&]()noexcept{poisoned_=true;report.result=cfggcprobe::Result::Indeterminate;return false;};
        for(unsigned index=0;index<used_;++index){
            auto& owner=owners_[index];const auto own=Address(index);report.owner=own;
            if(owner.state.load(std::memory_order_acquire)!=cfggcretain::State::Registered||!RetirementInputsPreserved(index))return fail();
            cfggcretain::ReferencerList before{},after{};
            std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{},fresh{};
            if(!access.Snapshot(before,entries)||before.count<=0||before.count>cfggcprobe::MaximumOwners)return fail();
            unsigned matches=0;for(int i=0;i<before.count;++i)matches+=entries[i]==own?1u:0u;
            if(matches!=1||!owner.BeginRemoval())return fail();
            // From here even a failed readback must permanently veto replay.
            poisoned_=true;report.stage=10;++report.calls;
            if(!access.Unregister(own)){owner.Removed(false);return fail();}
            report.stage=11;
            bool removed=access.Snapshot(after,fresh)&&after.data==before.data&&after.capacity==before.capacity&&after.count==before.count-1;
            // RemoveSingle may shift OR swap. Verify all other memberships,
            // including multiplicity, without depending on array order.
            for(int i=0;i<before.count&&removed;++i){
                unsigned oldCount=0,newCount=0;
                for(int j=0;j<before.count;++j)oldCount+=entries[j]==entries[i]?1u:0u;
                for(int j=0;j<after.count;++j)newCount+=fresh[j]==entries[i]?1u:0u;
                removed=newCount==(entries[i]==own?0u:oldCount);
            }
            removed=removed&&RetirementInputsPreserved(index);
            if(!owner.Removed(removed))return fail();
            ++report.removed;
        }
        // Every address is absent from the native list and every callback is
        // finished. Only this all-or-nothing result permits fresh match inputs.
        for(unsigned i=0;i<used_;++i){auto& owner=owners_[i];
            owner.slots={};owner.expected={};owner.callbacks.store(0);owner.completedCallbacks.store(0);
            owner.submissions.store(0);owner.clearedSlots.store(0);owner.destroyRequests.store(0);
            owner.state.store(cfggcretain::State::Empty,std::memory_order_release);
        }
        expected_={};reports_={};used_=0;poisoned_=false;
        report.stage=12;report.result=cfggcprobe::Result::Passed;return true;
    }
    // Validate the specific retained metadata owner, not an unrelated old
    // actor's collector-cleared slots. No bare Contains() lifetime authority.
    template<class Reader>bool CurrentReference(Reader& reader,std::uintptr_t value)const noexcept {
        if(!value||!used_||poisoned_)return false;
        for(unsigned i=0;i<used_;++i){bool found=false;for(const auto item:expected_[i])found|=item==value;
            if(!found)continue;
            cfggcprobe::OwnerObservation observed;cfggcprobe::OwnerObservationFailure failure{};
            return cfggcprobe::CaptureOwnerObservation(reader,Address(i),observed,failure)&&
                observed.InputsPreserved(reports_[i].references,expected_[i][0])&&observed.expected==expected_[i];
        }
        return false;
    }
    template<class Reader>bool Current(Reader& reader)const noexcept {
        if(!used_||poisoned_)return false;
        for(unsigned i=0;i<used_;++i){
            cfggcprobe::OwnerObservation observed;cfggcprobe::OwnerObservationFailure failure{};
            if(!cfggcprobe::CaptureOwnerObservation(reader,Address(i),observed,failure)||
               !observed.InputsPreserved(reports_[i].references,expected_[i][0])||
               observed.expected!=expected_[i])return false;
        }
        return true;
    }
    void Counters(unsigned& calls,unsigned& reads,unsigned& callbacks,unsigned& completed,
        unsigned& submissions,unsigned& cleared,unsigned& destroyed)const noexcept {
        calls=reads=callbacks=completed=submissions=cleared=destroyed=0;
        for(unsigned i=0;i<used_;++i){
            calls+=reports_[i].calls;reads+=reports_[i].reads;
            callbacks+=owners_[i].callbacks.load();completed+=owners_[i].completedCallbacks.load();
            submissions+=owners_[i].submissions.load();cleared+=owners_[i].clearedSlots.load();
            destroyed+=owners_[i].destroyRequests.load();
        }
    }
};
}
