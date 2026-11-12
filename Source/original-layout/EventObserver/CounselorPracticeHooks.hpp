// Native voice calls are forwarded with their incoming integer argument
// registers intact. This concrete void sink consumes component/FName only;
// unknown lines, foreign owners, wrong worlds and OFF all forward unchanged.
using PracticeVoiceOriginal=void(__fastcall*)(void*,cfgnames::Words,std::uintptr_t,std::uintptr_t);
void __fastcall J215NativeVoice(void* component,cfgnames::Words line,std::uintptr_t arg3,std::uintptr_t arg4){
 bool mute=false;
#if CFG_J215_PAMELA_MUTE
 if(GetCurrentThreadId()==plan.threadId&&bundleStarted&&bundleSchedule.active&&
    cfgpractice::Enabled(bundleConfiguration.counselorPractice,cfgpractice::PamelaMute)){
  Reader r;r.cap=1024;cfgsoundblip::Scope scope;cfgnames::Text text;
  ++bundleReport.j215Practice.voiceSeen;
  if(bundlePracticeVoiceHook.Current(r)&&
     ((J215PracticeScope(r,false,scope)&&cfgpracticeobs::Voice(r,scope,reinterpret_cast<std::uintptr_t>(component),line,text))||
      (J215PracticeScope(r,true,scope)&&cfgpracticeobs::Voice(r,scope,reinterpret_cast<std::uintptr_t>(component),line,text)))){
   mute=cfgpractice::MutePamela(text.View());
   if(mute)++bundleReport.j215Practice.voiceMuted;else ++bundleReport.j215Practice.voiceKept;
  }else ++bundleReport.j215Practice.voiceRefused;
  bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);
 }
#endif
 if(!mute&&bundlePracticeVoiceHook.original)reinterpret_cast<PracticeVoiceOriginal>(bundlePracticeVoiceHook.original)(component,line,arg3,arg4);
}
bool PrepareJ215PracticeHooks(std::uintptr_t module){
#if CFG_J215_PAMELA_MUTE
 Reader r;r.cap=32;namespace d=cfgpractice::disk;
 bundlePracticeVoiceInstallFailure=1;
 if(!cfgbundle::Code(r,module,d::PamelaWrapper,d::PamelaWrapperCode)||
    !cfgbundle::Code(r,module,d::VoiceNativeConstructor,d::VoiceNativeConstructorCode))return false;
 bundlePracticeVoiceInstallFailure=2;
 const bool okay=PrepareKnifeWitness(module,d::VoiceNative,d::VoiceNativeCode,reinterpret_cast<void*>(&J215NativeVoice),bundlePracticeVoiceHook);
 if(okay)bundlePracticeVoiceInstallFailure=0;return okay;
#else
 (void)module;return true;
#endif
}
bool J215HideHealth(void* object,void* function,void* parameters){
#if CFG_J215_MINIMAL_HUD
 if(!parameters||!cfgpractice::Enabled(bundleConfiguration.counselorPractice,cfgpractice::MinimalHud))return false;
 Reader r;r.cap=2048;cfgsweater::HudEvidence hud;cfgprobe::lifetime::ArrayHeader array;
 if(!J215PracticeHud(r,hud,array)||hud.widget.object!=reinterpret_cast<std::uintptr_t>(object))return false;
 cfgreflection::Observation<Reader> reflection(r,plan.module);cfgreflection::Field field;
 const auto fn=reinterpret_cast<std::uintptr_t>(function);
 if(!reflection.ReadField(fn,0,field)||field.kind.View()!=u"Function"||field.label.View()!=u"OnShowHealthBar")return false;
 std::uint32_t flags=0;std::uint8_t count=0,value=0;std::uint16_t bytes=0;std::uintptr_t owner=0,type=0,child=0;
 cfgreflection::Field argument;cfgreflection::Property property;
 if(!r.Read(fn+0x20,owner)||!r.Read(hud.widget.object+cfgprobe::ObjectClass,type)||!cfgspawn::ClassContains(r,type,owner)||
    !r.Read(fn+0x88,flags)||!(flags&0x08000000u)||!(flags&0x800u)||(flags&0x40u)||
    !r.Read(fn+0x8C,count)||count!=1||!r.Read(fn+0x8E,bytes)||bytes!=1||
    !r.Read(fn+cfgreflection::disk::Children,child)||!reflection.ReadField(child,fn,argument)||
    argument.label.View()!=u"bShow"||argument.kind.View()!=u"BoolProperty"||argument.next||
    !cfgmenu::capture::RawProperty(r,argument,property)||property.offset||property.dimension!=1||property.elementBytes!=1||
    !(property.flags&0x80u)||(property.flags&0x500u)||!r.Read(reinterpret_cast<std::uintptr_t>(parameters),value)||value>1){
  ++bundleReport.j215Practice.healthRefused;return false;
 }
 if(!value)return false;
 cfgsweater::HudEvidence fresh;cfgprobe::lifetime::ArrayHeader again;
 if(!J215PracticeHud(r,fresh,again)||fresh.widget.object!=hud.widget.object||!(fresh.controller==hud.controller))return false;
 ++bundleReport.j215Practice.healthHidden;
 bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);return true;
#else
 (void)object;(void)function;(void)parameters;return false;
#endif
}
