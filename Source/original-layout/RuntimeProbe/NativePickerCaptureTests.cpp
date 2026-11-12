#include "NativePickerCapturePolicy.hpp"
#include <cstdio>
#include <map>
struct Reader {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,cap=64;std::uintptr_t change=0;unsigned changeAt=0;
    template<class T> void Set(std::uintptr_t at,const T& value){const auto* input=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(value);++i)bytes[at+i]=input[i];}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t count){
        if(reads==cap)return false;++reads;if(changeAt==reads)bytes[change]^=1;
        auto* result=static_cast<unsigned char*>(out);for(std::size_t i=0;i<count;++i){const auto found=bytes.find(at+i);if(found==bytes.end())return false;result[i]=found->second;}return true;
    }
    template<class T> bool Read(std::uintptr_t at,T& value){T result{};if(!ReadBytes(at,&result,sizeof(result)))return false;value=result;return true;}
};
int main(){using namespace cfgmenu::pickercapture;unsigned failures=0,checks=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    check(Screen(Kind::Counselor,u"Counselor_Menu_C")&&Screen(Kind::Jason,u"Jason_Select_Widget_C"),"exact primary picker screen");
    for(const auto name:{u"OfflinePlayMenuWidget_C",u"OfflineBotsSettingsMenuWidget_C",u"PickOfflineBotsMapMenuWidget_C",u"Counselor_Menu_C_0"})
        check(!Screen(Kind::Jason,name)&&!Screen(Kind::Counselor,name),"unproven screen refuses");
    for(const auto name:{u"Construct",u"OnClick_Accept",u"OfflineBots_AcceptJason",u"GetPreviousMenu",u"OnPlayTransitionOut",u"ExecuteUbergraph_Jason_Select_Widget"})check(Function(name),"relevant primary function");
    for(const auto name:{u"ChatNotification",u"ReceiveTick",u"OnMouseEnter",u"OnParentTransitioningOut"})check(!Function(name),"unrelated function excluded");
    check(SelectionPointer(u"SelectedCharacterClass")&&!SelectionPointer(u"MenuHeaderWidget")&&!SelectionPointer(u"ActionBar")&&!SelectionPointer(u"ChatNotification"),"selection references only");
    for(unsigned fault=0;fault<7;++fault){Reader r;constexpr std::uintptr_t widget=0x100000,indexAt=widget+0x400;
        r.Set(indexAt,std::int32_t{fault==1?-1:fault==2?64:2});if(fault==3){r.changeAt=2;r.change=indexAt;}if(fault==4)r.cap=1;
        std::int32_t index=-1;check(Index(r,widget,fault==5?0x402:0x400,fault==6?0x403:0x500,index)==(fault==0),"typed index extent/range/stability/cap refusal");}
    for(unsigned fault=0;fault<10;++fault){Reader r;constexpr std::uintptr_t at=0x200000,data=0x210000;
        Array original{data,3,64};if(fault==1)original.count=0;if(fault==2)original.count=65;if(fault==3)original.capacity=2;
        if(fault==4)original.capacity=129;if(fault==5)original.data=UINTPTR_MAX-1;r.Set(at,original);
        std::array<unsigned char,40> row{};row[0]=42;r.Set(data+40,row);if(fault==6){r.changeAt=3;r.change=data+40;}
        if(fault==7){r.changeAt=4;r.change=at+8;}if(fault==8)r.cap=2;
        Array observed;std::array<unsigned char,MaximumRowBytes> bytes{};
        check(SelectedRow(r,at,fault==9?513:40,1,observed,bytes)==(fault==0),"selected used row shape/stability/cap refusal");
        if(!fault)check(bytes[0]==42&&observed.capacity==64&&r.reads==4,"only selected used row read; reserved capacity untouched");}
    std::printf("CFG read-only picker owner/index/selected-row policy: %s (%u checks, %u failures). Owned fixtures; no game access.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
