#include "../Core/JasonStartRoutePolicy.hpp"
#include <cassert>
#include <cstdio>
using cfgmenu::jasonstart::Route;
int main(){using namespace cfgmenu::jasonstart;
 assert(Choose(true,false)==Route::Counselor&&Choose(true,true)==Route::Counselor);
 assert(Choose(false,false)==Route::Forward);
 assert(Choose(false,true)==(CFG_J34_JASON_OPTIONS_FIX?Route::HumanJason:Route::Forward));
 unsigned human=0,stock=0;
 Dispatch(false,true,[&](){++human;return true;},[&](){++stock;});
 assert(human==static_cast<unsigned>(CFG_J34_JASON_OPTIONS_FIX!=0)&&stock==static_cast<unsigned>(!CFG_J34_JASON_OPTIONS_FIX));
 human=stock=0;
 Dispatch(false,true,[&](){++human;return false;},[&](){++stock;});
 assert(stock==1&&human==static_cast<unsigned>(CFG_J34_JASON_OPTIONS_FIX!=0)); // failed human attempt forwards exactly once
 human=stock=0;Dispatch(false,false,[&](){++human;return true;},[&](){++stock;});assert(!human&&stock==1);
 human=stock=0;Dispatch(true,true,[&](){++human;return true;},[&](){++stock;});assert(!human&&!stock); // caller owns counselor path
 std::printf("J34 human Jason startup route policy PASS (flag %d); no native actions.\n",CFG_J34_JASON_OPTIONS_FIX);
}
