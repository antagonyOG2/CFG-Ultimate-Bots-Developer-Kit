#pragma once
#include "NativeClassPathChoice.hpp"
#include "SelectedJasonClassObservation.hpp"
#include "NativeProfileSelectionObservation.hpp"
#include "../Core/SetupMenuGameplayHandoff.hpp"
namespace cfgmenu::launchselection {
constexpr unsigned ReadLimit=16384;
struct Observation {MenuCatalogueEvidence evidence{};maps::Selection map{};classchoice::Choice jason{};};
struct Diagnostic {unsigned phase=1,mapFailure=0,jasonRoute=0;int selectedClassSerial=0;profiles::Diagnostic profile{};classchoice::Diagnostic jason{};};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
// One synchronous owner-thread selection acquisition. Never retain a serial0
// save as an engine target. Legitimately unset CounselorPick remains deferred
// to native pre-birth selection, NOT a default counselor/possession repair.
// Existing CFG native map/Jason pickers remain authoritative.
template<class Reader> bool Observe(Reader& reader,const active::Context& context,std::uint32_t callerThread,
    std::uint64_t generation,std::uint64_t catalogueGeneration,unsigned maximumBots,bool exactImageVerified,
    Observation& out,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    out={};Bounded<Reader> r(reader);Diagnostic detail;
    const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;if(okay)detail.phase=0;if(diagnostic)*diagnostic=detail;return okay;};
    if(!exactImageVerified||callerThread!=context.thread||!generation||!catalogueGeneration||maximumBots>31)return finish(false);
    maps::Selection map;profiles::Selection profile;catalogues::Characters roster;std::uintptr_t killer=0,ctor=0;
    maps::Diagnostic mapDetail;
    detail.phase=2;
    if(!maps::Observe(r,context,callerThread,true,map,nullptr,&mapDetail)){
        detail.mapFailure=static_cast<unsigned>(mapDetail.failure);return finish(false);}
    detail.phase=3;
    if(!profiles::ObserveSnapshot(r,context,callerThread,true,profile,nullptr,&detail.profile))return finish(false);
    detail.phase=4;if(!profile.jason)return finish(false);
    detail.phase=5;
    if(!r.Read(context.module+cataloguedisk::SCKillerCharacterClass,killer)||
        !r.Read(context.module+cataloguedisk::SCKillerCharacterConstructor,ctor)||!cfgprobe::MatchingClassCache(killer,ctor))return finish(false);
    detail.phase=6;if(!catalogues::ObserveCharacters(r,context,true,roster))return finish(false);
    classchoice::Choice selected;cfgselectedjason::Choice explicitClass;
    detail.phase=7;
    if(!classchoice::MatchLoaded(r,context.module,profile.jason,killer,roster.jasons,selected,nullptr,&detail.jason)){
        // Only a complete, stable catalogue with no exact path match may use
        // the explicit loaded-class route. A failed read is never absence.
        cfgprobe::lifetime::ArrayHeader registry;
        if(detail.jason.stage!=7||detail.jason.matches||
            !cfgprobe::lifetime::ReadArrayHeader(r,context.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
            !cfgselectedjason::Observe(r,context.module,registry,profile.jason,killer,explicitClass))return finish(false);
        selected.type=profile.jason;selected.index=cfgselectedjason::NoCatalogueIndex;
    }
    detail.jasonRoute=explicitClass.Valid()?2u:1u;detail.selectedClassSerial=explicitClass.identity.serial;
    // Non-null counselor must also be the native counselor class family.
    detail.phase=8;if(profile.counselor){std::uintptr_t counselor=0,counselorCtor=0;
        if(!r.Read(context.module+cataloguedisk::SCCounselorCharacterClass,counselor)||
            !r.Read(context.module+cataloguedisk::SCCounselorCharacterConstructor,counselorCtor)||
            !cfgprobe::MatchingClassCache(counselor,counselorCtor)||!maps::Subclass(r,profile.counselor,counselor))return finish(false);}
    profiles::Selection final;maps::Selection finalMap;catalogues::Characters finalRoster;std::uintptr_t current=0;
    detail.phase=9;
    if(!profiles::ObserveSnapshot(r,context,callerThread,true,final,nullptr,&detail.profile)||final.save!=profile.save||final.saveType!=profile.saveType||
        !(final.identity==profile.identity)||!(final.cache==profile.cache)||final.counselor!=profile.counselor||final.jason!=profile.jason)return finish(false);
    detail.phase=10;
    if(!maps::Observe(r,context,callerThread,true,finalMap,nullptr,&mapDetail)){
        detail.mapFailure=static_cast<unsigned>(mapDetail.failure);return finish(false);}
    if(finalMap.map!=map.map||finalMap.mode!=map.mode||finalMap.entries!=map.entries)return finish(false);
    detail.phase=11;
    if(!catalogues::ObserveCharacters(r,context,true,finalRoster)||
        finalRoster.source!=roster.source||finalRoster.type!=roster.type||!(finalRoster.jasons==roster.jasons))return finish(false);
    detail.phase=12;
    if(!r.Read(context.module+cataloguedisk::SCKillerCharacterClass,current)||current!=killer||
        !r.Read(context.module+cataloguedisk::SCKillerCharacterConstructor,current)||current!=ctor||
        !active::Current(r,context,callerThread))return finish(false);
    if(explicitClass.Valid()){cfgprobe::lifetime::ArrayHeader registry;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,context.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
            !cfgselectedjason::Current(r,context.module,registry,killer,explicitClass))return finish(false);}
    Observation value;value.map=map;value.jason=selected;
    auto& evidence=value.evidence;
    evidence.frontend=context.widget;evidence.frontendWorld=context.world;evidence.generation=generation;
    evidence.catalogueGeneration=catalogueGeneration;evidence.maximumBots=maximumBots;evidence.map=map.map;evidence.jason=selected.type;
    evidence.counselor=profile.counselor;evidence.preserveNativeCounselor=!profile.counselor;evidence.selectedJason=selected.packet;
    evidence.selectedClass=explicitClass;
    evidence.cfgMetadata=evidence.ownedSelections=true;out=value;return finish(true);
}
}
