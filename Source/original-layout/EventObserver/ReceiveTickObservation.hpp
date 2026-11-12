#pragma once
#include "SpawnAssetObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/TickCallbackBindings.generated.hpp"
namespace cfgtick {
enum class Failure : std::uint32_t { None,Identity,Wrapper,NameGlobal,FunctionName,ParameterSize,
    RegistryBefore,ReceiverSlot,FunctionSlot,RegistryStable,FinalIdentity,FunctionFlagsRead,FunctionFlagged };
constexpr bool CompletedWorkCandidate(bool ownerThread,bool nested,bool returned,
    bool ownedActor,bool spawnPending) noexcept {
    return ownerThread&&!nested&&returned&&ownedActor&&!spawnPending;
}
// This observes callback identity only. The caller must independently enforce
// execution eligibility, owner thread, outermost scope and original completion.
template<class Reader>bool Observe(Reader& r,std::uintptr_t module,std::uintptr_t receiver,
    std::uintptr_t function,std::uintptr_t returnPc,Failure* failure=nullptr){
    const auto fail=[&](Failure stage){if(failure)*failure=stage;return false;};
    if(failure)*failure=Failure::Identity;
    if(returnPc!=module+disk::ReturnPc||!cfgprobe::ObjectPointer(receiver)||!cfgprobe::ObjectPointer(function))return fail(Failure::Identity);
    std::array<unsigned char,disk::WrapperCode.size()> code{};
    if(!r.Read(module+disk::Wrapper,code)||code!=disk::WrapperCode)return fail(Failure::Wrapper);
    cfgnames::Words name{},expected{},again{};std::uint16_t bytes=0;
    cfgprobe::lifetime::ArrayHeader before{},after{};
    cfgprobe::lifetime::SlotObservation pawnSlot{},functionSlot{},fresh{};
    if(!r.Read(module+disk::Name,expected)||expected.index<=0||expected.number)return fail(Failure::NameGlobal);
    if(!r.Read(function+cfgnames::disk::ObjectName,name)||!(name==expected))return fail(Failure::FunctionName);
    if(!r.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||bytes!=sizeof(float))return fail(Failure::ParameterSize);
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,before))return fail(Failure::RegistryBefore);
    if(!cfgspawn::ObjectSlot(r,before,receiver,pawnSlot)||!pawnSlot.HasAssignedSerial())return fail(Failure::ReceiverSlot);
    // `function` is the borrowed UFunction argument of the still-active native
    // dispatch frame (the probe runs in the detour's finally block before that
    // frame returns). Unlike the pawn, it is not retained or used after this
    // callback. Some valid generated ReceiveTick functions have an unassigned
    // FUObjectItem serial, so require exact array membership/slot identity and
    // revalidate it below instead of treating serial zero as a lifetime pin.
    if(!cfgprobe::lifetime::ReadSlot(r,before,function,functionSlot))return fail(Failure::FunctionSlot);
    std::uint32_t functionFlags=0;
    const auto functionItem=before.data+static_cast<std::uintptr_t>(functionSlot.index)*cfgprobe::lifetime::ItemBytes;
    if(!r.Read(functionItem+8,functionFlags))return fail(Failure::FunctionFlagsRead);
    if(functionFlags&0x20000000)return fail(Failure::FunctionFlagged);
    // Ordinary UObject allocations inside the completed engine callback may
    // grow or relocate the global registry. Permit only a sane endpoint
    // header, then prove these exact two objects still occupy their assigned
    // slots with their original serials in that endpoint storage.
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,after)||
       after.count<before.count)return fail(Failure::RegistryStable);
    if(!cfgspawn::ObjectSlot(r,after,receiver,fresh)||!(fresh==pawnSlot)||
       !cfgspawn::ObjectSlot(r,after,function,fresh)||!(fresh==functionSlot))return fail(Failure::RegistryStable);
    cfgprobe::lifetime::ArrayHeader final{};
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,final)||!(final==after))
        return fail(Failure::RegistryStable);
    if(!r.Read(module+disk::Name,again)||!(again==expected)||
       !r.Read(function+cfgnames::disk::ObjectName,again)||!(again==name)||
       !r.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||bytes!=sizeof(float))return fail(Failure::FinalIdentity);
    if(failure)*failure=Failure::None;
    return true;
}
}
