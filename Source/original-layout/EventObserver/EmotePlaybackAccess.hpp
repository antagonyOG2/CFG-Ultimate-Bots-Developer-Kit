#pragma once
#include "EmoteObservationAccess.hpp"
namespace cfgemoteobservation {
// Cached selection/catalogue proof has its own bounded reader. Native work
// still goes through the real tick's existing retained BundleAccess gateway;
// input observation never acquires this adapter or a native-call capability.
template<class Reader,class NativeHost>class PlaybackAccess:public Access<Reader> {
    NativeHost& native_;
public:
    PlaybackAccess(Reader& reader,const cfgobserver::Plan& saved,NativeHost& native):
        Access<Reader>(reader,saved),native_(native){}
    bool Context(){
#ifdef CFG_H1_JASON
        if(cfgprimary::CurrentHostedBinding().Matches(this->SavedPlan())){
            if(!native_.HostedContext(false))return false;
            this->context=native_.context;return true;
        }
#endif
        return Access<Reader>::Context();
    }
    bool MirrorHuman(cfgemotemirror::Slot& out){
#ifdef CFG_H1_JASON
        if(cfgprimary::CurrentHostedBinding().Matches(this->SavedPlan())){
            out=this->context.objects[3];return out.HasAssignedSerial();
        }
#endif
        return Access<Reader>::MirrorHuman(out);
    }
    bool Invoke(std::uintptr_t receiver,std::uintptr_t function,void* packet,bool actor){
        return native_.Invoke(receiver,function,packet,actor);
    }
    unsigned MirrorInterrupt(const cfgemotemirror::Recipient& target){return native_.MirrorInterrupt(target);}
    void MirrorPlayed(const cfgemotemirror::Recipient& target,std::uintptr_t montage){native_.MirrorPlayed(target,montage);}
};
}
