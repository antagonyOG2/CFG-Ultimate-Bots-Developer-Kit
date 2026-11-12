#pragma once
#include "Compatibility/CfgSummary35.generated.hpp"
// Existing feature names forward to the verified CFG3.5 compatibility definition.
namespace cfgsummarypresentation::disk {
using cfgcompat::cfg35::summary::StateReady;
using cfgcompat::cfg35::summary::StateReadyCode;
using cfgcompat::cfg35::summary::StatusText;
using cfgcompat::cfg35::summary::StatusTextCode;
using cfgcompat::cfg35::summary::StatusPanel;
using cfgcompat::cfg35::summary::StatusPanelCode;
using cfgcompat::cfg35::summary::CharacterPanel;
using cfgcompat::cfg35::summary::CharacterPanelCode;
using cfgcompat::cfg35::summary::TextReturn;
using cfgcompat::cfg35::summary::PanelReturn;
using cfgcompat::cfg35::summary::CharacterReturn;
inline bool Caller(std::uintptr_t module,std::uintptr_t address){return address==module+TextReturn||address==module+PanelReturn||address==module+CharacterReturn;}
}
