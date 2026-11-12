#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
namespace cfghostedlobby {
constexpr unsigned NodeLimit=128,BindingLimit=16,HistoryLimit=2;
enum class Failure:unsigned {None,Thread,Engine,World,Controller,Hud,Stack,Menu,Tree,Changed,Capacity,NoHost,NoNativeLease,NoCompatibleRow};
struct Node {
    std::uint64_t object=0,type=0,slot=0;std::int32_t serial=0;
    std::uint32_t parent=UINT32_MAX,index=0,kind=0,textState=0;
    std::array<char,40> name{};std::array<char,56> className{};std::array<char,72> authoredText{};
};
struct Member {std::uint64_t object=0,property=0;std::uint32_t offset=0;std::array<char,40> name{};};
struct Row {
    std::uint32_t node=UINT32_MAX,parent=UINT32_MAX,childIndex=0,bindings=0,truncated=0;
    std::array<Member,BindingLimit> members{};
};
struct Snapshot {
    std::uint64_t world=0,menu=0,hud=0;std::int32_t serial=0;
    std::uint32_t seen=0,complete=0,count=0,cursor=0,truncated=0,hostFlag=0,listenHost=0,lobbyType=0,rowsPlaced=0,placementRefusal=0;
    std::array<char,64> menuClass{};std::array<Node,NodeLimit> nodes{};Row team{},weather{};
};
struct ControllerTrace {
    std::uint32_t stage=0;std::int32_t count=0,capacity=0;
    std::uint64_t player=0,controller=0,type=0,base=0,backlink=0;
    std::array<char,64> className{};
};
struct OwnerEvidence {
    std::uint64_t value=0;std::int32_t serial=0;
    std::uint32_t internal=0,flags=0,role=0,index=0;
    std::array<char,40> name{};std::array<char,56> className{};
};
struct Report {
    static constexpr std::uint32_t Magic=0x314C3248,Version=9;
    std::uint32_t magic=Magic,version=Version,bytes=sizeof(Report),generation=0;
    std::uint32_t samples=0,failure=0,current=0,histories=0,peakReads=0,textRefused=0;
    ControllerTrace controller{};
    std::uint32_t rowCallbacks=0,rowAdmitted=0,rowStage=0,rowWarm=0,rowRootPoison=0,rowAdapterStage=0,rowState=0,rowOwned=0,rowCalls=0,rowFirstFailure=0,rowFirstAdapter=0,rowFirstReads=0,rowRetries=0;
    std::uint64_t rowStaticOwner=0,rowStaticDefault=0;
    std::array<char,64> rowStaticOwnerName{},rowStaticClassName{};
    std::uint32_t rowTypeReason=0,rowCurrentReason=0,rowInvokeReason=0,rowRootStage=0;
    std::uint32_t rowInputAttempts=0,rowInputFailures=0;
    std::uint32_t rowRootReason=0,rowRootRole=0,rowRootScopeFailure=0,rowRootInternal=0,rowRootFlags=0;
    std::int32_t rowRootSerial=-1;std::uint64_t rowRootValue=0;
    std::array<OwnerEvidence,13> rowOwners{};
    std::uint32_t packetReason=0,packetKind=0,packetIndex=0;std::uint64_t packetAddress=0;
    std::uint32_t f1Presses=0,f9Presses=0,f3Presses=0,f3Queued=0,f3Attempts=0,f3Completed=0,f3Refused=0,f3Reason=0;
    std::uint32_t f3NativeStage=0,f3NativeCalls=0,f3FirstReason=0,f3FirstStage=0,f3FirstCalls=0,f3TravelReturned=0,f3TravelReady=0;
    std::int32_t f3BeforePlayers=0,f3BeforeBots=0;
    std::uint32_t f3CapReached=0;std::int32_t f3CapPlayers=0;
    std::uint64_t f3Controller=0,f3State=0,f3Pawn=0;
    std::array<Snapshot,HistoryLimit> history{};
};
inline bool F3RosterFull(const Report& report)noexcept{return report.f3Refused!=0&&report.f3Reason==1;}
static_assert(std::is_trivially_copyable_v<Report>);
static_assert(sizeof(Report)<=65536,"Lobby dump must remain one bounded reader transfer");
inline bool Accept(const Report& a,const Report& b){return a.magic==Report::Magic&&a.version==Report::Version&&a.bytes==sizeof(Report)&&
    b.magic==a.magic&&b.version==a.version&&b.bytes==a.bytes&&!(a.generation&1u)&&a.generation==b.generation&&
    a.histories<=HistoryLimit&&a.current<HistoryLimit;}
}
