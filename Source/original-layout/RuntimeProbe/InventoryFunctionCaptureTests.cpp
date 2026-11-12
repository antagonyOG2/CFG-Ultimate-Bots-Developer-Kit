#include "InventoryFunctionCapture.hpp"
#include "InventoryNativeObservation.hpp"
#include <map>
#include <cstdio>
struct Reader {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,limit=16384;
    std::uintptr_t changeAt=0;
    template<class T>void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(std::size_t i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    bool ReadBytes(std::uintptr_t at,void* output,std::size_t size){
        if(reads>=limit)return false;++reads;
        if(at==changeAt)Set(at,std::uintptr_t{0});
        auto* p=reinterpret_cast<unsigned char*>(output);
        for(std::size_t i=0;i<size;++i){const auto it=bytes.find(at+i);if(it==bytes.end())return false;p[i]=it->second;}return true;
    }
    template<class T>bool Read(std::uintptr_t at,T& output){T value{};if(!ReadBytes(at,&value,sizeof(value)))return false;output=value;return true;}
};
struct Fixture {
    static constexpr std::uintptr_t Base=0x100000,Names=0x200000,Chunk=0x300000,Registry=0x400000,
        Pawn=0x500000,Inventory=0x510000,Type=0x520000,NativeType=0x530000,Item=0x540000,ItemType=0x550000,
        FunctionType=0x560000,ClassProperty=0x570000,ObjectProperty=0x580000,
        Find=0x600000,Spawn=0x610000,Add=0x620000;
    Reader r;cfgfeature::Report feature{};cfgprobe::lifetime::ArrayHeader registry{Registry,100};unsigned nameCount=0;
    void Name(std::uintptr_t object,const char* text){
        const auto index=nameCount++;const auto entry=0x700000+index*256;
        r.Set(Chunk+8*index,std::uintptr_t{entry});r.Set(entry,std::uint32_t{0});
        std::array<unsigned char,128> label{};for(unsigned i=0;text[i]&&i<127;++i)label[i]=static_cast<unsigned char>(text[i]);
        r.Set(entry+16,label);r.Set(object+24,cfgnames::Words{static_cast<int>(index),0});
    }
    void Slot(std::uintptr_t object,int index,int serial){r.Set(object+12,index);r.Set(Registry+24*static_cast<std::uintptr_t>(index),object);r.Set(Registry+24*static_cast<std::uintptr_t>(index)+16,serial);}
    void Field(std::uintptr_t object,std::uintptr_t owner,std::uintptr_t kind,const char* label,std::uintptr_t next){Name(object,label);r.Set(object+16,kind);r.Set(object+32,owner);r.Set(object+40,next);}
    void Parameter(std::uintptr_t object,std::uintptr_t function,const char* name,std::uintptr_t kind,int offset,std::uintptr_t next){
        Field(object,function,kind,name,next);r.Set(object+48,1);r.Set(object+52,8);r.Set(object+56,std::uint64_t{0x80});r.Set(object+68,offset);
    }
    void Function(std::uintptr_t function,std::uintptr_t owner,const char* label,std::uintptr_t next,bool add,bool blueprint){
        Field(function,owner,FunctionType,label,next);Slot(function,static_cast<int>((function-Find)/0x10000)+2,0);
        r.Set(function+56,function+0x200);r.Set(function+64,add?8:16);r.Set(function+136,std::uint32_t{0x400});
        r.Set(function+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(add?8:16));
        r.Set(function+176,std::uintptr_t{0x900000});r.Set(function+104,std::uintptr_t{0});
        r.Set(function+72,cfgmenu::capture::ScriptHeader{blueprint?function+0x400:0,blueprint?4:0,blueprint?4:0});
        if(blueprint)r.Set(function+0x400,std::array<unsigned char,4>{1,2,3,4});
        Parameter(function+0x200,function,add?"Item":"ItemClass",add?ObjectProperty:ClassProperty,0,add?0:function+0x300);
        if(!add)Parameter(function+0x300,function,"ReturnValue",ObjectProperty,8,0);
    }
    Fixture(){
        r.Set(Base+cfgnames::disk::Global,Names);r.Set(Names,Chunk);r.Set(Names+0x400,std::array<int,2>{100,1});
        r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+16,Registry);r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+28,100);
        Name(Type,"BP_SmallInventory_C");Name(FunctionType,"Function");Name(ClassProperty,"ClassProperty");Name(ObjectProperty,"ObjectProperty");
        Name(Item,"BP_FirstAid_C");Name(ItemType,"BP_FirstAid_C");
        r.Set(Type+48,NativeType);r.Set(NativeType+48,std::uintptr_t{0});r.Set(ItemType+48,std::uintptr_t{0});
        r.Set(Inventory+16,Type);r.Set(Item+16,ItemType);r.Set(Item+cfgfeature::disk::OuterInventory,Inventory);
        r.Set(Pawn+cfgfeature::disk::SmallInventory,Inventory);Slot(Inventory,0,10);Slot(Item,1,11);
        r.Set(Type+56,Spawn);r.Set(NativeType+56,Find);
        Function(Find,NativeType,"FindItem",Add,false,false);Function(Add,NativeType,"AddItem",0,true,false);Function(Spawn,Type,"SpawnItem",0,false,true);
        feature.classes[cfgfeature::disk::Inventory]=NativeType;feature.classes[cfgfeature::disk::Item]=ItemType;feature.inventories[1]=Inventory;
        feature.recordCount=1;feature.records[0].slot={Item,1,11};feature.records[0].owner=Inventory;feature.records[0].related=Inventory;
    }
    bool Capture(){return cfginventory::capture::Run(r,Base,registry,Pawn,feature);}
};
int main(){unsigned failures=0;const auto check=[&](bool value,const char* label){std::printf("%s %s\n",value?"PASS":"FAIL",label);if(!value)++failures;};
    Fixture positive;check(positive.Capture()&&positive.r.reads<16384,"held resource/native+Blueprint inventory capture within hard budget");
    for(const auto wanted:{u"FindItem",u"SpawnItem",u"AddItem"}){Fixture f;cfgmenu::capture::FunctionEvidence evidence;
        check(cfgmenu::capture::Function(f.r,Fixture::Base,Fixture::Type,wanted,evidence)&&evidence.fieldsCount==(std::u16string_view(wanted)==u"AddItem"?1u:2u),"real function metadata path finds exact inherited/Blueprint frame");}
    Fixture absent;absent.r.Set(Fixture::Type+56,std::uintptr_t{0});cfgmenu::capture::FunctionEvidence empty;
    check(!cfgmenu::capture::Function(absent.r,Fixture::Base,Fixture::Type,u"SpawnItem",empty)&&!empty.function.object,"missing Blueprint function is not invented");
    Fixture changed;changed.r.changeAt=Fixture::Inventory+16;check(!changed.Capture(),"changed inventory type refuses");
    Fixture owner;owner.r.Set(Fixture::Item+cfgfeature::disk::OuterInventory,std::uintptr_t{0});check(!owner.Capture(),"lost held-item ownership refuses");
    Fixture budget;budget.r.limit=1;check(!budget.Capture()&&budget.r.reads==1,"exhausted metadata budget cannot claim stable inventory");
    for(const bool add:{false,true}){
        Fixture f;const auto function=add?Fixture::Add:Fixture::Find;
        f.r.Set(function+136,add?cfginventory::disk::AddItemFlags:cfginventory::disk::FindItemFlags);
        f.r.Set(function+176,Fixture::Base+(add?cfginventory::disk::AddItemExecutor:cfginventory::disk::FindItemExecutor));
        f.r.Set(function+0x200+56,add?std::uint64_t{0x0018001040000280ULL}:std::uint64_t{0x001C001040000280ULL});
        if(!add)f.r.Set(function+0x300+56,std::uint64_t{0x0018001040000780ULL});
        cfgmenu::capture::FunctionEvidence actual{};
        check(cfgmenu::capture::Function(f.r,Fixture::Base,Fixture::Type,add?u"AddItem":u"FindItem",actual)&&
            cfginventory::NativeFrameMatches(actual,Fixture::Base,Fixture::NativeType,add),"captured CFG native ABI exact match (not call permission)");
        const auto& ownerBinding=cfgfeature::disk::Classes[cfgfeature::disk::Inventory];
        f.r.Set(Fixture::Base+ownerBinding.reflection,Fixture::NativeType);f.r.Set(Fixture::Base+ownerBinding.constructor,Fixture::NativeType);
        f.r.Set(Fixture::Base+(add?cfginventory::disk::AddItemCache:cfginventory::disk::FindItemCache),function);
        auto directReader=f.r;directReader.reads=0;directReader.limit=512;cfgmenu::capture::FunctionEvidence direct;
        check(cfginventory::ObserveCachedFrame(directReader,Fixture::Base,add,direct)&&directReader.reads<512,
            "known initialized cache avoids hierarchy search within512 reads");
        std::printf("Cached inventory ABI reads=%u (%s), no native calls\n",directReader.reads,add?"AddItem":"FindItem");
        auto emptyReader=directReader;emptyReader.reads=0;emptyReader.Set(Fixture::Base+(add?cfginventory::disk::AddItemCache:cfginventory::disk::FindItemCache),std::uintptr_t{0});
        check(!cfginventory::ObserveCachedFrame(emptyReader,Fixture::Base,add,direct)&&!direct.function.object,"uninitialized cache refuses without loading");
        auto changedReader=directReader;changedReader.reads=0;changedReader.Set(Fixture::Base+ownerBinding.constructor,Fixture::Type);
        check(!cfginventory::ObserveCachedFrame(changedReader,Fixture::Base,add,direct)&&!direct.function.object,"different native owner cache refuses");
        bool allFlags=true;
        for(unsigned bit=0;bit<32;++bit){auto changedFrame=actual;changedFrame.flags^=1u<<bit;
            allFlags&=!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add);}
        check(allFlags,"every native function flag mutation refuses");
        for(unsigned index=0;index<actual.fieldsCount;++index){
            bool allPropertyFlags=true;
            for(unsigned bit=0;bit<64;++bit){auto changedFrame=actual;changedFrame.fields[index].flags^=std::uint64_t{1}<<bit;
                allPropertyFlags&=!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add);}
            check(allPropertyFlags,"every property flag mutation refuses");
            auto changedFrame=actual;changedFrame.fields[index].offset+=8;
            check(!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add),"wrong native parameter offset refuses");
            changedFrame=actual;changedFrame.fields[index].field.owner=Fixture::Pawn;
            check(!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add),"foreign parameter owner refuses");
            changedFrame=actual;changedFrame.fields[index].dimension=2;
            check(!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add),"array parameter cannot masquerade as pointer");
        }
        for(unsigned fault=0;fault<7;++fault){auto changedFrame=actual;
            switch(fault){case 0:changedFrame.storageBytes+=8;break;case 1:changedFrame.destructorLink=Fixture::Pawn;break;
                case 2:changedFrame.nativeExecutor+=16;break;case 3:changedFrame.script.count=1;break;
                case 4:changedFrame.function.owner=Fixture::Type;break;case 5:changedFrame.fieldsCount=0;break;
                case 6:changedFrame.fields[0].elementBytes=4;break;}
            check(!cfginventory::NativeFrameMatches(changedFrame,Fixture::Base,Fixture::NativeType,add),"changed owner/frame/script/cleanup/executor refuses");
        }
        if(!add){auto duplicate=actual;duplicate.fields[1]=duplicate.fields[0];
            check(!cfginventory::NativeFrameMatches(duplicate,Fixture::Base,Fixture::NativeType,false),"duplicate input cannot replace return parameter");}
    }
    std::printf("InventoryFunctionCaptureTests: %u failures; synthetic buffers only\n",failures);return failures?1:0;
}
