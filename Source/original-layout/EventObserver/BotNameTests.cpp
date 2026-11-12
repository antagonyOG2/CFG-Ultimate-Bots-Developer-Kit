#include "../Core/BotNamePolicy.hpp"
#include "../RuntimeProbe/BotNameBindings.generated.hpp"
#include "BotNameObservation.hpp"
#define main BotNameResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include <cwchar>
#include <cstdio>
int main(){unsigned nameFailures=0;
 {Fixture f;f.cap=1536;const auto type=f.Object(),pawn=f.Object(),controller=f.Object(),state=f.Object(type);
  const auto human=f.Object(),humanController=f.Object(),humanState=f.Object(type);
  for(const auto object:{pawn,controller}){int index=0;f.Read(object+12,index);f.Put(f.objects.data+24*static_cast<std::uintptr_t>(index)+16,int{123});}
  f.Put(pawn+cfgescape::disk::Controller,controller);f.Put(controller+cfgescape::disk::Pawn,pawn);
  f.Put(pawn+cfgescape::disk::PlayerState,state);f.Put(controller+cfgescape::disk::PlayerState,state);
  f.Put(state+cfgspawn::ActorOwner,controller);f.Put(type+cfgprobe::ClassSuper,std::uintptr_t{});
  cfgprobe::lifetime::SlotObservation slot{};
  const auto check=[&](bool ok){if(!ok)++nameFailures;};
  const auto prove=[&](){f.reads=0;return cfgbotname::OwnedState(f,f.objects,pawn,controller,state,type,human,humanController,humanState,slot);};
  check(prove()&&slot.serial==0&&f.reads<128);
  f.Put(state+cfgspawn::ActorOwner,humanController);check(!prove());f.Put(state+cfgspawn::ActorOwner,controller);
  f.Put(controller+cfgescape::disk::PlayerState,humanState);check(!prove());f.Put(controller+cfgescape::disk::PlayerState,state);
  f.Put(state+cfgprobe::ObjectClass,controller);check(!prove());f.Put(state+cfgprobe::ObjectClass,type);
  check(!cfgbotname::OwnedState(f,f.objects,pawn,controller,state,type,pawn,humanController,humanState,slot));
  check(!cfgbotname::OwnedState(f,f.objects,pawn,controller,state,type,human,controller,humanState,slot));
  check(!cfgbotname::OwnedState(f,f.objects,pawn,controller,state,type,human,humanController,state,slot));
  int index=0;f.Read(controller+12,index);f.Put(f.objects.data+24*static_cast<std::uintptr_t>(index)+16,int{0});check(!prove());
 }
 for(unsigned seed=0;seed<256;++seed){
    std::array<cfgbotname::Name,cfgbotname::Maximum> names{};
    for(unsigned i=0;i<names.size();++i){names[i]=cfgbotname::Make(i,seed);const auto& n=names[i];
        if(n.count<2||n.count>31||n.text[static_cast<unsigned>(n.count-1)]||std::wcslen(n.text.data())!=static_cast<std::size_t>(n.count-1))++nameFailures;
        for(unsigned j=0;j<i;++j)if(std::wcscmp(n.text.data(),names[j].text.data())==0)++nameFailures;
    }
}if(cfgbotname::Make(cfgbotname::Maximum,0).count)++nameFailures;
std::printf("CFG fictional bot names/owned native state: %s; all256 seeds,31 unique bounded names each, extra-bot indexes, no account inputs (%u failures).\n",nameFailures?"FAIL":"PASS",nameFailures);return nameFailures?1:0;}
