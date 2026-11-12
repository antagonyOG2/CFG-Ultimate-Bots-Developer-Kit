#pragma once
#include "../RuntimeProbe/PickerPrecreationObservation.hpp"
#include "../RuntimeProbe/OfflineEntrySettingsObservation.hpp"
namespace cfgmenu::pickerarrival {
struct Diagnostic {
    unsigned stage=0,metadataStage=0,sourceStage=0,entryStage=0,refreshStage=0;
    entrysettings::ResetDiagnostic reset{};
};
// One complete picker metadata proof per branch. OwnedReceiverCurrent already
// includes Current through ReceiverCurrent; repeating Current before it spends
// the same shared read budget twice without adding an ownership condition.
// This observes an actual native arrival only; it grants no route token.
template<class Reader> bool Current(Reader& r,const active::Context& actual,const pickerprepare::Prepared& prepared,
    const pickerprepare::Executors& expected,const pickerprepare::Receiver& receiver,const entrysettings::Resources& entry,
    std::uint32_t thread,Diagnostic* diagnostic=nullptr){
    Diagnostic local;auto& d=diagnostic?*diagnostic:local;d={};d.stage=1;
    if(prepared.kind!=pickerprepare::Kind::Counselor||!prepared.type)return false;
    if(actual.type==prepared.type){
        d.stage=2;
        if(actual.widget!=receiver.widget||!pickerprepare::OwnedReceiverCurrent(r,actual,prepared,expected,receiver,thread))return false;
    }else{
        d.stage=3;
        if(actual.type!=entry.map&&actual.type!=entry.settings)return false;
        if(!pickerprepare::Current(r,actual,prepared,thread,expected,false,&d.metadataStage,&d.sourceStage))return false;
    }
    d.stage=4;
    if(!entrysettings::Retained(r,actual,entry,&d.entryStage,&d.reset))return false;
    d.stage=0;return true;
}
// A native arrival is ONE staged transaction. Previously the runtime refreshed
// and fully proved these resources, committed them, and immediately repeated
// that same picker proof in Coordinator::Arrive. The second observation could
// reject before custom settings were adopted even though preparation passed.
// Keep all source/metadata/entry/active checks, but commit only once both staged
// proofs pass. This never gives Start a reusable proof: Submit still calls the
// independent read-only Current above on its actual borrowed settings event.
template<class Reader> bool Initialize(Reader& r,const active::Context& actual,pickerprepare::Prepared& prepared,
    const pickerprepare::Executors& expected,const pickerprepare::Receiver& receiver,entrysettings::Resources& entry,
    std::uint32_t thread,Diagnostic* diagnostic=nullptr){
    Diagnostic local;auto& d=diagnostic?*diagnostic:local;d={};d.stage=1;
    if(prepared.kind!=pickerprepare::Kind::Counselor||!prepared.type)return false;
    if(actual.type==prepared.type)return Current(r,actual,prepared,expected,receiver,entry,thread,&d);
    d.stage=3;if(actual.type!=entry.map&&actual.type!=entry.settings)return false;
    auto picker=std::make_unique<pickerprepare::Prepared>(prepared);
    auto sources=std::make_unique<entrysettings::Resources>(entry);
    if(!pickerprepare::InitializedMetadata(r,actual,*picker,expected,thread,&d.refreshStage,&d.metadataStage,&d.sourceStage))return false;
    d.stage=4;
    if(!entrysettings::FirstNativeArrival(r,actual,*sources,&d.entryStage)){d.reset=sources->reset;return false;}
    d.reset=sources->reset;prepared=*picker;entry=*sources;d.stage=0;return true;
}
}
