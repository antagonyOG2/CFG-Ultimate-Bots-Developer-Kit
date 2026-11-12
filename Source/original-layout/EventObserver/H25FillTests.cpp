#include "../Core/HostedFillPolicy.hpp"
#include <cstdio>
struct Host {
 struct Ticket{bool requested=true,reserve=true;unsigned char before=1;};
 bool current=true,reserve=true;unsigned char value=1;int humans=2,bots=0;unsigned writes=0,forwards=0;
 bool Capture(Ticket& t){t={true,reserve,value};return current;}
 bool Exchange(const Ticket&,unsigned char before,unsigned char after){if(!current||value!=before)return false;value=after;++writes;return true;}
 void Stock(){++forwards;if(value)bots=cfghostedfill::CounselorTarget(humans,false,false);}
};
int main(){unsigned checks=0,failed=0;auto check=[&](bool ok){++checks;if(!ok){++failed;std::printf("FAIL %u\n",checks);}};
 using namespace cfghostedfill;
 // Exact live failure: two humans + native stock fill six = eight, so no Jason slot.
 Host old;old.Stock();check(old.humans+old.bots==8&&!CounselorRoom(8,true,false));
 for(int humans=1;humans<=8;++humans){Host f;f.humans=humans;Report r;
   AroundReserved(f,[&](){f.Stock();},r);check(f.forwards==1&&!f.bots&&f.value==1&&f.writes==2&&r.restored==1);
   int players=humans;while(CounselorRoom(players,true,false)){++players;++f.bots;}
   check(f.bots<=(8-humans-1<0?0:8-humans-1));check(!CounselorRoom(players,true,false));
   if(players<8){++players;check(players==(CFG_H257_F1_SLOT?(humans>6?humans+1:7):8));check(!CounselorRoom(players,true,true));} // one AI Jason state consumes reserve.
 }
 for(unsigned failure=0;failure<4;++failure){Host f;Report r;if(failure==0)f.value=0;if(failure==1)f.current=false;
   if(failure==2)f.reserve=false;
   AroundReserved(f,[&](){f.Stock();if(failure==3)f.value=2;},r);check(f.forwards==1);
   if(failure==0)check(!f.writes&&f.value==0&&!f.bots);
   if(failure==1||failure==2)check(!f.writes&&f.value==1&&f.bots==6); // no proved authority: forward, full badge; never guess.
   if(failure==3)check(f.value==2&&r.restoreRefused==1&&f.writes==1);
 }
 check(PlayerLimit(false,false)==8&&PlayerLimit(true,false)==(CFG_H257_F1_SLOT?6:7)&&PlayerLimit(true,true)==(CFG_H257_F1_SLOT?7:8));
 check(CounselorTarget(2,true,false)==(CFG_H257_F1_SLOT?4:5)&&CounselorTarget(2,false,false)==6);
 std::printf("H25 stock-fill live2+6 replay, reserved helper/F3 capacity and original-once/own-value restoration: %u checks, %u failures; no native calls.\n",checks,failed);
 return failed?1:0;
}
