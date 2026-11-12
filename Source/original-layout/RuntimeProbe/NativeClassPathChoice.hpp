#pragma once
#include "NativeFrontendCatalogues.hpp"
#include <algorithm>
namespace cfgmenu::classchoice {
constexpr unsigned ReadLimit=12288; // bounded full-path catalogue, within Start's16384 shared cap
struct Choice {std::uintptr_t type=0;unsigned index=0;catalogues::SoftClass packet{};};
struct Diagnostic {
    unsigned stage=0,length=0,matches=0,rosterCount=0;
    std::array<char16_t,512> selected{};
    std::array<std::array<char16_t,160>,32> roster{};
    std::array<unsigned,32> rosterLengths{};
};
// Compare a copied, accepted native asset path against a fresh loaded class.
// Never dereference the old menu's class pointer after map travel.
template<class Reader> bool MatchesPath(Reader& r,std::uintptr_t module,std::uintptr_t type,std::uintptr_t base,
    const catalogues::SoftClass& packet){
    if(!cfgprobe::ObjectPointer(type)||!maps::Subclass(r,type,base))return false;
    cfgnames::Observation<Reader> names(r,module);cfgnames::PathText klass,package,path,againClass,againPackage;
    std::uintptr_t outer=0,parent=1,again=0;cfgnames::Words words{};std::memcpy(&words,packet.data()+16,8);
    for(unsigned i=24;i<40;++i)if(packet[i])return false;
    if(!names.Decode(words,path)||!names.Object(type,klass)||!r.Read(type+0x20,outer)||!cfgprobe::ObjectPointer(outer)||
       !names.Object(outer,package)||!r.Read(outer+0x20,parent)||parent||package.View().substr(0,6)!=u"/Game/")return false;
    const auto target=path.View();
    if(target.size()!=package.count+1+klass.count||target.substr(0,package.count)!=package.View()||target[package.count]!=u'.'||
       target.substr(package.count+1)!=klass.View())return false;
    return names.Object(type,againClass)&&againClass.View()==klass.View()&&r.Read(type+0x20,again)&&again==outer&&
        names.Object(outer,againPackage)&&againPackage.View()==package.View()&&r.Read(outer+0x20,again)&&!again&&maps::Subclass(r,type,base);
}
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
};
// Preserve the existing explicit-choice allowance for a native derived variant
// after replacing menu pointers with copied asset paths. Only the fresh class's
// own bounded, stable superclass chain is eligible; no global lookup, name-only
// match, sibling variant, generic counselor base or asset loading is allowed.
template<class Reader> bool MatchesPathOrAncestor(Reader& reader,std::uintptr_t module,std::uintptr_t type,std::uintptr_t base,
    const catalogues::SoftClass& packet,unsigned* matchedDepth=nullptr){
    if(matchedDepth)*matchedDepth=0;
    Bounded<Reader> r(reader);
    if(!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(type)||!cfgprobe::ObjectPointer(base))return false;
    cfgnames::Words words{};std::memcpy(&words,packet.data()+16,8);
    for(unsigned i=24;i<40;++i)if(packet[i])return false;
    cfgnames::Observation<Bounded<Reader>> names(r,module);cfgnames::PathText path;
    if(!names.Decode(words,path)||path.View().substr(0,6)!=u"/Game/")return false;
    std::array<std::uintptr_t,cfgprobe::MaximumAncestry> classes{},parents{};
    unsigned count=0,matches=0,depth=0;std::uintptr_t current=type,matched=0;bool foundBase=false;
    while(current){
        if(!cfgprobe::ObjectPointer(current)||count==classes.size())return false;
        for(unsigned i=0;i<count;++i)if(classes[i]==current)return false;
        const auto index=count++;classes[index]=current;
        if(!r.Read(current+cfgprobe::ClassSuper,parents[index]))return false;
        if(current==base){foundBase=true;if(current!=type)break;}
        cfgnames::PathText klass,package;std::uintptr_t outer=0,parent=1;
        // A failed read is not a nonmatching candidate: refuse the entire proof.
        if(!names.Object(current,klass)||!r.Read(current+0x20,outer)||!cfgprobe::ObjectPointer(outer)||outer==current||
           !names.Object(outer,package)||!r.Read(outer+0x20,parent)||parent||package.View().substr(0,6)!=u"/Game/"||klass.View().empty())return false;
        const auto target=path.View();
        if(target.size()==package.count+1+klass.count&&target.substr(0,package.count)==package.View()&&target[package.count]==u'.'&&
           target.substr(package.count+1)==klass.View()){++matches;matched=current;depth=index;}
        if(foundBase)break;current=parents[index];
    }
    if(!foundBase||matches!=1||!MatchesPath(r,module,matched,base,packet))return false;
    for(unsigned i=0;i<count;++i){std::uintptr_t parent=0;if(!r.Read(classes[i]+cfgprobe::ClassSuper,parent)||parent!=parents[i])return false;}
    if(!maps::Subclass(r,type,base))return false;
    if(matchedDepth)*matchedDepth=depth;return true;
}
// Relate an already-selected, loaded native class VALUE to a supplied small
// catalogue. Exact full package.class path, not short label/first-loaded class.
// This does not resolve/load objects, manufacture a weak pointer or assert that
// frontend defaults are the eventual cooked world's roster. Arrival must match
// the chosen path against that world's real roster before spawning Jason.
template<class Reader> bool MatchLoaded(Reader& reader,std::uintptr_t module,std::uintptr_t selected,
    std::uintptr_t nativeBase,const catalogues::CharacterList& list,Choice& out,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    Diagnostic local{};auto& d=diagnostic?*diagnostic:local;d={};d.stage=1;
    if(!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(selected)||!cfgprobe::ObjectPointer(nativeBase)||
        !list.header.Sane(catalogues::MaximumCharacters)||!maps::Subclass(r,selected,nativeBase))return finish(false);
    cfgnames::Observation<Bounded<Reader>> names(r,module);cfgnames::PathText klass,package;
    std::uintptr_t outer=0,parent=1;
    d.stage=2;
    if(!names.Object(selected,klass)||!r.Read(selected+0x20,outer)||!cfgprobe::ObjectPointer(outer)||outer==selected||
        !names.Object(outer,package)||!r.Read(outer+0x20,parent)||parent||
        package.View().size()<7||package.View().substr(0,6)!=u"/Game/"||klass.View().empty())return finish(false);
    std::array<char16_t,512> full{};const auto length=package.count+1+klass.count;d.length=length;d.stage=3;
    if(length>=full.size())return finish(false);
    std::copy(package.data.begin(),package.data.begin()+package.count,full.begin());full[package.count]=u'.';
    std::copy(klass.data.begin(),klass.data.begin()+klass.count,full.begin()+package.count+1);
    d.selected=full;d.rosterCount=static_cast<unsigned>(list.header.count);
    Choice result;unsigned matches=0;
    for(unsigned i=0;i<static_cast<unsigned>(list.header.count);++i){
        d.stage=4;catalogues::SoftClass current{};cfgnames::Words path{};catalogues::StringHeader subpath{};cfgnames::PathText decoded;
        if(!r.Read(list.header.data+40*i,current)||current!=list.entries[i])return finish(false);
        std::memcpy(&path,current.data()+16,sizeof(path));std::memcpy(&subpath,current.data()+24,sizeof(subpath));
        // Class references must not select a nested subobject. No borrowed
        // FString storage survives into a submitted configuration.
        d.stage=5;if(subpath.data||subpath.count||subpath.capacity)return finish(false);
        if(path.index==0&&path.number==0)continue; // Native empty / None choice, never fallback.
        d.stage=6;if(!names.Decode(path,decoded))return finish(false);
        if(i<d.roster.size()){d.rosterLengths[i]=decoded.count;
            std::copy_n(decoded.data.begin(),(std::min)(decoded.count,159u),d.roster[i].begin());}
        if(decoded.View()==std::u16string_view(full.data(),length)){
            if(!matches)result={selected,i,current}; // first identical path is deterministic
            else{cfgnames::Words first{};std::memcpy(&first,result.packet.data()+16,sizeof(first));cfgnames::PathText firstPath;
                if(!names.Decode(first,firstPath)||firstPath.View()!=decoded.View())return finish(false);}
            ++matches;
        }
    }
    d.matches=matches;d.stage=7;if(!matches)return finish(false);
    // Reacquire names, owners, exact ancestry and entire small source after
    // matching. No partial catalogue or changed preference is adopted.
    d.stage=8;cfgnames::PathText klassAgain,packageAgain;std::uintptr_t again=0;
    if(!names.Object(selected,klassAgain)||klassAgain.View()!=klass.View()||!r.Read(selected+0x20,again)||again!=outer||
        !names.Object(outer,packageAgain)||packageAgain.View()!=package.View()||!r.Read(outer+0x20,again)||again||
        !maps::Subclass(r,selected,nativeBase)||!r.Read(list.header.data+40*result.index,result.packet)||
        result.packet!=list.entries[result.index])return finish(false);
    // Caller checks the owner's header (this function has no arbitrary owner).
    d.stage=9;for(unsigned i=0;i<static_cast<unsigned>(list.header.count);++i){catalogues::SoftClass againEntry;
        if(!r.Read(list.header.data+40*i,againEntry)||againEntry!=list.entries[i])return finish(false);}
    d.stage=0;out=result;return finish(true);
}
}
