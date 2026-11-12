#include "ControllerOwnedBotState.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
#include <functional>
namespace pop=cfgmenu::offlinepopulation;
namespace life=cfgprobe::lifetime;
struct ChangingReader:OwnedByteReader {
    std::uintptr_t trigger=0;unsigned occurrence=1,hits=0;std::function<void()> change;
    template<class T> bool Read(std::uintptr_t at,T& value){
        if(at==trigger&&++hits==occurrence&&change)change();
        return OwnedByteReader::Read(at,value);
    }
};
struct StateFixture {
    static constexpr std::uintptr_t Module=0x100000,Controller=0x200000,State=0x210000,
        ControllerClass=0x220000,StateClass=0x230000,Other=0x240000,Registry=0x300000;
    static constexpr std::uintptr_t ControllerSerial=Registry+life::ItemSerialOffset,
        StateEntry=Registry+life::ItemBytes,StateSerial=StateEntry+life::ItemSerialOffset;
    ChangingReader r;cfgmenu::promotion::Context owner{};
    life::SlotObservation controller{Controller,0,17},state{State,1,0};
    StateFixture(){
        owner.module=Module;owner.registry={Registry,2};owner.playerState=Other;
        r.Set(Controller+life::ObjectIndexOffset,std::int32_t{0});
        r.Set(State+life::ObjectIndexOffset,std::int32_t{1});
        r.Set(Registry,Controller);r.Set(StateEntry,State);
        r.Set(ControllerSerial,std::int32_t{17});r.Set(StateSerial,std::int32_t{0});
        r.Set(Module+cfgmenu::populationdisk::CounselorControllerReflection,ControllerClass);
        r.Set(Module+cfgmenu::populationdisk::CounselorControllerClassCache,ControllerClass);
        r.Set(Module+cfgspawn::Classes[cfgspawn::PlayerState].reflection,StateClass);
        r.Set(Module+cfgspawn::Classes[cfgspawn::PlayerState].constructor,StateClass);
        r.Set(Controller+cfgprobe::ObjectClass,ControllerClass);
        r.Set(State+cfgprobe::ObjectClass,StateClass);r.Set(StateClass+cfgprobe::ClassSuper,std::uintptr_t{0});
        r.Set(Other+cfgprobe::ClassSuper,std::uintptr_t{0});
        r.Set(Controller+0x388,State);r.Set(State+0x88,Controller);r.Set(State+0x394,std::uint32_t{4});
    }
    bool Check(){return pop::ControllerOwnedState(r,owner,controller,state);}
    void During(unsigned hit,std::function<void()> action){r.trigger=State+0x394;r.occurrence=hit;r.change=action;}
};
int main(){
    unsigned failures=0,cases=0;
    const auto check=[&](bool ok,const char* name){++cases;if(!ok){++failures;std::printf("FAIL %s\n",name);}};
    using F=StateFixture;
    {F f;check(f.Check()&&f.state.serial==0,"stable unassigned state");check(f.r.reads<100,"bounded reads");}
    {F f;f.state.serial=23;f.r.Set(F::StateSerial,std::int32_t{23});check(f.Check()&&f.state.serial==23,"stable assigned state");}
    {F f;f.r.Set(F::StateSerial,std::int32_t{23});check(f.Check()&&f.state.serial==23,"assignment before first read");
        f.r.Set(F::StateSerial,std::int32_t{24});check(!f.Check()&&f.state.serial==23,"latched positive cannot change");}
    {F f;f.During(2,[&](){f.r.Set(F::StateSerial,std::int32_t{23});});check(f.Check()&&f.state.serial==23,"assignment between reads");}
    for(unsigned fault=0;fault<14;++fault){F f;
        switch(fault){
        case 0:f.controller.serial=0;break;
        case 1:f.r.Set(F::ControllerSerial,std::int32_t{18});break;
        case 2:f.state.serial=-1;break;
        case 3:f.r.Set(F::StateSerial,std::int32_t{-1});break;
        case 4:f.state.index=0;break;
        case 5:f.r.Set(F::StateEntry,F::Other);break;
        case 6:f.r.Set(F::Controller+0x388,F::Other);break;
        case 7:f.r.Set(F::State+0x88,F::Other);break;
        case 8:f.r.Set(F::State+0x394,std::uint32_t{0});break;
        case 9:f.r.Set(F::State+cfgprobe::ObjectClass,F::Other);break;
        case 10:f.r.Set(F::Module+cfgmenu::populationdisk::CounselorControllerClassCache,F::Other);break;
        case 11:f.r.Set(F::Module+cfgspawn::Classes[cfgspawn::PlayerState].constructor,F::Other);break;
        case 12:f.owner.playerState=F::State;break;
        case 13:f.state.object=F::Controller;break;
        }
        const auto original=f.state;check(!f.Check()&&f.state==original,"invalid evidence rejected without publication");
    }
    for(unsigned fault=0;fault<10;++fault){F f;
        if(fault==0||fault==1)f.r.Set(F::StateSerial,std::int32_t{23});
        f.During(1,[&](){switch(fault){
        case 0:f.r.Set(F::StateSerial,std::int32_t{0});break;
        case 1:f.r.Set(F::StateSerial,std::int32_t{24});break;
        case 2:f.r.Set(F::State+life::ObjectIndexOffset,std::int32_t{0});break;
        case 3:f.r.Set(F::StateEntry,F::Other);break;
        case 4:f.r.Set(F::Controller+0x388,F::Other);break;
        case 5:f.r.Set(F::State+0x88,F::Other);break;
        case 6:f.r.Set(F::ControllerSerial,std::int32_t{18});break;
        case 7:f.r.Set(F::State+cfgprobe::ObjectClass,F::Other);break;
        case 8:f.r.Set(F::Controller+cfgprobe::ObjectClass,F::Other);break;
        case 9:f.r.Set(F::State+0x394,std::uint32_t{0});break;
        }});
        const auto original=f.state;check(!f.Check()&&f.state==original,"mutation between reads rejected without publication");
    }
    {F f;f.During(2,[&](){f.r.Set(F::StateSerial,std::int32_t{23});f.r.Set(F::ControllerSerial,std::int32_t{18});});
        check(!f.Check()&&f.state.serial==0,"failed final anchor does not publish assigned serial");}
    std::printf("CFG controller-owned newborn state: %s (%u cases, %u failures). Owned memory only.\n",failures?"FAIL":"PASS",cases,failures);
    return failures?1:0;
}
