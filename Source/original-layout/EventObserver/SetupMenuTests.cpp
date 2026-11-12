#include "../Core/SetupMenuModel.hpp"
#include <cstdio>
struct Host {
    unsigned step=0,rows=0,failAt=0;bool valid=true,rolledBack=false;
    bool CreateNativeDonorClass(cfgmenu::RowId){valid=valid&&step==0;step=1;return valid;}
    bool AttachToExistingPanel(cfgmenu::RowId){valid=valid&&step==1;step=2;return valid;}
    bool CopyOptionsAfterConstruction(cfgmenu::RowId){valid=valid&&step==2;step=3;return valid&&(!failAt||rows+1!=failAt);}
    bool SetOwnedText(cfgmenu::RowId,std::wstring_view){valid=valid&&step==3;step=0;++rows;return valid;}
    void RollbackOwnedRows(){rows=0;step=0;rolledBack=true;}
};
int main(){
    using namespace cfgmenu;unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};
    Capabilities all;all.verified.fill(true);all.automaticJason=true;
    check(Rows[0].title==L"Counselor Bot Skill");check(Rows[14].title==L"Disable Jason Boundary");
    check(!Available(Rows[13],Mode::Counselor,all));check(!Available(Rows[13],Mode::Jason,all));
    check(Available(Rows[13],Mode::Challenges,all));
    Capabilities none;Host hidden;check(AppendRows(hidden,Mode::Counselor,none)&&hidden.rows==0);
    Host counselor,jason,challenges;check(AppendRows(counselor,Mode::Counselor,all)&&counselor.rows==5);
    check(AppendRows(jason,Mode::Jason,all)&&jason.rows==6);check(AppendRows(challenges,Mode::Challenges,all)&&challenges.rows==8);
    Host broken;broken.failAt=2;check(!AppendRows(broken,Mode::Counselor,all)&&broken.rolledBack&&broken.rows==0);
    check(BaseOptions(RowId::Duration,Mode::Counselor).count==3);
    check(BaseOptions(RowId::Duration,Mode::Jason).count==4);
    check(BaseOptions(RowId::Weather,Mode::Counselor).initial==2);
    Session s;Selection result;std::uint64_t id=0;
    check(!s.ConsumeStart(0,result,id));check(s.Open(100,Mode::Counselor,all,10));
    auto gen=s.Generation();
    check(!s.Repick(100,gen,1,2,3,false));
    check(!s.Pick(101,gen,1,2,3));check(s.Pick(100,gen,1,2,3));check(!s.SetCount(gen,11));check(s.SetCount(gen,10));
    check(!s.SetChoice(gen,RowId::Duration,3));check(s.SetChoice(gen,RowId::Duration,2));
    check(s.SetChoice(gen,RowId::Weapon,4));check(!s.SetChoice(gen,RowId::Inventory,6));
    check(!s.Repick(101,gen,4,5,6,false)&&!s.Repick(100,gen+1,4,5,6,false));
    check(!s.Repick(100,gen,4,5,0,false)&&!s.Repick(100,gen,4,5,6,true));
    check(s.Repick(100,gen,4,5,6,false));
    check(!s.SetWeather(gen,9,false));check(s.SetWeather(gen,9,true));
    check(!s.SetToggle(gen,RowId::PreventEscapes,true));check(!s.Commit(101,gen));check(s.Commit(100,gen));
    check(s.ConsumeStart(gen,result,id)&&result.counselorCount==10&&id==1&&result.duration==2&&result.weapon==4&&result.weatherIdentity==9&&result.map==4&&result.jasonClass==5&&result.counselorClass==6);check(!s.ConsumeStart(gen,result,id));
    check(!s.Repick(100,gen,1,2,3,false));
    s.Reset();check(s.Open(200,Mode::Challenges,all,10));gen=s.Generation();check(s.Pick(200,gen,1,2,0));
    check(s.SetToggle(gen,RowId::PreventEscapes,true));s.Reset();
    check(s.Open(300,Mode::Counselor,all,10));gen=s.Generation();check(s.Pick(300,gen,1,2,3));check(s.Commit(300,gen));
    check(s.ConsumeStart(gen,result,id)&&!result.toggles[6]); // challenge flags do not leak
    check(!s.Open(1,static_cast<Mode>(3),all,10));check(!s.Open(1,Mode::Counselor,all,33));
    check(s.Open(400,Mode::Counselor,all,1));gen=s.Generation();check(s.Pick(400,gen,1,2,3));check(s.Commit(400,gen));
    check(!s.Open(0,Mode::Counselor,all,10));check(!s.ConsumeStart(gen,result,id));
    check(s.Open(400,Mode::Jason,all,10));auto old=s.Generation();check(s.Pick(400,old,1,2,0));
    check(s.Open(400,Mode::Jason,all,10));gen=s.Generation();check(gen!=old);
    check(!s.Pick(400,old,1,2,0));check(s.Pick(400,gen,1,2,0));
    check(!s.SetCount(old,5));check(!s.SetChoice(old,RowId::Difficulty,2));
    check(!s.SetToggle(old,RowId::Knives,true));check(!s.SetWeather(old,9,true));check(!s.Commit(400,old));
    check(s.Commit(400,gen));check(!s.ConsumeStart(old,result,id));check(s.ConsumeStart(gen,result,id));
    std::printf("CFG base-menu text/order/native-style contract/scoped auto-start tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
