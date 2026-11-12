#pragma once
#include "../Core/CounselorConvergencePolicy.hpp"
#include "../RuntimeProbe/BlackboardSchemaBindings.generated.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include <array>

namespace cfgconvergence {
// Read only the current positively-owned helper's strongly held blackboard
// asset/parents. No object census, FName conversion, factory or field writes.
// Native class pairs and24-byte entry layout are independently disk-proved.
class BlackboardSchema {
public:
    enum class State {Empty,Scanning,Ready,Unavailable};
    enum class Failure {None,Classes,Component,Asset,Array,ParentCycle,KeyType,Duplicate,Changed,Required};
    static constexpr unsigned MaxLayers=8,MaxEntries=128,CellsPerPhase=4;
private:
    using Slot=cfgprobe::lifetime::SlotObservation;
    struct Header {std::uintptr_t data=0;int count=0,capacity=0;
        bool operator==(const Header& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
    struct Layer {Slot slot{};std::uintptr_t type=0,parent=0;Header keys{};};
    struct KeyProof {std::uintptr_t cell=0,asset=0,type=0,baseClass=0;Slot slot{};Name name{};cfgprobe::Ancestry ancestry{};};
    State state_=State::Empty;Failure failure_=Failure::None;
    std::uintptr_t component_=0,asset_=0,module_=0;
    Slot componentSlot_{};std::array<std::uintptr_t,4> classes_{};
    std::array<Layer,MaxLayers> layers_{};std::array<KeyProof,KeyCount> keys_{};
    std::array<Name,KeyCount> names_{};
    unsigned layerCount_=0,cursor_=0,mask_=0,total_=0;
    bool sealed_=false;
    State Fail(Failure why){failure_=why;return state_=State::Unavailable;}
    template<class R> static bool Same(R& r,std::uintptr_t at,std::uintptr_t expected){std::uintptr_t v=0;return r.Read(at,v)&&v==expected;}
    template<class R> static bool Live(R& r,const cfgprobe::lifetime::ArrayHeader& a,std::uintptr_t p,Slot& out){
        std::uint32_t flags=0;return cfgprobe::lifetime::ReadSlot(r,a,p,out)&&
            r.Read(a.data+static_cast<std::uintptr_t>(out.index)*24+8,flags)&&!(flags&0x20000000u);
    }
    template<class R> static bool Typed(R& r,std::uintptr_t object,std::uintptr_t native){
        cfgprobe::Ancestry chain;return cfgprobe::CaptureAncestry(r,object,chain)&&chain.Contains(native)&&cfgprobe::StableAncestry(r,object,chain);
    }
    template<class R> bool Caches(R& r,std::uintptr_t module,bool first){
        constexpr std::array<schemadisk::TypeBinding,4> bindings{{schemadisk::BlackboardComponent,schemadisk::BlackboardData,schemadisk::BoolKey,schemadisk::ObjectKey}};
        for(unsigned i=0;i<bindings.size();++i){std::uintptr_t x=0,y=0;
            if(!r.Read(module+bindings[i].reflection,x)||!r.Read(module+bindings[i].constructor,y)||!cfgprobe::MatchingClassCache(x,y)||(!first&&x!=classes_[i]))return false;
            if(first)classes_[i]=x;
        }return true;
    }
    template<class R> bool ReadLayer(R& r,const cfgprobe::lifetime::ArrayHeader& a,std::uintptr_t object,Layer& layer){
        if(!Live(r,a,object,layer.slot)||!Typed(r,object,classes_[1])||!r.Read(object+cfgprobe::ObjectClass,layer.type)||
            !r.Read(object+schemadisk::Parent,layer.parent)||!r.Read(object+schemadisk::Keys,layer.keys))return false;
        const auto& h=layer.keys;
        return h.count>=0&&h.count<=static_cast<int>(MaxEntries)&&h.capacity>=h.count&&h.capacity<=1024&&
            (!h.count||cfgprobe::lifetime::Extent(h.data,static_cast<std::uintptr_t>(h.count)*schemadisk::EntryBytes));
    }
    template<class R> bool Links(R& r,const cfgprobe::lifetime::ArrayHeader& a){
        Slot slot{};
        if(!Caches(r,module_,false)||!Live(r,a,component_,slot)||!(slot==componentSlot_)||
            !Same(r,component_+schemadisk::BlackboardAsset,asset_))return false;
        for(unsigned i=0;i<layerCount_;++i){const auto& old=layers_[i];Header h{};
            if(!Live(r,a,old.slot.object,slot)||!(slot==old.slot)||!Same(r,old.slot.object+cfgprobe::ObjectClass,old.type)||
                !Same(r,old.slot.object+schemadisk::Parent,old.parent)||!r.Read(old.slot.object+schemadisk::Keys,h)||!(h==old.keys))return false;
        }return true;
    }
public:
    State Status()const{return state_;}Failure Why()const{return failure_;}unsigned Mask()const{return state_==State::Ready?mask_:0;}
    unsigned ObservedMask()const{return mask_;} // diagnostic only, never admission
    bool Supports(unsigned key)const{return key<KeyCount&&(Mask()&(1u<<key));}
    void Reset(){*this={};}
    template<class R> State Step(R& r,const cfgprobe::lifetime::ArrayHeader& a,std::uintptr_t module,
        std::uintptr_t ownedComponent,const Slot& componentSlot,const std::array<Name,KeyCount>& names){
        if(state_==State::Unavailable)return state_;
        if(state_==State::Empty){
            module_=module;component_=ownedComponent;componentSlot_=componentSlot;names_=names;
            if(!component_||componentSlot_.object!=component_||!Caches(r,module,true))return Fail(Failure::Classes);
            if(!Typed(r,component_,classes_[0])||!r.Read(component_+schemadisk::BlackboardAsset,asset_)||!cfgprobe::ObjectPointer(asset_))return Fail(Failure::Component);
            if(!ReadLayer(r,a,asset_,layers_[0]))return Fail(Failure::Asset);
            layerCount_=1;state_=State::Scanning;return state_;
        }
        if(module!=module_||ownedComponent!=component_||!(componentSlot==componentSlot_)||!Links(r,a))return Fail(Failure::Changed);
        if(state_==State::Ready){if(!Current(r,a))return Fail(Failure::Changed);return state_;}
        if(sealed_){
            // Donor obtains the weapon through GetCurrentWeapon if SCWeapon is absent.
            constexpr unsigned required=(1u<<Jason)|(1u<<Fight)|(1u<<Flee);
            if((mask_&required)!=required)return Fail(Failure::Required);
            if(!Current(r,a))return Fail(Failure::Changed);
            return state_=State::Ready;
        }
        const auto& layer=layers_[layerCount_-1];
        for(unsigned n=0;n<CellsPerPhase&&cursor_<static_cast<unsigned>(layer.keys.count);++n,++cursor_){
            const auto cell=layer.keys.data+static_cast<std::uintptr_t>(cursor_)*schemadisk::EntryBytes;
            Name name{};std::uintptr_t type=0;
            if(!r.Read(cell+schemadisk::EntryName,name)||!r.Read(cell+schemadisk::EntryKeyType,type))return Fail(Failure::Array);
            unsigned key=0;while(key<KeyCount&&(name.index!=names_[key].index||name.number!=names_[key].number))++key;
            if(key==KeyCount)continue;
            if(mask_&(1u<<key))return Fail(Failure::Duplicate);
            KeyProof proof;proof.cell=cell;proof.asset=layer.slot.object;proof.name=name;proof.type=type;
            if(!Live(r,a,type,proof.slot)||!Same(r,type+0x20,layer.slot.object)||
                !cfgprobe::CaptureAncestry(r,type,proof.ancestry)||!proof.ancestry.Contains(classes_[key<=Jason?3:2])||
                !cfgprobe::StableAncestry(r,type,proof.ancestry))return Fail(Failure::KeyType);
            if(key<=Jason&&(!r.Read(type+schemadisk::ObjectBaseClass,proof.baseClass)||!cfgprobe::ObjectPointer(proof.baseClass)))return Fail(Failure::KeyType);
            Name again{};
            if(!r.Read(cell+schemadisk::EntryName,again)||name.index!=again.index||name.number!=again.number||
                !Same(r,cell+schemadisk::EntryKeyType,type))return Fail(Failure::Changed);
            keys_[key]=proof;mask_|=1u<<key;
        }
        if(!Links(r,a))return Fail(Failure::Changed);
        if(cursor_==static_cast<unsigned>(layer.keys.count)){
            total_+=cursor_;if(total_>MaxEntries)return Fail(Failure::Array);
            if(!layer.parent){sealed_=true;return state_;}
            if(layerCount_==MaxLayers)return Fail(Failure::ParentCycle);
            for(unsigned i=0;i<layerCount_;++i)if(layers_[i].slot.object==layer.parent)return Fail(Failure::ParentCycle);
            Layer next;if(!ReadLayer(r,a,layer.parent,next))return Fail(Failure::Asset);
            layers_[layerCount_++]=next;cursor_=0;
        }
        return state_;
    }
    template<class R> bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& a){
        if(!Links(r,a))return false;
        for(unsigned key=0;key<KeyCount;++key)if(mask_&(1u<<key)){
            const auto& p=keys_[key];Name name{};Slot slot{};
            if(!r.Read(p.cell+schemadisk::EntryName,name)||name.index!=p.name.index||name.number!=p.name.number||
                !Same(r,p.cell+schemadisk::EntryKeyType,p.type)||!Same(r,p.type+0x20,p.asset)||
                !Live(r,a,p.type,slot)||!(slot==p.slot)||!cfgprobe::StableAncestry(r,p.type,p.ancestry)||
                (key<=Jason&&!Same(r,p.type+schemadisk::ObjectBaseClass,p.baseClass)))return false;
        }return true;
    }
    template<class R> bool AcceptsObject(R& r,unsigned key,std::uintptr_t object){
        return key<=Jason&&Supports(key)&&Typed(r,object,keys_[key].baseClass);
    }
};
}
