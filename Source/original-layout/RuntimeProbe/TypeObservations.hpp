#pragma once
#include "ClassIdentity.hpp"
namespace cfgspawn {
// Snapshot-local deduplication only: capture each shared class hierarchy once,
// then recheck all sampled object classes and parent links before publication.
// Never retained between snapshots, never used as native-call authorization.
template<class Reader,std::size_t Capacity=128> class TypeObservations {
    static_assert(Capacity>0 && Capacity<=512,"Finite snapshot-local type cache");
    struct ObjectType { std::uintptr_t object=0,type=0; };
    Reader& reader_;
    std::array<cfgprobe::Ancestry,Capacity> classes_{};
    std::array<ObjectType,Capacity> objects_{};
    unsigned classCount_=0,objectCount_=0;
    bool valid_=true;
public:
    explicit TypeObservations(Reader& reader):reader_(reader) {}
    bool ClassContains(std::uintptr_t type,std::uintptr_t wanted) {
        if (!valid_ || !cfgprobe::ObjectPointer(wanted)) return false;
        for (unsigned i=0;i<classCount_;++i)
            if (classes_[i].classes[0]==type) return classes_[i].Contains(wanted);
        if (!cfgprobe::ObjectPointer(type) || classCount_==classes_.size()) { valid_=false; return false; }
        cfgprobe::Ancestry candidate;
        auto current=type;
        while (current) {
            if (!cfgprobe::ObjectPointer(current) || candidate.count==cfgprobe::MaximumAncestry) { valid_=false; return false; }
            for (std::size_t i=0;i<candidate.count;++i)
                if (candidate.classes[i]==current) { valid_=false; return false; }
            std::uintptr_t parent=0;
            if (!reader_.Read(current+cfgprobe::ClassSuper,parent)) { valid_=false; return false; }
            candidate.classes[candidate.count]=current;
            candidate.parents[candidate.count++]=parent;
            current=parent;
        }
        classes_[classCount_++]=candidate;
        return candidate.Contains(wanted);
    }
    bool ObjectIs(std::uintptr_t object,std::uintptr_t wanted) {
        if (!valid_ || !cfgprobe::ObjectPointer(object) || !cfgprobe::ObjectPointer(wanted)) return false;
        std::uintptr_t type=0;
        if (!reader_.Read(object+cfgprobe::ObjectClass,type)) { valid_=false; return false; }
        unsigned i=0;
        for (;i<objectCount_;++i) if (objects_[i].object==object) break;
        if (i<objectCount_) {
            if (objects_[i].type!=type) { valid_=false; return false; }
        } else {
            if (objectCount_==objects_.size()) { valid_=false; return false; }
            objects_[objectCount_++]={object,type};
        }
        return ClassContains(type,wanted);
    }
    bool Revalidate() {
        if (!valid_) return false;
        std::uintptr_t value=0;
        for (unsigned i=0;i<objectCount_;++i)
            if (!reader_.Read(objects_[i].object+cfgprobe::ObjectClass,value) || value!=objects_[i].type) return false;
        for (unsigned i=0;i<classCount_;++i)
            for (std::size_t j=0;j<classes_[i].count;++j)
                if (!reader_.Read(classes_[i].classes[j]+cfgprobe::ClassSuper,value) || value!=classes_[i].parents[j]) return false;
        return true;
    }
};
} // namespace cfgspawn
