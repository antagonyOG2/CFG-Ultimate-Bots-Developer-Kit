#pragma once
#include <array>
namespace cfgcapture {
struct Results {std::array<int,4> exits{};unsigned passed=0;};
constexpr bool SnapshotWorldAllowed(bool networked,bool explicitHostedReadOnly){return !networked||explicitHostedReadOnly;}
// Failure in one bounded diagnostic must not hide the other sections or request
// an automatic human retry. The host uses one verified process handle.
template<class Host>Results Collect(Host& host,int observerExit){
    Results result;result.exits[0]=observerExit;
    for(unsigned section=1;section<4;++section){
        try{result.exits[section]=host.Section(section);}
        catch(...){result.exits[section]=-1;}
    }
    for(const auto exit:result.exits)result.passed+=exit==0?1u:0u;
    return result;
}
}
