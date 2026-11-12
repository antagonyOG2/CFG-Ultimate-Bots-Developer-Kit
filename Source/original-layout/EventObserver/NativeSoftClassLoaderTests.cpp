#include "NativeSoftClassLoader.hpp"
#include <cstdio>
namespace {
const wchar_t* originalPath=nullptr;bool abiGood=false;
std::uintptr_t __fastcall Load(void* packet){
    auto* bytes=static_cast<unsigned char*>(packet);std::uintptr_t path=0;int count=0;
    std::memcpy(&path,bytes+24,8);std::memcpy(&count,bytes+32,4);
    abiGood=count==4&&path!=reinterpret_cast<std::uintptr_t>(originalPath)&&
        std::memcmp(reinterpret_cast<const void*>(path),L"abc",8)==0;
    bytes[0]=99;return abiGood?0x10000:0;
}
struct Reader{
    template<class T> bool Read(std::uintptr_t at,T& out){std::memcpy(&out,reinterpret_cast<const void*>(at),sizeof(T));return true;}
};
}
int main(){
    Reader reader;cfgmenu::promotion::Packet source{},owned{};std::array<wchar_t,1024> buffer{};
    alignas(8) const wchar_t path[]=L"abc";originalPath=path;
    const std::uint64_t asset=123;const auto address=reinterpret_cast<std::uintptr_t>(path);int count=4;
    std::memcpy(source.bytes.data()+16,&asset,8);std::memcpy(source.bytes.data()+24,&address,8);
    std::memcpy(source.bytes.data()+32,&count,4);std::memcpy(source.bytes.data()+36,&count,4);
    if(!cfgmenu::population::softdetail::OwnPath(reader,source,owned,buffer))return 1;
    std::uintptr_t result=0;
    if(!cfgmenu::population::softdetail::Load(reinterpret_cast<std::uintptr_t>(&Load),owned,result)||!abiGood||result!=0x10000||source.bytes[0]!=0||owned.bytes[0]!=99)return 2;
    count=1025;std::memcpy(source.bytes.data()+32,&count,4);
    if(cfgmenu::population::softdetail::OwnPath(reader,source,owned,buffer))return 3;
    count=0;std::memcpy(source.bytes.data()+32,&count,4);
    if(!cfgmenu::population::softdetail::OwnPath(reader,source,owned,buffer))return 4;
    std::uintptr_t empty=1;std::memcpy(&empty,owned.bytes.data()+24,8);if(empty)return 5;
    std::puts("Native soft-class loader pointer ABI, private subpath ownership, cache isolation and bounds PASS; no game access.");return 0;
}
