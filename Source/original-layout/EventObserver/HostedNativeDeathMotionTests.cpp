#define CFG_H1_JASON 1
#include <cstdint>
#include <cstdio>
#include <map>
#include "../RuntimeProbe/EscapeBindings.generated.hpp"
// Exercise the actual member with owned observation/native stubs. These stubs
// test refusal ordering and one-shot semantics, not engine authority or offsets.
struct Slot {std::uintptr_t object=0;int serial=1;bool operator==(const Slot& b)const{return object==b.object&&serial==b.serial;}};
namespace cfgobserver {struct Plan {std::uintptr_t module=1,world=100,objectArray=3;};}
namespace cfgjasontick {struct Owner {struct {Slot pawn{200},killerController{300};}actor;struct {bool valid=true;std::uintptr_t controllerState=400;}adoption;};}
namespace cfgdonorhunt {enum class NativeOutcome {NotEntered,Uncertain,ReturnedTrue};}
struct Reader {
    bool safe=true,world=true,slots=true,type=true,stable=true;unsigned reads=0;
    std::map<std::uintptr_t,std::uintptr_t> cells;
    template<class T>bool Read(std::uintptr_t at,T& out){++reads;auto i=cells.find(at);if(i==cells.end())return false;out=static_cast<T>(i->second);return true;}
};
namespace cfgprimary {
constexpr std::uintptr_t CurrentKiller=11,CurrentKillerState=12;
bool LookupSafe(Reader& r,std::uintptr_t){return r.safe;}
bool RetainedWorld(Reader& r,const cfgobserver::Plan& p,cfgobserver::Plan& out){out=p;return r.world;}
}
namespace cfgprobe {constexpr std::uintptr_t WorldGameState=13;}
namespace cfgjason {bool SameSlot(Reader& r,std::uintptr_t,const Slot&){return r.slots;}}
namespace cfgspawn {bool ObjectIs(Reader& r,std::uintptr_t,std::uintptr_t){return r.type;}}
namespace cfgobserver {template<class T>bool Same(Reader& r,std::uintptr_t at,T wanted){T got{};return r.Read(at,got)&&got==wanted;}}
namespace cfgescape {template<class T>bool Same(Reader& r,std::uintptr_t at,T wanted){return r.stable&&cfgobserver::Same(r,at,wanted);}}
namespace cfgclosedtrace {void Mark(const char*,std::uintptr_t,unsigned){}}
struct Journal {unsigned clears=0;void Clear(){++clears;}};
struct Access {
    Reader r_;cfgobserver::Plan plan;
    bool adopted=true,hostedNativeDeathMotion=false,stopOkay=true;unsigned stops=0;
    std::uint64_t hostedNativeDeathMotionCheckAt=0,bundleMoveAt=10,bundleTargetLockUntil=10;
    struct {Slot pawn{200},controller{300};}hostedAdoptionReceipt;
    struct {std::uintptr_t gameState=500,killerBase=600;struct {Slot stateSlot{400};}primary;}spawnAssets;
    Journal bundlePursuit,bundleFollowPath,bundlePathCooldown;
    bool HostedAdoptionOwnsDriver(){return adopted;}
    bool DonorNativeStopMovement(const cfgjasontick::Owner&,cfgdonorhunt::NativeOutcome& outcome){
        ++stops;outcome=stopOkay?cfgdonorhunt::NativeOutcome::ReturnedTrue:cfgdonorhunt::NativeOutcome::Uncertain;return stopOkay;
    }
    Access(){r_.cells[plan.world+cfgprobe::WorldGameState]=spawnAssets.gameState;
        r_.cells[spawnAssets.gameState+cfgprimary::CurrentKiller]=hostedAdoptionReceipt.pawn.object;
        r_.cells[spawnAssets.gameState+cfgprimary::CurrentKillerState]=400;
        r_.cells[hostedAdoptionReceipt.pawn.object+cfgescape::disk::DyingFlags]=cfgescape::disk::DyingMask;}
#include "HostedNativeDeathMotionMembers.hpp"
};
int main(){unsigned checks=0,failed=0;const auto check=[&](bool v){++checks;if(!v)++failed;};
    const cfgjasontick::Owner owner{};
    {Access a;check(a.HostedNativeDeathOwnsMotion(owner,1000));check(a.stops==1&&a.hostedNativeDeathMotion);
        check(!a.bundleMoveAt&&!a.bundleTargetLockUntil&&a.bundlePursuit.clears==1&&a.bundleFollowPath.clears==1&&a.bundlePathCooldown.clears==1);
        check(a.HostedNativeDeathOwnsMotion(owner,2000)&&a.stops==1);}
    {Access a;a.stopOkay=false;check(a.HostedNativeDeathOwnsMotion(owner,1000));
        check(a.HostedNativeDeathOwnsMotion(owner,2000)&&a.stops==1);}
    for(unsigned refusal=0;refusal<12;++refusal){Access a;auto o=owner;
        switch(refusal){case 0:o.adoption.valid=false;break;case 1:a.adopted=false;break;
        case 2:++o.actor.pawn.serial;break;case 3:++o.actor.killerController.serial;break;
        case 4:a.r_.safe=false;break;case 5:a.r_.world=false;break;case 6:a.r_.slots=false;break;
        case 7:a.r_.type=false;break;case 8:a.r_.cells[a.plan.world+cfgprobe::WorldGameState]=999;break;
        case 9:a.r_.cells[500+cfgprimary::CurrentKiller]=999;break;
        case 10:a.r_.cells[500+cfgprimary::CurrentKillerState]=999;break;case 11:a.r_.stable=false;break;}
        check(!a.HostedNativeDeathOwnsMotion(o,1000)&&!a.stops&&!a.hostedNativeDeathMotion);}
    {Access a;const auto at=owner.actor.pawn.object+cfgescape::disk::DyingFlags;
        a.r_.cells[at]=0;check(!a.HostedNativeDeathOwnsMotion(owner,1000)&&!a.stops); // ordinary stun, no accepted hit requirement
        a.r_.cells[at]=cfgescape::disk::DyingMask;const auto reads=a.r_.reads;
        check(!a.HostedNativeDeathOwnsMotion(owner,1100)&&a.r_.reads==reads);
        check(a.HostedNativeDeathOwnsMotion(owner,1250)&&a.stops==1);}
    std::printf("Hosted native death motion actual member: %u checks, %u failures; native dying independent of custom kill, exact owner, one stop, no ambiguous replay.\n",checks,failed);
    return failed?1:0;
}
