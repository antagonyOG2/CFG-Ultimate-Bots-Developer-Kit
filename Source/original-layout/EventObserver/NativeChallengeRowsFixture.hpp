#pragma once
// Owned-memory UI engine model. Include NativeMenuAdapterTests fixture first.
struct ChallengeRowsEngine{
 NativeFixture& f;unsigned calls=0,creates=0,texts=0;std::array<std::uintptr_t,7> widgets{};
 std::uintptr_t expectedWorld=MenuFixture::Widget,visibilityFunction=0;unsigned ownerThread=42,visibilityCalls=0;
 unsigned Thread()const{return ownerThread;}bool BeginPhase(){return true;}bool Factory(std::uintptr_t,std::uintptr_t&){return false;}
 bool Invoke(std::uintptr_t target,std::uintptr_t function,void* packet){using M=MenuFixture;using F=NativeFixture;
  auto& r=f.fixture.r;++calls;
  if(function==visibilityFunction&&visibilityFunction){++visibilityCalls;return target==M::Donor&&*static_cast<unsigned char*>(packet)==1;}
  if(function==f.createFunction){auto& p=*static_cast<cfgmenu::reflection::CreateParameters*>(packet);
   if(creates==widgets.size()||p.world!=expectedWorld||p.type!=M::DonorType||p.player!=F::Controller)return false;
   const auto widget=std::uintptr_t{0x2600000}+0x10000*creates;widgets[creates++]=widget;p.result=widget;
   r.Set(widget+cfgprobe::ObjectClass,M::DonorType);r.Set(widget+0x180,std::uintptr_t{});
   r.Set(widget+0x188,widget+0x1000);r.Set(widget+0x190,widget+0x1100);
   r.Set(widget+0x1000+cfgprobe::ObjectClass,F::TextType);r.Set(widget+0x1100+cfgprobe::ObjectClass,F::TextType);return true;
  }
  if(function==f.addFunction){auto& p=*static_cast<cfgmenu::reflection::AddParameters*>(packet);if(target!=F::Panel||!p.content)return false;
   p.result=p.content+0x2000;r.Set(p.content+0x180,p.result);r.Set(p.result+cfgprobe::ObjectClass,F::SlotType);
   r.Set(p.result+0x180,F::Panel);r.Set(p.result+0x188,p.content);
   cfgmenu::panelcapture::Array slots{};if(!r.Read(F::Panel+0x200,slots)||slots.count>=64)return false;
   r.Set(slots.data+8*static_cast<unsigned>(slots.count),p.result);++slots.count;r.Set(F::Panel+0x200,slots);return true;
  }
  if(function==M::Options){cfgmenu::reflection::Array input;int selected=0;std::memcpy(&input,packet,16);std::memcpy(&selected,static_cast<unsigned char*>(packet)+16,4);
   if(input.count!=2||selected<0||selected>1)return false;
   struct Header{std::uintptr_t data;int count,capacity;};r.Set(target+0x200,Header{target+0x3000,2,2});r.Set(target+0x210,selected);
   for(int i=0;i<2;++i){auto str=input.data[i];const auto dest=target+0x4000+0x1000*i;
    for(int j=0;j<str.count;++j)r.Set(dest+2*j,str.data[j]);str.data=reinterpret_cast<const char16_t*>(dest);
    r.Set(target+0x3000+16*i,str);
   }return true;
  }
  if(function==f.convertFunction){auto& p=*static_cast<cfgmenu::reflection::TextConversion*>(packet);
   const std::uintptr_t pointers[]{0x2500000,0x2510000};std::memcpy(p.result.bytes.data(),pointers,16);return true;}
  if(function==f.textFunction){
   if(target==F::Title||target==F::Value){++texts;return true;}
   for(const auto widget:widgets)if(widget&&(target==widget+0x1000||target==widget+0x1100)){++texts;return true;}return false;}
  return false;
 }
};
