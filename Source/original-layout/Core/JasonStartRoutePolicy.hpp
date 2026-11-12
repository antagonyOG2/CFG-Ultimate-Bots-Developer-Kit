#pragma once
#ifndef CFG_J34_JASON_OPTIONS_FIX
#include "J24FeatureFlags.hpp"
#endif
namespace cfgmenu::jasonstart {
enum class Route:unsigned {Forward,HumanJason,Counselor};
// The feature flag affects ONLY the non-counselor production startup path.
// The caller independently checks the native receiver via Menu::Owns / the
// retiring widget guard; a false hint always forwards the stock Start once.
inline Route Choose(bool counselorClaim,bool ownedHumanJasonReceiver)noexcept{
 if(counselorClaim)return Route::Counselor;
 return CFG_J34_JASON_OPTIONS_FIX&&ownedHumanJasonReceiver?Route::HumanJason:Route::Forward;
}
template<class Human,class Stock>void Dispatch(bool counselorClaim,bool ownedHumanJasonReceiver,Human&& human,Stock&& stock){
 const auto route=Choose(counselorClaim,ownedHumanJasonReceiver);
 if(route==Route::HumanJason){if(!human())stock();}
 else if(route==Route::Forward)stock();
}
}
