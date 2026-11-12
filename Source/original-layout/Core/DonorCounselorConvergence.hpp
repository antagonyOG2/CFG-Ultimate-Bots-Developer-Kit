#pragma once
#include "DonorJasonTargeting.hpp"
#include <algorithm>
#include <cstring>
#include <string>
namespace cfgdonorconvergence {
using Pawn=std::uintptr_t;
using Position=cfgdonorhunt::Position;
// ACTIVE rc1 FJB9687-9836,10004-10455, not the inactive stock convergence.
// Host supplies current-world/positive-birth/native/retention proofs and all
// 3.5 bindings. No raw donor engine offset or retained UObject lease lives here.
struct State {
 static constexpr std::int32_t JasonAITargetCapacity=64;
 bool CounselorConvergenceActive=false;
 std::uint64_t NextCounselorConvergenceAt=0,NextConvergenceCombatSweepAt=0;
 std::int32_t SweaterPreconvergenceCursor=0,CounselorConvergenceCursor=0;
 Pawn CounselorConvergenceMeleeClass=0,CounselorConvergenceMacheteClass=0;
 std::int32_t ConvergenceWeaponClassScanCursor=0;
 std::uint64_t NextConvergenceWeaponClassScanAt=0;
 Pawn ConvergenceArmedCounselors[JasonAITargetCapacity]{};
 std::int32_t ConvergenceArmedCounselorCount=0;
 Pawn ConvergenceTravelCounselors[JasonAITargetCapacity]{};
 std::int32_t ConvergenceTravelCounselorCount=0;
 Position ConvergenceTravelLastLocations[JasonAITargetCapacity]{};
 std::uint64_t ConvergenceTravelLastProgressAt[JasonAITargetCapacity]{},ConvergenceTravelRecoveryUntil[JasonAITargetCapacity]{};
 bool ConvergenceTravelHaveLocation[JasonAITargetCapacity]{};
 Position ConvergenceTravelLastGoals[JasonAITargetCapacity]{};
 std::uint64_t ConvergenceTravelLastRouteAt[JasonAITargetCapacity]{};
 bool ConvergenceTravelHaveGoal[JasonAITargetCapacity]{};
    template<class Host> bool IsStrongConvergenceCounselor(Host& host,Pawn counselor){
        if(!counselor)return false;
        const char* strongTypes[]{"Jock","Tough","Biker","Tommy","Hero","Rob","Julius","Boxer","Mark"};
        for(const auto* type:strongTypes)if(host.ObjectDerivesFromNameContaining(counselor,type))return true;
        return false;
    }
    template<class Host> void PrecacheConvergenceWeaponClassesSlice(Host& host,std::uint64_t now){
        if(now<NextConvergenceWeaponClassScanAt||
           (CounselorConvergenceMeleeClass&&CounselorConvergenceMacheteClass))return;
        NextConvergenceWeaponClassScanAt=now+250;
        // Host reads the CURRENT 3.5 registry/count and validates each name/
        // class candidate. No donor GObjects offset or raw name-table access.
        const auto count=host.CurrentObjectCount();
        if(count<=0||count>=20000000)return;
        if(ConvergenceWeaponClassScanCursor>=count)ConvergenceWeaponClassScanCursor=0;
        const auto end=(std::min)(count,ConvergenceWeaponClassScanCursor+512);
        for(auto i=ConvergenceWeaponClassScanCursor;i<end;++i){
            Pawn object=0;std::string name;
            if(!host.ReadClassCandidate(i,object,name))continue;
            Pawn* cache=nullptr;
            if(name=="CounselorTwoHandedAxe_C")cache=&CounselorConvergenceMeleeClass;
            else if(name=="CounselorMachete_C")cache=&CounselorConvergenceMacheteClass;
            if(!cache||*cache||!host.CurrentUClass(object))continue;
            *cache=object;
        }
        ConvergenceWeaponClassScanCursor=end;
        if(end>=count&&(!CounselorConvergenceMeleeClass||!CounselorConvergenceMacheteClass)){
            ConvergenceWeaponClassScanCursor=0;NextConvergenceWeaponClassScanAt=now+30000;
        }
    }
    template<class Host> bool GiveConvergenceMeleeWeapon(Host& host,Pawn counselor,std::uint64_t now){
        if(!counselor||host.IsCounselorInPostGrabTransition(counselor,now))return false;
        PrecacheConvergenceWeaponClassesSlice(host,now);
        // Hosted users requested a uniform axe for every bot at convergence.
        // Keep J2.19's strong-archetype axe / ordinary machete rule offline.
        const bool strong=host.HostedOnlineBridge()||IsStrongConvergenceCounselor(host,counselor);
        auto weaponClass=strong?CounselorConvergenceMeleeClass:CounselorConvergenceMacheteClass;
        if(!weaponClass)weaponClass=CounselorConvergenceMeleeClass?CounselorConvergenceMeleeClass:CounselorConvergenceMacheteClass;
        if(!weaponClass||!host.CurrentUClass(weaponClass))return false;
        // Returns true only for a confirmed native grant. The host owns code/
        // current actor proofs, allocating-call retention and MarkUncertain.
        return host.GiveStartingItem(counselor,weaponClass);
    }
    std::int32_t FindConvergenceTravelCounselor(Pawn counselor)
    {
        for (std::int32_t i = 0; i < ConvergenceTravelCounselorCount; ++i)
        {
            if (ConvergenceTravelCounselors[i] == counselor)
                return i;
        }
        return -1;
    }

    void RememberConvergenceTravelCounselor(Pawn counselor)
    {
        if (!counselor ||
            FindConvergenceTravelCounselor(counselor) >= 0 ||
            ConvergenceTravelCounselorCount >=
                JasonAITargetCapacity)
        {
            return;
        }
        const std::int32_t index = ConvergenceTravelCounselorCount++;
        ConvergenceTravelCounselors[index] = counselor;
        ConvergenceTravelLastLocations[index] = Position{};
        ConvergenceTravelLastProgressAt[index] = 0;
        ConvergenceTravelRecoveryUntil[index] = 0;
        ConvergenceTravelHaveLocation[index] = false;
        ConvergenceTravelLastGoals[index] = Position{};
        ConvergenceTravelLastRouteAt[index] = 0;
        ConvergenceTravelHaveGoal[index] = false;
    }

    void ForgetConvergenceTravelCounselor(Pawn counselor)
    {
        const std::int32_t index = FindConvergenceTravelCounselor(counselor);
        if (index < 0)
            return;
        --ConvergenceTravelCounselorCount;
        ConvergenceTravelCounselors[index] =
            ConvergenceTravelCounselors[ConvergenceTravelCounselorCount];
        ConvergenceTravelLastLocations[index] =
            ConvergenceTravelLastLocations[ConvergenceTravelCounselorCount];
        ConvergenceTravelLastProgressAt[index] =
            ConvergenceTravelLastProgressAt[ConvergenceTravelCounselorCount];
        ConvergenceTravelRecoveryUntil[index] =
            ConvergenceTravelRecoveryUntil[ConvergenceTravelCounselorCount];
        ConvergenceTravelHaveLocation[index] =
            ConvergenceTravelHaveLocation[ConvergenceTravelCounselorCount];
        ConvergenceTravelLastGoals[index] =
            ConvergenceTravelLastGoals[ConvergenceTravelCounselorCount];
        ConvergenceTravelLastRouteAt[index] =
            ConvergenceTravelLastRouteAt[ConvergenceTravelCounselorCount];
        ConvergenceTravelHaveGoal[index] =
            ConvergenceTravelHaveGoal[ConvergenceTravelCounselorCount];
        ConvergenceTravelCounselors[
            ConvergenceTravelCounselorCount] = 0;
        ConvergenceTravelLastLocations[
            ConvergenceTravelCounselorCount] = Position{};
        ConvergenceTravelLastProgressAt[
            ConvergenceTravelCounselorCount] = 0;
        ConvergenceTravelRecoveryUntil[
            ConvergenceTravelCounselorCount] = 0;
        ConvergenceTravelHaveLocation[
            ConvergenceTravelCounselorCount] = false;
        ConvergenceTravelLastGoals[
            ConvergenceTravelCounselorCount] = Position{};
        ConvergenceTravelLastRouteAt[
            ConvergenceTravelCounselorCount] = 0;
        ConvergenceTravelHaveGoal[
            ConvergenceTravelCounselorCount] = false;
    }

    template<class Host> void ResumeAllConvergenceCounselorBrains(Host& host)
    {
        for (std::int32_t i = 0; i < ConvergenceTravelCounselorCount; ++i)
        {
            Pawn counselor = ConvergenceTravelCounselors[i];
            if (counselor && host.IsValidatedLiveCounselorPawn(counselor))
                host.SetCounselorBrainTickEnabled(counselor, true);
        }
        std::memset(
            ConvergenceTravelCounselors,
            0,
            sizeof(ConvergenceTravelCounselors));
        std::memset(
            ConvergenceTravelLastLocations,
            0,
            sizeof(ConvergenceTravelLastLocations));
        std::memset(
            ConvergenceTravelLastProgressAt,
            0,
            sizeof(ConvergenceTravelLastProgressAt));
        std::memset(
            ConvergenceTravelRecoveryUntil,
            0,
            sizeof(ConvergenceTravelRecoveryUntil));
        std::memset(
            ConvergenceTravelHaveLocation,
            0,
            sizeof(ConvergenceTravelHaveLocation));
        std::memset(
            ConvergenceTravelLastGoals,
            0,
            sizeof(ConvergenceTravelLastGoals));
        std::memset(
            ConvergenceTravelLastRouteAt,
            0,
            sizeof(ConvergenceTravelLastRouteAt));
        std::memset(
            ConvergenceTravelHaveGoal,
            0,
            sizeof(ConvergenceTravelHaveGoal));
        ConvergenceTravelCounselorCount = 0;
    }

    bool IsConvergenceCounselorArmed(Pawn counselor)
    {
        for (std::int32_t i = 0;
            i < ConvergenceArmedCounselorCount;
            ++i)
        {
            if (ConvergenceArmedCounselors[i] == counselor)
                return true;
        }
        return false;
    }

    void RememberConvergenceCounselor(Pawn counselor)
    {
        if (!counselor || IsConvergenceCounselorArmed(counselor) ||
            ConvergenceArmedCounselorCount >=
                JasonAITargetCapacity)
        {
            return;
        }
        ConvergenceArmedCounselors[
            ConvergenceArmedCounselorCount++] = counselor;
    }

    template<class Host> void DriveCounselorSweaterConvergence(Host& host, std::uint64_t now)
    {
        if (host.HostedOnlineBridge() && !host.IsHostedAuthorityWorld())
            return;
        if (now < NextCounselorConvergenceAt)
            return;

        if (!CounselorConvergenceActive)
        {
            bool sweaterAcquired = host.PermanentHumanSweaterLatched();
            // Mask removal is also an earned convergence trigger. Granting
            // Pamela power still requires the actual held mask inventory.
            if (!sweaterAcquired && host.HostedOnlineBridge())
                sweaterAcquired = host.RegisteredUnmaskedJasonInWorld();
            if (!sweaterAcquired)
            {
                // Before pickup, the old 1.5-second check validated the
                // entire roster on one game frame (31-63 ms in the live
                // startup trace). Inspect one pawn every 211 ms instead;
                // all seven are still covered within about 1.5 seconds.
                const std::int32_t count = (std::min)(
                    host.TargetCount(), JasonAITargetCapacity);
                if (count > 0)
                {
                    const std::int32_t index =
                        SweaterPreconvergenceCursor % count;
                    SweaterPreconvergenceCursor = (index + 1) % count;
                    Pawn counselor = host.Target(index);
                    sweaterAcquired =
                        host.IsValidatedLiveCounselorPawn(counselor) &&
                        host.HasPamelaSweater(counselor);
                }
            }
            if (!sweaterAcquired)
            {
                NextCounselorConvergenceAt = now + 211;
                return;
            }

            CounselorConvergenceActive = true;
            NextCounselorConvergenceAt = now;
            host.Log(
                "18L-BF sweater acquired or mask removed: surviving counselor melee convergence armed");
        }

        // Keep every armed counselor in the native melee branch throughout
        // both the trance and JasonDeath kneel. The final interaction is
        // attack-enabled, so a real bot swing must remain eligible to acquire
        // it; stopping convergence when the first slash created JasonDeath_C
        // left the whole roster staring at a stationary Jason. Retire the
        // convergence controller only after the stock interaction commits.
        if (host.KillTeamFinalInteractionCommitted())
        {
            // JasonDeath owns the native final interaction. Restore every
            // counselor brain before handing control to that stock sequence.
            ResumeAllConvergenceCounselorBrains(host);
            NextCounselorConvergenceAt = now + 2003;
            return;
        }
        // Hide/orientation are optional convergence hints. Do not delay the
        // entire rescue team waiting for unrelated flee-state name discovery.
        if (host.Key("ShouldFleeKiller") < 0 ||
            host.Key("ShouldFightBack") < 0 ||
            host.Key("ShouldArmedFightBack") < 0 ||
            host.Key("ShouldMeleeFightBack") < 0 ||
            host.Key("SeekWeaponWhileFleeing") < 0 ||
            host.Key("JasonCharacter") < 0)
        {
            NextCounselorConvergenceAt = now + 2003;
            return;
        }

        const std::int32_t count =
            (std::min)(host.TargetCount(), JasonAITargetCapacity);
        if (count <= 0)
        {
            NextCounselorConvergenceAt = now + 4513;
            return;
        }

        // Prime the entire live AI roster when convergence first becomes
        // active.  The old cursor-only path armed one counselor per slice,
        // which allowed the remaining stock brains to keep looting or fleeing
        // until their turn arrived.  A single bounded pass gives every bot a
        // Jason target and one persistent native move request; subsequent
        // slices continue servicing one bot at a time, so this does not create
        // a per-frame ProcessEvent/path-command burst.
        Position jasonLocation{};
        const bool haveJasonLocation =
            host.GetJasonAIActorLocation(host.GetJasonDeathTarget(), jasonLocation);

        // Do not refresh the whole roster here. MoveToLocation performs a
        // synchronous navigation rebuild, and the former 750 ms all-counselor
        // sweep stacked those rebuilds on one frame. The rotating service below
        // already republishes combat state and maintains each accepted path.
        std::int32_t rosterPrimed = 0;
        std::int32_t travelQueued = 0;
        for (std::int32_t i = 0; i < count; ++i)
        {
            Pawn counselor = host.Target(i);
            if (!counselor || counselor == host.LocalCounselorTarget() ||
                host.IsCounselorInPostGrabTransition(counselor, now) ||
                !host.IsValidatedLiveCounselorPawn(counselor) ||
                !host.IsAIControlledCounselor(counselor) ||
                IsConvergenceCounselorArmed(counselor))
            {
                continue;
            }

            host.GiveConvergenceMeleeWeapon(counselor);
            RememberConvergenceCounselor(counselor);
            host.SuppressConvergenceCounselorFear(counselor);

            Pawn controller = host.GetPawnControllerSafe(counselor);
            Pawn blackboard = controller
                ? host.GetCounselorBlackboardCached(controller)
                : 0;
            Pawn weapon = host.GetCounselorCurrentWeapon(counselor);
            if (!weapon && blackboard && host.Key("SCWeapon") >= 0)
            {
                weapon = host.GetBlackboardObject(blackboard, host.Key("SCWeapon"));
            }
            if (!weapon)
            {
                continue;
            }

            host.ArmKillTeamCounselorCombat(counselor);
            ++rosterPrimed;

            if (!controller || !haveJasonLocation)
            {
                continue;
            }

            Position counselorLocation{};
            if (!host.GetJasonAIActorLocation(counselor, counselorLocation))
            {
                continue;
            }
            const float dx = counselorLocation.X - jasonLocation.X;
            const float dy = counselorLocation.Y - jasonLocation.Y;
            const float distanceSquared = dx * dx + dy * dy;
            if (!std::isfinite(distanceSquared) ||
                distanceSquared <= 300.0f * 300.0f)
            {
                continue;
            }

            if (host.SetCounselorBrainTickEnabled(counselor, false))
            {
                RememberConvergenceTravelCounselor(counselor);
                // Queue the bot for the rotating route service instead of
                // issuing every initial path synchronously on this frame.
                ++travelQueued;
                host.Log(
                    "18L-BH counselor exclusive Jason travel engaged | pawn=" +
                    host.JasonAISafeName(counselor));
            }
        }
        if (rosterPrimed > 0)
        {
            host.Log(
                "18L-BF counselor convergence roster primed | armed=" +
                std::to_string(rosterPrimed) +
                " | staggeredPaths=" + std::to_string(travelQueued) +
                " | registeredTargets=" + std::to_string(count) +
                " | axeClass=" +
                    (CounselorConvergenceMeleeClass ? "Y" : "N") +
                " | macheteClass=" +
                    (CounselorConvergenceMacheteClass ? "Y" : "N"));
        }

        Pawn selected = 0;
        bool firstArm = false;
        for (std::int32_t attempt = 0; attempt < count; ++attempt)
        {
            const std::int32_t index =
                (CounselorConvergenceCursor + attempt) % count;
            Pawn counselor = host.Target(index);
            if (!counselor || counselor == host.LocalCounselorTarget() ||
                host.IsCounselorInPostGrabTransition(counselor, now) ||
                !host.IsValidatedLiveCounselorPawn(counselor) ||
                !host.IsAIControlledCounselor(counselor))
            {
                continue;
            }
            if (!IsConvergenceCounselorArmed(counselor))
            {
                selected = counselor;
                firstArm = true;
                CounselorConvergenceCursor = (index + 1) % count;
                break;
            }
            if (!selected)
            {
                selected = counselor;
                CounselorConvergenceCursor = (index + 1) % count;
            }
        }

        if (!selected)
        {
            NextCounselorConvergenceAt = now + 4513;
            return;
        }

        if (firstArm)
        {
            host.GiveConvergenceMeleeWeapon(selected);
            RememberConvergenceCounselor(selected);
        }

        // Service exactly one counselor per slice. With the 333 ms cadence
        // below, a full six-bot roster is refreshed in about 2.00 s. Their
        // behavior brains remain suspended during travel, so the stock
        // fear/loot service cannot reclaim them between these staggered paths.
        host.SuppressConvergenceCounselorFear(selected);

        Pawn controller = host.GetPawnControllerSafe(selected);
        Pawn blackboard = controller
            ? host.GetCounselorBlackboardCached(controller)
            : 0;
        Pawn weapon = host.GetCounselorCurrentWeapon(selected);
        if (!weapon && blackboard && host.Key("SCWeapon") >= 0)
            weapon = host.GetBlackboardObject(blackboard, host.Key("SCWeapon"));
        if (!weapon && !firstArm)
        {
            host.GiveConvergenceMeleeWeapon(selected);
            weapon = host.GetCounselorCurrentWeapon(selected);
            if (!weapon && blackboard && host.Key("SCWeapon") >= 0)
                weapon = host.GetBlackboardObject(
                    blackboard,
                    host.Key("SCWeapon"));
        }

        if (weapon)
        {
            // Publish fight state while the brain is still suspended so its
            // first resumed behavior-tree frame sees only the Jason branch.
            host.ArmKillTeamCounselorCombat(selected);

            Position counselorLocation{};
            Position currentJasonLocation{};
            const bool haveLocations =
                host.GetJasonAIActorLocation(selected, counselorLocation) &&
                host.GetJasonAIActorLocation(
                    host.GetJasonDeathTarget(),
                    currentJasonLocation);
            const float dx = haveLocations
                ? counselorLocation.X - currentJasonLocation.X
                : 0.0f;
            const float dy = haveLocations
                ? counselorLocation.Y - currentJasonLocation.Y
                : 0.0f;
            const float distanceSquared = dx * dx + dy * dy;
            const bool wasTraveling =
                FindConvergenceTravelCounselor(selected) >= 0;
            // Hysteresis avoids toggling the brain at the edge of melee range:
            // begin exclusive travel beyond 3 m, but do not hand control back
            // until the bot is within 2.2 m of Jason. The earlier 3.25 m
            // release allowed the stock tree to flee before melee was viable.
            const bool shouldTravel = haveLocations &&
                std::isfinite(distanceSquared) &&
                distanceSquared >
                    (wasTraveling
                        ? 220.0f * 220.0f
                        : 300.0f * 300.0f);

            if (shouldTravel)
            {
                if (!wasTraveling)
                {
                    if (host.SetCounselorBrainTickEnabled(selected, false))
                    {
                        RememberConvergenceTravelCounselor(selected);
                        host.Log(
                            "18L-BH counselor exclusive Jason travel engaged | pawn=" +
                            host.JasonAISafeName(
                                selected));
                    }
                    else if (firstArm)
                    {
                        host.Log(
                            "18L-BH counselor brain suspension unavailable; direct travel remains best-effort | pawn=" +
                            host.JasonAISafeName(
                                selected));
                    }
                }
                bool recoveryPulse = false;
                const std::int32_t travelIndex =
                    FindConvergenceTravelCounselor(selected);
                if (travelIndex >= 0)
                {
                    if (ConvergenceTravelRecoveryUntil[travelIndex] != 0)
                    {
                        if (now <
                            ConvergenceTravelRecoveryUntil[travelIndex])
                        {
                            recoveryPulse = true;
                        }
                        else
                        {
                            host.SetCounselorBrainTickEnabled(selected, false);
                            ConvergenceTravelRecoveryUntil[travelIndex] = 0;
                            ConvergenceTravelHaveLocation[travelIndex] = false;
                            ConvergenceTravelLastProgressAt[travelIndex] = now;
                        }
                    }

                    if (!recoveryPulse)
                    {
                        const Position& previous =
                            ConvergenceTravelLastLocations[travelIndex];
                        const float moveX = counselorLocation.X - previous.X;
                        const float moveY = counselorLocation.Y - previous.Y;
                        const bool progressed =
                            !ConvergenceTravelHaveLocation[travelIndex] ||
                            moveX * moveX + moveY * moveY > 80.0f * 80.0f;
                        if (progressed)
                        {
                            ConvergenceTravelLastLocations[travelIndex] =
                                counselorLocation;
                            ConvergenceTravelLastProgressAt[travelIndex] = now;
                            ConvergenceTravelHaveLocation[travelIndex] = true;
                        }
                        else if (ConvergenceTravelLastProgressAt[travelIndex] != 0 &&
                            now >=
                                ConvergenceTravelLastProgressAt[travelIndex] +
                                    3500)
                        {
                            // A closed door or smart-link traversal can require
                            // the counselor behavior tree. Give only the stuck
                            // pawn a short native-navigation pulse, then reclaim
                            // exclusive travel; no roster scan or group burst.
                            host.SetCounselorBrainTickEnabled(selected, true);
                            ConvergenceTravelRecoveryUntil[travelIndex] =
                                now + 1001;
                            recoveryPulse = true;
                            host.Log(
                                "18L-BI counselor travel obstruction recovery pulse | pawn=" +
                                host.JasonAISafeName(
                                    selected));
                        }
                    }
                }

                if (!recoveryPulse)
                {
                    const std::int32_t routeIndex =
                        FindConvergenceTravelCounselor(selected);
                    bool refreshRoute = routeIndex < 0;
                    if (routeIndex >= 0)
                    {
                        const Position& previousGoal =
                            ConvergenceTravelLastGoals[routeIndex];
                        const float goalX = currentJasonLocation.X - previousGoal.X;
                        const float goalY = currentJasonLocation.Y - previousGoal.Y;
                        const bool targetMoved =
                            ConvergenceTravelHaveGoal[routeIndex] &&
                            goalX * goalX + goalY * goalY > 400.0f * 400.0f;
                        refreshRoute =
                            !ConvergenceTravelHaveGoal[routeIndex] ||
                            targetMoved ||
                            now >=
                                ConvergenceTravelLastRouteAt[routeIndex] +
                                    6001;
                    }

                    if (refreshRoute &&
                        host.IssueAIMoveToLocationOnGameThread(
                            controller,
                            currentJasonLocation,
                            100.0f,
                            "SweaterConvergenceJason") &&
                        routeIndex >= 0)
                    {
                        ConvergenceTravelLastGoals[routeIndex] =
                            currentJasonLocation;
                        ConvergenceTravelLastRouteAt[routeIndex] = now;
                        ConvergenceTravelHaveGoal[routeIndex] = true;
                    }
                }
            }
            else if (wasTraveling)
            {
                // The direct path has delivered the bot. Resume only here so
                // the mature stock melee animation/attack loop can take over.
                host.SetCounselorBrainTickEnabled(selected, true);
                ForgetConvergenceTravelCounselor(selected);
                host.Log(
                    "18L-BH counselor reached Jason; native melee brain resumed | pawn=" +
                    host.JasonAISafeName(selected));
            }
            if (firstArm)
            {
                host.Log(
                    "18L-BF counselor armed and sent to Jason | pawn=" +
                    host.JasonAISafeName(
                        selected));
            }
        }
        else if (blackboard)
        {
            // If the cached axe was not present yet, use the stock weapon-seek
            // branch. A later slow pass promotes the bot to melee combat.
            host.SetBlackboardObject(
                blackboard,
                host.Key("JasonCharacter"),
                host.GetJasonDeathTarget());
            host.SetBlackboardBool(
                blackboard,
                host.Key("SeekWeaponWhileFleeing"),
                true);
            host.SetBlackboardBool(
                blackboard,
                host.Key("ShouldHide"),
                false);
        }

        // Always rotate one bot at a time. This caps synchronous navigation at
        // three requests per second for the entire roster and avoids the
        // rhythmic frame-time spike observed in the live trace.
        NextCounselorConvergenceAt = now + 333;
    }
};
}
