#pragma once
#include <string_view>
namespace cfgmenu::challengecapture {
inline bool Screen(std::u16string_view value){
    return value==u"SP_LobbyWidget_C"||value==u"SPChallengesSettingsMenuWidget_C";
}
inline bool Function(std::u16string_view value){
    return value==u"ExecuteUbergraph_SP_LobbyWidget"||value==u"OnStartPressed"||value==u"OnClicked_Start"||
        value==u"OnClicked_Back"||value==u"Construct"||value==u"KillerSelect"||value==u"NextChallenge"||
        value==u"PrevChallenge"||value==u"BeginChallenge"||value==u"StartMatch"||value==u"StartIntroSequence"||
        value==u"HandleMatchIsWaitingToStart"||value==u"HandleMatchHasStarted"||value==u"EndMatch"||
        value==u"ExecuteUbergraph_SPChallengesSettingsMenuWidget";
}
}
