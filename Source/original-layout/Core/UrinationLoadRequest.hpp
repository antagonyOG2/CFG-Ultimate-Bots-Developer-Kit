#pragma once
#include <cstdint>
namespace cfgurination {
// Bookkeeping for the donor's ONE owned LoadAsset request. This record grants
// no lookup/native/publication authority; the runtime separately proves the
// actual native constructor/delegate boundary and retained owner on every use.
enum class LoadStage:unsigned {Empty,Prepared,Requesting,Waiting,Completing,Ready,Rejected};
struct LoadIdentity {
    std::uintptr_t world=0,controller=0;
    std::int32_t worldSerial=0,controllerSerial=0;
    std::uint32_t thread=0,uuid=0;
    std::uint64_t generation=0;
    bool Valid()const noexcept{return world&&controller&&worldSerial>0&&controllerSerial>0&&thread&&uuid&&generation;}
};
class LoadRequest {
    LoadIdentity owner_{};LoadStage stage_=LoadStage::Empty;
    std::uintptr_t action_=0;std::uint64_t deadline_=0;
public:
    LoadStage Stage()const noexcept{return stage_;}
    const LoadIdentity& Owner()const noexcept{return owner_;}
    std::uintptr_t Action()const noexcept{return action_;}
    bool Prepare(const LoadIdentity& owner,std::uint64_t now,bool inputsRetained,bool outputReserved)noexcept{
        if(stage_!=LoadStage::Empty||!owner.Valid()||!inputsRetained||!outputReserved||now>UINT64_MAX-30000)return false;
        owner_=owner;deadline_=now+30000;stage_=LoadStage::Prepared;return true;
    }
    bool Enter(bool current)noexcept{
        if(stage_!=LoadStage::Prepared||!current)return false;
        stage_=LoadStage::Requesting;return true;
    }
    bool Capture(std::uintptr_t action,std::uintptr_t returned,std::uint32_t thread,std::uint32_t uuid,
        std::uintptr_t controller,bool exactConstructorAndPath)noexcept{
        if(stage_!=LoadStage::Requesting||action_||!action||returned!=action||thread!=owner_.thread||
           uuid!=owner_.uuid||controller!=owner_.controller||!exactConstructorAndPath)return false;
        action_=action;return true;
    }
    bool RequestReturned(bool normalReturn,bool current)noexcept{
        if(stage_!=LoadStage::Requesting)return false;
        if(!normalReturn||!current||!action_){Reject();return false;}
        stage_=LoadStage::Waiting;return true;
    }
    bool BeginCompletion(std::uintptr_t action,std::uint32_t thread,std::uint64_t generation,
        std::uint64_t now,bool current,bool exactDelegateAndHandle,bool completed,bool cancelled)noexcept{
        if(stage_!=LoadStage::Waiting||!action_||action!=action_||thread!=owner_.thread||generation!=owner_.generation)return false;
        if(now>=deadline_||!current||!exactDelegateAndHandle||!completed||cancelled){Reject();return false;}
        // Consume BEFORE registry work or forwarding can reenter. No raw loaded
        // UObject address enters this record, including failed publication.
        stage_=LoadStage::Completing;return true;
    }
    bool Complete(bool published)noexcept{
        if(stage_!=LoadStage::Completing)return false;
        action_=0;stage_=published?LoadStage::Ready:LoadStage::Rejected;return published;
    }
    void ObserveDestroyed(std::uintptr_t action)noexcept{
        if(action_&&action==action_&&stage_!=LoadStage::Ready)Reject();
    }
    void Expire(std::uint64_t now,bool current)noexcept{
        if(stage_!=LoadStage::Empty&&stage_!=LoadStage::Ready&&stage_!=LoadStage::Rejected&&(!current||now>=deadline_))Reject();
    }
    void Reject()noexcept{action_=0;stage_=LoadStage::Rejected;}
};
}
