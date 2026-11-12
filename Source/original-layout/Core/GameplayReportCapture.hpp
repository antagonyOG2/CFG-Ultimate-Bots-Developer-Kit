#pragma once
#include "GameplayBundlePolicy.hpp"
#include <cstring>
namespace cfgbundlecapture {
enum class Status { Unreadable, Changing, Invalid, Accepted };
inline Status Observe(bool first,bool second,const cfgbundle::Report& a,const cfgbundle::Report& b){
    if(!first||!second)return Status::Unreadable;
    if(std::memcmp(&a,&b,sizeof(a))||(b.generation&1))return Status::Changing;
    return cfgbundle::Valid(b)?Status::Accepted:Status::Invalid;
}
}
