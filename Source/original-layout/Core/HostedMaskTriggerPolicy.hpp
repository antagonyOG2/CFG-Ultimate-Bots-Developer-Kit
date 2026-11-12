#pragma once
#include <string_view>
namespace cfghostedmask {
// Donor CounselorHasJasonMaskInInventory: the stock pickup is a generated
// MaskPickup item, not necessarily an SCKillerMask subclass. Ownership, native
// inventory-item type and stable slots are separate required runtime proofs.
inline bool PickupClass(std::u16string_view name){
    return name.find(u"MaskPickup")!=name.npos||name.find(u"Mask_Pickup")!=name.npos||
        name.find(u"KillerMask")!=name.npos;
}
}
