#include "NativePickerStartAdmission.hpp"
#include <cstdio>
int main(){using namespace cfgmenu;using namespace pickerstart;unsigned checks=0,failures=0;
    const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    check(Admit(launch::State::Opening,counselorroute::State::SettingsReady,true)==Admission::Waiting);
    check(Admit(launch::State::Ready,counselorroute::State::SettingsReady,true)==Admission::Submit);
    check(Admit(launch::State::Ready,counselorroute::State::Submitted,true)==Admission::AlreadySubmitted);
    check(Admit(launch::State::Submitted,counselorroute::State::Submitted,true)==Admission::AlreadySubmitted);
    for(unsigned l=0;l<=static_cast<unsigned>(launch::State::Indeterminate);++l)
        for(unsigned r=0;r<=static_cast<unsigned>(counselorroute::State::Retired);++r){
            const auto state=static_cast<launch::State>(l);const auto route=static_cast<counselorroute::State>(r);
            check(Admit(state,route,false)==Admission::Refused);
            const bool permitted=(route==counselorroute::State::SettingsReady&&(state==launch::State::Opening||state==launch::State::Ready))||
                (route==counselorroute::State::Submitted&&(state==launch::State::Ready||state==launch::State::Submitted));
            check((Admit(state,route,true)!=Admission::Refused)==permitted);}
    check(Admit(static_cast<launch::State>(99),counselorroute::State::SettingsReady,true)==Admission::Refused);
    check(Admit(launch::State::Ready,static_cast<counselorroute::State>(99),true)==Admission::Refused);
    std::printf("Native picker early Start waiting/admission: %s (%u checks; no writes/calls).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
