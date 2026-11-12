#define CFG_J22_CHALLENGES_RETENTION 1
#define main PriorStartupRetentionFixture
#include "NativeStartupRetentionTests.cpp"
#undef main
#include "NativeSoftClassLoader.hpp"
#include <map>
struct PageNames {
    std::map<std::uintptr_t,unsigned char> bytes;
    template<class T>void Put(std::uintptr_t p,const T& value){const auto* b=reinterpret_cast<const unsigned char*>(&value);
        for(unsigned i=0;i<sizeof(T);++i)bytes[p+i]=b[i];}
    template<class T>bool Read(std::uintptr_t p,T& value){auto* b=reinterpret_cast<unsigned char*>(&value);
        for(unsigned i=0;i<sizeof(T);++i){auto it=bytes.find(p+i);if(it==bytes.end())return false;b[i]=it->second;}return true;}
    void Name(const char16_t* value){using namespace cfgnames;
        Put(0x100000+disk::Global,std::uintptr_t{0x500000});Put(0x500000+disk::ElementCount,std::array<int,2>{2,1});
        Put(0x500000,std::uintptr_t{0x600000});Put(0x600008,std::uintptr_t{0x700000});Put(0x700000,std::uint32_t{1});
        std::array<char16_t,128> text{};for(unsigned i=0;value[i]&&i<127;++i)text[i]=value[i];Put(0x700000+disk::EntryText,text);}
};
int main(){using namespace cfgmenu;using namespace startupretain;unsigned checked=0;
    const auto check=[&](bool value){++checked;if(!value)std::printf("J22 check %u failed\n",checked);CHECK(value);};
    PageNames names;names.Name(ChallengePagePath);promotion::Packet packet{};cfgnames::Words name{1,0};
    std::memcpy(packet.bytes.data()+16,&name,8);
    check(population::softdetail::ChallengePagePacket(names,0x100000,packet));
    unsigned calls=0;std::uintptr_t object=0;
    const auto native=[&](std::uintptr_t& output){++calls;output=0x20000;return true;};
    check(!population::softdetail::ScopedLoad(names,0x100000,packet,Call::FrontendChallengePageClass,object,native)&&!calls);
    {Host host;Scope scope(0x100000,Role::Frontend,host.Hooks(),true);
        check(scope.Entered());
        check(!population::softdetail::ScopedLoad(names,0x100000,packet,Call::LoadClass,object,native)&&!calls);
        check(population::softdetail::ScopedLoad(names,0x100000,packet,Call::FrontendChallengePageClass,object,native));
        check(calls==1&&object==0x20000&&host.lease.Claims()==1&&!host.held);
        host.Collect();host.held=true;check(host.Required());host.held=false;
        {Scope nested(0x100000,Role::Frontend,host.Hooks(),true);check(!nested.Entered());
            check(!population::softdetail::ScopedLoad(names,0x100000,packet,Call::FrontendChallengePageClass,object,native)&&calls==1);}
        names.Name(u"/Game/Other.Unapproved_C");check(!population::softdetail::ScopedLoad(names,0x100000,packet,Call::FrontendChallengePageClass,object,native)&&calls==1);
        names.Name(ChallengePagePath);auto wrong=packet;wrong.bytes[24]=1;
        check(!population::softdetail::ScopedLoad(names,0x100000,wrong,Call::FrontendChallengePageClass,object,native)&&calls==1);
        wrong=packet;wrong.bytes[20]=1;check(!population::softdetail::ScopedLoad(names,0x100000,wrong,Call::FrontendChallengePageClass,object,native)&&calls==1);
        check(host.Close());}
    for(auto role:{Role::Start,Role::Intro,Role::Match})check(!Allowed(role,Call::FrontendChallengePageClass));
    for(unsigned c=0;c<static_cast<unsigned>(Call::FrontendChallengePageClass);++c)check(!Allowed(Role::Frontend,static_cast<Call>(c)));
    {Host host;Scope scope(0x100000,Role::Frontend,host.Hooks(),true);
        check(!population::softdetail::ScopedLoad(names,0x100000,packet,Call::FrontendChallengePageClass,object,
            [&](std::uintptr_t& output){++calls;output=0x20000;host.current=false;return true;}));
        check(host.uncertain==1&&!Scope::Current(0x100000));}
    std::printf("J22 frontend-only exact page load retention: %u checks,%u failures; real FName decode/root lease, no Intro/Match borrow, arbitrary path/nesting/revocation refused. Owned model only.\n",checked,failures);
    return failures?1:0;
}
