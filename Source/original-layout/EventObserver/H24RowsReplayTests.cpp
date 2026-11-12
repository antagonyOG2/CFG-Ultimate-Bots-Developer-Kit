#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "HostedLobbyPlacement.hpp"
#include "HostedRowRetry.hpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 using namespace cfgmenu;using M=MenuFixture;using F=NativeFixture;
 NativeFixture f;OptionsReader reader{f.fixture.r};ModelEngine engine{f};engine.expectedWorld=F::Controller;
 auto text=std::make_unique<reflection::RetainedMenuText>();
 using Access=rows::NativeAccess<OptionsReader,ModelEngine>;auto access=std::make_unique<Access>(reader,engine,*text);
 active::Context context;check(active::Observe(reader,M::Base,context),"initial owned menu");
 f.fixture.r.Set(F::World+cfgprobe::WorldNetDriver,std::uintptr_t{0xA00000});f.fixture.Name(M::DonorType,"LobbyOptionComboBoxWidget_C");
 bool current=true;rows::NativeRetentionHooks hooks{&current,[](void* p,std::uintptr_t){return *static_cast<bool*>(p);},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t,void*){return true;},[](void*,std::uintptr_t,std::uintptr_t,void*){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return true;},[](void*,std::uintptr_t,std::uintptr_t){return true;},[](void*){},[](void*){},[](void*){return true;},nullptr};
 rows::HostedLobbyRetentionScope scope(M::Base,hooks,true);
 const auto proof=[](OptionsReader&,void* p){return *static_cast<bool*>(p);};
 check(access->PrepareHosted(context,91,M::Donor,F::Panel,&current,proof),"prepared, warm5/admission does not imply a native row dispatch");
 struct Host {std::unique_ptr<Access>& a;bool& current;
  bool ProvedHostLobbyLease(const cfghostedlobby::RowLayout&){return current;}
  bool AppendAfter(const cfghostedlobby::RowLayout&,unsigned index,std::uintptr_t& row,bool& attached){return a->AppendHostedAfter(index,row,attached);}
  bool CopyOptions(std::uintptr_t row,const rows::Spec& s){return a->CopyOptions(row,s);}
  bool Text(std::u16string_view s,reflection::RetainedMenuText::Request& out){return a->Text(s,out);}
  bool SetText(std::uintptr_t row,std::u16string_view field,const reflection::RetainedMenuText::Request& t){return a->SetText(row,field,t);}
  bool Index(std::uintptr_t row,unsigned& out){return a->Index(row,out);}bool RemoveOwned(std::uintptr_t row){return a->RemoveOwned(row);}
 }host{access,current};
 cfghostedlobby::Placement placement;cfghostedoptions::Store store;
 cfghostedlobby::RowLayout layout{F::World,M::Widget,M::DonorType,F::Panel,M::Donor,1,91};
 f.fixture.r.Set(F::CreateOwner+cfgspawn::ClassDefaultObject,std::uintptr_t{});
 (void)placement.Service(host,layout,0,store);(void)placement.Service(host,layout,25,store);
 check(placement.State()==static_cast<unsigned>(rows::State::Failed)&&placement.Placed()==0&&access->operationStage==102&&engine.calls==0,
    "replay Warm5/Stage40: failed row pre-dispatch persists despite admitted callbacks, exact CDO failure retained");
 for(unsigned i=0;i<5754;++i)(void)placement.Service(host,layout,50+25*i,store);
 check(!engine.calls&&!placement.Placed(),"old lifecycle stays failed through live-sized callback count; no fake placement");
 cfghostedlobby::RowRetry retry;check(!retry.Poll(true,false,0,false,false,0)&&retry.Poll(true,false,0,false,false,1000),"healthy pre-entry failure retries after backoff");
 f.fixture.r.Set(F::CreateOwner+cfgspawn::ClassDefaultObject,F::CreateOwner+0x800);
 access=std::make_unique<Access>(reader,engine,*text);check(access->PrepareHosted(context,91,M::Donor,F::Panel,&current,proof),"fresh current owner/class/parent proof on retry");
 placement.Detach();(void)placement.Service(host,layout,200000,store);
 check(engine.creates==1&&engine.attaches==1&&access->OwnedCount()==1,"proved controller world context creates and attaches exactly one row");
 for(unsigned i=0;i<10;++i)check(!retry.Poll(true,true,0,false,false,300000+i*1000),"never replay ANY entered Create even after owned count later becomes zero");
 cfghostedlobby::RowRetry limit;for(unsigned n=0;n<3;++n){const auto now=10000ULL*n;check(!limit.Poll(true,false,0,false,false,now),"schedule bounded retry");check(limit.Poll(true,false,0,false,false,now+(1000ULL<<n)),"dispatch owed retry");}
 check(!limit.Poll(true,false,0,false,false,1000000),"three pre-entry retries then stop");
 check(!retry.Poll(true,false,0,true,false,1000000)&&!retry.Poll(true,false,0,false,true,1000000),"poisoned/faulted provider cannot restart");
 cfghostedlobby::RowRetry budget;
 for(unsigned i=0;i<4000;++i)check(!budget.Due(true,true,1,false,false,i*25ULL),"terminal live failure opens no new provider");
 check(!budget.Due(true,false,0,false,false,100000)&&!budget.Due(true,false,0,false,false,100999),"backoff opens no provider before its due time");
 check(budget.Due(true,false,0,false,false,101000)&&budget.Due(true,false,0,false,false,101025)&&!budget.attempts,
       "provider admission failure leaves clean retry owed");
 check(budget.Poll(true,false,0,false,false,101050)&&budget.attempts==1,"consume retry only after provider admits");
 std::printf("H24 live5754/Warm5/Stage40 row replay and actor-world recovery: %u checks, %u failures; native engine model only.\n",checks,failures);return failures?1:0;
}
