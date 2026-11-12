#pragma once
#include <string_view>
#include <cstdint>
#include <string>
namespace cfgmenu::companion {
// The bounded startup watch can emit several full reports. Its previous64KiB
// cap discarded later completion evidence and converted a successful child
// exit into a watcher failure. This is external helper storage, not game work.
inline constexpr std::size_t OutputBudget(bool watch){return watch?4u*1024u*1024u:65536u;}
inline bool AppendOutput(std::string& output,std::string_view bytes,bool watch){
    const auto limit=OutputBudget(watch);
    if(output.size()>limit||bytes.size()>limit-output.size())return false;
    output.append(bytes);return true;
}
// Read-only presentation of the existing report. No game memory or authority.
inline bool Number(std::string_view report,std::string_view key,unsigned& value){
    value=0;
    for(std::size_t start=0;start<report.size();){
        const auto end=report.find('\n',start);const auto line=report.substr(start,end==report.npos?report.size()-start:end-start);
        if(line.size()>key.size()+1&&line.substr(0,key.size())==key&&line[key.size()]=='='){
            const auto text=line.substr(key.size()+1);bool digit=false;
            for(char c:text){if(c<'0'||c>'9')break;digit=true;if(value>(UINT32_MAX-static_cast<unsigned>(c-'0'))/10)return false;
                value=value*10+static_cast<unsigned>(c-'0');}
            return digit;
        }
        if(end==report.npos)break;start=end+1;
    }
    return false;
}
inline std::wstring_view Instruction(unsigned state,unsigned route=0,unsigned callbacks=0,unsigned labels=0,unsigned order=0){
    switch(state){
    case 8:
        if(callbacks<8||labels<6||(order!=4&&order!=9))return L"Preparing the native Offline Play entries. Wait for the counselor entry ready message before selecting Play as a Counselor.";
        if(route==1&&order==9)return L"Native counselor entry ready; original order preserved. Open Offline Bots - Play as a Counselor near the bottom. Menu ordering diagnostics saved.";
        if(route==1)return L"Native counselor entry ready. Open Offline Bots - Play as a Counselor.";
        if(route==3||route==4)return L"Highlight your counselor, then press X to use that counselor and open the map list. A customizes; it does not select for this match.";
        if(route==7)return L"Native map menu ready. Choose Crystal Lake Small to open in-game settings.";
        return L"Waiting for the native counselor menu transition. Do not use the old Back/Sandbox workaround.";
    case 9:return L"Preparing the native map route. Please wait; do not select a map yet.";
    case 10:return L"Map menu ready. Choose Crystal Lake Small to open the in-game settings.";
    case 1:case 2:return L"Building the native counselor settings. Please wait before pressing Start.";
    case 3:return L"Settings ready. Choose Counselors in-game, then press normal Start once. AI spawns automatically.";
    case 4:case 6:return L"Match starting. Waiting for normal counselor control and automatic AI startup.";
    case 7:return L"Starting AI Jason and helper bots. A queued request is not a completed spawn; waiting for confirmation.";
    case 5:return L"MOD SETUP FAILED. Do NOT press Start: that would launch stock Jason mode, not the counselor mod. Close CFG normally; the diagnostic log was saved. Do not enable again in this session.";
    default:return L"Waiting for a stable setup report. Do not enable a second time.";
    }
}
inline bool Finished(unsigned state){return state==5;}
class Lines {
    std::string pending_;
public:
    template<class Sink> bool Feed(std::string_view bytes,Sink&& sink){
        for(char c:bytes){
            if(c=='\n'){sink(std::string_view(pending_));pending_.clear();}
            else{if(pending_.size()==4096)return false;pending_+=c;}
        }
        return true;
    }
};
}
