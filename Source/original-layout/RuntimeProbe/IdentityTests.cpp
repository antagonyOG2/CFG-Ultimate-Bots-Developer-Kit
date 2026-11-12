#include "ClassIdentity.hpp"
#include "EntryPrefixObservation.hpp"
#include "NamedExportEvidence.hpp"
#include "ObjectLifetime.hpp"
#include "DispatchObservation.hpp"
#include <cstdio>
#include <map>
#include <limits>
#include <vector>

namespace {
struct FixtureReader {
    std::map<std::uintptr_t,std::uintptr_t> values;
    unsigned reads = 0;
    bool Read(std::uintptr_t address, std::uintptr_t& value) {
        ++reads;
        const auto found=values.find(address);
        if (found == values.end()) return false;
        value=found->second; return true;
    }
};
struct ExportFixture {
    static constexpr std::uintptr_t Base=0x10000;
    std::vector<unsigned char> bytes=std::vector<unsigned char>(4096,0);
    unsigned reads=0;
    bool touchedCode=false;
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t count) {
        if (++reads>128 || !output || address<Base || address-Base>bytes.size() ||
            count>bytes.size()-static_cast<std::size_t>(address-Base)) return false;
        if (address<Base+0xA00 && address+count>Base+0x800) touchedCode=true;
        std::memcpy(output,bytes.data()+address-Base,count); return true;
    }
    template<class T> bool Read(std::uintptr_t address,T& value) {
        return ReadBytes(address,&value,sizeof(value));
    }
    template<class T> void Put(std::size_t offset,const T& value) {
        std::memcpy(bytes.data()+offset,&value,sizeof(value));
    }
    void Text(std::size_t offset,const char* value) {
        std::memcpy(bytes.data()+offset,value,std::strlen(value)+1);
    }
    void Initialize() {
        bytes.assign(4096,0); reads=0; touchedCode=false;
        cfgprobe::ExportDirectory directory{};
        directory.ordinalBase=1; directory.functionCount=directory.nameCount=3;
        directory.functions=0x300; directory.names=0x320; directory.ordinals=0x340;
        Put(0x100,directory);
        for (std::uint32_t i=0; i<3; ++i) {
            Put(0x300+i*4,static_cast<std::uint32_t>(0x800+i*0x100));
            Put(0x340+i*2,static_cast<std::uint16_t>(i));
        }
        Put(0x320,std::uint32_t{0x400}); Put(0x324,std::uint32_t{0x410});
        Put(0x328,std::uint32_t{0x430});
        Text(0x400,"AFirst"); Text(0x410,"LoadLibraryW"); Text(0x430,"ZLast");
    }
};
struct DispatchFixture {
    std::map<std::uintptr_t,std::uint64_t> values;
    unsigned reads=0,cap=8;
    template<class T> bool Read(std::uintptr_t address,T& output) {
        if (reads>=cap) return false;
        ++reads;
        const auto found=values.find(address);
        if (found==values.end() || found->second>(std::numeric_limits<T>::max)()) return false;
        output=static_cast<T>(found->second); return true;
    }
};
int failures=0;
#define CHECK(test) do { if (!(test)) { std::printf("FAIL line %d: %s\n",__LINE__,#test); ++failures; } } while(false)
}
int main() {
    using namespace cfgprobe;
    constexpr std::uintptr_t object=0x10000, leaf=0x20000, native=0x30000, root=0x40000;
    FixtureReader reader;
    reader.values={{object+ObjectClass,leaf},{leaf+ClassSuper,native},
        {native+ClassSuper,root},{root+ClassSuper,0}};
    Ancestry ancestry;
    CHECK(CaptureAncestry(reader,object,ancestry));
    CHECK(ancestry.count == 3 && ancestry.Contains(native) && !ancestry.Contains(0));
    CHECK(!ancestry.Contains(0x50000));
    CHECK(StableAncestry(reader,object,ancestry));
    reader.values[object+ObjectClass]=native;
    CHECK(!StableAncestry(reader,object,ancestry));
    reader.values[object+ObjectClass]=leaf;
    reader.values[native+ClassSuper]=0;
    CHECK(!StableAncestry(reader,object,ancestry));
    reader.values[native+ClassSuper]=leaf;
    CHECK(!CaptureAncestry(reader,object,ancestry) && ancestry.count == 0);
    reader.values[native+ClassSuper]=0x30001;
    CHECK(!CaptureAncestry(reader,object,ancestry) && ancestry.count == 0);
    reader.values.erase(native+ClassSuper);
    CHECK(!CaptureAncestry(reader,object,ancestry));
    CHECK(!CaptureAncestry(reader,0,ancestry));
    reader.values[object+ObjectClass]=0;
    CHECK(!CaptureAncestry(reader,object,ancestry));
    FixtureReader deep;
    deep.values[object+ObjectClass]=leaf;
    for (std::size_t i=0; i<MaximumAncestry+1; ++i) {
        const auto current=leaf+i*0x100;
        deep.values[current+ClassSuper]=i == MaximumAncestry ? 0 : current+0x100;
    }
    CHECK(!CaptureAncestry(deep,object,ancestry));
    CHECK(deep.reads == MaximumAncestry+1 && ancestry.count == 0);
    deep.values[leaf+(MaximumAncestry-1)*0x100+ClassSuper]=0;
    CHECK(CaptureAncestry(deep,object,ancestry) && ancestry.count == MaximumAncestry);
    CHECK(StableAncestry(deep,object,ancestry));
    CHECK(MatchingClassCache(leaf,leaf) == leaf);
    CHECK(MatchingClassCache(0,leaf) == 0 && MatchingClassCache(leaf,0) == 0);
    CHECK(MatchingClassCache(leaf,native) == 0 && MatchingClassCache(0,0) == 0);
    CHECK(MatchingClassCache(0x20001,0x20001) == 0);
    std::array<unsigned char,16> prefix{};
    CHECK(ObservePrefix(leaf,prefix).kind==PrefixKind::Unrecognized);
    prefix[0]=0xE9; prefix[1]=0x10;
    CHECK(ObservePrefix(leaf,prefix).destination==leaf+5+0x10);
    prefix[1]=0xF0; prefix[2]=prefix[3]=prefix[4]=0xFF;
    CHECK(ObservePrefix(leaf,prefix).destination==leaf+5-16);
    prefix={}; prefix[0]=0xFF; prefix[1]=0x25; prefix[2]=0x10;
    CHECK(ObservePrefix(leaf,prefix).kind==PrefixKind::IndirectJump);
    CHECK(ObservePrefix(leaf,prefix).pointerSlot==leaf+6+0x10);
    prefix[2]=0xF0; prefix[3]=prefix[4]=prefix[5]=0xFF;
    CHECK(ObservePrefix(leaf,prefix).pointerSlot==leaf+6-16);
    CHECK(ObservePrefix(0,prefix).kind==PrefixKind::Unrecognized);
    CHECK(ObservePrefix(MaximumUserAddress,prefix).kind==PrefixKind::Unrecognized);
    std::uintptr_t sum=0;
    CHECK(!ObservedAdd(0x10000,-1,sum) && sum==0);
    CHECK(!ObservedAdd(MaximumUserAddress,1,sum) && sum==0);
    CHECK(!ObservedAdd(leaf,(std::numeric_limits<std::int64_t>::min)(),sum));
    ExportFixture image; image.Initialize();
    NamedExportEvidence exportEvidence;
    const auto locate=[&]() { return LocateNamedExport(image,ExportFixture::Base,4096,
        0x100,0x100,"LoadLibraryW",exportEvidence); };
    CHECK(locate());
    CHECK(exportEvidence.functionRva==0x900 && exportEvidence.ordinalIndex==1 && !exportEvidence.forwarded);
    CHECK(image.reads<32 && !image.touchedCode);
    const auto original=exportEvidence;
    CHECK(SameNamedExport(original,exportEvidence));
    image.bytes[0x900]=0xE9; // Inline instrumentation is not consulted or bypassed.
    image.reads=0; CHECK(locate() && SameNamedExport(original,exportEvidence));
    CHECK(!image.touchedCode);
    image.Put(0x304,std::uint32_t{0xA00}); image.reads=0;
    CHECK(locate() && !SameNamedExport(original,exportEvidence));
    image.Initialize(); image.Put(0x342,std::uint16_t{3}); CHECK(!locate());
    image.Initialize(); image.Put(0x304,std::uint32_t{0}); CHECK(!locate());
    image.Initialize(); image.Put(0x324,std::uint32_t{4095}); CHECK(!locate());
    image.Initialize(); image.Put(0x328,std::uint32_t{0x410}); CHECK(!locate()); // Duplicate.
    image.Initialize(); image.Text(0x430,"ASecond"); CHECK(!locate()); // Local ordering.
    image.Initialize(); image.Put(0x304,std::uint32_t{0x180});
    image.Text(0x180,"KERNELBASE.LoadLibraryW");
    CHECK(locate() && exportEvidence.forwarded &&
        std::strcmp(exportEvidence.forwarder.data(),"KERNELBASE.LoadLibraryW")==0);
    CHECK(!SameNamedExport(original,exportEvidence));
    image.Initialize(); image.Put(0x304,std::uint32_t{0x180});
    for (std::size_t i=0x180; i<0x200; ++i) image.bytes[i]='x';
    CHECK(!locate()); // Forwarder must terminate inside the export directory.
    image.Initialize();
    CHECK(!LocateNamedExport(image,ExportFixture::Base,4096,0x100,39,"LoadLibraryW",exportEvidence));
    CHECK(!LocateNamedExport(image,ExportFixture::Base,4096,0xFFFFFFF0,0x100,"LoadLibraryW",exportEvidence));
    CHECK(!LocateNamedExport(image,MaximumUserAddress-1024,4096,0x100,0x100,"LoadLibraryW",exportEvidence));
    CHECK(!LocateNamedExport(image,ExportFixture::Base,4096,0x100,0x100,"Missing",exportEvidence));
    CHECK(!LocateNamedExport(image,ExportFixture::Base,4096,0x100,0x100,nullptr,exportEvidence));
    CHECK(!ImageExtent(4096,0,1) && !ImageExtent(4096,4095,2));
    using namespace cfgprobe::lifetime;
    ExportFixture slots;
    const auto array=ExportFixture::Base+0x100, data=ExportFixture::Base+0x400;
    const std::array<std::uintptr_t,4> objects{{ExportFixture::Base+0x700,
        ExportFixture::Base+0x720,ExportFixture::Base+0x740,ExportFixture::Base+0x760}};
    const auto initializeSlots=[&]() {
        slots.Initialize();
        slots.Put(0x110,data); slots.Put(0x11C,std::int32_t{4});
        for (std::size_t i=0; i<objects.size(); ++i) {
            slots.Put(static_cast<std::size_t>(objects[i]-ExportFixture::Base+ObjectIndexOffset),static_cast<std::int32_t>(i));
            slots.Put(0x400+i*ItemBytes,objects[i]);
            slots.Put(0x400+i*ItemBytes+ItemSerialOffset,static_cast<std::int32_t>(1001+i));
        }
    };
    ArrayHeader header;
    std::array<SlotObservation,4> observations{};
    initializeSlots();
    CHECK(ObserveBatch(slots,array,objects,header,observations));
    CHECK(slots.reads==14 && header.data==data && header.count==4);
    CHECK(observations[3].index==3 && observations[3].serial==1004 && observations[3].HasAssignedSerial());
    CHECK(RevalidateBatch(slots,array,header,observations) && slots.reads==28);
    bool headerChanged=false;
    slots.Put(0x110,ExportFixture::Base+0x500);slots.Put(0x11C,std::int32_t{4});
    for(std::size_t i=0;i<objects.size();++i){
        slots.Put(0x500+i*ItemBytes,objects[i]);
        slots.Put(0x500+i*ItemBytes+ItemSerialOffset,static_cast<std::int32_t>(1001+i));
    }
    CHECK(RevalidateSlotsCurrentHeader(slots,array,header,observations,&headerChanged)&&headerChanged);
    slots.Put(0x500+2*ItemBytes+ItemSerialOffset,std::int32_t{4000});
    CHECK(!RevalidateSlotsCurrentHeader(slots,array,header,observations,&headerChanged));
    initializeSlots();
    slots.Put(0x400+ItemSerialOffset,std::int32_t{2000});
    CHECK(!RevalidateBatch(slots,array,header,observations)); // Reused address, new allocation.
    initializeSlots(); slots.Put(0x400+ItemSerialOffset,std::int32_t{0});
    CHECK(ObserveBatch(slots,array,objects,header,observations));
    CHECK(!observations[0].HasAssignedSerial() && observations[0].serial==0);
    CHECK(RevalidateBatch(slots,array,header,observations)); // Observe zero, never allocate.
    initializeSlots(); slots.Put(0x400+ItemSerialOffset,std::int32_t{-1});
    CHECK(!ObserveBatch(slots,array,objects,header,observations) && header.data==0 && observations[0].object==0);
    initializeSlots(); slots.Put(0x70C,std::int32_t{-1});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x70C,std::int32_t{4});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x400,objects[1]);
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x11C,std::int32_t{0});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x11C,std::int32_t{-1});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x11C,std::int32_t{MaximumDiagnosticObjects+1});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x110,std::uintptr_t{0});
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); slots.Put(0x110,LastUserByte-7);
    CHECK(!ObserveBatch(slots,array,objects,header,observations));
    initializeSlots(); CHECK(ObserveBatch(slots,array,objects,header,observations));
    slots.Put(0x11C,std::int32_t{5});
    CHECK(!RevalidateBatch(slots,array,header,observations));
    initializeSlots(); CHECK(ObserveBatch(slots,array,objects,header,observations));
    slots.Put(0x110,data+8);
    CHECK(!RevalidateBatch(slots,array,header,observations));
    initializeSlots(); CHECK(ObserveBatch(slots,array,objects,header,observations));
    slots.Put(0x70C,std::int32_t{1});
    CHECK(!RevalidateBatch(slots,array,header,observations));
    initializeSlots(); CHECK(ObserveBatch(slots,array,objects,header,observations));
    slots.Put(0x400+2*ItemBytes,objects[3]);
    CHECK(!RevalidateBatch(slots,array,header,observations)); // Pointer changed, serial unchanged.
    initializeSlots(); CHECK(ObserveBatch(slots,array,objects,header,observations));
    slots.bytes.resize(0x418+ItemSerialOffset+2); // Later slot unavailable/partial.
    CHECK(!RevalidateBatch(slots,array,header,observations));
    initializeSlots();
    slots.bytes.resize(0x400+3*ItemBytes+ItemSerialOffset+2);
    CHECK(!ObserveBatch(slots,array,objects,header,observations)); // Fail after earlier slots succeeded.
    CHECK(header.data==0 && header.count==0);
    for (const auto& observation : observations)
        CHECK(observation.object==0 && observation.index==-1 && observation.serial==0);
    initializeSlots(); slots.reads=127;
    CHECK(!ObserveBatch(slots,array,objects,header,observations)); // Reader budget exhausted mid-header.
    CHECK(header.data==0 && header.count==0);
    for (const auto& observation : observations)
        CHECK(observation.object==0 && observation.index==-1 && observation.serial==0);
    initializeSlots();
    std::array<std::uintptr_t,4> invalidObjects=objects; invalidObjects[0]=LastUserByte-7;
    CHECK(!ObserveBatch(slots,array,invalidObjects,header,observations));
    CHECK(!Extent(0,1) && !Extent(LastUserByte,2) && Extent(LastUserByte,1));
    DispatchFixture dispatchReader;
    constexpr std::uintptr_t dispatchModule=0x140000000, dispatchPawn=0x20000, dispatchTable=0x30000;
    const auto initializeDispatch=[&]() {
        dispatchReader.reads=0;
        dispatchReader.values={{dispatchModule+dispatch::ThreadFlagRva,1},
            {dispatchModule+dispatch::ThreadIdRva,1234},{dispatchPawn,dispatchTable},
            {dispatchTable+dispatch::EventSlotBytes,dispatchModule+dispatch::NativeActorDispatcherRva}};
    };
    dispatch::Observation dispatchObserved;
    initializeDispatch();
    CHECK(dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchReader.reads==4);
    CHECK(dispatch::Revalidate(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchReader.reads==8);
    initializeDispatch(); dispatchReader.values[dispatchModule+dispatch::ThreadFlagRva]=0;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchObserved.threadId==0);
    initializeDispatch(); dispatchReader.values[dispatchModule+dispatch::ThreadFlagRva]=2;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    initializeDispatch(); dispatchReader.values[dispatchModule+dispatch::ThreadIdRva]=0;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    initializeDispatch(); dispatchReader.values[dispatchPawn]=dispatchTable+1;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    initializeDispatch(); dispatchReader.values[dispatchTable+dispatch::EventSlotBytes]=0;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchObserved.vtable==0);
    initializeDispatch(); CHECK(dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    dispatchReader.values[dispatchModule+dispatch::ThreadIdRva]=2345;
    CHECK(!dispatch::Revalidate(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    initializeDispatch(); CHECK(dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    dispatchReader.values[dispatchTable+dispatch::EventSlotBytes]+=16;
    CHECK(!dispatch::Revalidate(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));
    initializeDispatch(); dispatchReader.values.erase(dispatchTable+dispatch::EventSlotBytes);
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchObserved.target==0);
    initializeDispatch(); dispatchReader.reads=7;
    CHECK(!dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved) && dispatchObserved.threadId==0);
    initializeDispatch();
    CHECK(!dispatch::Observe(dispatchReader,LastUserByte-7,dispatchPawn,dispatchObserved) && dispatchReader.reads==0);
    // Derived target addresses are valid observations, not automatically corrupt.
    initializeDispatch(); dispatchReader.values[dispatchTable+dispatch::EventSlotBytes]+=16;
    CHECK(dispatch::Observe(dispatchReader,dispatchModule,dispatchPawn,dispatchObserved));

    constexpr std::uintptr_t registryData=0x40000,callbackObject=0x50000,callbackVtable=0x60000,callbackTarget=0x70000;
    const auto registry=dispatchModule+dispatch::diagnostic::RegistryRva;
    const auto entry=registryData;
    const auto initializeRegistry=[&]() {
        dispatchReader.reads=0;dispatchReader.cap=1024;
        dispatchReader.values={{registry+dispatch::diagnostic::ArrayData,registryData},
            {registry+dispatch::diagnostic::ArrayCount,1},{registry+0x64,0},
            {entry+dispatch::diagnostic::EntryEnabled,1},
            {entry+dispatch::diagnostic::EntryCallbackObject,callbackObject},
            {callbackObject,callbackVtable},{callbackVtable+dispatch::diagnostic::VtableSlot,callbackTarget}};
    };
    dispatch::diagnostic::Registry registryObserved;
    initializeRegistry();
    CHECK(dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved));
    CHECK(registryObserved.count==1&&registryObserved.ActiveCount()==1&&
        registryObserved.callbacks[0].target==callbackTarget&&dispatchReader.reads==7);
    dispatch::diagnostic::Registry registryAgain;
    CHECK(dispatch::diagnostic::RevalidateRegistry(dispatchReader,dispatchModule,registryObserved,&registryAgain)&&
        registryAgain.dispatchDepth==registryObserved.dispatchDepth&&dispatchReader.reads==14);
    initializeRegistry();dispatchReader.values[registry+0x64]=1;
    CHECK(!dispatch::diagnostic::RevalidateRegistry(dispatchReader,dispatchModule,registryObserved));
    initializeRegistry();dispatchReader.values[entry+dispatch::diagnostic::EntryEnabled]=0;
    CHECK(dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved));
    CHECK(registryObserved.ActiveCount()==0&&registryObserved.callbacks[0].target==0&&dispatchReader.reads==4);
    initializeRegistry();dispatchReader.values[registry+dispatch::diagnostic::ArrayCount]=65;
    CHECK(!dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved)&&registryObserved.count==0);
    initializeRegistry();dispatchReader.values[registry+dispatch::diagnostic::ArrayData]=0;
    CHECK(!dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved));
    initializeRegistry();dispatchReader.values[entry+dispatch::diagnostic::EntryCallbackObject]=0;
    CHECK(!dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved));
    initializeRegistry();dispatchReader.values[registry+dispatch::diagnostic::ArrayCount]=0;
    CHECK(dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved)&&registryObserved.count==0);
    initializeRegistry();CHECK(dispatch::diagnostic::ObserveRegistry(dispatchReader,dispatchModule,registryObserved));
    dispatchReader.values[entry+dispatch::diagnostic::EntryCallbackObject]=0x50008;
    CHECK(!dispatch::diagnostic::RevalidateRegistry(dispatchReader,dispatchModule,registryObserved));
    std::printf("CFG identity policies: %s (%d failures). Synthetic data only.\n",
        failures ? "FAIL" : "PASS",failures);
    return failures ? 1 : 0;
}
