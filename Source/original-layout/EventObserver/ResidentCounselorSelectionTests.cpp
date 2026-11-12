#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/ResidentCounselorSelection.hpp"
#include "../Core/CounselorLoadPolicy.hpp"
#include <cstdio>
int main(){unsigned failed=0;const auto check=[&](bool v){if(!v)++failed;};
    check(!cfgresidentcounselor::Ordinary(u"Hero_Counselor_C"));check(!cfgresidentcounselor::Ordinary(u"Base_Counselor_C"));
    check(!cfgresidentcounselor::Ordinary(u"Hunter_Counselor_C"));
    check(!cfgresidentcounselor::Ordinary(u""));check(cfgresidentcounselor::Ordinary(u"Counselor_Female_C"));
    cfgbundle::RandomCounselors pool;check(pool.Begin(5,37));
    // Cold entries do not block later resident classes; same class is reusable
    // across separate births, like donor F3. Local class is fallback only.
    const std::uintptr_t classes[]{0,10,0,20,0};
    for(unsigned birth=0;birth<9;++birth){cfgresidentcounselor::Pass pass;unsigned index=0;std::uintptr_t selected=0;
        while(pass.Next(pool,index)){if(pass.Use(classes[index],10,index)){selected=classes[index];break;}}
        check(selected==20);check(pool.Confirm(selected,{3000.f+birth*2000.f,0,0}));pool.NextSelection();
    }
    check(pool.created==9);check(!pool.Confirm(20,{30000,0,0}));
    {cfgbundle::RandomCounselors diverse;check(diverse.Begin(10,19));
     for(unsigned birth=0;birth<9;++birth){cfgresidentcounselor::Pass pass;unsigned index=0;std::uintptr_t selected=0;
      while(pass.Next(diverse,index)){const auto type=100+index;if(pass.Use(type,100,index,diverse.Used(type))){selected=type;break;}}
      check(selected&&selected!=100&&!diverse.Used(selected));check(diverse.Confirm(selected,{3000.f+birth*2000.f,0,0}));diverse.NextSelection();}}
    {cfgcounselorload::Preload load;for(unsigned i=0;i<10;++i){check(load.Loading(10)&&load.cursor==i);load.Requested(10,100+i*100);}
     check(!load.Loading(10)&&load.Warming(2999)&&!load.Warming(3000));check(load.Pending(30999)&&!load.Pending(31000));}
    cfgbundle::RandomCounselors onlyLocal;check(onlyLocal.Begin(3,51));
    {cfgresidentcounselor::Pass pass;unsigned index=0,count=0;bool selected=false;
     while(pass.Next(onlyLocal,index)){++count;if(pass.Use(index==1?10:0,10,index)){selected=true;break;}}
     check(selected&&pass.fallback&&count==4);}
    cfgbundle::RandomCounselors cold;check(cold.Begin(4,7));
    {cfgresidentcounselor::Pass pass;unsigned index=0,count=0;while(pass.Next(cold,index)){++count;check(!pass.Use(0,10,index));}check(count==4);}
    {cfgresidentcounselor::Pass pass;unsigned index=0;pass.Retry(2);check(pass.Next(cold,index)&&index==2);check(!pass.Next(cold,index));}
    check(pool.LocationAllowed({30000,0,0},{0,0,0}));check(!pool.LocationAllowed({100,0,0},{0,0,0}));
    std::printf("Donor resident counselor selection / cold skip / local fallback / repeated F3 to nine helpers:%u failures.\n",failed);return failed?1:0;
}
