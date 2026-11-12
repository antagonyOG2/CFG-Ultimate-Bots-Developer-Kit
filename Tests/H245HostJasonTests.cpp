// H2.45 host-as-Jason lottery: shared layout pins, write gate and write offsets against mocked memory.
#include "../Core/HostedRolePreference.hpp"
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
namespace {
unsigned failures=0,checks=0;
void check(bool ok,int line){++checks;if(!ok){++failures;std::printf("H245 check failed at line %d\n",line);}}
#define CHECK(x) check((x),__LINE__)
struct Image{
    std::map<std::uintptr_t,unsigned char> bytes;
    void Put(std::uintptr_t rva,const unsigned char* p,std::size_t n){for(std::size_t i=0;i<n;++i)bytes[rva+i]=p[i];}
    void Load(const cfghostedrole::Layout& l){Put(l.entry,l.prologue,l.prologueSize);for(const auto& w:l.witness)if(w.bytes)Put(w.rva,w.bytes,w.size);}
    bool Read(std::uintptr_t rva,unsigned char* out,std::size_t n)const{
        for(std::size_t i=0;i<n;++i){auto it=bytes.find(rva+i);if(it==bytes.end())return false;out[i]=it->second;}return true;}
};
struct Memory{
    std::map<std::uintptr_t,std::uint32_t> writes;unsigned count=0;
    void W8(std::uintptr_t a,std::uint8_t v){writes[a]=v;++count;}
    void W32(std::uintptr_t a,std::int32_t v){writes[a]=static_cast<std::uint32_t>(v);++count;}
};
// Mirrors HostedApplyRolePreference's final stage: one gate, then the shared writer.
unsigned Apply(const cfghostedrole::Layout& l,unsigned pins,bool hosted,bool enabled,Memory& m,unsigned& refusal){
    refusal=cfghostedrole::WriteDecision(l,pins,hosted,enabled);if(refusal)return 0;
    const std::array<std::uintptr_t,3> state{0x10000,0x20000,0x30000};
    std::array<std::uint8_t,3> before{1,1,1},human{1,1,1},after{};
    CHECK(cfghostedrole::InvertHostPreference(before,human,3,0,after));
    return cfghostedrole::WriteRoles(l,state.data(),after.data(),3,0x90000,
        [&](std::uintptr_t a,std::uint8_t v){m.W8(a,v);},[&](std::uintptr_t a,std::int32_t v){m.W32(a,v);});
}
}
int main(){
    using namespace cfghostedrole;
    const Layout& l=Cfg35Layout;
    CHECK(l.entry==0x3DEAE0&&l.prologueSize==90&&l.preference==0x948&&l.priorName==0x310&&l.required==PinAll);
    CHECK(l.preference!=DonorLayout.preference&&l.priorName!=DonorLayout.priorName);
    CHECK(l.witness[0].rva==0x3DF2DA&&l.witness[1].rva==0x3DF556&&l.witness[2].rva==0x3DF171);
    // field-offset instruction bytes really encode +0x948 and +0x318/+0x310
    CHECK(Cfg35PrefOne[2]==0x48&&Cfg35PrefOne[3]==0x09&&Cfg35PrefTwo[3]==0x48&&Cfg35PrefTwo[4]==0x09);
    CHECK(Cfg35Prior[3]==0x18&&Cfg35Prior[4]==0x03&&Cfg35Prior[13]==0x10&&Cfg35Prior[14]==0x03);
    // Pins match -> mask 15 -> writes land on the 3.5 offsets only.
    Image good;good.Load(l);
    const auto read=[&](const Image& im){return [&im](std::uintptr_t r,unsigned char* o,std::size_t n){return im.Read(r,o,n);};};
    CHECK(VerifyPins(l,read(good))==PinAll&&PinsAccepted(l,PinAll));
    {Memory m;unsigned refusal=99;const unsigned n=Apply(l,PinAll,true,true,m,refusal);
     CHECK(refusal==0&&n==4&&m.count==4);
     CHECK(m.writes.size()==4&&m.writes.at(0x10000+0x948)==2&&m.writes.at(0x20000+0x948)==1&&m.writes.at(0x30000+0x948)==1);
     CHECK(m.writes.at(0x90000+0x310+8)==0);
     for(const auto& w:m.writes)CHECK(w.first!=0x10000+0x868&&w.first!=0x90000+0x2F0+8);}
    // Any single pin mismatch (every byte of every pin) -> zero writes, refusal 10.
    unsigned mismatchRuns=0;
    for(unsigned pin=0;pin<4;++pin){
        const unsigned char* data=pin==0?l.prologue:l.witness[pin-1].bytes;const std::size_t size=pin==0?l.prologueSize:l.witness[pin-1].size;
        const std::uintptr_t at=pin==0?l.entry:l.witness[pin-1].rva;
        for(std::size_t i=0;i<size;++i){
            Image bad;bad.Load(l);bad.bytes[at+i]=static_cast<unsigned char>(data[i]^0x01);
            const unsigned mask=VerifyPins(l,read(bad));CHECK(mask!=PinAll&&!PinsAccepted(l,mask)&&(mask&(1u<<pin))==0);
            Memory m;unsigned refusal=0;CHECK(Apply(l,mask,true,true,m,refusal)==0&&m.count==0&&refusal==RefusePinMismatch);++mismatchRuns;}
    }
    CHECK(mismatchRuns==90+7+9+17);
    {Image missing;Memory m;unsigned refusal=0;const unsigned mask=VerifyPins(l,read(missing));
     CHECK(mask==0&&Apply(l,mask,true,true,m,refusal)==0&&m.count==0&&refusal==RefusePinMismatch);}
    {Image donor;donor.Load(DonorLayout); // donor prologue alone must not satisfy the 3.5 layout
     CHECK(!PinsAccepted(l,VerifyPins(l,read(donor))));}
    // Offline world (NetDriver null) -> zero writes even with perfect pins.
    {Memory m;unsigned refusal=0;CHECK(Apply(l,PinAll,false,true,m,refusal)==0&&m.count==0&&refusal==RefuseOffline);}
    // Adoption gate off (default build) -> zero writes, distinct refusal.
    {Memory m;unsigned refusal=0;CHECK(Apply(l,PinAll,true,false,m,refusal)==0&&m.count==0&&refusal==RefuseAdoptionUnproven);}
    CHECK(WritesEnabledByBuild()==(!CFG_H245_HOST_JASON||CFG_H245_HOST_ADOPTION));
    // Donor layout still resolves through the same code with donor offsets (old behaviour preserved).
    {Image donor;donor.Load(DonorLayout);CHECK(VerifyPins(DonorLayout,read(donor))==PinPrologue&&PinsAccepted(DonorLayout,PinPrologue));
     Memory m;unsigned refusal=0;CHECK(Apply(DonorLayout,PinPrologue,true,true,m,refusal)==4&&m.writes.at(0x10000+0x868)==2&&m.writes.at(0x90000+0x2F0+8)==0);}
    std::printf("H245 host-jason layout/pin/write fixtures: %u checks, %u failures.\n",checks,failures);
    return failures?1:0;
}
