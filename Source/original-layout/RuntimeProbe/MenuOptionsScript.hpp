#pragma once
#include "MenuScriptObservation.hpp"
#include "MenuCopy.generated.hpp"
namespace cfgmenu::capture {
struct OptionsOperands {
    std::uintptr_t destinationItems=0,sourceItems=0,destinationSelected=0,sourceSelected=0;
    bool operator==(const OptionsOperands& b)const{return destinationItems==b.destinationItems&&sourceItems==b.sourceItems&&destinationSelected==b.destinationSelected&&sourceSelected==b.sourceSelected;}
};
// Decoder only, never an interpreter or permission to call ProcessEvent. CFG's
// exact script is two assignments followed by Return Nothing EndOfScript.
// Refuse calls, jumps, latent work, additional effects and partial bytecode.
template<std::size_t StorageFields> bool OptionsScript(const FunctionEvidenceStorage<StorageFields>& f,OptionsOperands& output){
    output={};if(f.script.count!=copydisk::SetOptionsBytes||f.script.capacity<f.script.count||f.fieldsCount!=2||f.parameterBytes!=20||
        f.storageBytes<f.parameterBytes||f.storageBytes>512||!cfgprobe::ObjectPointer(f.function.object)||!cfgprobe::ObjectPointer(f.function.owner))return false;
    const auto& a=f.bytes;
    if(a[0]!=copydisk::Let||a[9]!=copydisk::InstanceVariable||a[18]!=copydisk::LocalOutVariable||
       a[27]!=copydisk::Let||a[36]!=copydisk::InstanceVariable||a[45]!=copydisk::LocalVariable||
       a[54]!=copydisk::Return||a[55]!=copydisk::Nothing||a[56]!=copydisk::EndOfScript)return false;
    const auto pointer=[&](unsigned offset){std::uintptr_t p=0;std::memcpy(&p,a.data()+offset,sizeof(p));return p;};
    OptionsOperands candidate{pointer(1),pointer(19),pointer(28),pointer(46)};
    if(candidate.destinationItems!=pointer(10)||candidate.destinationSelected!=pointer(37))return false;
    std::array<std::uintptr_t,4> operands{candidate.destinationItems,candidate.sourceItems,candidate.destinationSelected,candidate.sourceSelected};
    for(unsigned i=0;i<operands.size();++i){if(!cfgprobe::ObjectPointer(operands[i]))return false;for(unsigned j=0;j<i;++j)if(operands[i]==operands[j])return false;}
    const cfgreflection::Property* items=nullptr;const cfgreflection::Property* selected=nullptr;
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];
        if(p.field.owner!=f.function.object||p.dimension!=1)return false;
        if(p.field.label.View()==u"Items"&&p.field.kind.View()==u"ArrayProperty"&&p.elementBytes==16&&p.offset==0&&p.flags==0x08000394ULL)items=&p;
        else if(p.field.label.View()==u"SelectedIndex"&&p.field.kind.View()==u"IntProperty"&&p.elementBytes==4&&p.offset==16&&p.flags==0x0/*CFG3.5 address withheld*/40000294ULL)selected=&p;
        else return false;
    }
    if(!items||!selected||items->field.object!=candidate.sourceItems||selected->field.object!=candidate.sourceSelected)return false;
    output=candidate;return true;
}
}
