#pragma once
#include "../EventObserver/OfflineContextObservation.hpp"
#include "NativeMapSelectionObservation.hpp"
#include "ReflectionObservation.hpp"
#include "WorldMapBindings.generated.hpp"
#include <algorithm>

namespace cfgmenu::worldmap {
constexpr unsigned ReadLimit=8192,MaximumPath=255;
enum class Variant { None, Normal, Snow, Both };
enum class Form { None, Package, Object };
enum class Failure { None, ArmedSource, Context, ClassIdentity, Metadata, PathStorage, UnsupportedSyntax, DifferentMap, Changed };
// Supplied ONLY from the still-armed native-menu submission. This provider does
// not arm/reconstruct a session. Host retains the selected definition/class and
// fresh Plan objects throughout the synchronous owner-thread observation.
struct ArmedMap {
    std::uintptr_t module=0,frontendWorld=0,definition=0;
    std::uint64_t generation=0,submission=0;
};
struct PathText {
    std::array<char16_t,MaximumPath+1> data{};unsigned count=0;
    std::u16string_view View()const{return {data.data(),count};}
    bool operator==(const PathText& other)const{return count==other.count&&data==other.data;}
};
struct StringHeader {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const StringHeader& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
struct Observation {
    std::uintptr_t world=0,rootPackage=0,definition=0,defaults=0;
    cfgprobe::lifetime::SlotObservation worldIdentity{},definitionIdentity{},defaultsIdentity{};
    cfgnames::Text actualPackage{},actualWorldName{};
    PathText normalPath{},snowPath{};
    Variant variant=Variant::None;Form normalForm=Form::None,snowForm=Form::None;
    std::uint64_t generation=0,submission=0;
};
template<class Reader> class Bounded {
    Reader& r_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):r_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return r_.Read(at,out);}
};
inline bool Atom(std::u16string_view value){
    if(value.empty())return false;
    for(auto c:value)if(!((c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z')||(c>=u'0'&&c<=u'9')||c==u'_'||c==u'-'))return false;
    return true;
}
inline bool PackageSyntax(std::u16string_view value){
    if(value.size()<7||value.substr(0,6)!=u"/Game/")return false;
    std::size_t at=6;
    while(at<value.size()){const auto end=value.find(u'/',at);const auto last=end==value.npos?value.size():end;
        if(!Atom(value.substr(at,last-at)))return false;if(end==value.npos)return true;at=end+1;}
    return false;
}
inline bool SupportedPath(std::u16string_view value){
    if(value.empty())return true;const auto dot=value.find(u'.');
    return dot==value.npos?PackageSyntax(value):PackageSyntax(value.substr(0,dot))&&Atom(value.substr(dot+1));
}
inline Form Match(std::u16string_view authored,std::u16string_view package,std::u16string_view object){
    if(authored==package)return Form::Package;
    if(authored.size()==package.size()+1+object.size()&&authored.substr(0,package.size())==package&&
       authored[package.size()]==u'.'&&authored.substr(package.size()+1)==object)return Form::Object;
    return Form::None;
}
template<class Reader> bool String(Reader& r,std::uintptr_t at,StringHeader& header,PathText& out){
    header={};out={};StringHeader a,b;PathText text,again;
    if(!r.Read(at,a)||a.count<0||a.count>static_cast<int>(MaximumPath+1)||a.capacity<a.count||a.capacity>4096||
       (a.capacity&&!cfgprobe::ObjectPointer(a.data))||(!a.capacity&&a.data)||
       (a.count&&!cfgprobe::lifetime::Extent(a.data,static_cast<std::uintptr_t>(a.count)*2)))return false;
    for(int i=0;i<a.count;++i){char16_t c=0;if(!r.Read(a.data+2*static_cast<unsigned>(i),c))return false;
        if(i+1==a.count){if(c)return false;}else{if(!c||c<32||c>126)return false;text.data[text.count++]=c;}}
    for(int i=0;i<a.count;++i){char16_t c=0;if(!r.Read(a.data+2*static_cast<unsigned>(i),c))return false;
        if(i+1==a.count){if(c)return false;}else again.data[again.count++]=c;}
    if(!(text==again)||!r.Read(at,b)||!(a==b))return false;header=a;out=text;return true;
}
struct Schema {
    std::uintptr_t filePath=0;std::array<std::uintptr_t,3> properties{};
    std::array<std::uint64_t,3> flags{};
    bool operator==(const Schema& b)const{return filePath==b.filePath&&properties==b.properties&&flags==b.flags;}
};
template<class Reader> bool ReadSchema(Reader& r,std::uintptr_t module,std::uintptr_t selectedClass,std::uintptr_t nativeMap,Schema& out){
    out={};Schema s;using namespace cfgreflection;
    if(!r.Read(module+worldmapdisk::FilePathStructCache,s.filePath)||!cfgprobe::ObjectPointer(s.filePath))return false;
    cfgreflection::Observation<Reader> reflection(r,module);cfgnames::Observation<Reader> names(r,module);cfgnames::Text structName,kind;
    std::uintptr_t structClass=0;int size=0;
    if(!names.Object(s.filePath,structName)||structName.View()!=u"FilePath"||!r.Read(s.filePath+cfgprobe::ObjectClass,structClass)||
       !names.Object(structClass,kind)||kind.View()!=u"ScriptStruct"||!r.Read(s.filePath+disk::StorageBytes,size)||size!=16)return false;
    const std::array<std::u16string_view,2> labels{{u"Path",u"SnowMapPath"}};
    const std::array<unsigned,2> offsets{{worldmapdisk::Path,worldmapdisk::SnowMapPath}};
    for(unsigned i=0;i<2;++i){Field f;std::uintptr_t pointer=0,vtable=0;int dim=0,bytes=0,offset=0;
        if(!reflection.Find(selectedClass,labels[i],f)||f.owner!=nativeMap||f.kind.View()!=u"StructProperty"||
           !r.Read(f.object,vtable)||vtable!=module+worldmapdisk::StructPropertyVtable||
           !r.Read(f.object+worldmapdisk::StructType,pointer)||pointer!=s.filePath||
           !r.Read(f.object+disk::PropertyDimensions,dim)||dim!=1||!r.Read(f.object+disk::PropertyElementSize,bytes)||bytes!=16||
           !r.Read(f.object+disk::PropertyOffset,offset)||offset!=static_cast<int>(offsets[i])||!r.Read(f.object+disk::PropertyFlags,s.flags[i]))return false;
        s.properties[i]=f.object;
    }
    Field member;Property string;std::uintptr_t head=0;
    if(!reflection.Find(s.filePath,u"FilePath",member,false)||member.next||!r.Read(s.filePath+disk::Children,head)||head!=member.object||
       !reflection.ReadProperty(member,string)||member.kind.View()!=u"StrProperty"||string.dimension!=1||string.elementBytes!=16||string.offset!=0)return false;
    s.properties[2]=member.object;s.flags[2]=string.flags;out=s;return true;
}

// Read-only association of an independently owned fresh world with the armed
// selected definition. Actual package/object names are read from world->Outer,
// not copied from the menu token. Strict exact syntax only: no shortname search,
// URL stripping, case folding, PIE-prefix removal, extension repair or native
// ValidateMap/asset-loader calls. Does not prove selected weather modifiers.
template<class Reader> bool Observe(Reader& reader,const cfgobserver::Plan& fresh,const ArmedMap& source,
    std::uint32_t callerThread,bool exactImageVerified,Observation& output,unsigned* consumed=nullptr,Failure* failure=nullptr,bool nativeOffline=false){
    output={};Bounded<Reader> r(reader);Failure why=Failure::ArmedSource;
    const auto finish=[&](bool ok){if(consumed)*consumed=r.reads;if(failure)*failure=ok?Failure::None:why;return ok;};
    using namespace cfgprobe;
    if(!source.generation||!source.submission||source.module!=fresh.module||!ObjectPointer(source.frontendWorld)||
       source.frontendWorld==fresh.world||!ObjectPointer(source.definition))return finish(false);
    why=Failure::Context;if(!exactImageVerified||callerThread!=fresh.threadId||!cfgobserver::ContextUnchanged(r,fresh))return finish(false);
    why=Failure::ClassIdentity;
    const auto& sandbox=NativeClasses[nativeOffline?OfflineBotsMode:SandboxMode];
    const std::array<std::uintptr_t,8> cacheRvas{{worldmapdisk::SCMapDefinitionClass,worldmapdisk::SCMapDefinitionConstructor,
        worldmapdisk::WorldClass,worldmapdisk::WorldConstructor,worldmapdisk::NativeClassCache,worldmapdisk::NativePackageCache,
        sandbox.reflectionCacheRva,sandbox.constructorCacheRva}};
    std::array<std::uintptr_t,8> caches{};
    for(unsigned i=0;i<caches.size();++i)if(!r.Read(fresh.module+cacheRvas[i],caches[i])||!ObjectPointer(caches[i]))return finish(false);
    if(!MatchingClassCache(caches[0],caches[1])||!MatchingClassCache(caches[2],caches[3])||!MatchingClassCache(caches[6],caches[7])||
       !maps::Subclass(r,source.definition,caches[0]))return finish(false);
    Ancestry meta,mode;if(!CaptureAncestry(r,source.definition,meta)||!meta.Contains(caches[4])||
       !CaptureAncestry(r,fresh.mode,mode)||!mode.Contains(caches[6]))return finish(false);
    if(nativeOffline&&mode.classes[0]!=caches[6])return finish(false);
    std::uintptr_t worldType=0;if(!r.Read(fresh.world+ObjectClass,worldType)||worldType!=caches[2])return finish(false);
    Observation a;a.world=fresh.world;a.worldIdentity=fresh.objects[0];a.generation=source.generation;a.submission=source.submission;
    std::uintptr_t type=0,outer=0;
    if(!r.Read(source.definition+0xF8,a.defaults)||!ObjectPointer(a.defaults)||a.defaults==source.definition||
       !r.Read(a.defaults+ObjectClass,type)||type!=source.definition||
       !lifetime::ReadSlot(r,fresh.objectArray,source.definition,a.definitionIdentity)||
       !lifetime::ReadSlot(r,fresh.objectArray,a.defaults,a.defaultsIdentity)||
       !r.Read(fresh.world+0x20,a.rootPackage)||!ObjectPointer(a.rootPackage)||a.rootPackage==fresh.world||
       !r.Read(a.rootPackage+ObjectClass,type)||type!=caches[5]||!r.Read(a.rootPackage+0x20,outer)||outer)return finish(false);
    why=Failure::Metadata;Schema schema;if(!ReadSchema(r,fresh.module,source.definition,caches[0],schema))return finish(false);
    cfgnames::Observation<Bounded<Reader>> names(r,fresh.module);
    if(!names.Object(a.rootPackage,a.actualPackage)||!names.Object(fresh.world,a.actualWorldName))return finish(false);
    why=Failure::PathStorage;StringHeader normalHeader,snowHeader;
    if(!String(r,a.defaults+worldmapdisk::Path,normalHeader,a.normalPath)||!String(r,a.defaults+worldmapdisk::SnowMapPath,snowHeader,a.snowPath))return finish(false);
    why=Failure::UnsupportedSyntax;
    if(!PackageSyntax(a.actualPackage.View())||!Atom(a.actualWorldName.View())||!SupportedPath(a.normalPath.View())||!SupportedPath(a.snowPath.View()))return finish(false);
    a.normalForm=Match(a.normalPath.View(),a.actualPackage.View(),a.actualWorldName.View());
    a.snowForm=Match(a.snowPath.View(),a.actualPackage.View(),a.actualWorldName.View());
    why=Failure::DifferentMap;if(a.normalForm==Form::None&&a.snowForm==Form::None)return finish(false);
    a.variant=a.normalForm!=Form::None?(a.snowForm!=Form::None?Variant::Both:Variant::Normal):Variant::Snow;
    why=Failure::Changed;Schema finalSchema;StringHeader nh,sh;PathText np,sp;cfgnames::Text packageName,worldName;
    lifetime::SlotObservation classSlot,defaultSlot;
    if(!ReadSchema(r,fresh.module,source.definition,caches[0],finalSchema)||!(schema==finalSchema)||
       !String(r,a.defaults+worldmapdisk::Path,nh,np)||!(nh==normalHeader)||!(np==a.normalPath)||
       !String(r,a.defaults+worldmapdisk::SnowMapPath,sh,sp)||!(sh==snowHeader)||!(sp==a.snowPath)||
       !names.Object(a.rootPackage,packageName)||packageName.View()!=a.actualPackage.View()||
       !names.Object(fresh.world,worldName)||worldName.View()!=a.actualWorldName.View()||
       !r.Read(fresh.world+0x20,outer)||outer!=a.rootPackage||!r.Read(a.rootPackage+0x20,outer)||outer||
       !r.Read(a.rootPackage+ObjectClass,type)||type!=caches[5]||!r.Read(fresh.world+ObjectClass,type)||type!=worldType||
       !r.Read(source.definition+0xF8,outer)||outer!=a.defaults||!r.Read(a.defaults+ObjectClass,type)||type!=source.definition||
       !lifetime::ReadSlot(r,fresh.objectArray,source.definition,classSlot)||!(classSlot==a.definitionIdentity)||
       !lifetime::ReadSlot(r,fresh.objectArray,a.defaults,defaultSlot)||!(defaultSlot==a.defaultsIdentity)||
       !StableAncestry(r,source.definition,meta)||!StableAncestry(r,fresh.mode,mode)||!maps::Subclass(r,source.definition,caches[0]))return finish(false);
    for(unsigned i=0;i<caches.size();++i){std::uintptr_t current=0;if(!r.Read(fresh.module+cacheRvas[i],current)||current!=caches[i])return finish(false);}
    if(!cfgobserver::ContextUnchanged(r,fresh))return finish(false);
    // Only now expose the matched definition token, backed by actual-world
    // package evidence. Metadata serial0 remains sampled/unassigned, not a pin.
    a.definition=source.definition;output=a;return finish(true);
}
}
