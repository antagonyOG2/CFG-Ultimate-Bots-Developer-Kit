#pragma once
#include "NameObservation.hpp"
#include "ReflectionLayout.generated.hpp"
#include "ClassIdentity.hpp"
namespace cfgreflection {
// Small, event-driven native-menu metadata reader. No GObject/name-table scans,
// native converters, raw FString/FText ownership transfers or gameplay polling.
struct Field {
    std::uintptr_t object=0,owner=0,type=0,next=0;
    cfgnames::Words name{};
    cfgnames::Text label{},kind{};
};
struct Property {
    Field field{};
    std::int32_t dimension=0,elementBytes=0,offset=0;
    std::uint64_t flags=0;
};
template<class Reader> class Observation {
    Reader& r_;cfgnames::Observation<Reader> names_;
    // These four existing fields occupy one contiguous32-byte CFG header.
    // Two fresh samples preserve before/after checks without eight separate
    // process reads. This is not a cached field/name/owner observation.
    struct Header {std::uintptr_t type;cfgnames::Words name;std::uintptr_t owner,next;};
    static_assert(sizeof(Header)==32&&cfgprobe::ObjectClass==0x10&&cfgnames::disk::ObjectName==0x18&&disk::Next==0x28);
    bool Stable(const Field& f){
        Header fresh{};
        return r_.Read(f.object+cfgprobe::ObjectClass,fresh)&&fresh.type==f.type&&fresh.owner==f.owner&&fresh.next==f.next&&fresh.name==f.name;
    }
public:
    Observation(Reader& reader,std::uintptr_t module):r_(reader),names_(reader,module){}
    // Exact current metadata, sampled anew on every invocation. Adjacent
    // dimension/size/flags occupy sixteen bytes; no padding or cached lease.
    bool CurrentProperty(const Property& expected){
        struct Shape {std::int32_t dimension,elementBytes;std::uint64_t flags;};
        static_assert(sizeof(Shape)==16&&disk::PropertyElementSize==disk::PropertyDimensions+4&&
            disk::PropertyFlags==disk::PropertyDimensions+8);
        Shape fresh{};std::int32_t offset=0;
        const auto kind=expected.field.kind.View();
        if(kind.size()<8||kind.substr(kind.size()-8)!=u"Property"||expected.dimension<1||expected.dimension>32||
            expected.elementBytes<1||expected.elementBytes>65536||expected.offset<0||expected.offset>1048576)return false;
        return Stable(expected.field)&&r_.Read(expected.field.object+disk::PropertyDimensions,fresh)&&
            fresh.dimension==expected.dimension&&fresh.elementBytes==expected.elementBytes&&fresh.flags==expected.flags&&
            r_.Read(expected.field.object+disk::PropertyOffset,offset)&&offset==expected.offset;
    }
    bool ReadField(std::uintptr_t object,std::uintptr_t owner,Field& output){
        output={};Field f;f.object=object;Header initial{};
        if(!cfgprobe::ObjectPointer(object)||!r_.Read(object+cfgprobe::ObjectClass,initial)||initial.owner!=owner||
            !cfgprobe::ObjectPointer(initial.type)||(initial.next&&!cfgprobe::ObjectPointer(initial.next)))return false;
        f.owner=initial.owner;f.type=initial.type;f.next=initial.next;f.name=initial.name;
        if(!names_.Decode(f.name,f.label)||
            !names_.Object(f.type,f.kind)||!Stable(f))return false;
        output=f;return true;
    }
    bool Find(std::uintptr_t structure,std::u16string_view wanted,Field& output,bool hierarchy=true){
        output={};if(wanted.empty()||wanted.size()>127)return false;
        std::array<std::uintptr_t,32> parents{};unsigned parentsSeen=0;
        std::array<std::uintptr_t,32> heads{},supers{};
        std::array<Field,256> fields{};unsigned count=0;
        Field match{};unsigned matches=0;
        // No cross-event caches: field replacement/relinking is observed again
        // at menu construction/commit, not re-traversed on every gameplay tick.
        auto owner=structure;
        while(owner){
            if(!cfgprobe::ObjectPointer(owner)||parentsSeen==parents.size())return false;
            for(unsigned i=0;i<parentsSeen;++i)if(parents[i]==owner)return false;
            parents[parentsSeen++]=owner;std::uintptr_t child=0,head=0,parent=0;
            if(!r_.Read(owner+disk::Children,head))return false;child=head;
            heads[parentsSeen-1]=head;
            while(child){
                if(count==fields.size())return false;
                for(unsigned i=0;i<count;++i)if(fields[i].object==child)return false;
                Field f;if(!ReadField(child,owner,f))return false;
                if(f.label.View()==wanted){match=f;++matches;}
                fields[count++]=f;child=f.next;
            }
            std::uintptr_t fresh=0;
            if(!r_.Read(owner+disk::Children,fresh)||fresh!=head)return false;
            // A derived override takes precedence just like the released base
            // FindPropertyInHierarchyByName; duplicates in one owner refuse.
            if(matches){
                if(matches!=1)return false;
                for(unsigned i=0;i<count;++i)if(!Stable(fields[i]))return false;
                for(unsigned i=0;i<parentsSeen;++i){
                    if(!r_.Read(parents[i]+disk::Children,fresh)||fresh!=heads[i])return false;
                    if(i+1<parentsSeen&&(!r_.Read(parents[i]+cfgprobe::ClassSuper,fresh)||fresh!=supers[i]))return false;
                }
                output=match;return true;
            }
            if(!hierarchy)break;
            if(!r_.Read(owner+cfgprobe::ClassSuper,parent))return false;supers[parentsSeen-1]=parent;owner=parent;
        }
        return false;
    }
    bool ReadProperty(const Field& f,Property& output){
        output={};Property p;p.field=f;
        const auto kind=f.kind.View();
        if(kind!=u"ObjectProperty"&&kind!=u"IntProperty"&&kind!=u"ArrayProperty"&&kind!=u"StrProperty"&&kind!=u"TextProperty"&&kind!=u"BoolProperty")return false;
        if(!r_.Read(f.object+disk::PropertyDimensions,p.dimension)||!r_.Read(f.object+disk::PropertyElementSize,p.elementBytes)||
            !r_.Read(f.object+disk::PropertyFlags,p.flags)||!r_.Read(f.object+disk::PropertyOffset,p.offset)||
            p.dimension<1||p.dimension>32||p.elementBytes<1||p.elementBytes>512||p.offset<0||p.offset>65536||
            static_cast<unsigned>(p.elementBytes*p.dimension)>65536-static_cast<unsigned>(p.offset)||!Stable(f))return false;
        std::int32_t dimension=0,bytes=0,offset=0;std::uint64_t flags=0;
        if(!r_.Read(f.object+disk::PropertyDimensions,dimension)||dimension!=p.dimension||
            !r_.Read(f.object+disk::PropertyElementSize,bytes)||bytes!=p.elementBytes||
            !r_.Read(f.object+disk::PropertyFlags,flags)||flags!=p.flags||
            !r_.Read(f.object+disk::PropertyOffset,offset)||offset!=p.offset)return false;
        output=p;return true;
    }
    bool ReadObjectMember(std::uintptr_t object,std::uintptr_t type,std::u16string_view wanted,std::uintptr_t& value){
        value=0;std::uintptr_t currentType=0,fresh=0,result=0;Field f;Property p;
        if(!cfgprobe::ObjectPointer(object)||!r_.Read(object+cfgprobe::ObjectClass,currentType)||currentType!=type||
            !Find(type,wanted,f)||!ReadProperty(f,p)||p.field.kind.View()!=u"ObjectProperty"||p.dimension!=1||p.elementBytes!=8||p.offset%8||
            !r_.Read(object+static_cast<unsigned>(p.offset),result)||(result&&!cfgprobe::ObjectPointer(result))||
            !r_.Read(object+cfgprobe::ObjectClass,fresh)||fresh!=type||!Stable(f)||
            !r_.Read(object+static_cast<unsigned>(p.offset),fresh)||fresh!=result)return false;
        // A stable field name/link does not establish stable value storage.
        // Re-read the property schema after sampling the object's value too.
        Property finalProperty;
        if(!ReadProperty(f,finalProperty)||finalProperty.dimension!=p.dimension||finalProperty.elementBytes!=p.elementBytes||
           finalProperty.flags!=p.flags||finalProperty.offset!=p.offset)return false;
        value=result;return true;
    }
};
}
