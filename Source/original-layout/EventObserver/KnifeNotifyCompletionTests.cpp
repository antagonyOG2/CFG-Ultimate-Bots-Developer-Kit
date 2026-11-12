#include "KnifeNotifyCompletion.hpp"
#include <cassert>
#include <cstdio>
int main(){
 using namespace cfgknife;
 NotifyWindow window{{1,10},{2,20},{3,30},6,true};
 Snapshot after{{1,10},{3,30},{2,20},5,false,true};
 assert(window.Missing(after,1,true));
 for(unsigned fault=0;fault<10;++fault){auto w=window;auto s=after;
  if(fault==0)w.stopped=true;if(fault==1)w.launched=true;if(fault==2)w.multicast=true;
  if(fault==3)w.consumed=true;if(fault==4)w.armed=false;if(fault==5)s.count=4;
  if(fault==6)s.driver.serial++;if(fault==7)s.targetLiving=false;
  if(fault==8)s.jason.serial++;if(fault==9)s.pressed=true;
  assert(!w.Missing(s,1,true));
 }
 assert(!window.Missing(after,99,true)&&!window.Missing(after,1,false));
 window.geometryReady=true;
 for(unsigned fault=0;fault<9;++fault){auto w=window;
  if(fault==0)w.stopped=true;if(fault==1)w.launched=true;if(fault==2)w.multicast=true;
  if(fault==3)w.consumed=true;if(fault==4)w.armed=false;if(fault==5)w.geometryReady=false;
  if(fault==6)w.geometryUsed=true;
  assert(!w.UseNativeGeometry(fault==7?99:2,fault==8?99:1));
 }
 assert(window.UseNativeGeometry(2,1));
 assert(!window.UseNativeGeometry(2,1)); // original forwarded once; no second correction
 Runtime runtime;runtime.nativeCompletionRequired=true;runtime.stage=Stage::Notify;runtime.release.driver=window.driver;
 assert(runtime.CompleteNativeNotify(window.driver,100)&&runtime.stage==Stage::Flight&&runtime.launches==1);
 assert(!runtime.CompleteNativeNotify(window.driver,101)&&runtime.launches==1);
 runtime.stage=Stage::Cleanup;assert(!runtime.CompleteNativeNotify(window.driver,102));
 std::puts("Native knife completion admission PASS: no duplicate/stopped/noncolliding/ambiguous launch; actual hook/native calls require physical test.");
}
