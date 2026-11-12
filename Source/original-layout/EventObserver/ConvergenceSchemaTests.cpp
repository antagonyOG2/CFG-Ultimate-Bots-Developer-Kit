#include "ConvergenceBlackboardSchema.hpp"
#include <map>
#include <vector>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <algorithm>
namespace {
struct Reader {
    std::map<std::uintptr_t,std::vector<unsigned char>> data;unsigned reads=0,cap=1536;
    template<class T> void Put(std::uintptr_t at,const T& v){std::vector<unsigned char> b(sizeof(v));std::memcpy(b.data(),&v,sizeof(v));data[at]=b;}
    template<class T> bool Read(std::uintptr_t at,T& v){if(reads==cap)return false;++reads;const auto found=data.find(at);
        if(found==data.end()||found->second.size()!=sizeof(v))return false;std::memcpy(&v,found->second.data(),sizeof(v));return true;}
};
struct Fixture {
    Reader r;cfgprobe::lifetime::ArrayHeader array{0x100000,1024};
    static constexpr std::uintptr_t Module=0x140000000,Component=0x200000,Child=0x210000,Parent=0x220000;
    std::array<std::uintptr_t,4> types{{0x300000,0x310000,0x320000,0x330000}};
    std::array<cfgconvergence::Name,cfgconvergence::KeyCount> names{};
    cfgprobe::lifetime::SlotObservation componentSlot{};int index=0;
    void Object(std::uintptr_t p,std::uintptr_t type,int serial=0){r.Put(p+0xC,index);r.Put(p+0x10,type);
        r.Put(array.data+24*static_cast<unsigned>(index),p);r.Put(array.data+24*static_cast<unsigned>(index)+8,std::uint32_t{0});
        r.Put(array.data+24*static_cast<unsigned>(index)+16,serial);++index;}
    void Entry(unsigned key,std::uintptr_t asset,std::uintptr_t cell){
        const std::uintptr_t type=0x500000+key*0x100;Object(type,types[key<=cfgconvergence::Jason?3:2]);
        r.Put(type+0x20,asset);r.Put(cell,names[key]);r.Put(cell+8,type);
        if(key<=cfgconvergence::Jason)r.Put(type+cfgconvergence::schemadisk::ObjectBaseClass,std::uintptr_t{0x340000});
    }
    Fixture(){
        using namespace cfgconvergence::schemadisk;
        constexpr std::array<TypeBinding,4> b{{BlackboardComponent,BlackboardData,BoolKey,ObjectKey}};
        for(unsigned i=0;i<4;++i){r.Put(Module+b[i].reflection,types[i]);r.Put(Module+b[i].constructor,types[i]);r.Put(types[i]+0x30,std::uintptr_t{0});}
        r.Put(std::uintptr_t{0x340030},std::uintptr_t{0});
        for(unsigned i=0;i<names.size();++i)names[i]={static_cast<int>(42+i),0};
        Object(Component,types[0],17);Object(Child,types[1]);Object(Parent,types[1]);
        r.Put(Component+BlackboardAsset,Child);r.Put(Child+cfgconvergence::schemadisk::Parent,Parent);
        r.Put(Parent+cfgconvergence::schemadisk::Parent,std::uintptr_t{0});
        struct Header{std::uintptr_t data;int count,capacity;};
        r.Put(Child+Keys,Header{0x400000,3,3});r.Put(Parent+Keys,Header{0x410000,6,6});
        for(unsigned i=0;i<9;++i)Entry(i,i<3?Child:Parent,(i<3?0x400000:0x410000)+(i<3?i:i-3)*EntryBytes);
        assert(cfgprobe::lifetime::ReadSlot(r,array,Component,componentSlot));r.reads=0;
    }
    cfgconvergence::BlackboardSchema::State Finish(cfgconvergence::BlackboardSchema& s,unsigned& peak){
        for(unsigned n=0;n<30;++n){r.reads=0;const auto state=s.Step(r,array,Module,Component,componentSlot,names);
            peak=(std::max)(peak,r.reads);assert(r.reads<1536);
            if(state==cfgconvergence::BlackboardSchema::State::Ready||state==cfgconvergence::BlackboardSchema::State::Unavailable)return state;}
        assert(false);return s.Status();
    }
};
}
int main(){
    using S=cfgconvergence::BlackboardSchema;unsigned peak=0;
    {Fixture f;S s;assert(f.Finish(s,peak)==S::State::Ready&&s.Mask()==511);
        f.r.reads=0;assert(s.Current(f.r,f.array));peak=(std::max)(peak,f.r.reads);
        f.Object(0x700000,0x340000,33);assert(s.AcceptsObject(f.r,cfgconvergence::Weapon,0x700000));
        assert(!s.AcceptsObject(f.r,cfgconvergence::Fight,0x700000));
        f.r.Put(Fixture::Component+cfgconvergence::schemadisk::BlackboardAsset,Fixture::Parent);f.r.reads=0;assert(!s.Current(f.r,f.array));}
    {Fixture f;S s;f.r.Put(std::uintptr_t{0x410030},cfgconvergence::Name{500,0}); // optional SeekWeapon absent
        assert(f.Finish(s,peak)==S::State::Ready&&!(s.Mask()&(1u<<cfgconvergence::SeekWeapon)));}
    {Fixture f;S s;f.r.Put(std::uintptr_t{0x400000},cfgconvergence::Name{999,0});
        assert(f.Finish(s,peak)==S::State::Ready&&!(s.Mask()&(1u<<cfgconvergence::Weapon)));}
    for(unsigned fault=0;fault<7;++fault){Fixture f;S s;
        switch(fault){
        case 0:f.r.Put(std::uintptr_t{0x500010},f.types[2]);break; // Weapon not Object key
        case 1:f.r.Put(std::uintptr_t{0x500020},Fixture::Parent);break; // wrong strong Outer
        case 2:f.r.Put(Fixture::Parent+cfgconvergence::schemadisk::Parent,Fixture::Child);break;
        case 3:f.r.Put(std::uintptr_t{0x410000},f.names[cfgconvergence::Fight]);break; // duplicate requested key
        case 4:f.r.Put(std::uintptr_t{0x400018},cfgconvergence::Name{999,0});break; // required Jason absent
        case 5:f.r.Put(std::uintptr_t{0x500310},f.types[3]);break; // Armed key is Object, not Bool
        case 6:f.r.Put(Fixture::Module+cfgconvergence::schemadisk::BoolKey.constructor,f.types[3]);break;
        }
        assert(f.Finish(s,peak)==S::State::Unavailable);
    }
    {Fixture f;S s;assert(f.Finish(s,peak)==S::State::Ready);
        f.r.Put(std::uintptr_t{0x400008},std::uintptr_t{0x500100});f.r.reads=0;assert(!s.Current(f.r,f.array));}
    {Fixture f;S s;assert(f.Finish(s,peak)==S::State::Ready);
        f.r.Put(std::uintptr_t{0x500008},std::uintptr_t{0}); // unrelated field must not become a new scan
        f.r.cap=10;f.r.reads=0;assert(!s.Current(f.r,f.array)&&f.r.reads==10);}
    std::printf("CFG convergence schema PASS: inherited/optional/typed keys, strong serial0 metadata, changed links/cycles/required failures; peak=%u/1536 reads\n",peak);
}
