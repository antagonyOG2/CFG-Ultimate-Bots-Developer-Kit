#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/ReflectionLayout.generated.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include <array>

namespace cfgkill {
// Cold discovery only: a typed, retained primary Jason class and an independently
// FeatureFunction-validated Actor.ReceiveAnyDamage are required at Begin. No name
// decoding, global-object search, allocation, native calls or writes occur here.
// All reads use the caller's existing budget. At most 16 fields/classes/functions
// are inspected per Advance. The caller must run this OUTSIDE ProcessEvent and
// invalidate it when its retained world/primary Jason identity changes.
// Matches is intentionally a zero-read event gate. Validate is only for that one
// selected function AFTER Matches, and samples known slots/header schema only.
// Metadata is not GC-pinned: these observations require the host's game-thread,
// offline, positive-lifetime guards; they are not an independent call permission.
class DamageGate {
public:
    enum class State : unsigned { Empty, Scan, VerifyFields, VerifyOwners, ParentChain, Ready, Unavailable };
    // Native CFG ancestry already has 961 fields before Blueprint classes.
    // Capacity is cold storage only; each callback still reads at most 16.
    static constexpr unsigned PerPhase=16,MaxOwners=32,MaxFields=4096;
private:
    using Array=cfgprobe::lifetime::ArrayHeader;
    using Slot=cfgprobe::lifetime::SlotObservation;
    struct Field {
        Slot slot{};std::uintptr_t type=0,outer=0,next=0;cfgnames::Words name{};
        bool operator==(const Field& b)const{return slot==b.slot&&type==b.type&&outer==b.outer&&next==b.next&&name==b.name;}
    };
    struct Owner {
        Slot slot{};std::uintptr_t type=0,parent=0,children=0;
        bool operator==(const Owner& b)const{return slot==b.slot&&type==b.type&&parent==b.parent&&children==b.children;}
    };
    struct Function {
        Field field{};std::uintptr_t parent=0;std::uint32_t flags=0,storage=0;std::uint16_t bytes=0;
        bool operator==(const Function& b)const{return field==b.field&&parent==b.parent&&flags==b.flags&&storage==b.storage&&bytes==b.bytes;}
    };
    State state_=State::Empty;Array array_{};Function native_{};
    std::array<Owner,MaxOwners> owners_{};std::array<Field,MaxFields> fields_{};
    std::array<Function,MaxOwners> functions_{};
    unsigned ownerCount_=0,fieldCount_=0,functionCount_=0,verify_=0,lastFunctionOwner_=0;
    std::uintptr_t root_=0,actor_=0,currentOwner_=0,currentField_=0,localMatch_=0,selected_=0,parentFunction_=0;
    bool openOwner_=false;
    State failedPhase_=State::Empty;
    State Fail(){if(state_!=State::Unavailable)failedPhase_=state_;selected_=0;return state_=State::Unavailable;}
    bool Header(const Array& a)const{return a.Sane()&&a.data==array_.data&&a.count>=array_.count;}
    template<class Reader> static bool Live(Reader& r,const Array& a,std::uintptr_t p,Slot& slot){
        std::uint32_t flags=0;
        // UClass/UField/UFunction metadata is strongly reached through the
        // retained Jason class -> owner/children chain and pinned native cache.
        // UE4 assigns weak-reference serials lazily; zero is legal metadata,
        // NOT permission to adopt a zero-serial pawn or a free-standing object.
        // Exact slot/type/Outer/Next/name/schema two-pass equality remains.
        return cfgprobe::lifetime::ReadSlot(r,a,p,slot)&&
            r.Read(a.data+static_cast<std::uintptr_t>(slot.index)*cfgprobe::lifetime::ItemBytes+8,flags)&&!(flags&0x20000000u);
    }
    template<class Reader> static bool ReadField(Reader& r,const Array& a,std::uintptr_t p,Field& out){
        return Live(r,a,p,out.slot)&&r.Read(p+cfgprobe::ObjectClass,out.type)&&cfgprobe::ObjectPointer(out.type)&&
            r.Read(p+0x20,out.outer)&&r.Read(p+cfgreflection::disk::Next,out.next)&&
            r.Read(p+cfgnames::disk::ObjectName,out.name)&&out.name.index>=0&&out.name.number>=0;
    }
    template<class Reader> static bool ReadOwner(Reader& r,const Array& a,std::uintptr_t p,Owner& out){
        return Live(r,a,p,out.slot)&&r.Read(p+cfgprobe::ObjectClass,out.type)&&cfgprobe::ObjectPointer(out.type)&&
            r.Read(p+cfgprobe::ClassSuper,out.parent)&&r.Read(p+cfgreflection::disk::Children,out.children);
    }
    template<class Reader> static bool ReadFunction(Reader& r,const Array& a,std::uintptr_t p,Function& out){
        return ReadField(r,a,p,out.field)&&r.Read(p+cfgprobe::ClassSuper,out.parent)&&
            r.Read(p+cfgreflection::disk::FunctionFlags,out.flags)&&
            r.Read(p+cfgreflection::disk::StorageBytes,out.storage)&&
            r.Read(p+cfgreflection::arraydisk::FunctionParameterBytes,out.bytes);
    }
    bool Schema(const Function& f)const{
        // Event + BlueprintEvent are inherited semantic flags. Never accept a
        // same-named RPC. Other legitimate override flags are snapshotted exactly.
        return f.field.type==native_.field.type&&f.field.name==native_.field.name&&
            f.bytes==32&&f.storage>=32&&f.storage<=65536&&
            (f.flags&0x08000800u)==0x08000800u&&!(f.flags&0x40u);
    }
public:
    State Status()const noexcept{return state_;}
    State FailedPhase()const noexcept{return failedPhase_;}
    unsigned ObservedFields()const noexcept{return fieldCount_;}
    unsigned ObservedOwners()const noexcept{return ownerCount_;}
    std::uintptr_t FunctionToken()const noexcept{return state_==State::Ready?selected_:0;}
    std::uintptr_t ClassToken()const noexcept{return root_;}
    bool Matches(std::uintptr_t function)const noexcept{return function&&function==FunctionToken();}
    // External cold prerequisites (native factory/owned class) can fail before
    // Begin. Latch those failures too; do not retry discovery each gameplay lane.
    State Refuse(){return Fail();}
    void Reset(){*this={};}
    template<class Reader> bool Begin(Reader& r,const Array& a,std::uintptr_t typedJasonClass,
                                      std::uintptr_t validatedNativeFunction,std::uintptr_t nativeActorClass){
        Reset();array_=a;root_=currentOwner_=typedJasonClass;actor_=nativeActorClass;
        if(!a.Sane()||!cfgprobe::ObjectPointer(root_)||!cfgprobe::ObjectPointer(actor_)||
            !ReadFunction(r,a,validatedNativeFunction,native_)||native_.field.outer!=actor_||
            native_.parent||native_.bytes!=32||native_.storage!=32||
            native_.flags!=disk::ReceiveAnyDamageFlags||!Schema(native_)){Fail();return false;}
        state_=State::Scan;return true;
    }
    template<class Reader> State Advance(Reader& r,const Array& a){
        if(state_==State::Empty||state_==State::Ready||state_==State::Unavailable)return state_;
        if(!Header(a))return Fail();
        if(state_==State::Scan){
            if(!openOwner_){
                if(!currentOwner_||ownerCount_==MaxOwners)return Fail();
                for(unsigned i=0;i<ownerCount_;++i)if(owners_[i].slot.object==currentOwner_)return Fail();
                Owner owner{};if(!ReadOwner(r,a,currentOwner_,owner))return Fail();
                owners_[ownerCount_++]=owner;currentField_=owner.children;localMatch_=0;openOwner_=true;
                return state_; // Owner preparation is its own bounded phase.
            }
            for(unsigned n=0;n<PerPhase&&currentField_;++n){
                if(fieldCount_==MaxFields)return Fail();
                for(unsigned i=0;i<fieldCount_;++i)if(fields_[i].slot.object==currentField_)return Fail();
                Field field{};if(!ReadField(r,a,currentField_,field)||field.outer!=currentOwner_)return Fail();
                fields_[fieldCount_++]=field;
                if(field.name==native_.field.name){
                    if(localMatch_||field.type!=native_.field.type)return Fail();
                    localMatch_=currentField_;
                }
                currentField_=field.next;
            }
            if(currentField_)return state_;
            if(!selected_&&localMatch_)selected_=localMatch_;
            if(currentOwner_==actor_){
                if(localMatch_!=native_.field.slot.object||!selected_)return Fail();
                verify_=0;state_=State::VerifyFields;
            }else {currentOwner_=owners_[ownerCount_-1].parent;openOwner_=false;}
            return state_;
        }
        if(state_==State::VerifyFields){
            for(unsigned n=0;n<PerPhase&&verify_<fieldCount_;++n,++verify_){
                Field fresh{};if(!ReadField(r,a,fields_[verify_].slot.object,fresh)||!(fresh==fields_[verify_]))return Fail();
            }
            if(verify_==fieldCount_){verify_=0;state_=State::VerifyOwners;}return state_;
        }
        if(state_==State::VerifyOwners){
            for(unsigned n=0;n<PerPhase&&verify_<ownerCount_;++n,++verify_){
                Owner fresh{};if(!ReadOwner(r,a,owners_[verify_].slot.object,fresh)||!(fresh==owners_[verify_]))return Fail();
            }
            if(verify_==ownerCount_){parentFunction_=selected_;state_=State::ParentChain;}return state_;
        }
        if(state_==State::ParentChain){
            for(unsigned n=0;n<PerPhase;++n){
                if(!parentFunction_||functionCount_==MaxOwners)return Fail();
                for(unsigned i=0;i<functionCount_;++i)if(functions_[i].field.slot.object==parentFunction_)return Fail();
                Function function{};if(!ReadFunction(r,a,parentFunction_,function)||!Schema(function))return Fail();
                unsigned owner=0;while(owner<ownerCount_&&owners_[owner].slot.object!=function.field.outer)++owner;
                if(owner==ownerCount_||(functionCount_&&owner<=lastFunctionOwner_))return Fail();
                // It must actually be an observed field of that owner, not an
                // object with fabricated Outer/super/name headers.
                bool found=false;for(unsigned i=0;i<fieldCount_;++i)if(fields_[i]==function.field){found=true;break;}
                if(!found)return Fail();
                functions_[functionCount_++]=function;lastFunctionOwner_=owner;
                if(parentFunction_==native_.field.slot.object){
                    if(!(function==native_))return Fail();return state_=State::Ready;
                }
                parentFunction_=function.parent;
            }
        }
        return state_;
    }
    template<class Reader> bool Validate(Reader& r,const Array& a,std::uintptr_t currentJasonClass,std::uintptr_t function){
        // Pointer mismatch must perform zero reads, including no registry lookup.
        if(!Matches(function))return false;
        if(currentJasonClass!=root_||!Header(a)){Fail();return false;}
        for(unsigned i=0;i<ownerCount_;++i){
            Owner fresh{};if(!ReadOwner(r,a,owners_[i].slot.object,fresh)||!(fresh==owners_[i])){Fail();return false;}
        }
        for(unsigned i=0;i<functionCount_;++i){
            Function fresh{};if(!ReadFunction(r,a,functions_[i].field.slot.object,fresh)||!(fresh==functions_[i])){Fail();return false;}
        }
        return true;
    }
};
}
