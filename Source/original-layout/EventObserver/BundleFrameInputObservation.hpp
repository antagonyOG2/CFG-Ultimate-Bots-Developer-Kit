#pragma once
#include "../RuntimeProbe/NativeFrameInputObservation.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/WorldMapBindings.generated.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
namespace cfgretainframe {
template<class R>class RuntimeHost {
    R& r_;std::uintptr_t module_;
public:
    RuntimeHost(R& reader,std::uintptr_t module):r_(reader),module_(module){}
    bool Frame(std::uintptr_t function,unsigned& bytes,std::uintptr_t& head){
        std::uint16_t extent=0;
        if(!r_.Read(function+0x8E,extent)||!r_.Read(function+cfgreflection::disk::Children,head))return false;
        bytes=extent;return true;
    }
    bool PointerAt(std::uintptr_t at,std::uintptr_t& value){return r_.Read(at,value);}
    bool StringArray(std::uintptr_t property,unsigned bytes){
        if(bytes!=16)return false;
        std::uintptr_t inner=0;int dimension=0,size=0;
        cfgreflection::Observation<R> reflection(r_,module_);cfgreflection::Field field;
        return r_.Read(property+cfgreflection::arraydisk::Inner,inner)&&inner&&
            reflection.ReadField(inner,property,field)&&field.kind.View()==u"StrProperty"&&
            r_.Read(inner+cfgreflection::disk::PropertyDimensions,dimension)&&dimension==1&&
            r_.Read(inner+cfgreflection::disk::PropertyElementSize,size)&&size==16;
    }
    bool FloatStructure(std::uintptr_t property,unsigned bytes){
        if(bytes!=12)return false; // existing FVector / FRotator-sized frame inputs
        std::uintptr_t type=0,head=0;int storage=0;
        if(!r_.Read(property+cfgmenu::worldmapdisk::StructType,type)||!cfgprobe::ObjectPointer(type)||
           !r_.Read(type+cfgreflection::disk::StorageBytes,storage)||storage!=12||
           !r_.Read(type+cfgreflection::disk::Children,head))return false;
        cfgreflection::Observation<R> reflection(r_,module_);unsigned mask=0;
        for(unsigned i=0;i<3;++i){cfgreflection::Field field;int dimension=0,size=0,offset=0;
            if(!head||!reflection.ReadField(head,type,field)||field.kind.View()!=u"FloatProperty"||
               !r_.Read(head+cfgreflection::disk::PropertyDimensions,dimension)||dimension!=1||
               !r_.Read(head+cfgreflection::disk::PropertyElementSize,size)||size!=4||
               !r_.Read(head+cfgreflection::disk::PropertyOffset,offset)||offset<0||offset>8||offset%4)return false;
            const auto bit=1u<<(offset/4);if(mask&bit)return false;mask|=bit;head=field.next;
        }
        return !head&&mask==7;
    }
    bool PropertyAt(std::uintptr_t child,std::uintptr_t owner,Property& out){
        cfgreflection::Observation<R> reflection(r_,module_);cfgreflection::Field field;
        if(!reflection.ReadField(child,owner,field))return false;
        struct Shape{int dimension,bytes;std::uint64_t flags;};Shape shape{};int offset=0;
        if(!r_.Read(child+cfgreflection::disk::PropertyDimensions,shape)||
           !r_.Read(child+cfgreflection::disk::PropertyOffset,offset)||
           shape.dimension<1||shape.bytes<1||offset<0)return false;
        out={field.next,shape.flags,static_cast<unsigned>(offset),static_cast<unsigned>(shape.bytes),
            static_cast<unsigned>(shape.dimension),Kind::Unsupported};
        const auto kind=field.kind.View();
        if(kind==u"ObjectProperty"||kind==u"ClassProperty")out.kind=Kind::Object;
        else if(kind==u"BoolProperty"||kind==u"ByteProperty"||kind==u"IntProperty"||kind==u"Int64Property"||
                kind==u"UInt32Property"||kind==u"FloatProperty"||kind==u"DoubleProperty"||kind==u"NameProperty"||
                kind==u"StrProperty"||kind==u"TextProperty")out.kind=Kind::Value;
        else if(kind==u"StructProperty"&&FloatStructure(child,out.bytes))out.kind=Kind::Value;
        else if(kind==u"ArrayProperty"&&StringArray(child,out.bytes))out.kind=Kind::Value;
        // enum class : uint8 parameters (e.g. SERVER_OnInteract InteractMethod,
        // codegen kind29) are one scalar byte with no object roots. Accept only
        // a single 1-byte element; wider enums stay unsupported.
        else if(kind==u"EnumProperty"&&shape.dimension==1&&shape.bytes==1)out.kind=Kind::Value;
        // Containers and other input structures remain unsupported rather than
        // guessing pointer ownership. Existing typed adapters still apply.
        return true;
    }
};
}
