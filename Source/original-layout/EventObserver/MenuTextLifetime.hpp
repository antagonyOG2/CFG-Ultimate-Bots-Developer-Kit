#pragma once
#include "MenuReflectionParameters.hpp"
namespace cfgmenu::reflection {
// CFG-specific opaque packets, NOT a native-call provider. Owner-thread-only.
// Keep this pool resident until process exit. Never memcpy Text into a new owner,
// decrement its shared references, free it, or reuse a slot after dispatch begins.
// A verified provider must validate the exact functions/objects/thread before it
// dispatches these buffers. Exceptional/ambiguous calls consume a slot forever.
struct alignas(8) OpaqueText { std::array<unsigned char,24> bytes{}; };
struct TextConversion { String input{};OpaqueText result{}; };
static_assert(sizeof(TextConversion)==40&&offsetof(TextConversion,result)==16);
class RetainedMenuText {
public:
    // Fifteen setup rows may expose up to32 labels each, plus their titles.
    // Reserve once, reuse across visits; never grow/allocate this pool in play.
    static constexpr unsigned Capacity=512;
private:
    enum class State { Empty,Pending,Ready,Abandoned };
    struct Slot {std::array<char16_t,128> input{};TextConversion packet{};State state=State::Empty;};
    std::array<Slot,Capacity> slots_{};
    unsigned used_=0;
public:
    struct Request {unsigned slot=Capacity;TextConversion* packet=nullptr;};
    RetainedMenuText()=default;
    RetainedMenuText(const RetainedMenuText&)=delete;
    RetainedMenuText& operator=(const RetainedMenuText&)=delete;
    RetainedMenuText(RetainedMenuText&&)=delete;
    RetainedMenuText& operator=(RetainedMenuText&&)=delete;
    bool Begin(std::u16string_view label,Request& out){
        out={};if(used_==slots_.size()||label.empty()||label.size()>=128||label.find(char16_t{0})!=std::u16string_view::npos)return false;
        auto& slot=slots_[used_];std::memcpy(slot.input.data(),label.data(),label.size()*2);
        const auto count=static_cast<std::int32_t>(label.size()+1);
        slot.packet.input={slot.input.data(),count,count};slot.state=State::Pending;
        out={used_++,&slot.packet};return true;
    }
    // Only after normal return from the validated native conversion. On any
    // exception/refusal/ambiguity pass false; do not retry or clear native output.
    bool Complete(const Request& request,bool returnedNormally){
        if(request.slot>=used_)return false;auto& slot=slots_[request.slot];
        if(request.packet!=&slot.packet||slot.state!=State::Pending)return false;
        slot.state=State::Abandoned;
        std::uintptr_t data=0,reference=0;
        std::memcpy(&data,slot.packet.result.bytes.data(),8);std::memcpy(&reference,slot.packet.result.bytes.data()+8,8);
        if(!returnedNormally||!Pointer(data)||!Pointer(reference))return false;
        slot.state=State::Ready;return true;
    }
    // Borrow the SAME native-owned conversion output as the 24-byte SetText
    // parameter buffer. No fabricated second shared-reference owner is created.
    // Pointer shape is not runtime identity or safe-execution proof.
    const OpaqueText* Borrow(const Request& request)const{
        if(request.slot>=used_)return nullptr;const auto& slot=slots_[request.slot];
        return request.packet==&slot.packet&&slot.state==State::Ready?&slot.packet.result:nullptr;
    }
    unsigned Used()const{return used_;}
    // Reuse the same retained native FText for fixed setup labels across menu
    // visits. Never copy/refcount/fabricate another FText owner. Conversion and
    // any failed/ambiguous output are not reusable.
    bool Find(std::u16string_view label,Request& out)const{
        out={};
        for(unsigned i=0;i<used_;++i){const auto& slot=slots_[i];
            if(slot.state!=State::Ready||slot.packet.input.count!=static_cast<std::int32_t>(label.size()+1))continue;
            if(std::u16string_view(slot.input.data(),label.size())==label){
                out={i,const_cast<TextConversion*>(&slot.packet)};return true;
            }
        }
        return false;
    }
};
}
