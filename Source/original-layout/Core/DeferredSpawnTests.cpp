#include "NativeDeferredSpawnParameters.hpp"
#include <cstdio>
#include <limits>
#include <cstring>
#include <array>
namespace {
int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
template<std::size_t N> bool Zero(const std::uint8_t (&bytes)[N]) { for (auto byte:bytes) if (byte) return false; return true; }
}
int main() {
    using namespace cfgspawn::deferred;
    BeginParameters begin; FinishParameters finish;
    CHECK(BuildBegin(0x10000000,0x20000000,{10,20,30},begin));
    CHECK(begin.world==0x10000000 && begin.actorClass==0x20000000 && !begin.owner && !begin.result && !begin.collisionOverride);
    CHECK(begin.transform.rotation[3]==1 && begin.transform.rotation[0]==0 && begin.transform.scale.x==1 && begin.transform.scale.y==1 && begin.transform.scale.z==1);
    CHECK(begin.transform.translation.x==10 && begin.transform.translation.y==20 && begin.transform.translation.z==30);
    CHECK(Zero(begin.collisionPadding) && Zero(begin.tailPadding) && begin.transform.translationPadding==0 && begin.transform.scalePadding==0);
    CHECK(BuildFinish(0x30000000,{10,20,30},finish));
    CHECK(finish.actor==0x30000000 && !finish.result && Zero(finish.actorPadding) && Zero(finish.tailPadding));
    CHECK(std::memcmp(&begin.transform,&finish.transform,sizeof(Transform))==0);
    for (const auto pointer:std::array<std::uintptr_t,4>{0,0x9998,0x10001,0x0000800000000000}) {
        CHECK(!BuildBegin(pointer,0x20000000,{},begin) && !begin.world && !begin.actorClass);
        CHECK(!BuildBegin(0x10000000,pointer,{},begin) && !begin.world && !begin.actorClass);
        CHECK(!BuildFinish(pointer,{},finish) && !finish.actor);
    }
    for (const auto number:std::array<float,4>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),10000001,-10000001})
        for (unsigned axis=0;axis<3;++axis) {
            cfgspawn::Vec3 location{};
            if (axis==0) location.x=number; else if (axis==1) location.y=number; else location.z=number;
            CHECK(!BuildBegin(0x10000000,0x20000000,location,begin) && !begin.world && !begin.result);
            CHECK(!BuildFinish(0x30000000,location,finish) && !finish.actor && !finish.result);
        }
    std::printf("CFG deferred packet/layout checks %s (%d failures). No native calls or game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
