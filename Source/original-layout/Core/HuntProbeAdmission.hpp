#pragma once
namespace cfghuntprobe {
// Independent optional probes must repay an ordinary hunt turn before another
// can queue. A700ms native Hunt cadence plus two1000ms probe timers otherwise
// alternates forever without reaching burst/melee/MoveTo. Pending work keeps
// its own native guards; this class grants no mutation authority.
struct Policy {
    bool ordinaryOwed=false;
    bool preferCar=false;
    bool MayProbe(){if(ordinaryOwed){ordinaryOwed=false;return false;}return true;}
    bool MorphFirst(bool carPossible)const{return !preferCar||!carPossible;}
    void Queued(bool car=false){ordinaryOwed=true;preferCar=!car;}
};
}
