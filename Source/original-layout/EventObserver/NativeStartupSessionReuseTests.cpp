#define main PreviousStartupRetentionMain
#include "NativeStartupRetentionTests.cpp"
#undef main
#include "../Core/NativeStartupSessionReuse.hpp"
namespace {
struct Session {
 Host& host;cfgretain::CallbackRootLease<decltype(Host::bank),64> lease;
 bool finished=false,callback=false,admitted=false;
 std::uintptr_t world=0;
 Session(Host& h,std::uintptr_t w):host(h),lease(h.bank,0x100000),world(w){}
 bool Retirable()const{return cfgstartupreuse::Retirable(finished,callback,admitted,host.bank.Healthy(),lease);}
 bool Retain(std::uintptr_t value){host.held=true;const bool okay=lease.Retain(host,std::array<std::uintptr_t,1>{value});host.held=false;return okay;}
 bool Finish(){host.held=true;const bool okay=lease.Close(host);host.held=false;finished=okay;return okay;}
};
}
int main(){const int earlier=PreviousStartupRetentionMain();const auto before=failures;
 {Host host;std::unique_ptr<Session> session;unsigned made=0;
  const auto first=[&](){++made;return std::make_unique<Session>(host,0x20000);};
  CHECK(!cfgstartupreuse::Prepare(session,false,true,false,first)&&!made);
  CHECK(!cfgstartupreuse::Prepare(session,true,false,false,first)&&!made);
  CHECK(!cfgstartupreuse::Prepare(session,true,true,true,first)&&!made);
  CHECK(cfgstartupreuse::Prepare(session,true,true,false,first)&&made==1);
  CHECK(session->Retain(0x30000)&&session->lease.Claims()==1&&!session->Retirable());
  const auto prior=session.get();
  CHECK(cfgstartupreuse::Prepare(session,false,false,false,first)&&session.get()==prior&&made==1);
  CHECK(!cfgstartupreuse::Prepare(session,true,true,false,first)&&session.get()==prior&&made==1);
  CHECK(session->Finish()&&session->Retirable()&&host.bank.Healthy()&&host.list.count==1);
  const auto owners=host.bank.Owners();
  CHECK(!cfgstartupreuse::Prepare(session,true,true,true,first)&&session.get()==prior&&made==1);
  const auto next=[&](){++made;return std::make_unique<Session>(host,0x40000);};
  CHECK(cfgstartupreuse::Prepare(session,true,true,false,next)&&made==2&&session->world==0x40000);
  CHECK(host.bank.Healthy()&&host.bank.Owners()==owners&&host.list.count==1);
  CHECK(session->Retain(0x50000)&&session->lease.Claims()==1&&session->Finish());
  host.Collect();CHECK(host.bank.Healthy()&&session->Retirable());
  session->callback=true;CHECK(!cfgstartupreuse::Prepare(session,true,true,false,next)&&made==2);session->callback=false;
  session->admitted=true;CHECK(!cfgstartupreuse::Prepare(session,true,true,false,next)&&made==2);session->admitted=false;
  host.bank.MarkUncertain();CHECK(!cfgstartupreuse::Prepare(session,true,true,false,next)&&made==2&&host.bank.Poisoned());
 }
 {Host host;auto session=std::make_unique<Session>(host,0x20000);CHECK(session->Retain(0x30000));
  host.held=true;CHECK(session->lease.PrepareCall(host,1));host.held=false;
  session->finished=true;unsigned made=0;
  CHECK(!cfgstartupreuse::Prepare(session,true,true,false,[&](){++made;return std::make_unique<Session>(host,0x40000);})&&!made);
  host.held=true;CHECK(session->lease.CancelCall(host));host.held=false;CHECK(session->Finish());
 }
 std::printf("Native startup closed-session reuse: %u failures; real resident bank/leases, two worlds, active/pending/poison refusal; no engine calls.\n",failures-before);
 return earlier||failures?1:0;
}
