#include "OfflineEntrySettingsObservation.hpp"
#include <cstdio>
#include <map>
#include <vector>
namespace {
void Text(cfgnames::Text& out,std::u16string_view value){out.count=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());}
void Pointer(cfgmenu::capture::DiagnosticFunctionEvidence& f,unsigned at,std::uintptr_t value){std::memcpy(f.bytes.data()+at,&value,8);}
struct Memory {
    std::map<std::uintptr_t,std::vector<unsigned char>> cells;unsigned reads=0;bool change=false;
    template<class T> void Put(std::uintptr_t at,const T& value){auto& bytes=cells[at];bytes.resize(sizeof(T));std::memcpy(bytes.data(),&value,sizeof(T));}
    template<class T> bool Read(std::uintptr_t at,T& out){return ReadBytes(at,&out,sizeof(T));}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t length){++reads;auto it=cells.find(at);if(it==cells.end()||it->second.size()!=length)return false;
        std::memcpy(out,it->second.data(),length);if(change){it->second[0]^=1;change=false;}return true;}
};
}
int main(){using namespace cfgmenu;unsigned failures=0;
    constexpr std::uintptr_t module=0x100000,entry=0x200000,map=0x210000,settings=0x220000,push=0x230000,pending=0x240000;
    for(unsigned fault=0;fault<17;++fault){auto f=std::make_unique<capture::DiagnosticFunctionEvidence>();
        f->function.owner=entry;Text(f->function.label,u"ExecuteUbergraph_OfflinePlayMenuWidget");Text(f->function.kind,u"Function");
        f->flags=0x00800000;f->parameterBytes=4;f->storageBytes=113;f->script.count=1944;f->nativeExecutor=module+startdisk::ScriptExecutor;
        f->bytes[0x3E8]=0x97;f->bytes[0x3E9]=0x87;f->bytes[0x3EA]=0xF5;Pointer(*f,0x3EB,push);
        f->bytes[0x3F3]=0x89;Pointer(*f,0x3F4,map);f->bytes[0x3FC]=0x97;f->bytes[0x3FD]=0x87;
        if(fault==1)++f->function.owner;if(fault==2)f->flags=0;if(fault==3)--f->parameterBytes;if(fault==4)--f->storageBytes;
        if(fault==5)--f->script.count;if(fault==6)++f->nativeExecutor;if(fault==7)f->destructorLink=push;
        if(fault>=8&&fault<=13){constexpr std::array<unsigned,6> offsets{{0x3E8,0x3E9,0x3EA,0x3F3,0x3FC,0x3FD}};f->bytes[offsets[fault-8]]^=1;}
        if(fault==14)Pointer(*f,0x3EB,push+8);if(fault==15)Pointer(*f,0x3F4,0);if(fault==16)Text(f->function.label,u"ExecuteUbergraph_PauseMenuWidget");
        std::uintptr_t out=0;const bool okay=entrysettings::EntryReference(*f,entry,module,push,out);
        if(okay!=(fault==0)||(okay&&out!=map))++failures;
    }
    for(unsigned fault=0;fault<19;++fault){auto f=std::make_unique<capture::DiagnosticFunctionEvidence>();
        f->function.owner=map;Text(f->function.label,u"ExecuteUbergraph_PickOfflineBotsMapMenuWidget");Text(f->function.kind,u"Function");
        f->parameterBytes=4;f->storageBytes=193;f->script.count=1867;f->nativeExecutor=module+startdisk::ScriptExecutor;f->destructorLink=pending+8;
        f->bytes[0x108]=0x0D;f->bytes[0x109]=0xC6;f->bytes[0x10A]=0x76;f->bytes[0x113]=9;Pointer(*f,0x117,pending);
        f->bytes[0x11F]=0xDB;Pointer(*f,0x120,pending);f->bytes[0x128]=0x89;Pointer(*f,0x129,settings);f->bytes[0x131]=0x87;
        if(fault==1)++f->function.owner;if(fault==2)f->flags=1;if(fault==3)--f->parameterBytes;if(fault==4)--f->storageBytes;
        if(fault==5)--f->script.count;if(fault==6)++f->nativeExecutor;if(fault==7)f->destructorLink=0;
        if(fault>=8&&fault<=14){constexpr std::array<unsigned,7> offsets{{0x108,0x109,0x10A,0x113,0x11F,0x128,0x131}};f->bytes[offsets[fault-8]]^=1;}
        if(fault==15)Pointer(*f,0x117,pending+8);if(fault==16)Pointer(*f,0x120,pending+8);if(fault==17)Pointer(*f,0x129,0);
        if(fault==18)f->bytes[0x115]=1;
        std::uintptr_t out=0;const bool okay=entrysettings::SettingsReference(*f,map,module,pending,out);
        if(okay!=(fault==0)||(okay&&out!=settings))++failures;
    }
    for(unsigned fault=0;fault<3;++fault){Memory r;auto f=std::make_unique<capture::DiagnosticFunctionEvidence>();
        f->function.object=entry;f->script={map,4,4};f->bytes[0]=0x89;
        r.Put(entry+scriptdisk::Script,f->script);r.Put(map,std::array<unsigned char,4>{{0x89,0,0,0}});
        if(fault==1)f->bytes[0]=0x88;if(fault==2)r.change=true;
        if(entrysettings::StableScript(r,*f)!=(fault==0))++failures;
    }
    // Exact archived bytes from CFG-Frontend-Menu-21645-31640.txt at0x5D2.
    // These archived addresses are compared as operands only, never read.
    constexpr char resetHex[]=
        "0DC67680233F88080200000900000000DA843D08020000DB00DA843D080200007B"
        "0DC67680233F88080200000900000080DA843D08020000DB80DA843D080200007B"
        "0DC67680233F88080200000900000080D9843D08020000DB80D9843D080200007B";
    static_assert(sizeof(resetHex)==199);
    for(unsigned fault=0;fault<100;++fault){auto f=std::make_unique<capture::DiagnosticFunctionEvidence>();f->script.count=1867;
        const auto nibble=[](char c){return c<='9'?c-'0':c-'A'+10;};
        for(unsigned i=0;i<99;++i)f->bytes[0x5D2+i]=static_cast<unsigned char>(16*nibble(resetHex[i*2])+nibble(resetHex[i*2+1]));
        if(fault)f->bytes[0x5D2+fault-1]^=1;
        std::array<std::uintptr_t,3> properties{};std::uintptr_t local=0;
        const bool okay=entrysettings::ResetReferences(*f,0x2083D84D980ULL,properties,local);
        if(okay!=(fault==0)||(okay&&(properties[0]!=0x2083D84DA00ULL||properties[1]!=0x2083D84DA80ULL||local!=0x208883F2380ULL)))++failures;
    }
    for(unsigned fault=0;fault<2;++fault){Memory r;active::Context context;context.thread=1;
        auto out=std::make_unique<entrysettings::Resources>();out->settings=settings;
        if(entrysettings::Observe(r,context,fault?2:1,!fault?false:true,*out)||r.reads||out->settings)++failures;
    }
    std::printf("Offline Play authored map/settings reference guards: %s (141 cases; no native calls/writes; no gameplay approval).\n",failures?"FAIL":"PASS");return failures?1:0;
}
