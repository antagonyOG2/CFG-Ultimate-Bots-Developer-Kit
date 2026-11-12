#pragma once
#include "ClassIdentity.hpp"
#include <array>
#include <cstdint>
#include "Compatibility/CfgBuildProfile.hpp"

namespace cfgprobe::lifetime {
// Exact disk corroboration: Inspect-CfgObjectLifetime.ps1. READ-ONLY evidence,
// not permission to call native functions or claim GC/thread/lifetime safety.
constexpr std::uintptr_t ObjectArrayRva=cfgcompat::cfg35::objectArray::GlobalRva;
constexpr std::uintptr_t ObjectIndexOffset=cfgcompat::cfg35::objectArray::ObjectIndex;
constexpr std::uintptr_t ArrayDataOffset=cfgcompat::cfg35::objectArray::Data;
constexpr std::uintptr_t ArrayCountOffset=cfgcompat::cfg35::objectArray::Count;
constexpr std::uintptr_t ItemBytes=cfgcompat::cfg35::objectArray::ItemBytes;
constexpr std::uintptr_t ItemObjectOffset=cfgcompat::cfg35::objectArray::Object;
constexpr std::uintptr_t ItemSerialOffset=cfgcompat::cfg35::objectArray::Serial;
constexpr std::uintptr_t LastUserByte=0x00007FFFFFFFFFFF;
constexpr std::int32_t MaximumDiagnosticObjects=8388608; // Bound, not gameplay capacity.
inline bool Extent(std::uintptr_t address,std::uintptr_t bytes) noexcept {
    return bytes && address>=0x0/*CFG3.5 address withheld*/ && bytes<=LastUserByte && address<=LastUserByte-(bytes-1);
}
struct ArrayHeader {
    std::uintptr_t data=0;
    std::int32_t count=0;
    bool Sane() const noexcept {
        return ObjectPointer(data) && count>0 && count<=MaximumDiagnosticObjects &&
            Extent(data,static_cast<std::uintptr_t>(count)*ItemBytes);
    }
    bool operator==(const ArrayHeader& other) const noexcept {
        return data==other.data && count==other.count;
    }
};
struct SlotObservation {
    std::uintptr_t object=0;
    std::int32_t index=-1, serial=0;
    bool HasAssignedSerial() const noexcept { return ObjectPointer(object) && index>=0 && serial>0; }
    bool operator==(const SlotObservation& other) const noexcept {
        return object==other.object && index==other.index && serial==other.serial;
    }
};
template<class Reader> bool ReadArrayHeader(Reader& reader,std::uintptr_t array,ArrayHeader& output) {
    output={};
    ArrayHeader candidate;
    if (!ObjectPointer(array) || !Extent(array,ArrayCountOffset+sizeof(candidate.count)) ||
        !reader.Read(array+ArrayDataOffset,candidate.data) ||
        !reader.Read(array+ArrayCountOffset,candidate.count) || !candidate.Sane()) return false;
    output=candidate; return true;
}
template<class Reader> bool ReadSlot(Reader& reader,const ArrayHeader& header,
    std::uintptr_t object,SlotObservation& output) {
    output={};
    if (!header.Sane() || !ObjectPointer(object) ||
        !Extent(object,ObjectIndexOffset+sizeof(std::int32_t))) return false;
    SlotObservation candidate; candidate.object=object;
    if (!reader.Read(object+ObjectIndexOffset,candidate.index) ||
        candidate.index<0 || candidate.index>=header.count) return false;
    // Sane() checked the full finite array extent; this index is within it.
    const auto slot=header.data+static_cast<std::uintptr_t>(candidate.index)*ItemBytes;
    std::uintptr_t slotObject=0;
    if (!reader.Read(slot+ItemObjectOffset,slotObject) || slotObject!=object ||
        !reader.Read(slot+ItemSerialOffset,candidate.serial) || candidate.serial<0) return false;
    // Zero is a valid observation of an UNASSIGNED serial, not a reusable identity.
    // Never call the native serial allocator to make this observation positive.
    output=candidate; return true;
}
template<class Reader,std::size_t N> bool ObserveBatch(Reader& reader,std::uintptr_t array,
    const std::array<std::uintptr_t,N>& objects,ArrayHeader& header,
    std::array<SlotObservation,N>& output) {
    static_assert(N>0 && N<=8,"Bounded known-object diagnostic only");
    header={}; output={};
    ArrayHeader candidateHeader;
    std::array<SlotObservation,N> candidate{};
    if (!ReadArrayHeader(reader,array,candidateHeader)) return false;
    for (std::size_t i=0; i<N; ++i)
        if (!ReadSlot(reader,candidateHeader,objects[i],candidate[i])) return false;
    header=candidateHeader; output=candidate; return true;
}
template<class Reader,std::size_t N> bool RevalidateBatch(Reader& reader,std::uintptr_t array,
    const ArrayHeader& header,const std::array<SlotObservation,N>& observed) {
    static_assert(N>0 && N<=8,"Bounded known-object diagnostic only");
    ArrayHeader freshHeader;
    if (!header.Sane() || !ReadArrayHeader(reader,array,freshHeader) || !(freshHeader==header)) return false;
    for (std::size_t i=0; i<N; ++i) {
        SlotObservation fresh;
        if (!ReadSlot(reader,freshHeader,observed[i].object,fresh) || !(fresh==observed[i])) return false;
    }
    return true; // Stable sampled identity ONLY, not a lifetime pin or native-call approval.
}
// Diagnostic captures run while ordinary gameplay allocates and retires objects.
// A global array relocation/count change is not itself evidence that these known
// roots changed identity; re-read each bounded slot against the current header.
template<class Reader,std::size_t N> bool RevalidateSlotsCurrentHeader(Reader& reader,std::uintptr_t array,
    const ArrayHeader& observedHeader,const std::array<SlotObservation,N>& observed,
    bool* headerChanged=nullptr) {
    static_assert(N>0 && N<=8,"Bounded known-object diagnostic only");
    ArrayHeader current;
    if(!observedHeader.Sane()||!ReadArrayHeader(reader,array,current))return false;
    if(headerChanged)*headerChanged=!(current==observedHeader);
    for(std::size_t i=0;i<N;++i){
        SlotObservation fresh;
        if(!ReadSlot(reader,current,observed[i].object,fresh)||!(fresh==observed[i]))return false;
    }
    return true; // Current slot identities match; no lifetime pin is implied.
}
} // namespace cfgprobe::lifetime
