#include "KillDamageGate.hpp"
#include "KillNativeResource.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
using cfgkill::DamageGate;
using State=DamageGate::State;
struct Reader {
    std::map<std::uintptr_t,unsigned char> memory;
    unsigned reads=0;
    template<class T> void Put(std::uintptr_t p,const T& value){
        const auto* bytes=reinterpret_cast<const unsigned char*>(&value);
        for(unsigned i=0;i<sizeof(T);++i)memory[p+i]=bytes[i];
    }
    template<class T> bool Read(std::uintptr_t p,T& value){
        ++reads;auto* bytes=reinterpret_cast<unsigned char*>(&value);
        for(unsigned i=0;i<sizeof(T);++i){const auto it=memory.find(p+i);if(it==memory.end())return false;bytes[i]=it->second;}
        return true;
    }
};
struct Fixture {
    Reader reader;cfgprobe::lifetime::ArrayHeader array{0x100000,8192};
    static constexpr std::uintptr_t Root=0x200000,Actor=0x201000,Base=0x210000,Override=0x211000;
    static constexpr std::uintptr_t FunctionClass=0x300000,ClassClass=0x301000,OtherClass=0x302000;
    std::int32_t nextIndex=0;
    void Object(std::uintptr_t p,std::uintptr_t type){
        reader.Put(p+0x0C,nextIndex);reader.Put(p+0x10,type);
        reader.Put(array.data+24*static_cast<std::uintptr_t>(nextIndex),p);
        reader.Put(array.data+24*static_cast<std::uintptr_t>(nextIndex)+8,std::uint32_t{0});
        reader.Put(array.data+24*static_cast<std::uintptr_t>(nextIndex)+16,std::int32_t{3});++nextIndex;
    }
    void Owner(std::uintptr_t p,std::uintptr_t parent,std::uintptr_t children){
        Object(p,ClassClass);reader.Put(p+0x30,parent);reader.Put(p+0x38,children);
    }
    void Field(std::uintptr_t p,std::uintptr_t owner,std::uintptr_t next,std::int32_t name,std::uintptr_t type=FunctionClass){
        Object(p,type);reader.Put(p+0x18,cfgnames::Words{name,0});reader.Put(p+0x20,owner);reader.Put(p+0x28,next);
    }
    void Function(std::uintptr_t p,std::uintptr_t owner,std::uintptr_t parent){
        Field(p,owner,0,42);reader.Put(p+0x30,parent);reader.Put(p+0x40,std::uint32_t{32});
        reader.Put(p+0x88,cfgkill::disk::ReceiveAnyDamageFlags);reader.Put(p+0x8E,std::uint16_t{32});
    }
    Fixture(){Owner(Root,Actor,Override);Owner(Actor,0,Base);Function(Base,Actor,0);Function(Override,Root,Base);}
    bool Begin(DamageGate& gate){return gate.Begin(reader,array,Root,Base,Actor);}
    State Finish(DamageGate& gate){
        for(unsigned i=0;i<600&&gate.Status()!=State::Ready&&gate.Status()!=State::Unavailable;++i){
            reader.reads=0;gate.Advance(reader,array);assert(reader.reads<=16*12);
        }return gate.Status();
    }
    void Serial(std::uintptr_t object,std::int32_t serial){
        std::int32_t index=0;assert(reader.Read(object+0xC,index));reader.Put(array.data+24*static_cast<std::uintptr_t>(index)+16,serial);
    }
};
int main(){
    for(unsigned test=0;test<4;++test){Fixture f;
        const auto module=std::uintptr_t{0x140000000},level=std::uintptr_t{0x700000},native=std::uintptr_t{0x710000},world=std::uintptr_t{0x720000};
        f.Object(level,native);f.Object(native,Fixture::ClassClass);f.reader.Put(native+0x30,std::uintptr_t{0});
        f.reader.Put(Fixture::Override+0x20,level);f.reader.Put(level+cfgkill::disk::OwningWorld,world);
        f.reader.Put(module+cfgkill::disk::LevelReflection,native);f.reader.Put(module+cfgkill::disk::LevelConstructor,native);
        cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t type=0;
        f.reader.reads=0;assert(cfgkill::ActorWorldCurrent(f.reader,f.array,module,Fixture::Override,world,slot,type));
        assert(slot.object==level&&type==native&&f.reader.reads==18);
        switch(test){
        case 0:f.reader.Put(level+cfgkill::disk::OwningWorld,std::uintptr_t{0x730000});break;
        case 1:f.reader.Put(Fixture::Override+0x20,Fixture::Actor);break;
        case 2:f.reader.Put(module+cfgkill::disk::LevelConstructor,Fixture::Actor);break;
        case 3:f.reader.Put(level+0x10,Fixture::OtherClass);break;
        }
        assert(!cfgkill::ActorWorldCurrent(f.reader,f.array,module,Fixture::Override,world,slot,type));
    }
    // The actual warm native-resource gate rejects owner/frame/slot/cache
    // changes without ever resolving a function or inspecting names again.
    for(unsigned test=0;test<7;++test){Fixture f;const auto module=std::uintptr_t{0x140000000};
        const auto& binding=cfgkill::disk::EnterKillStance;
        cfgprobe::lifetime::SlotObservation function{},owner{};
        assert(cfgspawn::ObjectSlot(f.reader,f.array,Fixture::Base,function));
        assert(cfgspawn::ObjectSlot(f.reader,f.array,Fixture::Actor,owner));
        f.reader.Put(Fixture::Base+0x8E,std::uint16_t{0});
        f.reader.Put(module+binding.ownerReflection,Fixture::Actor);f.reader.Put(module+binding.ownerConstructor,Fixture::Actor);
        f.reader.Put(module+binding.cache,Fixture::Base);f.reader.reads=0;
        assert(cfgkill::ResourceCurrent(f.reader,f.array,module,binding,Fixture::Base,Fixture::Actor,function,owner));assert(f.reader.reads==13);
        switch(test){
        case 0:f.reader.Put(Fixture::Base+0x8E,std::uint16_t{8});break;
        case 1:f.reader.Put(Fixture::Base+0x20,Fixture::Root);break;
        case 2:f.Serial(Fixture::Base,4);break;
        case 3:f.Serial(Fixture::Actor,4);break;
        case 4:f.reader.Put(module+binding.ownerReflection,Fixture::Root);break;
        case 5:f.reader.Put(module+binding.ownerConstructor,Fixture::Root);break;
        case 6:f.reader.Put(module+binding.cache,Fixture::Override);break;
        }
        f.reader.reads=0;assert(!cfgkill::ResourceCurrent(f.reader,f.array,module,binding,Fixture::Base,Fixture::Actor,function,owner));assert(f.reader.reads<=13);
    }
    // A host prerequisite can fail before Begin. Refusal is terminal and all
    // later advances/mismatched event validations must be read-free.
    {Fixture f;DamageGate g;assert(g.Refuse()==State::Unavailable);f.reader.reads=0;
        for(unsigned i=0;i<100;++i){assert(g.Advance(f.reader,f.array)==State::Unavailable);
            assert(!g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));}
        assert(f.reader.reads==0);g.Reset();assert(g.Status()==State::Empty);assert(f.Begin(g));assert(f.Finish(g)==State::Ready);
        assert(g.Refuse()==State::Unavailable&&!g.FunctionToken());}
    {Fixture f;DamageGate g;assert(f.Begin(g));assert(!g.Matches(Fixture::Override));assert(f.Finish(g)==State::Ready);
        assert(g.FunctionToken()==Fixture::Override&&g.ClassToken()==Fixture::Root);
        f.reader.reads=0;for(unsigned i=0;i<10000;++i){assert(!g.Matches(0x999000));assert(!g.Validate(f.reader,f.array,Fixture::Root,0x999000));}
        assert(f.reader.reads==0);assert(g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));assert(f.reader.reads<50);
        ++f.array.count;assert(g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));}
    {Fixture f;DamageGate g;f.reader.Put(Fixture::Root+0x38,std::uintptr_t{0});assert(f.Begin(g));assert(f.Finish(g)==State::Ready);assert(g.Matches(Fixture::Base));}
    // Unassigned weak serials on strongly linked native/Blueprint METADATA are
    // normal UE4 state. They must not disable native damage/stance observation.
    {Fixture f;DamageGate g;for(const auto p:{Fixture::Root,Fixture::Actor,Fixture::Base,Fixture::Override})f.Serial(p,0);
        assert(f.Begin(g)&&f.Finish(g)==State::Ready);assert(g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));
        f.reader.Put(Fixture::Override+0x20,Fixture::Actor);assert(!g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));}
    {Fixture f;const auto module=std::uintptr_t{0x140000000};const auto& binding=cfgkill::disk::EnterKillStance;
        f.Serial(Fixture::Base,0);f.Serial(Fixture::Actor,0);
        cfgprobe::lifetime::SlotObservation function{},owner{};
        assert(cfgspawn::ObjectSlot(f.reader,f.array,Fixture::Base,function)&&cfgspawn::ObjectSlot(f.reader,f.array,Fixture::Actor,owner));
        f.reader.Put(Fixture::Base+0x8E,std::uint16_t{0});f.reader.Put(module+binding.ownerReflection,Fixture::Actor);
        f.reader.Put(module+binding.ownerConstructor,Fixture::Actor);f.reader.Put(module+binding.cache,Fixture::Base);
        assert(cfgkill::ResourceCurrent(f.reader,f.array,module,binding,Fixture::Base,Fixture::Actor,function,owner));
        f.reader.Put(module+binding.cache,Fixture::Override);
        assert(!cfgkill::ResourceCurrent(f.reader,f.array,module,binding,Fixture::Base,Fixture::Actor,function,owner));}
    for(unsigned test=0;test<9;++test){Fixture f;DamageGate g;
        switch(test){
        case 0:f.reader.Put(Fixture::Base+0x88,std::uint32_t{0});break;
        case 1:f.reader.Put(Fixture::Base+0x20,Fixture::Root);break;
        case 2:f.reader.Put(Fixture::Base+0x8E,std::uint16_t{31});break;
        case 3:f.reader.Put(Fixture::Base+0x30,Fixture::Override);break;
        case 4:f.Serial(Fixture::Base,-1);break;
        case 5:f.reader.Put(Fixture::Override+0x8E,std::uint16_t{40});break;
        case 6:f.reader.Put(Fixture::Override+0x30,Fixture::Override);break;
        case 7:f.reader.Put(Fixture::Override+0x10,Fixture::OtherClass);break;
        case 8:f.reader.Put(Fixture::Override+0x88,std::uint32_t{0x08020844});break;
        }
        if(f.Begin(g))assert(f.Finish(g)==State::Unavailable);assert(!g.FunctionToken());
    }
    // Duplicate FName, field/class cycles, field wrong Outer, absent Actor anchor.
    for(unsigned test=0;test<5;++test){Fixture f;DamageGate g;
        switch(test){
        case 0:f.Function(0x212000,Fixture::Root,Fixture::Base);f.reader.Put(Fixture::Override+0x28,std::uintptr_t{0x212000});break;
        case 1:f.reader.Put(Fixture::Override+0x28,Fixture::Override);break;
        case 2:f.reader.Put(Fixture::Root+0x30,Fixture::Root);break;
        case 3:f.reader.Put(Fixture::Override+0x20,Fixture::Actor);break;
        case 4:f.reader.Put(Fixture::Actor+0x38,std::uintptr_t{0});break;
        }
        assert(f.Begin(g));assert(f.Finish(g)==State::Unavailable);
    }
    // Two-pass mutation detection: the first observation cannot publish a gate.
    {Fixture f;DamageGate g;assert(f.Begin(g));while(g.Status()==State::Scan)g.Advance(f.reader,f.array);
        f.reader.Put(Fixture::Override+0x28,std::uintptr_t{0x212000});assert(f.Finish(g)==State::Unavailable);}
    // Hot metadata and lifetime changes revoke the gate; stale ptr then has no reads.
    for(unsigned test=0;test<7;++test){Fixture f;DamageGate g;assert(f.Begin(g));assert(f.Finish(g)==State::Ready);
        switch(test){
        case 0:f.Serial(Fixture::Override,4);break;
        case 1:f.reader.Put(Fixture::Override+0x88,std::uint32_t{0x08020805});break;
        case 2:f.reader.Put(Fixture::Override+0x30,std::uintptr_t{0});break;
        case 3:f.reader.Put(Fixture::Root+0x38,std::uintptr_t{0});break;
        case 4:f.Serial(Fixture::Root,4);break;
        case 5:f.reader.Put(Fixture::Override+0x18,cfgnames::Words{43,0});break;
        case 6:f.array.data+=8;break;
        }
        assert(!g.Validate(f.reader,f.array,Fixture::Root,Fixture::Override));assert(!g.Matches(Fixture::Override));
    }
    {Fixture f;DamageGate g;assert(f.Begin(g));assert(f.Finish(g)==State::Ready);
        f.reader.reads=0;assert(!g.Validate(f.reader,f.array,Fixture::Actor,Fixture::Override));assert(f.reader.reads==0);}
    // 16 fields per phase despite a realistic long class child list.
    {Fixture f;DamageGate g;constexpr unsigned count=1536;
        for(unsigned i=0;i<count;++i)f.Field(0x400000+i*256,Fixture::Root,i+1<count?0x400000+(i+1)*256:Fixture::Override,100+static_cast<std::int32_t>(i),Fixture::OtherClass);
        f.reader.Put(Fixture::Root+0x38,std::uintptr_t{0x400000});assert(f.Begin(g));assert(f.Finish(g)==State::Ready);}
    // Global field cap never follows the 4097th field.
    {Fixture f;DamageGate g;constexpr unsigned count=4097;
        for(unsigned i=0;i<count;++i)f.Field(0x400000+i*256,Fixture::Root,i+1<count?0x400000+(i+1)*256:Fixture::Override,100+static_cast<std::int32_t>(i),Fixture::OtherClass);
        f.reader.Put(Fixture::Root+0x38,std::uintptr_t{0x400000});assert(f.Begin(g));assert(f.Finish(g)==State::Unavailable);}
    // Class ancestry cap, including otherwise well-formed known-object slots.
    {Fixture f;DamageGate g;
        for(unsigned i=0;i<33;++i)f.Owner(0x600000+i*256,i+1<33?0x600000+(i+1)*256:Fixture::Actor,0);
        f.reader.Put(Fixture::Root+0x30,std::uintptr_t{0x600000});assert(f.Begin(g));assert(f.Finish(g)==State::Unavailable);}
    std::puts("CFG cold damage function gate: PASS (zero-read unrelated events; bounded two-pass discovery)");
}
