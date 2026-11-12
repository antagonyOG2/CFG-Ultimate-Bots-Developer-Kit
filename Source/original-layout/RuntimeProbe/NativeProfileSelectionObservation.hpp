#pragma once
#include "NativeMapSelectionObservation.hpp"
namespace cfgmenu::profiles {
constexpr unsigned MaximumSaves=32,ReadLimit=8192;
struct Selection {
    std::uintptr_t save=0,saveType=0,defaults=0,counselor=0,jason=0;maps::Array cache{};
    cfgprobe::lifetime::SlotObservation identity{};
};
struct Diagnostic {const char* stage="image_or_current_owner";unsigned reads=0,matches=0;
    std::uintptr_t save=0,saveType=0,defaults=0,counselor=0,jason=0;std::int32_t serial=0;maps::Array cache{};
    bool defaultsRead=false,counselorRead=false,jasonRead=false;};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& value){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,value);}
};
// Mirror CFG's existing-save lookup2B21C0 through the CURRENT local player's
// SettingsSaveGameCache. No settings load/create, global save-object search,
// default Vanessa/Jason substitution, save writes or profile polling.
namespace detail {
template<bool RequireAssignedSerial,class Reader> bool Sample(Reader& reader,const active::Context& context,std::uint32_t callerThread,
    bool exactImageVerified,Selection& out,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    out={};Bounded<Reader> r(reader);Diagnostic detail;
    const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;detail.reads=r.reads;if(okay)detail.stage="none";
        if(diagnostic)*diagnostic=detail;return okay;};
    using namespace selectiondisk;
    const std::array<std::uintptr_t,4> addresses{{ILLLocalPlayerClass,ILLLocalPlayerConstructor,SCCharacterSelectionsSaveGameClass,SCCharacterSelectionsSaveGameConstructor}};
    std::array<std::uintptr_t,4> classes{};std::uintptr_t playerType=0,reverse=0;
    if(!exactImageVerified||!active::Current(r,context,callerThread))return finish(false);
    detail.stage="native_cache_or_player_ownership";
    for(unsigned i=0;i<addresses.size();++i)if(!r.Read(context.module+addresses[i],classes[i]))return finish(false);
    if(!cfgprobe::MatchingClassCache(classes[0],classes[1])||!cfgprobe::MatchingClassCache(classes[2],classes[3])||
        !r.Read(context.player+cfgprobe::ObjectClass,playerType)||!maps::Subclass(r,playerType,classes[0])||
        !r.Read(context.controller+0x3D0,reverse)||reverse!=context.player)return finish(false);
    const auto sample=[&](Selection& value){
        detail.stage="local_player_save_cache";
        if(!r.Read(context.player+SettingsSaveGameCache,value.cache)||!value.cache.Sane(MaximumSaves))return false;
        detail.cache=value.cache;detail.stage="cache_entry_ancestry";
        std::array<std::uintptr_t,MaximumSaves> entries{};unsigned matches=0;
        for(unsigned i=0;i<static_cast<unsigned>(value.cache.count);++i){
            auto& entry=entries[i];if(!r.Read(value.cache.data+8*i,entry))return false;
            if(!entry)continue;if(!cfgprobe::ObjectPointer(entry))return false;
            std::uintptr_t type=0;if(!r.Read(entry+cfgprobe::ObjectClass,type))return false;
            // Verify all ancestors even for non-matching cache entries. A
            // failed read is not equivalent to a non-matching save type.
            cfgprobe::Ancestry ancestry;
            if(!cfgprobe::CaptureAncestry(r,entry,ancestry)||!cfgprobe::StableAncestry(r,entry,ancestry))return false;
            if(ancestry.Contains(classes[2])){value.save=entry;++matches;}
        }
        detail.save=value.save;detail.matches=matches;detail.stage="matching_save_count";
        if(matches!=1)return false;detail.stage="save_registry_slot";
        // Context equality intentionally tolerates registry growth while the
        // menu's four positive identities remain current. A newly created
        // native save can therefore be outside context.registry.count. Sample
        // the current registry for THIS synchronous save observation; never
        // expand an old extent or turn serial0 into a retained identity.
        cfgprobe::lifetime::ArrayHeader registry;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,context.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfgprobe::lifetime::ReadSlot(r,registry,value.save,value.identity))return false;
        detail.serial=value.identity.serial;detail.stage="save_serial_unassigned";
        if(RequireAssignedSerial&&!value.identity.HasAssignedSerial())return false;detail.stage="save_type_read";
        if(!r.Read(value.save+cfgprobe::ObjectClass,value.saveType))return false;detail.saveType=value.saveType;
        detail.stage="save_defaults_read";detail.defaultsRead=r.Read(value.saveType+cfgspawn::ClassDefaultObject,value.defaults);detail.defaults=value.defaults;
        if(!detail.defaultsRead)return false;detail.stage="save_defaults_identity";
        // Null lazy CDO is observable metadata, not an instruction to create it.
        // An actual CDO in the player's cache is still rejected on both paths.
        if((RequireAssignedSerial&&!cfgprobe::ObjectPointer(value.defaults))||
            (value.defaults&&!cfgprobe::ObjectPointer(value.defaults))||value.defaults==value.save)return false;
        detail.stage="saved_choice_reads";
        detail.counselorRead=r.Read(value.save+CounselorPick,value.counselor);detail.counselor=value.counselor;
        detail.jasonRead=r.Read(value.save+KillerPick,value.jason);detail.jason=value.jason;
        if(!detail.counselorRead||!detail.jasonRead)return false;detail.stage="saved_choice_pointer";
        if((value.counselor&&!cfgprobe::ObjectPointer(value.counselor))||(value.jason&&!cfgprobe::ObjectPointer(value.jason)))return false;
        detail.stage="saved_choice_unset";
        if(RequireAssignedSerial&&(!value.counselor||!value.jason))return false;
        // These are class selections, not permission to load assets or possess
        // a human. Native counselor/killer ancestry is validated by the caller.
        detail.stage="cache_entries_changed";
        for(unsigned i=0;i<static_cast<unsigned>(value.cache.count);++i){std::uintptr_t again=0;
            if(!r.Read(value.cache.data+8*i,again)||again!=entries[i])return false;}
        return true;
    };
    Selection first,second;
    if(!sample(first)||!sample(second))return finish(false);detail.stage="sample_changed";
    if(first.save!=second.save||first.saveType!=second.saveType||first.defaults!=second.defaults||first.counselor!=second.counselor||first.jason!=second.jason||
        !(first.cache==second.cache)||!(first.identity==second.identity))return finish(false);
    detail.stage="native_cache_changed";
    for(unsigned i=0;i<addresses.size();++i){std::uintptr_t again=0;if(!r.Read(context.module+addresses[i],again)||again!=classes[i])return finish(false);}
    detail.stage="current_owner_changed";
    if(!active::Current(r,context,callerThread))return finish(false);
    out=second;return finish(true);
}
}
// Existing retained observation keeps its positive-serial contract unchanged.
template<class Reader> bool Observe(Reader& reader,const active::Context& context,std::uint32_t callerThread,
    bool exactImageVerified,Selection& out,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    return detail::Sample<true>(reader,context,callerThread,exactImageVerified,out,consumed,diagnostic);
}
// Diagnostic consistency only. Null class values are reported as unset, never
// replaced by first-loaded/default characters. An external snapshot is NOT atomic,
// and a serial0 save is NOT a reusable identity, lifetime pin or call target.
template<class Reader> bool ObserveSnapshot(Reader& reader,const active::Context& context,std::uint32_t callerThread,
    bool exactImageVerified,Selection& out,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    return detail::Sample<false>(reader,context,callerThread,exactImageVerified,out,consumed,diagnostic);
}
struct Values {std::uintptr_t counselor=0,jason=0;};
// Invoke only inside the verified native owner-thread callback, without a
// yield/native call/reentrancy during extraction. SettingsSaveGameCache is a
// reflected ordinary ObjectProperty array (not Weak/Lazy/Soft). Reacquire it
// each time; publish only typed class VALUES, never retain or return the save.
// Native class selection alone still does not authorize asset loading/travel.
template<class Reader,class Engine> bool EventLocalValues(Reader& reader,Engine& engine,const active::Context& context,
    bool exactImageVerified,Values& out,unsigned* consumed=nullptr){
    out={};if(consumed)*consumed=0;
    if(engine.Thread()!=context.thread)return false;
    Bounded<Reader> r(reader);Selection snapshot;
    const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    if(!ObserveSnapshot(r,context,engine.Thread(),exactImageVerified,snapshot)||!snapshot.counselor||!snapshot.jason)return finish(false);
    std::array<std::uintptr_t,4> native{};
    const std::array<std::uintptr_t,4> addresses{{cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,
        cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,
        cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva}};
    for(unsigned i=0;i<addresses.size();++i)if(!r.Read(context.module+addresses[i],native[i]))return finish(false);
    if(!cfgprobe::MatchingClassCache(native[0],native[1])||!cfgprobe::MatchingClassCache(native[2],native[3])||
        !maps::Subclass(r,snapshot.counselor,native[0])||!maps::Subclass(r,snapshot.jason,native[2]))return finish(false);
    // Fresh acquisition also closes the window after validating the classes.
    Selection final;
    if(!ObserveSnapshot(r,context,engine.Thread(),exactImageVerified,final)||snapshot.save!=final.save||snapshot.saveType!=final.saveType||snapshot.defaults!=final.defaults||
        !(snapshot.identity==final.identity)||!(snapshot.cache==final.cache)||snapshot.counselor!=final.counselor||snapshot.jason!=final.jason)
        return finish(false);
    for(unsigned i=0;i<addresses.size();++i){std::uintptr_t fresh=0;
        if(!r.Read(context.module+addresses[i],fresh)||fresh!=native[i])return finish(false);}
    if(engine.Thread()!=context.thread||!active::Current(r,context,engine.Thread()))return finish(false);
    out={final.counselor,final.jason};return finish(true);
}
}
