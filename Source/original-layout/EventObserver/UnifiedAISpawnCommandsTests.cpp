#include "../Core/UnifiedAISpawnCommands.hpp"
#include <cstdio>
#include <initializer_list>
using namespace cfgspawncommand;
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
 Scope s{1,2,0,1,1,0,Mode::Offline,true,true,false,true,true};
 for(auto mode:{Mode::Offline,Mode::PrivateHosted}){s.route=mode;s.netDriver=mode==Mode::Offline?0:3;s.driverSerial=mode==Mode::Offline?0:1;s.privateLobby=true;
  Queue q;check(q.Request(s,Kind::ArmJason,true,false,7)==Result::Queued);
  check(q.Request(s,Kind::ArmJason,true,false,7)==Result::AlreadyQueued);
  check(q.Consume(s,Kind::ArmJason)&&!q.Consume(s,Kind::ArmJason));
  check(q.Request(s,Kind::ArmJason,true,true,7)==Result::AlreadyPresent);
  check(q.Request(s,Kind::AddCounselor,true,true,7)==Result::Queued);
  check(q.Request(s,Kind::AddCounselor,true,true,7)==Result::AlreadyQueued);
  check(q.Consume(s,Kind::AddCounselor));
  check(q.Request(s,Kind::AddCounselor,true,true,10)==Result::Refused);
  check(q.Request(s,Kind::AddCounselor,true,false,7)==Result::Refused);
  check(q.Request(s,Kind::ArmJason,false,false,7)==Result::Refused);
  for(unsigned fault=0;fault<7;++fault){auto bad=s;
   if(fault==0)bad.authority=false;if(fault==1)bad.gameThread=false;if(fault==2)bad.inProgress=false;
   if(fault==3)bad.bindings=false;if(fault==4)bad.worldSerial=0;if(fault==5)bad.mode=0;
   if(fault==6){if(mode==Mode::Offline)bad.netDriver=3;else bad.privateLobby=false;}
   check(q.Request(bad,Kind::ArmJason,true,false,7)==Result::Refused);}
  auto changed=s;++changed.worldSerial;check(q.Request(changed,Kind::ArmJason,true,false,7)==Result::Refused);
  check(!q.Consume(s,Kind::ArmJason));
 }
 std::printf("Unified offline/private-host AI command policy: %u checks %u failures; no native adapter enabled\n",checks,failures);
 return failures?1:0;
}
