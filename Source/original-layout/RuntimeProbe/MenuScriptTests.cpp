#include "MenuScriptObservation.hpp"
#include "MenuClassCache.hpp"
#include <map>
#include <cstdio>
struct Reader {
    std::map<std::uintptr_t,unsigned char> memory;unsigned reads=0;bool mutate=false;
    template<class T>void Set(std::uintptr_t address,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)memory[address+i]=p[i];}
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t size){++reads;auto* p=static_cast<unsigned char*>(output);
        if(mutate&&reads==3)memory[0x200000]^=1;
        for(std::size_t i=0;i<size;++i){auto it=memory.find(address+i);if(it==memory.end())return false;p[i]=it->second;}return true;}
    template<class T>bool Read(std::uintptr_t address,T& out){T value{};if(!ReadBytes(address,&value,sizeof(value)))return false;out=value;return true;}
};
int main(){using namespace cfgmenu::capture;unsigned failures=0;auto check=[&](bool b){if(!b)++failures;};
    for(unsigned fault=0;fault<7;++fault){Reader r;ScriptHeader initial{0x200000,4,4},header;std::array<unsigned char,cfgmenu::scriptdisk::MaxBytes> out;
        if(fault==1)initial.count=-1;if(fault==2)initial.capacity=3;if(fault==3)initial.count=20000;
        if(fault==4)initial.data=~std::uintptr_t{0};if(fault==5)r.mutate=true;
        if(fault==6)initial={0,0,0};r.Set(0x100000+cfgmenu::scriptdisk::Script,initial);r.Set(0x200000,std::array<unsigned char,4>{1,2,3,4});
        const bool ok=Script(r,0x100000,header,out);check(ok==(fault==0||fault==6));if(!ok)check(header.count==0&&out[0]==0);
        if(fault==0)check(header==initial&&out[3]==4);check(r.reads<=4);
    }
    {Reader r;ScriptHeader initial{0x200000,20000,65536},header;
        std::array<unsigned char,20000> data{};data.back()=73;
        r.Set(0x100000+cfgmenu::scriptdisk::Script,initial);r.Set(0x200000,data);
        std::array<unsigned char,cfgmenu::scriptdisk::MaxBytes> small{};
        check(!Script(r,0x100000,header,small)&&header.count==0);
        auto large=std::make_unique<std::array<unsigned char,65536>>();r.reads=0;
        check(Script(r,0x100000,header,*large)&&header==initial&&(*large)[19999]==73&&r.reads==4);
        r.Set(0x100000+cfgmenu::scriptdisk::Script,ScriptHeader{0x200000,65537,65537});
        check(!Script(r,0x100000,header,*large)&&header.count==0&&(*large)[19999]==0);
        r.Set(0x100000+cfgmenu::scriptdisk::Script,initial);r.reads=0;r.mutate=true;
        check(!Script(r,0x100000,header,*large)&&header.count==0);
    }
    MenuClassCache classes;bool menu=true;check(!classes.Lookup(0,menu));classes.Store(0,true);check(!classes.Lookup(0,menu));
    for(unsigned i=1;i<=12000;++i){const auto key=std::uintptr_t{0x100000}+static_cast<std::uintptr_t>(i)*16;
        classes.Store(key,i%3==0);menu=false;check(classes.Lookup(key,menu)&&menu==(i%3==0));}
    check(classes.evictions>0&&classes.hits==12000);
    MenuClassCache collision;constexpr std::uintptr_t first=0x100000,second=0x110010;
    collision.Store(first,true);collision.Store(second,false);menu=true;
    check(collision.evictions==1&&!collision.Lookup(first,menu)&&collision.Lookup(second,menu)&&!menu);
    std::printf("MenuScriptTests: %u failures; bounded script snapshots and class-cache eviction, no game access\n",failures);return failures?1:0;
}
