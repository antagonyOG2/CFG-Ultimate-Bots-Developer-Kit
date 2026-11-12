#pragma once

namespace cfghostedmovement {
inline bool UseLegacyGapCloser(bool,bool)noexcept{
    return false;
}
inline bool LegacyGapCloserDue(bool,bool,float,unsigned long long,unsigned long long)noexcept{
    return false;
}
inline bool FootBurstAllowed(bool)noexcept{return true;}
}
