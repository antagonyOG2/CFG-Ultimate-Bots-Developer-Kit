#include "../Core/ForegroundKeyEdge.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};bool down=false;
 check(cfginput::ForegroundKeyEdge(true,static_cast<std::int16_t>(0x8001),down)&&down);
 check(!cfginput::ForegroundKeyEdge(true,static_cast<std::int16_t>(0x8000),down)&&down);
 check(!cfginput::ForegroundKeyEdge(true,0,down)&&!down);
 check(cfginput::ForegroundKeyEdge(true,1,down)&&!down); // tap between polls
 check(!cfginput::ForegroundKeyEdge(true,0,down));
 check(!cfginput::ForegroundKeyEdge(false,1,down)&&!down);
 check(!cfginput::ForegroundKeyEdge(false,static_cast<std::int16_t>(0x8001),down)&&!down);
 check(cfginput::ForegroundKeyEdge(true,static_cast<std::int16_t>(0x8000),down)&&down);
 check(!cfginput::ForegroundKeyEdge(true,1,down)&&!down); // no second request while prior sample was held
 std::printf("Foreground F3 edge, held key, short tap and foreign-window refusal: %u failures; no keyboard/game access.\n",failures);return failures?1:0;}
