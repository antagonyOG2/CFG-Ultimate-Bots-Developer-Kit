#include "SweaterPickup.hpp"
#include "OwnedByteReader.hpp"
#include "CounselorTerminalObservation.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;auto check=[&](bool okay){++checks;if(!okay)++failures;};
 constexpr std::uintptr_t module=0x100000,replacement=0xA00000;OwnedByteReader r;cfgsweater::PickupTable source{},out{};
 check(cfgsweater::pickupdisk::RuntimeVerified);
 check(!cfgsweater::BuildPickupTable(r,module,replacement,out));
 source[0]=module+cfgsweater::pickupdisk::Prefix;
 for(unsigned i=0;i<cfgsweater::pickupdisk::Entries;++i)source[i+1]=module+cfgsweater::pickupdisk::VtableRvas[i];
 r.Set(module+cfgsweater::pickupdisk::Vtable-8,source);
 check(cfgsweater::BuildPickupTable(r,module,replacement,out));
 for(unsigned i=0;i<out.size();++i)check(out[i]==(i==1+cfgsweater::pickupdisk::Slot/8?replacement:source[i]));
 source[8]^=8;r.Set(module+cfgsweater::pickupdisk::Vtable-8,source);
 check(!cfgsweater::BuildPickupTable(r,module,replacement,out));
 check(!cfgsweater::BuildPickupTable(r,module,0,out));
 cfgsweater::PickupState s;s.installed=1;s.clone=replacement;s.module=module;s.world=0x200000;s.source={0x300000,2,0};
 check(cfgsweater::PickupScope(s,module,s.world,s.source.object));check(!cfgsweater::PickupScope(s,module,s.world+8,s.source.object));
 check(!cfgsweater::PickupScope(s,module,s.world,s.source.object+8));s.installed=0;check(!cfgsweater::PickupScope(s,module,s.world,s.source.object));
 // Byte-valued guards must not silently compare eight neighboring bytes.
 r.Set(0x400000,std::array<std::uint8_t,8>{0,0xFF,0xFE,0xFD,0xFC,0xFB,0xFA,0xF9});
 check(cfgescape::Same(r,0x400000,std::uint8_t{}));check(!cfgescape::Same(r,0x400000,std::uintptr_t{}));
 constexpr std::uintptr_t field=0x500000,items=0x510000,wanted=0x600000;
 cfgsweater::PickupList list{},header{items,3,3};unsigned index=99;
 r.Set(field,header);r.Set(items,std::array<std::uintptr_t,3>{0x610000,0x620000,wanted});
 check(cfgsweater::LocatePickup(r,field,wanted,list,index)&&index==2);
 // Another pickup compacts WorldItems; the same source is now slot0.
 r.Set(field,cfgsweater::PickupList{items,1,3});r.Set(items,wanted);
 check(cfgsweater::LocatePickup(r,field,wanted,list,index)&&index==0);
 r.Set(field,header);r.Set(items,std::array<std::uintptr_t,3>{wanted,0x620000,wanted});
 check(!cfgsweater::LocatePickup(r,field,wanted,list,index));
 r.Set(items,std::array<std::uintptr_t,3>{0x610000,0x620000,0x630000});
 check(!cfgsweater::LocatePickup(r,field,wanted,list,index));
 r.Set(field,cfgsweater::PickupList{items,257,257});check(!cfgsweater::LocatePickup(r,field,wanted,list,index));
 r.Set(field,cfgsweater::PickupList{items,1,3});r.Set(items,wanted);
 r.changeAt=items;r.changeSlot=field;r.newPointer=items+8;
 check(!cfgsweater::LocatePickup(r,field,wanted,list,index));
 std::printf("Sweater pickup checks=%u failures=%u; exact retained source survives list compaction; missing/duplicate/mutated lists refuse\n",checks,failures);return failures?1:0;
}
