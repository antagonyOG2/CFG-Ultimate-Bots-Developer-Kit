#pragma once
#include "NativeMapSelectionObservation.hpp"
#include "FrontendCatalogues.generated.hpp"
#include "NameObservation.hpp"
#include <cstring>
namespace cfgmenu::catalogues {
constexpr unsigned MaximumModes=32,MaximumCharacters=64,ReadLimit=8192;
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):reader_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
struct StringHeader {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const StringHeader& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
struct StringValue {
    StringHeader header{};std::array<char16_t,128> bytes{};
    std::u16string_view View()const noexcept{return {bytes.data(),header.count?static_cast<unsigned>(header.count-1):0};}
    bool operator==(const StringValue& b)const{return header==b.header&&bytes==b.bytes;}
};
template<class Reader> bool String(Reader& r,std::uintptr_t at,StringValue& out){
    out={};StringValue a;StringHeader final;
    if(!r.Read(at,a.header)||a.header.count<0||a.header.count>128||a.header.capacity<a.header.count||
        (a.header.count&&!cfgprobe::lifetime::Extent(a.header.data,static_cast<std::uintptr_t>(a.header.count)*2))||
        (!a.header.capacity&&a.header.data)||
        (a.header.count&&!r.ReadBytes(a.header.data,a.bytes.data(),static_cast<unsigned>(a.header.count)*2))||
        (a.header.count&&a.bytes[static_cast<unsigned>(a.header.count-1)])||
        !r.Read(at,final)||!(final==a.header))return false;
    for(int i=0;i<a.header.count-1;++i)if(!a.bytes[static_cast<unsigned>(i)])return false;
    out=a;return true;
}
struct ModeEntry {
    std::uintptr_t type=0,defaults=0;cfgnames::Text name{};StringValue alias{};
    maps::Array supported{};std::array<std::uintptr_t,maps::MaximumMaps> maps{};
    bool supportsSelectedMap=false;
    bool operator==(const ModeEntry& b)const{return type==b.type&&defaults==b.defaults&&name.View()==b.name.View()&&
        alias==b.alias&&supported==b.supported&&maps==b.maps&&supportsSelectedMap==b.supportsSelectedMap;}
};
struct Modes {
    std::uintptr_t registryType=0,defaults=0,selectedMap=0;maps::Array header{};
    std::array<ModeEntry,MaximumModes> entries{};unsigned count=0;
};
// Native shipping registry reference only. No asset/object/name-table census,
// no developer modes, no guessed Sandbox alias, no class creation or travel.
// A copied supported-map membership does not replace native package validity.
template<class Reader> bool ObserveModes(Reader& reader,const active::Context& context,bool exactImageVerified,
    Modes& out,unsigned* consumed=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    maps::Selection source;
    if(!exactImageVerified||!maps::Observe(r,context,context.thread,true,source))return finish(false);
    std::uintptr_t modeBase=0,modeCtor=0,mapBase=0,mapCtor=0;
    if(!r.Read(context.module+selectiondisk::SCModeDefinitionClass,modeBase)||
        !r.Read(context.module+selectiondisk::SCModeDefinitionConstructor,modeCtor)||!cfgprobe::MatchingClassCache(modeBase,modeCtor)||
        !r.Read(context.module+selectiondisk::SCMapDefinitionClass,mapBase)||
        !r.Read(context.module+selectiondisk::SCMapDefinitionConstructor,mapCtor)||!cfgprobe::MatchingClassCache(mapBase,mapCtor))return finish(false);
    const auto sample=[&](Modes& value){
        value.registryType=source.registryClass;value.defaults=source.registryDefaults;value.selectedMap=source.map;
        if(!r.Read(value.defaults+cataloguedisk::ShippingGameModes,value.header)||!value.header.Sane(MaximumModes))return false;
        value.count=static_cast<unsigned>(value.header.count);cfgnames::Observation<Bounded<Reader>> names(r,context.module);
        for(unsigned i=0;i<value.count;++i){auto& entry=value.entries[i];std::uintptr_t defaultsType=0;
            if(!r.Read(value.header.data+8*i,entry.type)||!maps::Subclass(r,entry.type,modeBase)||
                !r.Read(entry.type+cfgspawn::ClassDefaultObject,entry.defaults)||!cfgprobe::ObjectPointer(entry.defaults)||
                !r.Read(entry.defaults+cfgprobe::ObjectClass,defaultsType)||defaultsType!=entry.type||
                !names.Object(entry.type,entry.name)||!String(r,entry.defaults+selectiondisk::Alias,entry.alias)||
                !r.Read(entry.defaults+selectiondisk::SupportedMaps,entry.supported)||!entry.supported.Sane(maps::MaximumMaps))return false;
            for(unsigned prior=0;prior<i;++prior)if(value.entries[prior].type==entry.type)return false;
            unsigned matches=0;
            for(unsigned j=0;j<static_cast<unsigned>(entry.supported.count);++j){auto& map=entry.maps[j];
                if(!r.Read(entry.supported.data+8*j,map)||!maps::Subclass(r,map,mapBase))return false;
                for(unsigned prior=0;prior<j;++prior)if(entry.maps[prior]==map)return false;
                matches+=map==source.map?1u:0u;
            }
            entry.supportsSelectedMap=matches==1;
        }
        return true;
    };
    Modes first,second;
    if(!sample(first)||!sample(second)||first.registryType!=second.registryType||first.defaults!=second.defaults||
        first.selectedMap!=second.selectedMap||!(first.header==second.header)||first.count!=second.count||first.entries!=second.entries)return finish(false);
    maps::Selection fresh;std::uintptr_t value=0;
    if(!maps::Observe(r,context,context.thread,true,fresh)||fresh.registryClass!=source.registryClass||fresh.registryDefaults!=source.registryDefaults||
        fresh.map!=source.map||fresh.mode!=source.mode||fresh.settingsClass!=source.settingsClass||fresh.entries!=source.entries||
        !(fresh.maps==source.maps)||!(fresh.supported==source.supported)||
        !r.Read(context.module+selectiondisk::SCModeDefinitionClass,value)||value!=modeBase||
        !r.Read(context.module+selectiondisk::SCModeDefinitionConstructor,value)||value!=modeCtor||
        !r.Read(context.module+selectiondisk::SCMapDefinitionClass,value)||value!=mapBase||
        !r.Read(context.module+selectiondisk::SCMapDefinitionConstructor,value)||value!=mapCtor)return finish(false);
    out=second;return finish(true);
}
using SoftClass=std::array<unsigned char,cataloguedisk::SoftClassBytes>;
struct CharacterList {
    maps::Array header{};std::array<SoftClass,MaximumCharacters> entries{};
    bool operator==(const CharacterList& b)const{return header==b.header&&entries==b.entries;}
};
struct Characters {std::uintptr_t source=0,type=0;bool nativeDefaults=false;CharacterList counselors{},jasons{};};
inline bool Nonempty(const SoftClass& entry)noexcept{for(auto b:entry)if(b)return true;return false;}
template<class Reader> bool CharacterSample(Reader& r,std::uintptr_t at,CharacterList& out){
    out={};CharacterList a;
    if(!r.Read(at,a.header)||a.header.count<0||a.header.count>static_cast<int>(MaximumCharacters)||a.header.capacity<a.header.count||
        (!a.header.capacity&&a.header.data)||(a.header.count&&(!cfgprobe::ObjectPointer(a.header.data)||
            !cfgprobe::lifetime::Extent(a.header.data,static_cast<std::uintptr_t>(a.header.count)*cataloguedisk::SoftClassBytes))))return false;
    for(unsigned i=0;i<static_cast<unsigned>(a.header.count);++i)
        if(!r.Read(a.header.data+cataloguedisk::SoftClassBytes*i,a.entries[i]))return false;
    // Preserve native ordering, including empty/duplicate opaque references.
    // These are NOT concrete class pointers or an implicit character choice.
    out=a;return true;
}
template<class Reader> bool ObserveCharacters(Reader& reader,const active::Context& context,bool exactImageVerified,
    Characters& out,unsigned* consumed=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    std::uintptr_t native=0,ctor=0,type=0,source=context.mode;
    if(!exactImageVerified||!active::Current(r,context,context.thread)||
        !r.Read(context.module+cataloguedisk::GameModeClass,native)||!r.Read(context.module+cataloguedisk::GameModeConstructor,ctor)||
        !cfgprobe::MatchingClassCache(native,ctor)||!r.Read(context.mode+cfgprobe::ObjectClass,type))return finish(false);
    cfgprobe::Ancestry ownerAncestry;
    if(!cfgprobe::CaptureAncestry(r,context.mode,ownerAncestry)||!cfgprobe::StableAncestry(r,context.mode,ownerAncestry))return finish(false);
    const bool useDefaults=!ownerAncestry.Contains(native);
    if(useDefaults){
        // SCGame_Menu is a different native mode, not a gameplay roster owner.
        // Its bytes at560/750 must never be interpreted as SCGameMode arrays.
        // Observe ONLY an already-constructed, exact native SCGameMode CDO.
        // Native constructor383E00 populates these two arrays; frontend and
        // gameplay modes are siblings, not a shared roster layout. This
        // is diagnostic defaults, NOT an actual cooked roster/selection/pin.
        if(!r.Read(native+cfgspawn::ClassDefaultObject,source)||!cfgprobe::ObjectPointer(source)||
            !r.Read(source+cfgprobe::ObjectClass,type)||type!=native)return finish(false);
    }
    const auto sample=[&](Characters& c){c.source=source;c.type=type;c.nativeDefaults=useDefaults;
        return CharacterSample(r,source+cataloguedisk::CounselorCharacterClasses,c.counselors)&&
        CharacterSample(r,source+cataloguedisk::KillerCharacterClasses,c.jasons);};
    Characters first,second;std::uintptr_t fresh=0;
    if(!sample(first)||!sample(second)||!(first.counselors==second.counselors)||!(first.jasons==second.jasons)||
        !r.Read(context.module+cataloguedisk::GameModeClass,fresh)||fresh!=native||
        !r.Read(context.module+cataloguedisk::GameModeConstructor,fresh)||fresh!=ctor||
        !r.Read(source+cfgprobe::ObjectClass,fresh)||fresh!=type||!maps::Subclass(r,type,native)||
        !cfgprobe::StableAncestry(r,context.mode,ownerAncestry)||
        !active::Current(r,context,context.thread))return finish(false);
    if(useDefaults&&(!r.Read(native+cfgspawn::ClassDefaultObject,fresh)||fresh!=source))return finish(false);
    out=second;return finish(true);
}
}
