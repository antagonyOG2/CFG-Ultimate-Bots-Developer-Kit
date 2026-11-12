#pragma once
#include "MenuOptionsScript.hpp"
namespace cfgmenu::capture {
struct OptionsEvidence {
    OptionsOperands operands{};
    cfgreflection::Property destinationItems{},destinationInner{},sourceInner{},destinationSelected{};
};
inline bool SameProperty(const cfgreflection::Property& a,const cfgreflection::Property& b){
    return a.field.object==b.field.object&&a.field.owner==b.field.owner&&a.field.type==b.field.type&&a.field.next==b.field.next&&
        a.field.name==b.field.name&&a.dimension==b.dimension&&a.elementBytes==b.elementBytes&&a.offset==b.offset&&a.flags==b.flags;
}
template<class Reader,std::size_t StorageFields> bool OptionsDependencies(Reader& reader,std::uintptr_t module,const FunctionEvidenceStorage<StorageFields>& function,OptionsEvidence& output){
    output={};OptionsOperands operands;if(!OptionsScript(function,operands))return false;
    const auto sample=[&](OptionsEvidence& evidence){
        evidence.operands=operands;cfgreflection::Observation<Reader> reflection(reader,module);
        const auto member=[&](std::uintptr_t address,std::u16string_view kind,int bytes,cfgreflection::Property& value){
            std::uintptr_t owner=0;cfgreflection::Field f,resolved;
            return reader.Read(address+0x20,owner)&&reflection.ReadField(address,owner,f)&&f.kind.View()==kind&&
                reflection.Find(function.function.owner,f.label.View(),resolved)&&resolved.object==address&&resolved.owner==owner&&
                reflection.ReadProperty(f,value)&&value.dimension==1&&value.elementBytes==bytes&&value.offset%(bytes==16?8:bytes)==0;
        };
        const auto inner=[&](std::uintptr_t array,cfgreflection::Property& value){
            std::uintptr_t address=0,owner=0,again=0;cfgreflection::Field f;
            return reader.Read(array+cfgreflection::arraydisk::Inner,address)&&cfgprobe::ObjectPointer(address)&&
                reader.Read(address+0x20,owner)&&cfgprobe::ObjectPointer(owner)&&reflection.ReadField(address,owner,f)&&
                f.kind.View()==u"StrProperty"&&reflection.ReadProperty(f,value)&&value.dimension==1&&value.elementBytes==16&&value.offset==0&&
                reader.Read(array+cfgreflection::arraydisk::Inner,again)&&again==address;
        };
        return member(operands.destinationItems,u"ArrayProperty",16,evidence.destinationItems)&&
            member(operands.destinationSelected,u"IntProperty",4,evidence.destinationSelected)&&
            inner(operands.destinationItems,evidence.destinationInner)&&inner(operands.sourceItems,evidence.sourceInner)&&
            !(evidence.destinationItems.flags&copydisk::PodMask)&&!(evidence.destinationInner.flags&copydisk::PodMask)&&
            !(evidence.sourceInner.flags&copydisk::PodMask)&&
            (evidence.destinationItems.offset+16<=evidence.destinationSelected.offset||evidence.destinationSelected.offset+4<=evidence.destinationItems.offset);
    };
    OptionsEvidence a,b;ScriptHeader header;std::array<unsigned char,scriptdisk::MaxBytes> bytes{};
    if(!sample(a)||!sample(b)||!SameProperty(a.destinationItems,b.destinationItems)||!SameProperty(a.destinationInner,b.destinationInner)||
       !SameProperty(a.sourceInner,b.sourceInner)||!SameProperty(a.destinationSelected,b.destinationSelected)||
       !Script(reader,function.function.object,header,bytes)||!(header==function.script)||bytes!=function.bytes)return false;
    // Stable shape/copy evidence, not lifetime pinning or native call approval.
    output=a;return true;
}
}
