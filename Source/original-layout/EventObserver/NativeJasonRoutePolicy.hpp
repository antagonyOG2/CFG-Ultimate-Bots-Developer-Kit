#pragma once
#include "../RuntimeProbe/JasonPickerObservation.hpp"
#include <type_traits>
namespace cfgmenu::jasonpicker {
// Records the original native accept transaction before its synchronous
// transition, like the donor's accept ordering, WITHOUT substituting an index
// mapping, writing a profile, changing destination or invoking any menu.
class NativeAcceptScope {
    bool accepting_=false;unsigned accepts_=0;
public:
    bool Accepting()const noexcept{return accepting_;}unsigned Accepts()const noexcept{return accepts_;}
    template<class Host,class Forward> bool Run(Host& host,Callback callback,Forward&& forward){
        static_assert(std::is_nothrow_invocable_r_v<bool,Forward>,"Native forwarding must be noexcept and protect native failures");
        if(accepting_||!host.Current()||!host.Owned(callback)||(callback!=Callback::Accept&&callback!=Callback::OfflineAccept))return forward();
        accepting_=true;++accepts_;const bool result=forward();accepting_=false;return result;
    }
    // A proved settings event can be handed to settings preparation only while
    // a native accept is in flight. This is observation, NOT permission to
    // rewrite the already-started OnPlayTransitionOut event or to call Start.
    template<class Host> bool SettingsObserved(Host& host,const Reference& destination)const{
        return accepting_&&host.Current()&&host.Owned(Callback::Transition)&&host.ReferenceCurrent(destination)&&
            destination.destination==Destination::Settings&&destination.name.View()==u"OfflineBotsSettingsMenuWidget_C";
    }
};
}
