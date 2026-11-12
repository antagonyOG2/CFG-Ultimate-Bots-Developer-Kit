#pragma once
#include <array>
#include <cstdint>
#include <string_view>
// Diagnostic only. No native calls, writes, dynamic allocations, retained
// UObject pointers or collision edits. Count at most twelve DISTINCT labels.
namespace cfgj29walls {
struct Entry {std::array<char,64> name{};std::uint32_t count=0;std::uint64_t fingerprint=0;std::uint32_t sourceLength=0;};
struct Census {
 std::array<Entry,12> entries{};std::uint32_t distinct=0,total=0;
 static constexpr bool Letter(char16_t c,char a){return c==static_cast<char16_t>(a)||
  (a>='A'&&a<='Z'&&c==static_cast<char16_t>(a+('a'-'A')));}
 static bool Contains(std::u16string_view name,std::u16string_view token){
  if(token.size()>name.size())return false;
  for(std::size_t i=0;i+token.size()<=name.size();++i){std::size_t j=0;
   for(;j<token.size();++j){const char16_t a=name[i+j],b=token[j];
    if(a==b)continue;
    if(b>='A'&&b<='Z'&&Letter(a,static_cast<char>(b)))continue;
    if(b>='a'&&b<='z'&&(a==b-32||a==b))continue;
    break;
   }
   if(j==token.size())return true;
  }return false;
 }
 static bool Match(std::u16string_view name){
  for(const auto token:{u"Volume",u"Blocking",u"Boundary",u"Wall",u"Block",u"Kill",u"Barrier",u"Bounds"}){
   if(Contains(name,token))return true;
  }return false;
 }
 static std::uint64_t Fingerprint(std::u16string_view name){
  std::uint64_t value=14695981039346656037ull;
  for(char16_t c:name){value^=static_cast<std::uint64_t>(c);value*=1099511628211ull;}
  return value;
 }
 bool Observe(std::u16string_view name,bool budgetSafe=true){
  if(!budgetSafe||!Match(name))return false;
  if(total!=UINT32_MAX)++total;
  const auto fingerprint=Fingerprint(name);
  for(unsigned i=0;i<distinct;++i)if(entries[i].fingerprint==fingerprint&&entries[i].sourceLength==name.size()){
   if(entries[i].count!=UINT32_MAX)++entries[i].count;
   return true;
  }
  if(distinct>=entries.size())return true;
  auto& entry=entries[distinct++];entry={};std::size_t i=0;
  for(;i+1<entry.name.size()&&i<name.size();++i)entry.name[i]=name[i]<128?static_cast<char>(name[i]):'?';
  entry.name[i]='\0';entry.count=1;entry.fingerprint=fingerprint;entry.sourceLength=static_cast<std::uint32_t>(name.size());return true;
 }
};
}
