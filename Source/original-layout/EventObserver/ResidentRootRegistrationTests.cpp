#include "ResidentRootRegistration.hpp"
#include "RootSlotLedger.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Registry {
    cfggcretain::ReferencerList list{0x10000,1,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{0x20000};
    unsigned calls=0,snapshots=0;bool window=true,failAfter=false,unknown=false;
    bool ProvenSlotWindow()noexcept{return window;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{
        ++snapshots;if(failAfter&&calls)return false;out=list;values=entries;return true;
    }
    bool Register(std::uintptr_t address)noexcept{
        ++calls;entries[static_cast<unsigned>(list.count++)]=address;
        auto& native=*reinterpret_cast<cfggcretain::Owner*>(address);
        auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(address);
        CHECK(native.state.load()==cfggcretain::State::Registering);
        owner.CollectWith([](std::uintptr_t*)noexcept{},!unknown);return true;
    }
};
}
int main(){
    for(unsigned fault=0;fault<7;++fault){cfgretain::SynchronizedRootOwner owner;owner.module=0x100000;
        Registry registry;cfggcprobe::Report report;
        if(fault==1)registry.window=false;
        if(fault==2)registry.list.capacity=registry.list.count;
        if(fault==3)registry.list.count=cfggcprobe::MaximumOwners;
        if(fault==4)registry.failAfter=true;
        if(fault==5)registry.unknown=true;
        if(fault==6)owner.module=0;
        const bool okay=cfgretain::RegisterResidentRootOwnerLocked(registry,owner,report);
        CHECK(okay==(fault==0));
        if(fault==0){CHECK(registry.calls==1&&report.permanent&&report.added&&report.references==0);
            CHECK(owner.native.callbacks.load()==1&&owner.native.submissions.load()==0);
            cfgretain::RootSlotLedger<> ledger(owner.native.slots);cfgretain::RootSlotLedger<>::Handle old{};
            for(unsigned i=0;i<10000;++i){
                CHECK(owner.TryPublication(registry,[&](cfggcretain::Owner&)noexcept{
                    cfgretain::RootSlotLedger<>::Reservation reserve;std::array<cfgretain::RootSlotLedger<>::Handle,1> handles{};
                    std::uintptr_t object=0;
                    if(!ledger.Reserve(1,reserve)||!ledger.Commit(reserve,std::array<std::uintptr_t,1>{0x40000+8u*i},handles)||
                       ledger.Resolve(old,object))return false;
                    old=handles[0];return ledger.Resolve(old,object)&&object==0x40000+8u*i;
                }));
                owner.CollectWith([](std::uintptr_t* slot)noexcept{*slot=0;},true);
            }
            CHECK(registry.calls==1&&registry.list.count==2&&owner.native.submissions.load()==10000);
            const auto address=&owner.native;
            CHECK(!cfgretain::RegisterResidentRootOwnerLocked(registry,owner,report)&&registry.calls==1&&&owner.native==address);
        }else if(fault==4||fault==5){CHECK(registry.calls==1&&!report.permanent&&report.result==cfggcprobe::Result::Indeterminate);}
        else CHECK(!registry.calls&&!report.permanent);
    }
    std::printf("Resident empty-owner fixed registration/10k singleton roots/spare-capacity/unknown-callback/refusal: %u failures; SYNTHETIC native list, no live engine calls\n",failures);
    return failures?1:0;
}
