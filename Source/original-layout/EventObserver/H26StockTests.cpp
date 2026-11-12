#include "../Core/HostedFillPolicy.hpp"
#include "../Core/HostedKeyAction.hpp"
#include <cstdio>
struct Stock {int humans=2,bots=0;unsigned flag=1,writes=0,calls=0;
 void Forward(){++calls;if(flag)bots=6;}};
int main(){unsigned bad=0,n=0;auto check=[&](bool ok){++n;if(!ok){++bad;std::printf("FAIL %u\n",n);}};
 for(unsigned flag=0;flag<2;++flag){Stock s;s.flag=flag;cfghostedfill::Report r;
  cfghostedfill::AroundStock(s,[&](){s.Forward();},r);
  check(s.calls==1&&!s.writes&&s.flag==flag&&s.bots==(flag?6:0)&&r.entries==1&&!r.applied&&!r.restored);
  if(flag)check(s.humans+s.bots==8&&!cfghostedfill::CounselorRoom(8,true,false));}
 using namespace cfghostedkeys;
 check(Select(true,false,false)==Action::None); // F1/F3 no longer dispatch through H22
 check(Select(false,true,true)==Action::None);
 check(Select(true,true,true)==Action::Disarm);
 check(Select(true,true,false)==Action::Arm);
 std::printf("H26 live2+6 stock-fill preservation + H22 F8/F9-only policy; F1/F3 are owned by J2.19: %u checks, %u failures; no native calls.\n",n,bad);return bad?1:0;}
