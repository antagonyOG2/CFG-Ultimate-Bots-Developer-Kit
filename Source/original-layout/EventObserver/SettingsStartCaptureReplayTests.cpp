#include "../RuntimeProbe/SettingsStartObservation.hpp"
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <stdexcept>

// Parse the real read-only capture, independently of the synthetic fixtures.
// Only Validate is exercised: captured addresses are NEVER dereferenced. The
// source report lacks script allocation/slot headers, so inert context/script
// addresses below satisfy structural preconditions, NOT runtime call admission.
namespace {
void Text(cfgnames::Text& out,const std::string& value){
    if(value.size()>=out.data.size())throw std::runtime_error("oversized label");
    out={};out.count=out.baseCount=static_cast<unsigned>(value.size());
    for(unsigned i=0;i<out.count;++i)out.data[i]=static_cast<char16_t>(value[i]);
}
struct Replay {
    std::array<cfgmenu::capture::DiagnosticFunctionEvidence,3> functions{};
    std::array<cfgmenu::startbinding::FastCall,3> fast{};
    std::array<unsigned,3> bytes{},fields{};
    cfgmenu::active::Context context{};
    explicit Replay(const char* path){
        std::ifstream input(path);if(!input)throw std::runtime_error("missing real source capture");
        const std::regex rolePattern(R"(^SettingsSourceRole=([0-2]) Name=(\S+) Stage=0 FunctionStage=none Fields=(\d+) ScriptBytes=(\d+) FastRead=yes FastGraph=0x([0-9A-Fa-f]+) FastEntry=(\d+)\s*$)");
        const std::regex functionPattern(R"(^EntrySourceFunction=(\S+) Object=0x([0-9A-Fa-f]+) Owner=0x([0-9A-Fa-f]+) Flags=0x([0-9A-Fa-f]+) Frame=(\d+) Storage=(\d+) ScriptBytes=(\d+) Executor=0x([0-9A-Fa-f]+) Cleanup=0x([0-9A-Fa-f]+)\s*$)");
        const std::regex fieldPattern(R"(^EntrySourceField=0x([0-9A-Fa-f]+) Name=(\S+) Kind=(\S+) Offset=(\d+) Bytes=(\d+) Dimension=(\d+) Flags=0x([0-9A-Fa-f]+)\s*$)");
        const std::regex scriptPattern(R"(^EntrySourceScript\+0x([0-9A-Fa-f]+)=([0-9A-Fa-f]+)\s*$)");
        int role=-1;unsigned roles=0,headers=0;bool complete=false;std::string line;std::smatch match;
        while(std::getline(input,line)){
            if(line.find("SettingsSourceSnapshot=CAPTURED Completed=3/3 Stage=0 MetadataMask=7 FastMask=7")==0)complete=true;
            if(std::regex_match(line,match,rolePattern)){
                role=std::stoi(match[1]);if(roles&(1u<<role))throw std::runtime_error("duplicate role");roles|=1u<<role;
                fields[role]=static_cast<unsigned>(std::stoul(match[3]));
                fast[role]={std::stoull(match[5],nullptr,16),std::stoi(match[6])};
            }else if(std::regex_match(line,match,functionPattern)){
                if(role<0||(headers&(1u<<role)))throw std::runtime_error("bad function order");headers|=1u<<role;
                auto& f=functions[role];Text(f.function.label,match[1]);Text(f.function.kind,"Function");
                f.function.object=std::stoull(match[2],nullptr,16);f.function.owner=std::stoull(match[3],nullptr,16);
                f.flags=static_cast<std::uint32_t>(std::stoul(match[4],nullptr,16));
                f.parameterBytes=static_cast<std::uint16_t>(std::stoul(match[5]));f.storageBytes=std::stoi(match[6]);
                f.script={0x1000000u+0x10000u*static_cast<unsigned>(role),std::stoi(match[7]),std::stoi(match[7])};
                f.nativeExecutor=std::stoull(match[8],nullptr,16);f.destructorLink=std::stoull(match[9],nullptr,16);
            }else if(std::regex_match(line,match,fieldPattern)){
                if(role<0)throw std::runtime_error("field without role");auto& f=functions[role];
                if(f.fieldsCount==f.fields.size())throw std::runtime_error("too many fields");auto& p=f.fields[f.fieldsCount++];
                p.field.object=std::stoull(match[1],nullptr,16);p.field.owner=f.function.object;
                Text(p.field.label,match[2]);Text(p.field.kind,match[3]);p.offset=std::stoi(match[4]);
                p.elementBytes=std::stoi(match[5]);p.dimension=std::stoi(match[6]);p.flags=std::stoull(match[7],nullptr,16);
            }else if(std::regex_match(line,match,scriptPattern)){
                if(role<0)throw std::runtime_error("script without role");auto& f=functions[role];const std::string hex=match[2];
                if(std::stoul(match[1],nullptr,16)!=bytes[role]||hex.size()%2||bytes[role]+hex.size()/2>f.bytes.size())throw std::runtime_error("incomplete script");
                for(std::size_t i=0;i<hex.size();i+=2)f.bytes[bytes[role]++]=static_cast<unsigned char>(std::stoul(hex.substr(i,2),nullptr,16));
            }
        }
        if(!complete||roles!=7||headers!=7)throw std::runtime_error("capture did not complete");
        for(unsigned i=0;i<3;++i)if(bytes[i]!=static_cast<unsigned>(functions[i].script.count)||fields[i]!=functions[i].fieldsCount)throw std::runtime_error("truncated evidence");
        context.thread=42;context.widget=0x200000;context.type=functions[0].function.owner;
        context.identities[2]={context.widget,1,123};context.module=functions[0].nativeExecutor-cfgmenu::startdisk::ScriptExecutor;
    }
    bool Validate(cfgmenu::startbinding::Binding& out)const{
        return cfgmenu::startbinding::Validate(context,functions[0],functions[1],functions[2],fast[0],fast[1],out,fast[2]);
    }
};
}
int main(int argc,char** argv){unsigned failures=0;const auto check=[&](bool ok,const char* label){if(!ok){++failures;std::printf("FAIL %s\n",label);}};
    try{
        auto source=std::make_unique<Replay>(argc>1?argv[1]:"Diagnostics/CFG35-SettingsSource-20261023-222453.txt");
        cfgmenu::startbinding::Binding binding;check(source->Validate(binding),"real 3.5 wrappers, 89 locals and complete 10300-byte graph");
        check(source->functions[2].fieldsCount==89&&binding.graphBytes.size()==10300&&binding.graph.storage==969,"independent captured shape");
        check(!binding.start.fast.graph&&!binding.construct.fast.graph,"actual null fast aliases preserved");
        check(std::equal(binding.graphBytes.begin(),binding.graphBytes.end(),source->functions[2].bytes.begin()),"full graph copied, including added tail");
        for(unsigned fault=0;fault<9;++fault){auto changed=std::make_unique<Replay>(*source);auto& graph=changed->functions[2];
            switch(fault){
            case 0:graph.storageBytes=937;break;
            case 1:graph.script.count=9497;break;
            case 2:{const std::int32_t old=0x2506;std::memcpy(changed->functions[0].bytes.data()+14,&old,4);break;}
            case 3:{const std::int32_t old=0x2511;std::memcpy(changed->functions[1].bytes.data()+14,&old,4);break;}
            case 4:{const std::uint32_t old=0x23C9;std::memcpy(graph.bytes.data()+0x282A,&old,4);break;}
            case 5:{const std::uint32_t old=0x22A0;std::memcpy(graph.bytes.data()+0x2835,&old,4);break;}
            case 6:graph.fields[graph.fieldsCount-1].offset=969;break;
            case 7:graph.fields[0].flags^=1;break;
            case 8:changed->fast[0]={graph.function.object,0x2506};break;
            }
            cfgmenu::startbinding::Binding out;check(!changed->Validate(out)&&!out.context.widget,"old 3.4 or altered evidence fails closed");
        }
    }catch(const std::exception& e){std::printf("FAIL capture replay: %s\n",e.what());++failures;}
    std::printf("Actual CFG3.5 settings capture replay: %u failures; no process access/native calls.\n",failures);return failures?1:0;
}
