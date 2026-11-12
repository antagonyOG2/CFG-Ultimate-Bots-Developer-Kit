#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cassert>
#include <cstdio>
struct Fixture {
    static constexpr std::uintptr_t Base=0x100000,Table=0x200000,Chunk=0x300000,Owner=0x500000,Parent=0x510000,
        Field=0x520000,Other=0x530000,PropertyClass=0x540000,Widget=0x550000;
    OwnedByteReader r;unsigned nextName=0;
    void Name(std::uintptr_t object,const char* label){
        const auto index=nextName++;const auto entry=std::uintptr_t{0x400000}+index*0x100;
        r.Set(Chunk+8*index,entry);r.Set(entry,std::uint32_t{0});std::array<unsigned char,128> text{};
        for(unsigned i=0;label[i]&&i<127;++i)text[i]=static_cast<unsigned char>(label[i]);
        r.Set(entry+cfgnames::disk::EntryText,text);r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});
    }
    void Member(std::uintptr_t field,std::uintptr_t owner,const char* name,std::uintptr_t next,int offset){
        Name(field,name);r.Set(field+cfgprobe::ObjectClass,PropertyClass);r.Set(field+0x20,owner);r.Set(field+cfgreflection::disk::Next,next);
        r.Set(field+cfgreflection::disk::PropertyDimensions,1);r.Set(field+cfgreflection::disk::PropertyElementSize,8);
        r.Set(field+cfgreflection::disk::PropertyFlags,std::uint64_t{0});r.Set(field+cfgreflection::disk::PropertyOffset,offset);
    }
    Fixture(){
        r.Set(Base+cfgnames::disk::Global,Table);r.Set(Table+cfgnames::disk::ElementCount,std::array<int,2>{100,1});r.Set(Table,Chunk);
        Name(PropertyClass,"ObjectProperty");Member(Field,Owner,"DifficultyComboBox",Other,0x180);Member(Other,Owner,"CounselorCountComboBox",0,0x188);
        r.Set(Owner+cfgreflection::disk::Children,Field);r.Set(Owner+cfgprobe::ClassSuper,Parent);
        r.Set(Parent+cfgreflection::disk::Children,std::uintptr_t{0});r.Set(Parent+cfgprobe::ClassSuper,std::uintptr_t{0});
        r.Set(Widget+cfgprobe::ObjectClass,Owner);r.Set(Widget+0x180,std::uintptr_t{0x600000});
    }
};
int main(){
    // Same receiver-resolved lookup used by CFG's spectator-status wrapper:
    // native base cache must not win over the visible widget implementation.
    {Fixture status;status.Name(Fixture::PropertyClass,"Function");
     status.Member(Fixture::Field,Fixture::Owner,"OnSpectatorUpdateStatus",0,0);
     status.Member(Fixture::Other,Fixture::Parent,"OnSpectatorUpdateStatus",0,0);
     status.r.Set(Fixture::Parent+cfgreflection::disk::Children,Fixture::Other);
     cfgreflection::Observation<OwnedByteReader> lookup(status.r,Fixture::Base);cfgreflection::Field selected;
     assert(lookup.Find(Fixture::Owner,u"OnSpectatorUpdateStatus",selected,true)&&selected.object==Fixture::Field);
     unsigned visibleUpdates=0;const auto dispatch=[&](std::uintptr_t function){if(function==Fixture::Field)++visibleUpdates;};
     dispatch(Fixture::Other);assert(!visibleUpdates);dispatch(selected.object);assert(visibleUpdates==1);
     status.r.Set(Fixture::Owner+cfgreflection::disk::Children,std::uintptr_t{});
     assert(lookup.Find(Fixture::Owner,u"OnSpectatorUpdateStatus",selected,true)&&selected.object==Fixture::Other);
     status.r.Set(Fixture::Owner+cfgreflection::disk::Children,Fixture::Field);
     status.Member(Fixture::Widget,Fixture::Owner,"OnSpectatorUpdateStatus",0,0);
     status.r.Set(Fixture::Field+cfgreflection::disk::Next,Fixture::Widget);
     assert(!lookup.Find(Fixture::Owner,u"OnSpectatorUpdateStatus",selected,true));}
    using namespace cfgreflection;Fixture f;Observation<OwnedByteReader> observe(f.r,Fixture::Base);Field field;Property property;std::uintptr_t value=0;
    assert(observe.Find(Fixture::Owner,u"DifficultyComboBox",field)&&field.object==Fixture::Field);
    assert(observe.ReadProperty(field,property)&&property.offset==0x180&&property.elementBytes==8);
    assert(observe.ReadObjectMember(Fixture::Widget,Fixture::Owner,u"DifficultyComboBox",value)&&value==0x600000);
    assert(!observe.Find(Fixture::Owner,u"Missing",field)&&!field.object);
    Fixture cycle;cycle.r.Set(Fixture::Other+disk::Next,Fixture::Field);Observation<OwnedByteReader> cycleReader(cycle.r,Fixture::Base);
    assert(!cycleReader.Find(Fixture::Owner,u"DifficultyComboBox",field)&&!field.object);
    Fixture duplicate;duplicate.Member(Fixture::Other,Fixture::Owner,"DifficultyComboBox",0,0x188);Observation<OwnedByteReader> duplicateReader(duplicate.r,Fixture::Base);
    assert(!duplicateReader.Find(Fixture::Owner,u"DifficultyComboBox",field));
    Fixture wrongOwner;wrongOwner.r.Set(Fixture::Field+0x20,Fixture::Parent);Observation<OwnedByteReader> wrongOwnerReader(wrongOwner.r,Fixture::Base);
    assert(!wrongOwnerReader.Find(Fixture::Owner,u"DifficultyComboBox",field));
    Fixture inherited;inherited.r.Set(Fixture::Owner+disk::Children,std::uintptr_t{0});inherited.r.Set(Fixture::Parent+disk::Children,Fixture::Field);
    inherited.r.Set(Fixture::Field+0x20,Fixture::Parent);inherited.r.Set(Fixture::Other+0x20,Fixture::Parent);Observation<OwnedByteReader> inheritedReader(inherited.r,Fixture::Base);
    assert(inheritedReader.Find(Fixture::Owner,u"DifficultyComboBox",field)&&field.owner==Fixture::Parent);
    Fixture mismatch;Observation<OwnedByteReader> mismatchReader(mismatch.r,Fixture::Base);mismatch.r.Set(Fixture::Widget+cfgprobe::ObjectClass,Fixture::Parent);
    assert(!mismatchReader.ReadObjectMember(Fixture::Widget,Fixture::Owner,u"DifficultyComboBox",value)&&!value);
    Fixture bad;Observation<OwnedByteReader> badReader(bad.r,Fixture::Base);assert(badReader.Find(Fixture::Owner,u"DifficultyComboBox",field));
    bad.r.Set(Fixture::Field+disk::PropertyOffset,65535);assert(!badReader.ReadProperty(field,property));
    Fixture unaligned;unaligned.r.Set(Fixture::Field+disk::PropertyOffset,0x181);Observation<OwnedByteReader> unalignedReader(unaligned.r,Fixture::Base);
    assert(!unalignedReader.ReadObjectMember(Fixture::Widget,Fixture::Owner,u"DifficultyComboBox",value));
    Fixture relink;relink.r.changeAt=Fixture::Field+cfgnames::disk::ObjectName;relink.r.changeSlot=Fixture::Owner+disk::Children;relink.r.newPointer=Fixture::Other;
    Observation<OwnedByteReader> relinking(relink.r,Fixture::Base);assert(!relinking.Find(Fixture::Owner,u"DifficultyComboBox",field));
    Fixture storageChange;storageChange.r.changeAt=Fixture::Widget+0x180;
    storageChange.r.changeSlot=Fixture::Field+disk::PropertyOffset;storageChange.r.newPointer=0x188;
    Observation<OwnedByteReader> changingStorage(storageChange.r,Fixture::Base);
    assert(!changingStorage.ReadObjectMember(Fixture::Widget,Fixture::Owner,u"DifficultyComboBox",value)&&!value);
    Fixture parentChange;parentChange.r.Set(Fixture::Owner+disk::Children,std::uintptr_t{0});
    parentChange.r.Set(Fixture::Parent+disk::Children,Fixture::Field);parentChange.r.Set(Fixture::Field+0x20,Fixture::Parent);parentChange.r.Set(Fixture::Other+0x20,Fixture::Parent);
    parentChange.r.changeAt=Fixture::Field+cfgnames::disk::ObjectName;parentChange.r.changeSlot=Fixture::Owner+cfgprobe::ClassSuper;parentChange.r.newPointer=0;
    Observation<OwnedByteReader> changingParent(parentChange.r,Fixture::Base);assert(!changingParent.Find(Fixture::Owner,u"DifficultyComboBox",field));
    Fixture numbered;cfgnames::Words numberedName{};assert(numbered.r.Read(Fixture::Field+cfgnames::disk::ObjectName,numberedName));
    numberedName.number=1;numbered.r.Set(Fixture::Field+cfgnames::disk::ObjectName,numberedName);
    Observation<OwnedByteReader> numberedReader(numbered.r,Fixture::Base);
    assert(!numberedReader.Find(Fixture::Owner,u"DifficultyComboBox",field));
    assert(numberedReader.Find(Fixture::Owner,u"DifficultyComboBox_0",field));
    std::puts("CFG reflected menu member observations PASS: named native fields, inherited lookup, duplicate/cycle/owner/relink/schema refusal; no game access.");
    return 0;
}
