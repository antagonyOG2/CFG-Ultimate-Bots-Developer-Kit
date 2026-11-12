#include "MenuPanelChildrenObservation.hpp"
#include <map>
#include <vector>
#include <cstring>
#include <cstdio>
struct Reader {
    std::map<std::uintptr_t,std::vector<unsigned char>> cells;unsigned reads=0;std::uintptr_t changeAt=0;
    template<class T> void Set(std::uintptr_t at,const T& value){auto& bytes=cells[at];bytes.resize(sizeof(T));std::memcpy(bytes.data(),&value,sizeof(T));}
    template<class T> bool Read(std::uintptr_t at,T& value){++reads;auto it=cells.find(at);if(it==cells.end()||it->second.size()!=sizeof(T))return false;std::memcpy(&value,it->second.data(),sizeof(T));
        if(at==changeAt){cells.erase(at);changeAt=0;}return true;}
};
int main(){using namespace cfgmenu::panelcapture;unsigned failures=0;
    for(unsigned fault=0;fault<13;++fault){Reader r;constexpr std::uintptr_t panel=0x100000,slots=0x110000,slot=0x120000,widget=0x130000,type=0x140000;
        r.Set(panel+0x100,Array{slots,2,1000000});
        for(unsigned i=0;i<2;++i){r.Set(slots+8*i,slot+0x100*i);r.Set(slot+0x100*i+0x30,panel);r.Set(slot+0x100*i+0x38,widget+0x100*i);r.Set(widget+0x100*i+cfgprobe::ObjectClass,type);}
        if(fault==1)r.Set(panel+0x100,Array{slots,65,65});
        if(fault==2)r.Set(panel+0x100,Array{slots,2,1});
        if(fault==3)r.Set(slot+0x30,std::uintptr_t{panel+8});
        if(fault==4)r.Set(slots+8,slot);
        if(fault==5)r.Set(slot+0x100+0x38,widget);
        if(fault==6)r.Set(widget+cfgprobe::ObjectClass,std::uintptr_t{0});
        if(fault==7)r.changeAt=slots+8;
        if(fault==8)r.changeAt=panel+0x100;
        if(fault==9)r.Set(panel+0x100,Array{slots,-1,2});
        if(fault==10)r.Set(panel+0x100,Array{slots,0,2});
        Snapshot out;bool okay=Observe(r,panel,0x100,fault==11?0x31:0x30,fault==12?0x30:0x38,out);
        if(okay!=(fault==0))++failures;
        if(fault==0&&(r.reads!=19||out.slots.capacity!=1000000||out.children[1].widget!=widget+0x100))++failures;
        if(fault!=0&&out.slots.count)++failures;
    }
    std::printf("Bounded current-panel child observation: %s (13 cases; spare capacity ignored; zero calls/writes).\n",failures?"FAIL":"PASS");return failures?1:0;
}
