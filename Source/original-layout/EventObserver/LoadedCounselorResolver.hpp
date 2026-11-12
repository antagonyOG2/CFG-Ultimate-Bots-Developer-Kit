#pragma once
#include "StartingPlayerObservation.hpp"
#include "../RuntimeProbe/LoadedCounselorBindings.generated.hpp"
#include <algorithm>

namespace cfgmenu::loadedcounselor {
constexpr unsigned MaximumReads=4096;
enum class Reason { None, Context, EmptyPick, Uncached, StaleCache, InvalidObject, WrongType, PathMismatch, Changed, Ready };
template<class Reader> class Budget {
    Reader& reader_;
public:
    unsigned reads=0;explicit Budget(Reader& r):reader_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==MaximumReads)return false;++reads;return reader_.Read(at,out);}
};
class WindowsEnvironment {
public:
    DWORD Thread()const noexcept{return GetCurrentThreadId();}
    bool Ready(const promotion::Context& c) noexcept {
        if(!c.exactImageVerified||Thread()!=c.thread||c.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
        promotion::CurrentProcessReader r;DWORD owner=0;unsigned char initialized=0;
        return r.Read(c.module+cfgprobe::dispatch::ThreadIdRva,owner)&&owner==c.thread&&
            r.Read(c.module+cfgprobe::dispatch::ThreadFlagRva,initialized)&&initialized==1&&
            promotion::detail::Code(c.module,c.module+loadeddisk::WeakGet,loadeddisk::WeakGetCode)&&
            promotion::detail::Code(c.module,c.module+loadeddisk::ClassGetter,loadeddisk::ClassGetterCode)&&
            promotion::detail::Code(c.module,c.module+loadeddisk::PackageGetter,loadeddisk::PackageGetterCode);
    }
};
// A read-only implementation of native7C0210's already-cached branch, with
// stronger class/path/current-owner checks. It NEVER enters7CAFF0's broader
// path search, loads assets, constructs FNames, writes a weak cache or destroys
// caller-owned soft storage. The40-byte packet is only a borrowed byte snapshot.
// Host owns objects/class/code for this synchronous owner callback; positive
// serials detect replacement, not GC pins. No cross-event result caching.
template<class Reader,class Environment> class Resolver {
    Reader& reader_;Environment& environment_;promotion::Context c_{};
    bool busy_=false;Reason reason_=Reason::None;unsigned reads_=0;
    template<class R> bool Owner(R& r){return environment_.Ready(c_)&&
        promotion::gate::OwnerCurrent(r,c_,environment_.Thread(),c_.modeVtable,true)&&starting::ActorTypes(r,c_);}
public:
    Resolver(Reader& r,Environment& e,const promotion::Context& owner):reader_(r),environment_(e),c_(owner){}
    Resolver(const Resolver&)=delete;Resolver& operator=(const Resolver&)=delete;
    Reason LastReason()const noexcept{return reason_;}
    unsigned Reads()const noexcept{return reads_;}
    bool ResolveLoaded(std::uintptr_t source,const promotion::Packet& observed,std::uintptr_t& selected){
        using namespace cfgprobe;selected=0;reads_=0;reason_=Reason::Context;
        if(busy_)return false;busy_=true;
        struct Reset{bool& busy;~Reset(){busy=false;}}reset{busy_};
        Budget<Reader> r(reader_);const auto finish=[&](Reason why){reads_=r.reads;reason_=why;return why==Reason::Ready;};
        if(source!=c_.playerState+birthdisk::PickedCounselorClass||!Owner(r))return finish(Reason::Context);
        promotion::Packet current;if(!r.Read(source,current)||current.bytes!=observed.bytes)return finish(Reason::Changed);
        std::int32_t index=-1,serial=0;cfgnames::Words path{};
        struct Subpath{std::uintptr_t data;std::int32_t count,capacity;}subpath{};
        std::memcpy(&index,observed.bytes.data(),4);std::memcpy(&serial,observed.bytes.data()+4,4);
        std::memcpy(&path,observed.bytes.data()+16,8);std::memcpy(&subpath,observed.bytes.data()+24,16);
        if(!path.index&&!path.number)return finish(Reason::EmptyPick);
        if(subpath.data||subpath.count||subpath.capacity)return finish(Reason::PathMismatch); // No subobject class choices.
        if(index<0||serial<=0)return finish(Reason::Uncached);
        if(index>=c_.registry.count)return finish(Reason::StaleCache);
        const auto item=c_.registry.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes;
        std::uintptr_t candidate=0;std::int32_t foundSerial=0;std::uint32_t flags=0;
        if(!r.Read(item,candidate)||!ObjectPointer(candidate)||!r.Read(item+lifetime::ItemSerialOffset,foundSerial)||foundSerial!=serial||
           !r.Read(item+loadeddisk::ItemFlags,flags)||(flags&loadeddisk::InvalidItemFlags))return finish(Reason::StaleCache);
        lifetime::SlotObservation identity;
        if(!lifetime::ReadSlot(r,c_.registry,candidate,identity)||identity.index!=index||identity.serial!=serial)return finish(Reason::InvalidObject);
        std::uintptr_t classType=0,packageType=0;
        if(!r.Read(c_.module+loadeddisk::NativeClassCache,classType)||!ObjectPointer(classType)||
           !r.Read(c_.module+loadeddisk::NativePackageCache,packageType)||!ObjectPointer(packageType))return finish(Reason::WrongType);
        Ancestry meta;if(!CaptureAncestry(r,candidate,meta)||!meta.Contains(classType)||!starting::CounselorClass(r,c_,candidate))return finish(Reason::WrongType);
        cfgnames::Observation<Budget<Reader>> names(r,c_.module);cfgnames::Text softName,objectName,packageName;
        std::uintptr_t outer=0,outerType=0,parent=0;
        if(!names.Decode(path,softName)||!names.Object(candidate,objectName)||!r.Read(candidate+0x20,outer)||!ObjectPointer(outer)||outer==candidate||
           !r.Read(outer+ObjectClass,outerType)||outerType!=packageType||!r.Read(outer+0x20,parent)||parent||
           !names.Object(outer,packageName)||packageName.View().size()<7||packageName.View().substr(0,6)!=u"/Game/"||objectName.View().empty())return finish(Reason::PathMismatch);
        std::array<char16_t,160> full{};const auto length=packageName.count+1+objectName.count;
        if(length>127)return finish(Reason::PathMismatch);
        std::copy(packageName.data.begin(),packageName.data.begin()+packageName.count,full.begin());full[packageName.count]=u'.';
        std::copy(objectName.data.begin(),objectName.data.begin()+objectName.count,full.begin()+packageName.count+1);
        if(softName.View()!=std::u16string_view(full.data(),length))return finish(Reason::PathMismatch);
        // Recheck live bytes, typed names, exact weak slot and current owner
        // after every relation. No inferred weak pointer or stale class result.
        promotion::Packet finalPacket;cfgnames::Text finalObject,finalPackage;std::uintptr_t again=0;
        lifetime::SlotObservation finalIdentity;std::uint32_t finalFlags=0;
        if(!r.Read(source,finalPacket)||finalPacket.bytes!=observed.bytes||!names.Object(candidate,finalObject)||finalObject.View()!=objectName.View()||
           !r.Read(candidate+0x20,again)||again!=outer||!names.Object(outer,finalPackage)||finalPackage.View()!=packageName.View()||
           !r.Read(outer+ObjectClass,again)||again!=packageType||!r.Read(outer+0x20,again)||again||
           !r.Read(c_.module+loadeddisk::NativeClassCache,again)||again!=classType||!r.Read(c_.module+loadeddisk::NativePackageCache,again)||again!=packageType||
           !StableAncestry(r,candidate,meta)||!starting::CounselorClass(r,c_,candidate)||
           !lifetime::ReadSlot(r,c_.registry,candidate,finalIdentity)||!(identity==finalIdentity)||
           !r.Read(item+loadeddisk::ItemFlags,finalFlags)||finalFlags!=flags||!Owner(r))return finish(Reason::Changed);
        selected=candidate;return finish(Reason::Ready);
    }
};

// Concrete production wiring: normal current-process reads and code validation
// only. Construct from a FRESH observed/bound owner, then explicitly pass this
// object to starting::ResolvePicked. No native/game operation runs in constructor.
class WindowsLoadedCounselorResolver {
    promotion::CurrentProcessReader reader_{};WindowsEnvironment environment_{};
    Resolver<promotion::CurrentProcessReader,WindowsEnvironment> resolver_;
public:
    explicit WindowsLoadedCounselorResolver(const promotion::Context& owner):resolver_(reader_,environment_,owner){}
    bool ResolveLoaded(std::uintptr_t source,const promotion::Packet& observed,std::uintptr_t& selected){return resolver_.ResolveLoaded(source,observed,selected);}
    Reason LastReason()const noexcept{return resolver_.LastReason();}
    unsigned Reads()const noexcept{return resolver_.Reads();}
};
}
