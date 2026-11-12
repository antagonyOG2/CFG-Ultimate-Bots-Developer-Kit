#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/FrontendControllerObservation.hpp"
namespace cfgmenu::screengate {
template<class Reader> bool Entry(Reader& reader,const active::Context& context){
    cfgnames::Observation<Reader> names(reader,context.module);cfgnames::Text label;
    return names.Object(context.type,label)&&label.View()==u"OfflinePlayMenuWidget_C";
}
template<class Reader> bool Settings(Reader& reader,const active::Context& context){
    // A valid current HUD alone does NOT establish the intended menu screen.
    // Run before loading the frontend DLL and again in its preparation.
    cfgnames::Observation<Reader> names(reader,context.module);cfgnames::Text label;
    return names.Object(context.type,label)&&label.View()==u"OfflineBotsSettingsMenuWidget_C";
}
}
