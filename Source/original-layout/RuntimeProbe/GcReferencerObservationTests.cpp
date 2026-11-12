#include "GcReferencerObservation.hpp"
#define main ExistingAiFixtureMain
#include "AiResourceTests.cpp"
#undef main
struct RetentionFixture:Fixture {
    std::uintptr_t type=0,referencer=0;
    RetentionFixture(){
        type=Object();referencer=Object(type);
        Put(module+cfgcompat::cfg35::gcRetention::ReferencerGlobalRva,referencer);
        Put(module+cfgcompat::cfg35::gcRetention::ReferencerClassCacheRva,type);
        Put(referencer+0x28,cfggcretain::ReferencerList{0x710000,2,4});
        Put(module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,objects.data);
        Put(module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,objects.count);
    }
    bool Observe(){cfggcretain::Observation out;return cfggcretain::ObserveReferencer(*this,module,out);}
};
struct ListWindowFixture:RetentionFixture {
    bool locked=false;unsigned unlockedListReads=0,lockedListReads=0;
    template<class T>bool Read(std::uintptr_t address,T& value){
        const bool okay=Fixture::Read(address,value);
        if(address==referencer+cfgcompat::cfg35::gcRetention::OwnerArray){
            if(locked)++lockedListReads;
            else{++unlockedListReads;Put(address,cfggcretain::ReferencerList{0x710000+0x1000u*unlockedListReads,2,4});}
        }
        return okay;
    }
};
int main(){
    {RetentionFixture f;CHECK(f.Observe()&&f.reads<64);}
    {RetentionFixture f;f.Put(f.module+cfgcompat::cfg35::gcRetention::ReferencerGlobalRva,std::uintptr_t{});CHECK(!f.Observe());}
    {RetentionFixture f;f.Put(f.referencer+cfgprobe::ObjectClass,f.type+8);CHECK(!f.Observe());}
    {RetentionFixture f;f.Put(f.referencer+0x28,cfggcretain::ReferencerList{0x710000,5,4});CHECK(!f.Observe());}
    {RetentionFixture f;f.Put(f.referencer+0x28,cfggcretain::ReferencerList{0,2,4});CHECK(!f.Observe());}
    {RetentionFixture f;f.Put(f.referencer+0x28,cfggcretain::ReferencerList{0,0,0});CHECK(f.Observe());}
    {RetentionFixture f;f.trigger=f.module+cfgcompat::cfg35::gcRetention::ReferencerGlobalRva;
     f.mutation=f.trigger;f.newPointer=f.referencer+8;CHECK(!f.Observe());}
    {RetentionFixture f;f.trigger=f.referencer+0x28;f.mutation=f.trigger;f.newPointer=0x720000;CHECK(!f.Observe());}
    {ListWindowFixture f;cfggcretain::Observation out;
     CHECK(!cfggcretain::ObserveReferencer(f,f.module,out)&&f.unlockedListReads==2);
     f.unlockedListReads=0;cfggcretain::ReferencerWindowFailure failure{};unsigned locks=0;
     CHECK(cfggcretain::ObserveReferencerWindow(f,f.module,[&](const cfggcretain::Observation& endpoint){
         CHECK(!f.unlockedListReads&&endpoint.object==f.referencer&&endpoint.type==f.type);
         ++locks;f.locked=true;f.Put(f.referencer+0x28,cfggcretain::ReferencerList{0x730000,3,8});return true;
     },out,failure));
     CHECK(locks==1&&!f.unlockedListReads&&f.lockedListReads==2&&out.owners.count==3&&out.owners.data==0x730000&&failure==cfggcretain::ReferencerWindowFailure::None);}
    for(unsigned fault=0;fault<5;++fault){ListWindowFixture f;cfggcretain::Observation out;
     cfggcretain::ReferencerWindowFailure failure{};unsigned locks=0;
     if(fault==0)f.Put(f.referencer+cfgprobe::ObjectClass,f.type+8);
     CHECK(!cfggcretain::ObserveReferencerWindow(f,f.module,[&](const cfggcretain::Observation&){
         ++locks;if(fault==1)return false;f.locked=true;
         if(fault==2)f.Put(f.referencer+0x28,cfggcretain::ReferencerList{0x710000,5,4});
         if(fault==3)f.Put(f.module+cfgcompat::cfg35::gcRetention::ReferencerGlobalRva,f.referencer+8);
         if(fault==4){cfgprobe::lifetime::SlotObservation slot;CHECK(cfgprobe::lifetime::ReadSlot(f,f.objects,f.referencer,slot));
             f.Put(f.objects.data+24u*static_cast<unsigned>(slot.index)+16,slot.serial+1);}
         return true;
     },out,failure));
     CHECK(!out.object&&!f.unlockedListReads&&locks==(fault?1u:0u));
     if(fault==0)CHECK(failure==cfggcretain::ReferencerWindowFailure::Endpoint);
     if(fault==1)CHECK(failure==cfggcretain::ReferencerWindowFailure::ListLock);
     if(fault==2||fault==3)CHECK(failure==cfggcretain::ReferencerWindowFailure::LockedObservation);
     if(fault==4)CHECK(failure==cfggcretain::ReferencerWindowFailure::ChangedEndpoint);
    }
    std::printf("GC referencer identity/list/mutation/bounds: %u failures; synthetic memory only\n",failures);
    return failures?1:0;
}
