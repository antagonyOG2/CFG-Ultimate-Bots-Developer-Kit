#include "FeatureSnapshot.hpp"
#include "InventoryFunctionCapture.hpp"
#include <unordered_map>
#include <limits>

namespace {
struct Reader {
    std::unordered_map<std::uintptr_t,unsigned char> memory;
    unsigned reads=0;
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t bytes) {
        if(reads==4096) return false;
        ++reads;
        auto* target=static_cast<unsigned char*>(output);
        for(std::size_t i=0;i<bytes;++i) {
            const auto found=memory.find(address+i);
            if(found==memory.end()) return false;
            target[i]=found->second;
        }
        return true;
    }
    template<class T> bool Read(std::uintptr_t address,T& output) {
        T value{};if(!ReadBytes(address,&value,sizeof(value))) return false;output=value;return true;
    }
    template<class T> void Put(std::uintptr_t address,const T& value) {
        const auto* source=reinterpret_cast<const unsigned char*>(&value);
        for(std::size_t i=0;i<sizeof(value);++i) memory[address+i]=source[i];
    }
};
struct Fixture {
    Reader reader;
    static constexpr std::uintptr_t Module=0x10000000,State=0x40000000,Pawn=0x40010000,
        Root=0x40020000,Ability=0x40030000,Table=0x60000000,Scene=0x200F0000;
    cfgprobe::lifetime::ArrayHeader table{Table,100};
    Fixture() {
        for(unsigned i=0;i<cfgfeature::disk::ClassCount;++i) {
            const auto type=static_cast<std::uintptr_t>(0x20000000+i*0x1000);
            reader.Put(Module+cfgfeature::disk::Classes[i].reflection,type);
            reader.Put(Module+cfgfeature::disk::Classes[i].constructor,type);
            reader.Put(type+0x30,std::uintptr_t{0});
        }
        reader.Put(Module+cfgposition::disk::SceneClassCache,Scene);
        reader.Put(Module+cfgposition::disk::SceneConstructorCache,Scene);
        reader.Put(Scene+0x30,std::uintptr_t{0});
        reader.Put(Module+cfgposition::disk::LocationThunk,cfgposition::disk::LocationBytes);
        reader.Put(Module+cfgprobe::lifetime::ObjectArrayRva+0x10,Table);
        reader.Put(Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,std::int32_t{100});
        Object(State,0,0);Object(Pawn,1,1);Object(Root,2,99);Object(Ability,3,2);
        reader.Put(Pawn+cfgposition::disk::ActorRootComponent,Root);
        reader.Put(Root+0x10,Scene);reader.Put(Root+0xE0,Pawn);
        reader.Put(Root+cfgposition::disk::SceneWorldTranslation,cfgfeature::Vec{12,34,56});
        reader.Put(Pawn+cfgfeature::disk::AbilityField,Ability);
        reader.Put(Pawn+cfgfeature::disk::HasSweater,static_cast<unsigned char>(1));
        reader.Put(Pawn+cfgfeature::disk::WearingSweater,static_cast<unsigned char>(1));
        reader.Put(Pawn+cfgfeature::disk::UnlimitedSweater,static_cast<unsigned char>(0));
        reader.Put(Ability+cfgfeature::disk::AbilityOwner,Pawn);
        reader.Put(Ability+0x98,static_cast<unsigned char>(1));
        reader.Put(Pawn+cfgfeature::disk::SpecialInventory,std::uintptr_t{0});
        reader.Put(Pawn+cfgfeature::disk::SmallInventory,std::uintptr_t{0});
        reader.Put(State+cfgfeature::disk::Car4,std::uintptr_t{0});
        reader.Put(State+cfgfeature::disk::Car2,std::uintptr_t{0});
        reader.Put(State+cfgfeature::disk::Phone,std::uintptr_t{0});
        reader.Put(State+cfgfeature::disk::WorldItems,cfgfeature::Array{});
    }
    void Object(std::uintptr_t object,std::int32_t index,unsigned kind) {
        reader.Put(object+0xC,index);
        reader.Put(object+0x10,static_cast<std::uintptr_t>(0x20000000+kind*0x1000));
        reader.Put(Table+static_cast<std::uintptr_t>(index)*24,object);
        reader.Put(Table+static_cast<std::uintptr_t>(index)*24+0x10,std::int32_t{10+index});
    }
    bool Run(cfgfeature::Report& report) {
        cfgfeature::Capture<Reader> capture(reader,Module,table);
        return capture.Run(State,Pawn,true,report);
    }
};
unsigned failures=0;
void Check(bool result,const char* test) {
    if(!result) {++failures;std::printf("FAIL: %s\n",test);}
}
}
int main() {
    using namespace cfgfeature;
    Check(SaneArray({},256),"empty array");
    Check(!SaneArray({0,1,1},256),"null positive array");
    Check(!SaneArray({0x10000,257,257},256),"world bound");
    Check(!SaneArray({0x10000,2,1},32),"capacity under count");
    Check(!SanePosition({std::numeric_limits<float>::quiet_NaN(),0,0}),"NaN position");
    Report report{};
    {
        Reader empty;
        Check(!cfginventory::capture::Run(empty,Fixture::Module,{Fixture::Table,100},Fixture::Pawn,report)&&!empty.reads,"empty inventory metadata never claims stable samples or scans");
        report.inventories[1]=0x40090000;
        Check(!cfginventory::capture::Run(empty,Fixture::Module,{Fixture::Table,100},Fixture::Pawn,report)&&empty.reads==1,"missing inventory link refuses metadata before following pointer");
        report={};
    }
    Fixture good;
    Check(good.Run(report) && report.recordCount==3 && report.abilityObserved &&
        report.abilityOwner==Fixture::Pawn && report.consumedCandidate==1 &&
        report.records[1].positioned && report.records[1].position.z==56 && good.reader.reads<4096,
        "typed positive identities / root owner / sweater evidence");
    Fixture badOwner;badOwner.reader.Put(Fixture::Root+0xE0,Fixture::State);
    Check(!badOwner.Run(report) && report.recordCount==0,"wrong root owner discards output");
    Fixture badCode;badCode.reader.Put(Fixture::Module+cfgposition::disk::LocationThunk,static_cast<unsigned char>(0));
    Check(!badCode.Run(report),"changed location code rejects capture");
    Fixture badClass;badClass.reader.Put(Fixture::Module+disk::Classes[disk::GameState].constructor,std::uintptr_t{0});
    Check(!badClass.Run(report),"mismatched mandatory native class cache");
    Fixture oversized;oversized.reader.Put(Fixture::State+disk::WorldItems,Array{0x70000000,257,257});
    Check(!oversized.Run(report),"oversized world roster refuses without scanning");
    Fixture wrongPawn;wrongPawn.reader.Put(Fixture::Pawn+0x10,std::uintptr_t{0x20000000});
    Check(!wrongPawn.Run(report),"non-counselor cannot expose counselor fields");
    Fixture exhausted;exhausted.reader.reads=4096;
    Check(!exhausted.Run(report) && exhausted.reader.reads==4096,"hard read cap");
    Fixture mask;
    constexpr std::uintptr_t Mask=0x40040000,Items=0x70000000;
    mask.Object(Mask,4,disk::Mask);
    mask.reader.Put(Mask+cfgposition::disk::ActorRootComponent,std::uintptr_t{0});
    mask.reader.Put(Items,Mask);
    mask.reader.Put(Fixture::State+disk::WorldItems,Array{Items,1,1});
    Check(mask.Run(report) && report.recordCount==4 &&
        std::strcmp(report.records[3].role,"killer_mask")==0 && !report.records[3].positioned,
        "native world-list mask / absent root is not invented position");
    Fixture unassigned;
    unassigned.reader.Put(Fixture::Table+3*24+0x10,std::int32_t{0});
    Check(unassigned.Run(report) && report.unassigned==1 && report.recordCount==2,
        "unassigned serial is counted, never manufactured");
    constexpr std::uintptr_t Car=0x40050000;
    Fixture car;car.Object(Car,5,disk::Vehicle);car.reader.Put(Car+disk::Driver,Fixture::Pawn);
    DriverObservation driver;
    Check(ObserveDriver(car.reader,car.table,Car,0x20000000+disk::Vehicle*0x1000,
        0x20000000+disk::Counselor*0x1000,driver) && driver.driver==Fixture::Pawn,
        "vehicle pawn resolves typed positive counselor driver without possession changes");
    Fixture noDriver;noDriver.Object(Car,5,disk::Vehicle);noDriver.reader.Put(Car+disk::Driver,std::uintptr_t{0});
    Check(!ObserveDriver(noDriver.reader,noDriver.table,Car,0x20000000+disk::Vehicle*0x1000,
        0x20000000+disk::Counselor*0x1000,driver) && !driver.driver,"empty driver fails closed");
    Fixture notCounselor;notCounselor.Object(Car,5,disk::Vehicle);notCounselor.reader.Put(Car+disk::Driver,Fixture::State);
    Check(!ObserveDriver(notCounselor.reader,notCounselor.table,Car,0x20000000+disk::Vehicle*0x1000,
        0x20000000+disk::Counselor*0x1000,driver),"non-counselor driver never read as counselor");
    std::printf("CFG read-only feature snapshot fixtures: %u failures. No game access.\n",failures);
    return failures?1:0;
}
