#pragma once
#include <cstdint>
namespace cfgmenu::pickerdiagnostic {
// Diagnostic-only values copied from reads the admission already performs.
// Zero-valued addresses/serials are meaningful only when their observation bit
// is set. No probe, fallback, retry or authorization is added by this record.
enum class Guard:std::uint32_t {None,Prerequisites,CheapFrame,OtherRole,EntryReceiver,ActiveContext,
    EntryBorrowed,EntrySource,OwnedConstruct,FirstConstruct,OwnedReceiver,SettingsSource,TransitionConsumer,TransitionFrame,Exception};
enum class Frame:std::uint32_t {None,Extent,Type,Node,Object,Code,Native,Locals,Entry};
enum class Construct:std::uint32_t {None,Metadata,Extent,Type,Slot,Serial,Node,Object,Code,Native,Role,Entry,Recheck};
enum Observed:std::uint32_t {Type=1,Node=2,Object=4,Code=8,Native=16,Serial=32};
struct Trace {
    std::uint32_t guardStage=0,frameStage=0,constructStage=0,metadataStage=0;
    std::uint32_t reads=0,cap=0,error=0,budgetAtLimit=0,observed=0;
    std::int32_t receiverSerial=0;
    std::uint32_t metadataRefreshStage=0,nativeSerialAssignments=0,sourceStage=0;
    std::uintptr_t node=0,object=0,code=0,native=0,type=0;
};
}
