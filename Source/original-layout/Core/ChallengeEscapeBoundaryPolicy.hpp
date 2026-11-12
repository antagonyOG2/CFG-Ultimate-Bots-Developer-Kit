#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

// Standalone donor movement policy. No object dereferences, offsets, dispatcher
// budgets, engine ABI, damage, or escape-flag writes live in this layer.
namespace cfgchallengeescape::boundary {
struct Identity {
    std::uintptr_t address = 0;
    std::int32_t index = -1;
    std::int32_t serial = 0;
    bool Positive() const { return address != 0 && index >= 0 && serial > 0; }
};
inline bool operator==(Identity a, Identity b) {
    return a.address == b.address && a.index == b.index && a.serial == b.serial;
}
inline bool operator!=(Identity a, Identity b) { return !(a == b); }
struct Scope {
    std::uint64_t generation = 0;
    Identity world{}, mode{}, state{};
    bool enabled = false, stockChallenge = false, localAuthority = false;
    bool networkSession = true, inProgress = false;
    bool Eligible() const {
        return generation && world.Positive() && mode.Positive() && state.Positive() &&
            enabled && stockChallenge && localAuthority && !networkSession && inProgress;
    }
};
inline bool operator==(const Scope& a, const Scope& b) {
    return a.generation == b.generation && a.world == b.world && a.mode == b.mode &&
        a.state == b.state && a.enabled == b.enabled && a.stockChallenge == b.stockChallenge &&
        a.localAuthority == b.localAuthority && a.networkSession == b.networkSession && a.inProgress == b.inProgress;
}
struct Point { double x = 0, y = 0, z = 0; };
inline bool Finite(Point p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); }
struct Transform { Point position{}, rotation{}; };
inline bool Finite(const Transform& t) { return Finite(t.position) && Finite(t.rotation); }
struct Sphere { Point center{}; double radius = 0; };
inline bool Finite(const Sphere& s) {
    return Finite(s.center) && std::isfinite(s.radius) && s.radius > 0 && s.radius <= 100000;
}
// Donor 3D swept-center segment/sphere test, with overflow rejected as unknown.
inline bool SegmentTouchesSphere(Point from, Point to, const Sphere& sphere, bool& valid) {
    valid = Finite(from) && Finite(to) && Finite(sphere);
    if (!valid) return false;
    const Point d{to.x-from.x, to.y-from.y, to.z-from.z};
    const Point o{sphere.center.x-from.x, sphere.center.y-from.y, sphere.center.z-from.z};
    const double length2 = d.x*d.x+d.y*d.y+d.z*d.z;
    const double dot = o.x*d.x+o.y*d.y+o.z*d.z;
    if (!Finite(d) || !Finite(o) || !std::isfinite(length2) || !std::isfinite(dot)) { valid = false; return false; }
    const double t = length2 > 0 ? std::clamp(dot/length2, 0.0, 1.0) : 0;
    const Point n{from.x+t*d.x-sphere.center.x, from.y+t*d.y-sphere.center.y, from.z+t*d.z-sphere.center.z};
    const double distance2 = n.x*n.x+n.y*n.y+n.z*n.z;
    valid = Finite(n) && std::isfinite(distance2);
    return valid && distance2 <= sphere.radius*sphere.radius;
}
enum class Kind { Unknown, Pawn, Car };
struct Snapshot {
    Identity actor{}, controller{}, movement{}, root{};
    Kind kind = Kind::Unknown;
    bool authority = false, living = false, human = false, aiPossessed = false;
    bool unpossessed = false, seated = false, grabbed = false, escaping = false;
    bool seatsComplete = false, occupantsVerified = false;
    std::array<Identity, 8> occupants{};
    std::size_t occupantCount = 0;
};
enum class Seam { Overlap, CharacterStart, Presentation, Completion, VehicleStart };
struct Decision { bool blockNative = false, recoveryAttempted = false, recovered = false; };
struct Record {
    Identity actor{};
    bool knownAi = false, humanInvalidated = false, hasSafePoint = false, hasRecovery = false;
    Transform safe{};
    std::uint64_t sampledAt = 0, recoveredAt = 0;
    bool sampleAttemptStarted = false;
    std::uint64_t sampleAttemptAt = 0;
    bool hasRecoveryAttempt = false;
    std::uint64_t recoveryAttemptAt = 0;
};

// Host contract (single game-thread only):
// ScopeCurrent(scope) verifies positive current world/mode/state identities,
// authority, stock mission, committed option, InProgress and no network session.
// Current(scope,id,out) verifies this exact live incarnation, world membership,
// authority/classes and current component/possession/seat identities. It MUST
// report a human possession even when ordinary AI eligibility has failed.
// Geometry(scope,volume,out) verifies volume+SphereComponent, actual component
// center and scaled radius. Overlap(scope,volume,actor,out) is the native query.
// ReadTransform(scope,snapshot,out) returns the actual finite actor transform.
// Teleport(scope,snapshot,transform) uses collision-tested K2_TeleportTo.
// StopController, StopMovement, Throttle(...,0), Brake(...,1), Linear(...,Point{}),
// Angular(...,Point{}) return actual dispatch success. Host revalidates at its
// call boundary too; Current checks below are not permission to cache pointers.
// Current must positively identify reciprocal AI possession (aiPossessed), or
// positively establish no controller (unpossessed); unreadable is neither.
// occupantsVerified is optional. Only a host that freshly verifies EVERY seat,
// living occupant, controller/history eligibility and reciprocal ownership may
// set it. It saves duplicate occupant queries, not any native-edge proof.
class Policy {
public:
    static constexpr std::size_t PawnCapacity = 24, CarCapacity = 24, VolumeCapacity = 48;
    static constexpr std::uint64_t SampleMs = 250, SafeAgeMs = 1500, CooldownMs = 500;
    static constexpr unsigned FootActionLimit = 3, CarActionLimit = 9;
    void Configure(const Scope& scope) {
        if (!(scope == scope_) || !scope.Eligible()) Retire();
        scope_ = scope;
    }
    void Shutdown() { Retire(); scope_ = {}; }
    // A partial/overflow roster can still block eligible native attempts, but
    // cannot authorize saving or using safe transforms.
    void SetVolumes(const Identity* volumes, std::size_t count, bool complete) {
        if (recovering_) return;
        bool changed = count != volumeCount_ || count > VolumeCapacity;
        const std::size_t bounded = (std::min)(count, VolumeCapacity);
        for (std::size_t i = 0; i < bounded; ++i)
            changed = changed || !volumes || volumes_[i] != volumes[i];
        if (changed) {
            for (auto& r : pawns_) r.hasSafePoint = false;
            for (auto& r : cars_) r.hasSafePoint = false;
        }
        volumes_ = {}; volumeCount_ = bounded;
        geometryRosterComplete_ = complete && count > 0 && count <= VolumeCapacity && volumes;
        if (volumes) for (std::size_t i = 0; i < bounded; ++i) {
            volumes_[i] = volumes[i];
            geometryRosterComplete_ = geometryRosterComplete_ && volumes[i].Positive();
            for (std::size_t j = 0; j < i; ++j)
                if (volumes[j] == volumes[i]) geometryRosterComplete_ = false;
        }
    }
    // Ownership-only caller proofs can observe a human before movement/root
    // readiness. Invalidation never learns a new identity or restores old AI.
    void InvalidateHuman(const Scope& scope, Identity actor) {
        if (!(scope == scope_) || !scope.Eligible() || !actor.Positive()) return;
        if (auto* r = Find(pawns_,actor)) {
            r->humanInvalidated = true; r->knownAi = false; r->hasSafePoint = false;
        }
    }
    template<class Host> bool TrackPawn(Host& host, Identity actor) {
        Snapshot s{};
        return Refresh(host, actor, s) && s.kind == Kind::Pawn && KnownPawn(s, true);
    }
    template<class Host> bool TrackCar(Host& host, Identity actor) {
        Snapshot s{};
        if (!Refresh(host, actor, s) || s.kind != Kind::Car || !EligibleCar(host, s)) return false;
        return Acquire(cars_, actor) != nullptr;
    }
    template<class Host> Decision Gate(Host& host, Identity actor, Seam seam, std::uint64_t now, bool allowRecovery=true) {
        Decision result{}; Snapshot s{};
        if (!Refresh(host, actor, s)) return result;
        const Scope scope = scope_;
        bool eligible = false;
        if (s.kind == Kind::Pawn && seam != Seam::VehicleStart) {
            const bool known = KnownPawn(s, true);
            if (seam == Seam::Overlap) eligible = known && s.aiPossessed && Free(s);
            else if (seam == Seam::CharacterStart) eligible = known && s.aiPossessed;
            else eligible = known;
        } else if (s.kind == Kind::Car && (seam == Seam::Overlap || seam == Seam::VehicleStart)) {
            eligible = EligibleCar(host, s);
            if (eligible) Acquire(cars_, actor);
        }
        // Recheck after occupant queries; a synchronous read may reenter.
        Snapshot current{};
        if (!eligible || !(scope_ == scope) || !Refresh(host, actor, current) || !Same(s, current)) return result;
        // A nested call can observe human possession and invalidate the cached
        // incarnation, even if the host has returned to the original snapshot.
        if (s.kind == Kind::Pawn ? !KnownPawn(current,false) : !EligibleCar(host,current)) return result;
        if (!(scope_ == scope) || !Refresh(host,actor,current) || !Same(s,current)) return result;
        if (!HistoryAllows(current)) return result;
        result.blockNative = true;
        if (allowRecovery && !recovering_) {
            Record* r = s.kind == Kind::Pawn ? Find(pawns_, actor) : Find(cars_, actor);
            if (r && Recoverable(*r, s, now)) {
                result.recoveryAttempted = true;
                result.recovered = Recover(host, *r, s, now);
            }
        }
        return result;
    }
    template<class Host> void Pump(Host& host, std::uint64_t now) {
        if (!Live(host) || recovering_ || (sampleStarted_ && now >= lastSample_ && now-lastSample_ < SampleMs)) return;
        lastSample_ = now; sampleStarted_ = true;
        Sample(host, pawns_, now); Sample(host, cars_, now);
    }
    // Production schedulers can spend one bounded actor opportunity per
    // callback. Failed observations consume only this actor's 250 ms cadence;
    // sampledAt remains the age of its last positively verified safe point.
    template<class Host> void PumpOne(Host& host, Identity actor, std::uint64_t now) {
        if (!Live(host) || recovering_) return;
        if (Find(pawns_,actor)) SampleOne(host,pawns_,actor,now);
        else if (Find(cars_,actor)) SampleOne(host,cars_,actor,now);
    }
    const Record* Pawn(Identity id) const { return Find(pawns_, id); }
    const Record* Car(Identity id) const { return Find(cars_, id); }
    const Scope& CurrentScope() const { return scope_; }
private:
    Scope scope_{};
    std::array<Record, PawnCapacity> pawns_{};
    std::array<Record, CarCapacity> cars_{};
    std::array<Identity, VolumeCapacity> volumes_{};
    std::size_t volumeCount_ = 0;
    bool geometryRosterComplete_ = false, recovering_ = false, sampleStarted_ = false;
    std::uint64_t lastSample_ = 0;
    void Retire() {
        pawns_ = {}; cars_ = {}; volumes_ = {}; volumeCount_ = 0;
        geometryRosterComplete_ = false; sampleStarted_ = false; lastSample_ = 0;
        // Keep the recursion guard even when a host callback shuts us down.
    }
    template<std::size_t N> static Record* Find(std::array<Record,N>& rows, Identity id) {
        for (auto& r : rows) if (r.actor == id && id.Positive()) return &r;
        return nullptr;
    }
    template<std::size_t N> static const Record* Find(const std::array<Record,N>& rows, Identity id) {
        for (const auto& r : rows) if (r.actor == id && id.Positive()) return &r;
        return nullptr;
    }
    template<std::size_t N> static Record* Acquire(std::array<Record,N>& rows, Identity id) {
        if (!id.Positive()) return nullptr;
        if (auto* r = Find(rows,id)) return r;
        for (auto& r : rows) if (!r.actor.Positive() || r.actor.address == id.address || r.actor.index == id.index) {
            r = {}; r.actor = id; return &r;
        }
        return nullptr;
    }
    template<class Host> bool Live(Host& host) {
        if (scope_.Eligible() && host.ScopeCurrent(scope_)) return true;
        Retire(); scope_ = {}; return false;
    }
    template<class Host> bool Refresh(Host& host, Identity actor, Snapshot& s) {
        if (!actor.Positive() || !Live(host)) return false;
        const Scope before = scope_;
        if (!host.Current(before, actor, s) || !(scope_ == before) || !Live(host) || s.actor != actor) return false;
        if (!(scope_ == before)) return false;
        if (s.kind == Kind::Pawn && s.human) {
            if (auto* r = Find(pawns_,actor)) {
                r->humanInvalidated = true; r->knownAi = false; r->hasSafePoint = false;
            }
        }
        return true;
    }
    bool KnownPawn(const Snapshot& s, bool learn) {
        if (s.kind != Kind::Pawn || !s.authority || !s.living || s.human) return false;
        Record* r = Find(pawns_,s.actor);
        if (r && r->humanInvalidated) return false;
        if (s.aiPossessed && s.controller.Positive()) {
            if (!r && learn) r = Acquire(pawns_,s.actor);
            if (r) r->knownAi = true;
            return true; // Capacity exhaustion cannot unblock a verified native attempt.
        }
        return s.unpossessed && r && r->knownAi;
    }
    template<class Host> bool EligibleCar(Host& host, const Snapshot& s) {
        if (!s.authority || s.human || !s.seatsComplete || !s.occupantCount || s.occupantCount > s.occupants.size()) return false;
        if (s.occupantsVerified) {
            bool known = true;
            for (std::size_t i=0; i<s.occupantCount; ++i) {
                const Record* r = Find(pawns_,s.occupants[i]);
                known = known && r && r->knownAi && !r->humanInvalidated;
                for (std::size_t j=0; j<i; ++j) if (s.occupants[i] == s.occupants[j]) return false;
            }
            if (known) return true;
        }
        for (std::size_t i=0; i<s.occupantCount; ++i) {
            Snapshot pawn{};
            if (!Refresh(host,s.occupants[i],pawn) || !KnownPawn(pawn,true)) return false;
            for (std::size_t j=0; j<i; ++j) if (s.occupants[i] == s.occupants[j]) return false;
        }
        return true;
    }
    static bool Free(const Snapshot& s) { return !s.seated && !s.grabbed && !s.escaping; }
    bool HistoryAllows(const Snapshot& s) {
        if (s.kind == Kind::Pawn) return KnownPawn(s,false);
        if (s.kind != Kind::Car || s.occupantCount > s.occupants.size()) return false;
        for (std::size_t i=0; i<s.occupantCount; ++i) {
            const Record* r = Find(pawns_,s.occupants[i]);
            if (r && r->humanInvalidated) return false;
        }
        return true;
    }
    static bool Same(const Snapshot& a, const Snapshot& b) {
        return a.actor == b.actor && a.kind == b.kind && a.controller == b.controller &&
            a.movement == b.movement && a.root == b.root && a.authority == b.authority &&
            a.living == b.living && a.human == b.human && a.aiPossessed == b.aiPossessed &&
            a.unpossessed == b.unpossessed && a.seated == b.seated && a.grabbed == b.grabbed &&
            a.escaping == b.escaping && a.seatsComplete == b.seatsComplete && a.occupantsVerified == b.occupantsVerified &&
            a.occupantCount == b.occupantCount && a.occupants == b.occupants;
    }
    template<class Host> bool Still(Host& host, const Scope& scope, const Snapshot& s) {
        Snapshot fresh{};
        if (!(scope_ == scope) || !Refresh(host,s.actor,fresh) || !Same(s,fresh)) return false;
        const bool eligible = s.kind == Kind::Car ? EligibleCar(host,fresh) : KnownPawn(fresh,false) && fresh.aiPossessed && Free(fresh);
        return eligible && scope_ == scope && Refresh(host,s.actor,fresh) && Same(s,fresh) && HistoryAllows(fresh);
    }
    bool Recoverable(const Record& r, const Snapshot& s, std::uint64_t now) const {
        return r.hasSafePoint && Finite(r.safe) && now >= r.sampledAt && now-r.sampledAt <= SafeAgeMs &&
            (!r.hasRecovery || (now >= r.recoveredAt && now-r.recoveredAt >= CooldownMs)) &&
            (!r.hasRecoveryAttempt || (now >= r.recoveryAttemptAt && now-r.recoveryAttemptAt >= CooldownMs)) &&
            (s.kind == Kind::Car ? s.root.Positive() && s.movement.Positive() :
                s.aiPossessed && Free(s) && s.controller.Positive() && s.movement.Positive());
    }
    template<class Host> bool CheckGeometry(Host& host, const Snapshot& s, Point from, Point to, bool& hit) {
        hit = false;
        if (!geometryRosterComplete_ || !volumeCount_) return false;
        const Scope scope = scope_;
        const auto volumes = volumes_; const std::size_t count = volumeCount_;
        for (std::size_t i=0; i<count; ++i) {
            Sphere sphere{}; bool valid = false;
            if (!(scope_ == scope) || !Live(host) || !host.Geometry(scope,volumes[i],sphere) ||
                !(scope_ == scope) || !Finite(sphere)) return false;
            const bool touches = SegmentTouchesSphere(from,to,sphere,valid);
            if (!valid) return false;
            hit = hit || touches;
        }
        Snapshot fresh{};
        return Refresh(host,s.actor,fresh) && Same(s,fresh) && geometryRosterComplete_ &&
            volumeCount_ == count && volumes_ == volumes;
    }
    template<class Host> bool Outside(Host& host, const Snapshot& s) {
        if (!geometryRosterComplete_ || !volumeCount_) return false;
        const Scope scope = scope_; const auto volumes = volumes_; const std::size_t count = volumeCount_;
        for (std::size_t i=0; i<count; ++i) {
            Snapshot fresh{}; bool overlaps = true;
            if (!(scope_ == scope) || !Refresh(host,s.actor,fresh) || !Same(s,fresh) ||
                !host.Overlap(scope,volumes[i],s.actor,overlaps) || overlaps) return false;
        }
        Snapshot fresh{};
        return scope_ == scope && Refresh(host,s.actor,fresh) && Same(s,fresh) &&
            geometryRosterComplete_ && volumeCount_ == count && volumes_ == volumes;
    }
    template<class Host> bool Recover(Host& host, const Record& record, const Snapshot& s, std::uint64_t now) {
        const Record saved = record; const Scope scope = scope_;
        // Donor RecoveredAt is success-only: its sampler retries failures every
        // 250 ms, and native gates can retry on every callback. Keep successful
        // recovery reporting unchanged, but throttle ALL recovery attempts for
        // this exact incarnation to 500 ms, including unavailable geometry/API.
        // Stamp before any host callback, so reentry or a failed proof cannot
        // authorize another attempt. Scope retirement/reincarnation clears it.
        Record* attempt = s.kind == Kind::Pawn ? Find(pawns_,s.actor) : Find(cars_,s.actor);
        if (!attempt) return false;
        attempt->hasRecoveryAttempt = true; attempt->recoveryAttemptAt = now;
        bool hit = true;
        recovering_ = true;
        if (!Still(host,scope,s) || !CheckGeometry(host,s,saved.safe.position,saved.safe.position,hit) || hit) {
            recovering_ = false; return false;
        }
        // Real native hosts reserve the full remaining immutable recovery
        // packet before its first mutation: 3/9 action calls, actual transform
        // (2 calls), and actual geometry/outside evidence (3 per volume).
        // Scope/actor/component checks still occur at EVERY action boundary.
        // The optional fallback is only for abstract legacy fixture hosts;
        // production Access mutators require their own positive reservation.
        const unsigned actionCalls = s.kind == Kind::Car ? CarActionLimit : FootActionLimit;
        if (!Preflight(host,scope,s,actionCalls,volumeCount_,0)) {
            recovering_ = false; return false;
        }
        // Each action performs fresh scope/actor/component/occupant validation.
        // There are exactly 3 foot actions or at most 9 car actions, no retries.
        bool ok = true;
        if (s.kind == Kind::Car) {
            ok = Action(host,scope,s,[&]{ return host.Throttle(scope,s,0.0); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Brake(scope,s,1.0); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Linear(scope,s,Point{}); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Angular(scope,s,Point{}); }) && ok;
        }
        const bool moved = Action(host,scope,s,[&]{ return host.Teleport(scope,s,saved.safe); });
        ok = moved && ok;
        if (s.kind == Kind::Car) {
            ok = Action(host,scope,s,[&]{ return host.Throttle(scope,s,0.0); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Brake(scope,s,1.0); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Linear(scope,s,Point{}); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.Angular(scope,s,Point{}); }) && ok;
        } else if (moved) {
            ok = Action(host,scope,s,[&]{ return host.StopController(scope,s); }) && ok;
            ok = Action(host,scope,s,[&]{ return host.StopMovement(scope,s); }) && ok;
        }
        Transform actual{}; bool actualHit = true;
        ok = ok && Still(host,scope,s) && host.ReadTransform(scope,s,actual) && Finite(actual) &&
            CheckGeometry(host,s,actual.position,actual.position,actualHit) && !actualHit && Outside(host,s) && Still(host,scope,s);
        if (ok) {
            Record* current = s.kind == Kind::Pawn ? Find(pawns_,s.actor) : Find(cars_,s.actor);
            if (current) { current->hasRecovery = true; current->recoveredAt = now; }
        }
        recovering_ = false;
        return ok;
    }
    template<class Host, class Call> bool Action(Host& host, const Scope& scope, const Snapshot& s, Call call) {
        return Still(host,scope,s) && call();
    }
    template<class Host> static auto Preflight(Host& host, const Scope& scope, const Snapshot& s,
        unsigned actionCalls, std::size_t volumeCount, int)
        -> decltype(host.PreflightRecovery(scope,s,actionCalls,volumeCount)) {
        return host.PreflightRecovery(scope,s,actionCalls,volumeCount);
    }
    template<class Host> static bool Preflight(Host&, const Scope&, const Snapshot&, unsigned, std::size_t, long) {
        return true;
    }
    template<class Host, std::size_t N> void Sample(Host& host, std::array<Record,N>& rows, std::uint64_t now) {
        // Stable identity list: host callbacks may retire all records.
        std::array<Identity,N> ids{};
        const Scope scope = scope_;
        for (std::size_t i=0; i<N; ++i) ids[i] = rows[i].actor;
        for (Identity id : ids) {
            if (!(scope_ == scope)) return;
            SampleOne(host,rows,id,now);
        }
    }
    template<class Host, std::size_t N> void SampleOne(Host& host, std::array<Record,N>& rows, Identity id, std::uint64_t now) {
        Record* r = Find(rows,id);
        if (!r || (r->sampleAttemptStarted && now >= r->sampleAttemptAt && now-r->sampleAttemptAt < SampleMs)) return;
        r->sampleAttemptStarted = true; r->sampleAttemptAt = now;
        const Scope scope = scope_;
        Snapshot s{};
        if (!Refresh(host,id,s)) return;
        const bool eligible = s.kind == Kind::Pawn ? KnownPawn(s,false) && s.aiPossessed && Free(s) : EligibleCar(host,s);
        r = Find(rows,id);
        if (!eligible || !r || (r->hasRecovery && now >= r->recoveredAt && now-r->recoveredAt < CooldownMs)) return;
        Transform actual{};
        const Record saved = *r;
        if (!host.ReadTransform(scope_,s,actual) || !Finite(actual) || !(scope_ == scope)) return;
        const Point from = s.kind == Kind::Car && saved.hasSafePoint ? saved.safe.position : actual.position;
        bool hit = false;
        const bool complete = CheckGeometry(host,s,from,actual.position,hit);
        const bool outside = Outside(host,s);
        r = Find(rows,id);
        if (!r || !(scope_ == scope) || !HistoryAllows(s)) return;
        if (hit || !outside) {
            if (Recoverable(*r,s,now)) Recover(host,*r,s,now);
            return;
        }
        if (complete) {
            r->safe = actual; r->sampledAt = now; r->hasSafePoint = true;
        }
    }
};
}
