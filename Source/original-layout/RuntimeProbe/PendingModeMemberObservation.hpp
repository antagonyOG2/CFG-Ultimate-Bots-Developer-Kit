#pragma once
#include "NativeFrontendCatalogues.hpp"
#include "ReflectionObservation.hpp"
namespace cfgmenu::pendingmember {
constexpr unsigned ReadLimit=4096;
struct PropertyDiagnostic {unsigned failure=0;std::uintptr_t field=0;std::uint64_t flags=0;int dimension=0,elementBytes=0,offset=0;};
struct Binding {
    active::Context context{};
    cfgprobe::lifetime::SlotObservation modeIdentity{};
    std::uintptr_t nativeMenu=0,nativeDefinition=0,modeType=0;
    cfgreflection::Property pending{},alias{};
    bool authored=false;
};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
};
// Deliberately not a general ClassProperty argument/write whitelist. This
// reader approves one named native instance pointer, never a player class,
// CDO, save, ownership or possession field. Schema is rechecked at each write.
// The caller must supply a descriptor from its verified authored graph, or an
// already-proved retained binding. This validates that exact descriptor without
// depending on unrelated members in the owner's entire reflected child list.
template<class Reader> bool PropertyAt(Reader& r,const active::Context& c,std::uintptr_t object,std::uintptr_t owner,
    std::u16string_view name,std::u16string_view kind,int size,int offset,cfgreflection::Property& out,PropertyDiagnostic* diagnostic=nullptr){
    out={};cfgreflection::Observation<Reader> reflection(r,c.module);
    cfgreflection::Field field,again;cfgreflection::Property p,final;
    PropertyDiagnostic detail;const auto fail=[&](unsigned reason){detail.failure=reason;if(diagnostic)*diagnostic=detail;return false;};
    if(diagnostic)*diagnostic={};
    if(!reflection.ReadField(object,owner,field))return fail(1);
    detail.field=field.object;
    if(field.owner!=owner||field.label.View()!=name||field.kind.View()!=kind)return fail(2);
    const bool schema=r.Read(field.object+cfgreflection::disk::PropertyDimensions,p.dimension)&&p.dimension==1&&
        r.Read(field.object+cfgreflection::disk::PropertyElementSize,p.elementBytes)&&p.elementBytes==size&&
        r.Read(field.object+cfgreflection::disk::PropertyOffset,p.offset)&&p.offset==offset&&
        r.Read(field.object+cfgreflection::disk::PropertyFlags,p.flags);
    detail.dimension=p.dimension;detail.elementBytes=p.elementBytes;detail.offset=p.offset;detail.flags=p.flags;
    if(!schema)return fail(3);
    if(!reflection.ReadField(field.object,owner,again)||again.type!=field.type||!(again.name==field.name)||again.next!=field.next||
        again.label.View()!=name||again.kind.View()!=kind||
        !r.Read(field.object+cfgreflection::disk::PropertyDimensions,final.dimension)||final.dimension!=p.dimension||
        !r.Read(field.object+cfgreflection::disk::PropertyElementSize,final.elementBytes)||final.elementBytes!=p.elementBytes||
        !r.Read(field.object+cfgreflection::disk::PropertyOffset,final.offset)||final.offset!=p.offset||
        !r.Read(field.object+cfgreflection::disk::PropertyFlags,final.flags)||final.flags!=p.flags)return fail(4);
    if(diagnostic)*diagnostic=detail;p.field=field;out=p;return true;
}
template<class Reader> bool Property(Reader& r,const active::Context& c,std::uintptr_t owner,
    std::u16string_view name,std::u16string_view kind,int size,int offset,cfgreflection::Property& out,PropertyDiagnostic* diagnostic=nullptr){
    out={};if(diagnostic)*diagnostic={};
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgreflection::Field field;
    if(!reflection.Find(owner,name,field,false)){if(diagnostic)diagnostic->failure=1;return false;}
    cfgreflection::Property property;
    if(!PropertyAt(r,c,field.object,owner,name,kind,size,offset,property,diagnostic))return false;
    if(property.field.type!=field.type||!(property.field.name==field.name)||property.field.next!=field.next){
        if(diagnostic)diagnostic->failure=4;return false;
    }
    out=property;return true;
}
inline bool SameSchema(const cfgreflection::Property& a,const cfgreflection::Property& b){
    return a.field.object==b.field.object&&a.field.owner==b.field.owner&&a.field.type==b.field.type&&
        a.field.name==b.field.name&&a.dimension==b.dimension&&
        a.elementBytes==b.elementBytes&&a.offset==b.offset&&a.flags==b.flags;
}
inline bool Same(const cfgreflection::Property& a,const cfgreflection::Property& b){return SameSchema(a,b)&&a.field.next==b.field.next;}
template<class Reader> bool ObserveImpl(Reader& reader,const active::Context& c,std::uint32_t thread,
    bool exactImageVerified,const cfgreflection::Property* authoredMode,Binding& out,unsigned* consumed){
    out={};Bounded<Reader> r(reader);Binding b;b.context=c;
    const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    std::uintptr_t menuCtor=0,definitionCtor=0;
    if(!exactImageVerified||thread!=c.thread||!active::Current(r,c,thread)||
        !r.Read(c.module+selectiondisk::SCGame_MenuClass,b.nativeMenu)||
        !r.Read(c.module+selectiondisk::SCGame_MenuConstructor,menuCtor)||!cfgprobe::MatchingClassCache(b.nativeMenu,menuCtor)||
        !r.Read(c.module+selectiondisk::SCModeDefinitionClass,b.nativeDefinition)||
        !r.Read(c.module+selectiondisk::SCModeDefinitionConstructor,definitionCtor)||!cfgprobe::MatchingClassCache(b.nativeDefinition,definitionCtor)||
        !r.Read(c.mode+cfgprobe::ObjectClass,b.modeType)||!maps::Subclass(r,b.modeType,b.nativeMenu)||
        !cfgprobe::lifetime::ReadSlot(r,c.registry,c.mode,b.modeIdentity)||!b.modeIdentity.HasAssignedSerial())return finish(false);
    if(authoredMode){
        if(authoredMode->field.owner!=b.nativeMenu||
           !PropertyAt(r,c,authoredMode->field.object,b.nativeMenu,u"PendingOfflineMode",u"ClassProperty",8,
               selectiondisk::PendingOfflineMode,b.pending)||b.pending.flags!=0x0/*CFG3.5 address withheld*/40000204ULL||
           !Same(b.pending,*authoredMode))return finish(false);
        b.authored=true;
    }else if(!Property(r,c,b.nativeMenu,u"PendingOfflineMode",u"ClassProperty",8,selectiondisk::PendingOfflineMode,b.pending))return finish(false);
    if(!Property(r,c,b.nativeDefinition,u"Alias",u"StrProperty",16,selectiondisk::Alias,b.alias)||
        !active::Current(r,c,thread))return finish(false);
    out=b;return finish(true);
}
template<class Reader> bool Observe(Reader& reader,const active::Context& c,std::uint32_t thread,
    bool exactImageVerified,Binding& out,unsigned* consumed=nullptr){
    return ObserveImpl(reader,c,thread,exactImageVerified,nullptr,out,consumed);
}
// Only a descriptor proved by the accepted source graph may enter this path.
// The current native owner, positive mode serial, exact member schema and Alias
// are checked again here; a stored raw address cannot grant write authority.
template<class Reader> bool ObserveWithAuthoredMode(Reader& reader,const active::Context& c,std::uint32_t thread,
    bool exactImageVerified,const cfgreflection::Property& provedMode,Binding& out,unsigned* consumed=nullptr){
    return ObserveImpl(reader,c,thread,exactImageVerified,&provedMode,out,consumed);
}
template<class Reader> bool Current(Reader& reader,const Binding& b,std::uint32_t thread,unsigned* consumed=nullptr){
    Binding fresh;unsigned reads=0;
    const bool okay=ObserveImpl(reader,b.context,thread,true,b.authored?&b.pending:nullptr,fresh,&reads)&&fresh.authored==b.authored&&fresh.nativeMenu==b.nativeMenu&&
        fresh.nativeDefinition==b.nativeDefinition&&fresh.modeType==b.modeType&&fresh.modeIdentity==b.modeIdentity&&
        Same(fresh.pending,b.pending)&&Same(fresh.alias,b.alias);
    if(consumed)*consumed=reads;return okay;
}
}
