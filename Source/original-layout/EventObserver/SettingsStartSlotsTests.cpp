#include "NativeSettingsStartSlots.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
struct SlotReader:OwnedByteReader {
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t size){++reads;auto* p=static_cast<unsigned char*>(out);
        for(std::size_t i=0;i<size;++i){const auto found=bytes.find(at+i);if(found==bytes.end())return false;p[i]=found->second;}return true;}
};
template class cfgmenu::startslots::NativeAccess<SlotReader>;
struct SlotHost {
    cfgmenu::startslots::Slots& slots;
    unsigned checks=0,writes=0;std::uintptr_t wrapper=0x900000,graph=0x900000;
    unsigned fault=0;bool owner=true,reentered=false;
    bool ThunksResident(std::uintptr_t,std::uintptr_t){return fault!=1;}
    bool Current(std::uintptr_t w,std::uintptr_t g){++checks;
        if(fault==2&&checks==1)return false;
        if(fault==6&&checks==2)owner=false;
        if(fault==9&&checks==3)owner=false;
        return owner&&wrapper==w&&graph==g;
    }
    cfgmenu::sandboxstart::WriteResult Exchange(cfgmenu::startroute::SlotKind kind,std::uintptr_t old,std::uintptr_t next){
        using W=cfgmenu::sandboxstart::WriteResult;++writes;
        if(fault==3&&writes==1)return W::Refused;
        if(fault==4&&writes==1)return W::Indeterminate;
        if(fault==5&&writes==2)return W::Refused;
        if(fault==7&&writes==2)return W::Indeterminate;
        if(fault==8&&writes==2){graph=0x930000;return W::Refused;}
        if(fault==11&&writes==3)return W::Refused;
        if(fault==12&&writes==4)return W::Indeterminate;
        auto& value=kind==cfgmenu::startroute::SlotKind::Wrapper?wrapper:graph;
        if(value!=old)return W::Refused;value=next;
        if(fault==10&&writes==1){const auto before=writes;reentered=!slots.Install(*this,0x900000,0x910000,0x920000)&&before==writes;}
        return W::Changed;
    }
};
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<13;++fault){startslots::Slots slots;SlotHost host{slots};host.fault=fault;
        const bool okay=slots.Install(host,0x900000,0x910000,0x920000);
        const bool expected=fault==0||fault>=10;
        check(okay==expected,"both scoped slots must publish together");
        if(fault==5)check(host.wrapper==0x900000&&host.graph==0x900000&&host.writes==3&&slots.Current()==startslots::State::Refused,
            "definite second refusal restores only our unchanged first slot");
        if(fault==4||fault==6||fault==7||fault==8||fault==9)check(slots.Current()==startslots::State::Indeterminate,
            "fault/stale owner/third-party replacement never guessed restored");
        if(fault==10)check(host.reentered,"publishing guard rejects recursive install");
        const auto writes=host.writes,checks=host.checks;
        check(!slots.Install(host,0x900000,0x910000,0x920000)&&host.writes==writes&&host.checks==checks,
            "no duplicate install/retry/metadata work");
        if(okay){
            check(slots.Restore(host)==(fault!=11&&fault!=12),"retire only exact own live slots");
            if(fault==11||fault==12)check(slots.Current()==startslots::State::Indeterminate,"retirement partial failure not falsely closed");
            else {const auto oldWrites=host.writes,oldChecks=host.checks;
                check(slots.Restore(host)&&host.writes==oldWrites&&host.checks==oldChecks,"duplicate completed retirement zero work");}
        }
    }
    {startslots::Slots slots;SlotHost host{slots};host.wrapper=0x910000;host.graph=0x920000;
        check(slots.Adopt(host,0x900000,0x910000,0x920000)&&!host.writes,"new settings owner adopts exactly installed slots without republishing");
        check(!slots.Adopt(host,0x900000,0x910000,0x920000),"slot owner cannot be adopted twice");}
    {startslots::Slots slots;SlotHost host{slots};host.wrapper=0x990000;host.graph=0x920000;
        check(!slots.Adopt(host,0x900000,0x910000,0x920000)&&!host.writes,"foreign slot refuses adoption without mutation");}
    {SlotReader reader;startbinding::Binding binding;startslots::NativeAccess<SlotReader> windows(reader,binding,nullptr,false);
        startslots::Slots slots;
        check(!slots.Install(windows,0x900000,0x910000,0x920000)&&reader.reads==0,"unverified Windows adapter performs no writes/pins/reads");}
    std::printf("CFG class-local Start publication/retirement/Windows adapter: %s (%u failures). Owned fixtures, no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
