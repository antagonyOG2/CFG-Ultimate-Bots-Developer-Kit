#pragma once
#include <cstdint>
#include <array>
namespace cfgmenu {
struct NativeMetadataDiagnostic {
    std::uint32_t operation=0,step=0,freshStage=0,borrowStage=0,borrowState=0,flags=0,role=0;
    std::uint32_t observedMask=0,packetMask=0,hostState=0,publishStatus=0;
    std::uintptr_t pawnClass=0,spawnedClass=0,primaryActor=0,publishObserved=0;
    std::uintptr_t expectedPawnClass=0,expectedKillerClass=0,publishAddress=0,publishExpected=0,publishNext=0;
    std::array<unsigned char,40> active{},klass{};
};
struct NativeHumanDiagnostic {
    std::uint32_t restartReturned=0,stage=0,ownerStage=0,reads=0;
    std::uintptr_t pawn=0,acknowledged=0,controller=0,state=0,type=0,spawnedClass=0;
    std::int32_t serial=0;
};
// Samples from existing startup reads only; no scans, retries or game calls.
struct NativeStartupDiagnostic {
    std::uint32_t phase=0,birthPhase=0,skillVerified=0,rosterIndex=0;
    std::int32_t dimensions=0,elementBytes=0,offset=0,count=0,capacity=0;
    std::uintptr_t field=0,selectedClass=0,controller=0,state=0,pawn=0;
    std::int32_t controllerSerial=0,stateSerial=0,pawnSerial=0;
    std::uint32_t entered=0;
    NativeHumanDiagnostic human{};
    NativeMetadataDiagnostic metadata{};
};
}
