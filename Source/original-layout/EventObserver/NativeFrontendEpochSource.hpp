#pragma once
#include "../RuntimeProbe/NativePickerSourceObservation.hpp"
#include "../RuntimeProbe/FrontendControllerObservation.hpp"
namespace cfgmenu::epochsource {
// Explicit epoch adapter, NOT a relaxation of Retained or ordinary picker
// guards. Caller supplies a positively observed actual post-game frontend top.
// Class/graph/reference identities must survive; optional parent widget changes.
template<class R>bool Rebind(R& r,const active::Context& actual,const pickersource::Resources& saved,pickersource::Resources& out){
 out={};const auto& previous=saved.source;
 if(actual.module!=previous.module||actual.engine!=previous.engine||actual.viewport!=previous.viewport||
    actual.player!=previous.player||actual.gameInstance!=previous.gameInstance||actual.thread!=previous.thread||
    actual.type!=previous.type||actual.array.count<2||!controllergate::Menu(r,actual.module,actual.controller)||
    !active::Current(r,actual,actual.thread))return false;
 auto staged=saved;staged.source=actual;parentcapture::Record parent;
 // Rebuilt native frontend stacks may retain an extra transition underneath
 // MainMenu -> OfflinePlay. Prove the immediate retained parent, not depth2.
 // Its exact authored class/graphs remain required; unrelated tops refuse.
 if(!parentcapture::Observe(r,actual,actual.array.count-2,parent)||parent.menuClass!=saved.root.menuClass)return false;
 staged.root=parent;if(!pickersource::Retained(r,actual,staged,actual.thread))return false;
 out=staged;return true;
}
}
