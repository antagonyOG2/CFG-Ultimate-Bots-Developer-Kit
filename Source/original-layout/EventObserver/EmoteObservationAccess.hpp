#pragma once
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
namespace cfgemoteobservation {
// Before/after native input events can OBSERVE a fresh living local counselor;
// they cannot borrow a completed-tick lease to warm functions, pin assets or
// invoke emotes. Existing mirror policy merely queues work for a later real tick.
template<class Reader>class Access {
    Reader& r_;const cfgobserver::Plan& saved_;
public:
    cfgobserver::Plan context{};
    Access(Reader& r,const cfgobserver::Plan& saved):r_(r),saved_(saved),context(saved){}
protected:
    const cfgobserver::Plan& SavedPlan()const{return saved_;}
public:
    bool Context(){
        if(GetCurrentThreadId()!=saved_.threadId||!cfgprimary::LookupVetoClearForObservation(r_,saved_.module))return false;
        context=saved_;return true;
    }
    bool MirrorHuman(cfgemotemirror::Slot& out){out=saved_.objects[3];return out.HasAssignedSerial()&&out.object==saved_.pawn;}
    bool FeatureFunction(const cfgvehicle::disk::Binding& b,std::uintptr_t& function,std::uintptr_t& owner){
        function=owner=0;std::uintptr_t ctor=0;std::uint16_t bytes=0;cfgreflection::Field field;
        cfgreflection::Observation<Reader> reflection(r_,saved_.module);
        const auto label=b.cache==cfgemotemirror::disk::SERVER_PlayEmote.cache?u"SERVER_PlayEmote":
            b.cache==cfgemotemirror::disk::SERVER_CancelEmote.cache?u"SERVER_CancelEmote":u"";
        if(!*label||!r_.Read(saved_.module+b.ownerReflection,owner)||!r_.Read(saved_.module+b.ownerConstructor,ctor)||
           !cfgprobe::MatchingClassCache(owner,ctor)||!cfgbundle::Code(r_,saved_.module,b.factory,b.code)||
           !r_.Read(saved_.module+b.cache,function)||!function||
           !reflection.ReadField(function,owner,field)||field.kind.View()!=u"Function"||field.label.View()!=label||
           !r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||bytes!=b.nativeBytes||
           !cfgobserver::Same(r_,saved_.module+b.cache,function)||
           !cfgobserver::Same(r_,saved_.module+b.ownerReflection,owner)||!cfgobserver::Same(r_,saved_.module+b.ownerConstructor,owner))return false;
        return true; // cached metadata only, NO factory/retention/dispatch
    }
};
}
