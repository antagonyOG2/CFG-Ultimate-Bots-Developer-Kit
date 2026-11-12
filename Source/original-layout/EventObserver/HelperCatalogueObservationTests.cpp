#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "HelperCatalogueObservation.hpp"
struct Host {Fixture& r;unsigned calls=0;bool valid=true,finalValid=true;
    bool HelperContext(){++calls;for(unsigned i=0;i<140;++i){std::uint32_t value=0;if(!r.Read(0xF00000,value))return false;}
        return valid&&(calls!=2||finalValid);}};
int main(){
    for(unsigned fault=0;fault<10;++fault){
        Fixture f;f.cap=1536;f.Put(0xF00000,std::uint32_t{});const auto data=std::uintptr_t{0xF10000};
        cfgspawn::PlayerArray header{data,18,22};const auto field=f.mode+cfgmenu::cataloguedisk::CounselorCharacterClasses;
        f.Put(field,header);for(unsigned i=0;i<18;++i){cfgprimary::SoftClassBytes p{};p[0]=static_cast<unsigned char>(i+1);f.Put(data+40ULL*i,p);}
        Host host{f};cfgspawn::PlayerArray out{0xFA0000,1,1};std::array<cfgprimary::SoftClassBytes,64> entries{};entries[0][0]=99;
        const auto original=entries;cfghelpercatalogue::Diagnostic diag;
        if(fault==1)host.valid=false;
        if(fault==2){header.count=0;f.Put(field,header);}
        if(fault==3){header.count=65;header.capacity=65;f.Put(field,header);}
        if(fault==4){header.capacity=1;f.Put(field,header);}
        if(fault==5)f.memory.erase(data+40*17);
        if(fault==6){f.trigger=field;f.triggerAt=2;f.mutation=field;f.newPointer=data+0x10000;}
        if(fault==7){f.trigger=data;f.triggerAt=2;f.mutation=data;f.newPointer=55;}
        if(fault==8)host.finalValid=false;
        if(fault==9)f.cap=150;
        f.reads=0;const auto okay=cfghelpercatalogue::Capture(f,host,f.mode,out,entries,diag);
        CHECK(okay==(fault==0));
        if(!fault){CHECK(out.data==data&&out.count==18&&out.capacity==22&&entries[17][0]==18&&f.reads<1536);
            std::printf("Actual catalogue capture18entries:%u/1536 reads with140-read owner fixtures,no native calls.\n",f.reads);}
        else CHECK(out.data==0xFA0000&&entries==original&&diag.stage!=cfghelpercatalogue::Stage::Ready);
    }
    std::printf("Helper catalogue transactional observation:%d failures; empty/shape/change/budget/owner refusal,no partial commit.\n",failures);
    return failures?1:0;
}
