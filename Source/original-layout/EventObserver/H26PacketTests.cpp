#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "HostedCounselorSource.hpp"
#include "../Core/HostedRolePreference.hpp"
int main(){unsigned n=0,bad=0;auto check=[&](bool ok,const char* why){++n;if(!ok){++bad;std::printf("FAIL %s\n",why);}};
 {std::array<std::uint8_t,4> before{1,2,0,0},human{1,1,0,0},after{};
  check(cfghostedrole::InvertHostPreference(before,human,2,0,after)&&after[0]==2&&after[1]==1,
      "invert donor role inputs for host Jason / remote counselor");
  human[1]=0;check(!cfghostedrole::InvertHostPreference(before,human,2,0,after),"fail closed without a distinct remote human");
  human[1]=1;before[1]=3;check(!cfghostedrole::InvertHostPreference(before,human,2,0,after),"fail closed on unknown native role value");}
 NativeFixture f;OptionsReader r{f.fixture.r};using namespace cfghostedpacket;
 constexpr std::uintptr_t mode=NativeFixture::Mode,data=0x1A00000,state=0x1B00000,nameObject=0x1C00000;
 f.fixture.Name(nameObject,"/Game/Characters/Counselors/Test.Test_C");cfgnames::Words words{};
 check(r.Read(nameObject+0x18,words),"fixture exact FName");
 cfgmenu::promotion::Packet p;std::memcpy(p.bytes.data()+16,&words,8);
 f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{data,2,2});
 f.fixture.r.Set(data,cfgmenu::promotion::Packet{});f.fixture.r.Set(data+40,p);
 // Both human preferences absent, exactly the live 2-human failure. Empty subpath is valid.
 f.fixture.r.Set(state+cfgmenu::birthdisk::PickedCounselorClass,cfgmenu::promotion::Packet{});
 std::array<std::uintptr_t,2> states{state,state};Source selected;Diagnostic d;
 check(Choose(r,MenuFixture::Base,mode,states,2,0,selected,d)&&selected.kind==1&&selected.index==1&&!d.reason,"native catalogue FName works with zero substring/2 empty human preferences");
 check(Current(r,MenuFixture::Base,selected,d),"source revalidated before native copy");
 f.fixture.r.Set(data+40,cfgmenu::promotion::Packet{});check(!Current(r,MenuFixture::Base,selected,d)&&d.reason==2,"changed/empty packet refuses");
 f.fixture.r.Set(data+40,p);f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{data+80,2,2});
 check(!Current(r,MenuFixture::Base,selected,d)&&d.reason==8,"replaced catalogue refuses");
 f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{});f.fixture.r.Set(state+cfgmenu::birthdisk::PickedCounselorClass,p);
 check(Choose(r,MenuFixture::Base,mode,states,2,0,selected,d)&&selected.kind==2,"registered preference fallback uses FName ABI, no substring requirement");
 // H210: native chosen counselor must win even with a valid different catalogue entry.
 f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{data,2,2});
 check(Preferred(r,MenuFixture::Base,state,selected,d)&&selected.kind==2&&selected.owner==state,"displaced human keeps own chosen counselor before random fallback");
 check(!Preferred(r,MenuFixture::Base,0,selected,d),"missing selected human cannot borrow another preference");
 f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{});
 p.bytes[24]=1;f.fixture.r.Set(state+cfgmenu::birthdisk::PickedCounselorClass,p);
 check(!Choose(r,MenuFixture::Base,mode,states,2,0,selected,d)&&d.reason==3,"nonempty borrowed subpath refuses");
 // H243: donor OfflineSetup.cpp:10230-10259. The displaced human's PickedCounselorClass matches catalogue entry 3 -> restores entry 3.
 {constexpr std::uintptr_t cat=0x1D00000,pstate=0x1E00000;cfgmenu::promotion::Packet e[4],unknown;const char* names[4]={"/Game/Characters/Counselors/Vanessa.Vanessa_C","/Game/Characters/Counselors/Jenny.Jenny_C","/Game/Characters/Counselors/Chad.Chad_C","/Game/Characters/Counselors/AJ.AJ_C"};
  for(unsigned i=0;i<4;++i){const auto obj=0x1F00000+0x1000*i;f.fixture.Name(obj,names[i]);cfgnames::Words w{};check(r.Read(obj+0x18,w),"H243 name");
   std::memcpy(e[i].bytes.data()+16,&w,8);f.fixture.r.Set(cat+40*i,e[i]);}
  f.fixture.Name(0x1F10000,"/Game/Characters/Counselors/Unlisted.Unlisted_C");cfgnames::Words uw{};check(r.Read(0x1F10000+0x18,uw),"H243 unlisted name");std::memcpy(unknown.bytes.data()+16,&uw,8);
  f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{cat,4,4});
  f.fixture.r.Set(pstate+cfgmenu::birthdisk::PickedCounselorClass,e[3]);
  Source s;Diagnostic dd;
  check(Preferred(r,MenuFixture::Base,pstate,s,dd,mode)&&s.kind==2&&s.owner==pstate&&s.index==3&&dd.matchState==1&&dd.catalogueCount==4&&s.packet.bytes==e[3].bytes,
        "H243 PickedCounselorClass == catalogue entry 3 restores entry 3 (own PlayerState, not Vanessa/index 0)");
  check(Current(r,MenuFixture::Base,s,dd),"H243 matched source still revalidates");
  f.fixture.r.Set(pstate+cfgmenu::birthdisk::PickedCounselorClass,e[0]);
  check(Preferred(r,MenuFixture::Base,pstate,s,dd,mode)&&s.index==0&&dd.matchState==1&&s.packet.bytes==e[0].bytes,"H243 entry 0 matches as entry 0, reported as a real match");
  f.fixture.r.Set(pstate+cfgmenu::birthdisk::PickedCounselorClass,unknown);
  check(Preferred(r,MenuFixture::Base,pstate,s,dd,mode)&&s.index==0xFFFFu&&dd.matchState==2&&s.packet.bytes==unknown.bytes,"H243 unmatched preference kept (no new refusal), reported 65535");
  f.fixture.r.Set(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,cfgspawn::PlayerArray{});
  check(Preferred(r,MenuFixture::Base,pstate,s,dd,mode)&&dd.matchState==3&&s.index==0xFFFEu,"H243 unreadable catalogue keeps preference, reported 65534");
  check(Preferred(r,MenuFixture::Base,pstate,s,dd)&&dd.matchState==0,"H243 legacy call without mode unchanged");}
 std::printf("H26 exact soft-class FName/catalogue/2-human packet replay: %u checks, %u failures; no loading/native calls.\n",n,bad);return bad?1:0;}
