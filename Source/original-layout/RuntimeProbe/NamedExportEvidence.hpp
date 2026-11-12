#pragma once
#include "EntryPrefixObservation.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace cfgprobe {
// Microsoft PE export directory, not executable code. All reads are bounded
// image metadata; this never follows an inline jump or chooses a load address.
struct ExportDirectory {
    std::uint32_t flags=0, timestamp=0;
    std::uint16_t major=0, minor=0;
    std::uint32_t name=0, ordinalBase=0, functionCount=0, nameCount=0;
    std::uint32_t functions=0, names=0, ordinals=0;
};
static_assert(sizeof(ExportDirectory)==40,"PE export directory layout");
struct NamedExportEvidence {
    std::uint32_t functionRva=0, nameRva=0;
    std::uint16_t ordinalIndex=0;
    bool forwarded=false;
    std::array<char,128> forwarder{};
};
inline bool ImageExtent(std::uint32_t imageSize, std::uint32_t rva,
    std::uint64_t bytes) noexcept {
    return rva!=0 && bytes!=0 && rva<imageSize && bytes<=imageSize-rva;
}
template<class Reader>
bool ExportAscii(Reader& reader, std::uintptr_t base, std::uint32_t rva,
    std::uint32_t exclusiveEnd, std::array<char,128>& output) {
    output={};
    if (!ImageExtent(exclusiveEnd,rva,1)) return false;
    const auto remaining=static_cast<std::size_t>(exclusiveEnd-rva);
    const auto bytes=remaining<output.size() ? remaining:output.size();
    if (!reader.ReadBytes(base+rva,output.data(),bytes)) return false;
    for (std::size_t i=0; i<bytes; ++i) {
        if (!output[i]) return i!=0;
        const auto byte=static_cast<unsigned char>(output[i]);
        if (byte<0x20 || byte>0x7E) return false;
    }
    return false; // No terminator within the bounded metadata read.
}
template<class Reader>
bool LocateNamedExport(Reader& reader, std::uintptr_t base, std::uint32_t imageSize,
    std::uint32_t directoryRva, std::uint32_t directoryBytes, const char* name,
    NamedExportEvidence& output) {
    output={};
    std::uintptr_t end=0;
    if (!name || !*name || !ObservedAdd(base,imageSize,end) ||
        directoryBytes<sizeof(ExportDirectory) || directoryBytes>1024*1024 ||
        !ImageExtent(imageSize,directoryRva,directoryBytes)) return false;
    ExportDirectory directory{};
    if (!reader.Read(base+directoryRva,directory) || !directory.functionCount ||
        directory.functionCount>65536 || !directory.nameCount ||
        directory.nameCount>65536 ||
        !ImageExtent(imageSize,directory.functions,static_cast<std::uint64_t>(directory.functionCount)*4) ||
        !ImageExtent(imageSize,directory.names,static_cast<std::uint64_t>(directory.nameCount)*4) ||
        !ImageExtent(imageSize,directory.ordinals,static_cast<std::uint64_t>(directory.nameCount)*2)) return false;
    const auto readName=[&](std::uint32_t index, std::uint32_t& rva,
        std::array<char,128>& text) {
        return index<directory.nameCount && reader.Read(base+directory.names+index*4,rva) &&
            ExportAscii(reader,base,rva,imageSize,text);
    };
    // PE name pointers are lexically ordered. Bounded binary lookup, no scan.
    std::uint32_t lo=0, hi=directory.nameCount;
    for (unsigned step=0; lo<hi && step<17; ++step) {
        const auto index=lo+(hi-lo)/2;
        std::uint32_t nameRva=0; std::array<char,128> text{};
        if (!readName(index,nameRva,text)) return false;
        const int comparison=std::strcmp(text.data(),name);
        if (comparison<0) { lo=index+1; continue; }
        if (comparison>0) { hi=index; continue; }
        // Reject local ordering errors/duplicate names at the matched record.
        for (const auto neighbor : {index ? index-1:directory.nameCount,index+1}) {
            if (neighbor>=directory.nameCount) continue;
            std::uint32_t adjacentRva=0; std::array<char,128> adjacent{};
            if (!readName(neighbor,adjacentRva,adjacent)) return false;
            const int order=std::strcmp(adjacent.data(),name);
            if ((neighbor<index && order>=0) || (neighbor>index && order<=0)) return false;
        }
        std::uint16_t ordinal=0; std::uint32_t function=0;
        if (!reader.Read(base+directory.ordinals+index*2,ordinal) || ordinal>=directory.functionCount ||
            !reader.Read(base+directory.functions+static_cast<std::uint32_t>(ordinal)*4,function) ||
            !ImageExtent(imageSize,function,1)) return false;
        NamedExportEvidence found{};
        found.functionRva=function; found.nameRva=nameRva; found.ordinalIndex=ordinal;
        const auto directoryEnd=directoryRva+directoryBytes;
        found.forwarded=function>=directoryRva && function<directoryEnd;
        if (found.forwarded && !ExportAscii(reader,base,function,directoryEnd,found.forwarder)) return false;
        output=found;
        return true;
    }
    return false;
}
inline bool SameNamedExport(const NamedExportEvidence& left,
    const NamedExportEvidence& right) noexcept {
    return left.functionRva && left.functionRva==right.functionRva &&
        left.nameRva==right.nameRva && left.ordinalIndex==right.ordinalIndex &&
        left.forwarded==right.forwarded && (!left.forwarded || left.forwarder==right.forwarder);
}
} // namespace cfgprobe
