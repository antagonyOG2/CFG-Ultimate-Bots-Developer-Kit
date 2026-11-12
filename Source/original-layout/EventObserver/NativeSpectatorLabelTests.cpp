#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
struct HostedLabelFixtureReport {unsigned hostedLabelEntered=0,hostedLabelApplied=0,hostedLabelRefused=0;};
inline HostedLabelFixtureReport hostedH1Report{};
inline bool HostedRoutineActive(){return false;}
struct LabelFixture:SpectatorFixture {
#include "NativeSpectatorLabelMembers.hpp"
    void NameForLabel(std::uintptr_t object,int index,const char* text){
        const auto entry=std::uintptr_t{0x830000}+static_cast<unsigned>(index)*256;
        r_.Put(0x810000+8ULL*index,entry);r_.Put(entry,std::uint32_t{});
        std::array<char,128> chars{};for(unsigned i=0;text[i]&&i<127;++i)chars[i]=text[i];
        r_.Put(entry+cfgnames::disk::EntryText,chars);r_.Put(object+cfgnames::disk::ObjectName,cfgnames::Words{index,0});
    }
    LabelFixture(){
        r_.cap=100000;
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateReflection,r_.classes[cfgspawn::PlayerState]);
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,r_.classes[cfgspawn::PlayerState]);
        r_.Put(localState+0x88,plan.controller);r_.Put(localState+cfgescape::disk::bEscaped,std::uint8_t{});
        Serial(localState);
        r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,bundleJason.pawn.object);
        CameraView(fixtureSpectator);Serial(fixtureCamera);
        r_.Put(fixtureSpectator+0x3F8,bundleJason.pawn.object);
        context.objectArray=r_.objects;r_.reads=0;r_.cap=1536;
    }
};
#ifndef CFG_LABEL_FIXTURE_ONLY
int main(){
    {LabelFixture f;f.r_.cap=100000;
     f.r_.objects.count=512;f.context.objectArray=f.r_.objects;
     f.r_.Put(f.plan.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,f.r_.objects.count);
     const auto hudType=f.r_.Object(),widgetType=f.r_.Object(),hud=f.r_.Object(hudType),widget=f.r_.Object(widgetType);
     const auto functionKind=f.r_.Object(),stringKind=f.r_.Object(),function=f.r_.Object(functionKind);
     const auto textField=f.r_.Object(stringKind),modeField=f.r_.Object(stringKind);
     f.Serial(hud);f.Serial(widget);
     f.bundleSpectatorStatusFunction=function;
     f.NameForLabel(functionKind,1,"Function");f.NameForLabel(function,2,"OnSpectatorUpdateStatus");
     f.NameForLabel(stringKind,3,"StrProperty");f.NameForLabel(textField,4,"StatusText");f.NameForLabel(modeField,5,"NextMode");
     f.r_.Put(0x800000+cfgnames::disk::ElementCount,std::array<int,2>{6,1});
     for(const auto cache:{cfgsweater::huddisk::SCInGameHUDReflection,cfgsweater::huddisk::SCInGameHUDConstructor})f.r_.Put(f.plan.module+cache,hudType);
     for(const auto cache:{cfgsweater::huddisk::SCHUDWidgetReflection,cfgsweater::huddisk::SCHUDWidgetConstructor})f.r_.Put(f.plan.module+cache,widgetType);
     f.r_.Put(f.plan.controller+cfgsweater::huddisk::ControllerHUD,hud);f.r_.Put(hud+cfgsweater::huddisk::HUDWidget,widget);
     // Reproduce the real HUD's large unrelated member list. FName lookup
     // must fit the production budget without decoding every member's name.
     auto head=function;
     for(unsigned i=0;i<192;++i){const auto member=f.r_.Object(stringKind);
         f.r_.Put(member+0x20,widgetType);f.r_.Put(member+cfgreflection::disk::Next,head);head=member;}
     f.r_.Put(widgetType+cfgreflection::disk::Children,head);f.r_.Put(function+0x20,widgetType);
     f.r_.Put(function+0x8C,std::uint8_t{2});f.r_.Put(function+0x8E,std::uint16_t{32});
     f.r_.Put(function+cfgreflection::disk::Children,textField);f.r_.Put(textField+0x20,function);f.r_.Put(modeField+0x20,function);
     f.r_.Put(textField+cfgreflection::disk::Next,modeField);
     for(const auto field:{textField,modeField}){f.r_.Put(field+0x30,1);f.r_.Put(field+0x34,16);f.r_.Put(field+0x38,std::uint64_t{0x80});}
     f.r_.Put(textField+0x44,0);f.r_.Put(modeField+0x44,16);
     // Real Blueprint overrides carry locals after their two declared inputs.
     // These are not extra parameters and must not invalidate the HUD frame.
     auto locals=std::uintptr_t{};
     for(unsigned i=0;i<24;++i){const auto local=f.r_.Object(stringKind);
         f.r_.Put(local+0x20,function);f.r_.Put(local+cfgreflection::disk::Next,locals);
         f.r_.Put(local+cfgreflection::disk::PropertyFlags,std::uint64_t{});locals=local;}
     f.r_.Put(modeField+cfgreflection::disk::Next,locals);
     std::array<char16_t,96> text{};std::copy_n(u"Spectating: Other",18,text.data());
     std::array<char16_t,64> mode{};std::copy_n(u"Camp Cameras",13,mode.data());
     f.r_.Put(0x910000,text);f.r_.Put(0x920000,mode);
     f.r_.Put(0x900000,cfgspectatorstatus::Frame{{0x910000,18,96},{0x920000,13,64}});
     LabelFixture::NativeLabelEvent event;f.r_.reads=0;f.r_.cap=1536;
     std::uintptr_t currentWidget=0;
     CHECK(f.NativeLabelWidget(currentWidget)&&currentWidget==widget&&!f.calls_&&!f.teleports&&!f.selections);
     f.r_.reads=0;
     CHECK(f.ResolveNativeLabelEvent(widget,event));f.r_.reads=0;
     cfgspectatorstatus::Frame replacement{};
     CHECK(f.NativeLabelWidget(currentWidget)&&currentWidget==widget&&f.NativeLabelEventIdentityCurrent(event)&&
         f.NativeJasonLabel(widget,function,0x900000,replacement)&&f.NativeLabelEventCurrent(event));
     std::printf("Composed label HUD/frame/native-view/metadata proof reads=%u/1536 overrides=%u\n",f.r_.reads,f.bundleReport.spectatorLabelOverrides);
     CHECK(f.r_.reads<=1536&&f.bundleReport.spectatorLabelOverrides==1&&!f.calls_&&!f.teleports&&!f.selections);
     const auto newWidget=f.r_.Object(widgetType);f.Serial(newWidget);
     f.r_.Put(hud+cfgsweater::huddisk::HUDWidget,newWidget);f.r_.reads=0;
     CHECK(f.NativeLabelWidget(currentWidget)&&currentWidget==newWidget&&currentWidget!=widget);
     CHECK(!f.NativeJasonLabel(widget,function,0x900000,replacement));
     f.r_.Put(hud+cfgsweater::huddisk::HUDWidget,widget);f.r_.reads=0;
      f.r_.Put(f.plan.controller+cfgspectator::disk::SpectatingPlayer,f.r_.bot);f.r_.reads=0;
      cfgescape::State terminal=cfgescape::State::Unknown;
      CHECK(f.NativeJasonLabelProof(&terminal)&&terminal==cfgescape::State::Dead); // selected pointer may lag while stock attachment already views Jason
      f.r_.Put(f.fixtureSpectator+0x3F8,f.r_.bot);f.r_.reads=0;
      CHECK(!f.NativeJasonLabelProof());f.r_.reads=0;
     CHECK(f.NativeLabelEventCurrent(event)&&f.NativeNeutralLabel(widget,function,0x900000,replacement)&&f.NativeLabelEventCurrent(event));
     CHECK(replacement.text.count==11&&replacement.mode.data==0x920000&&f.bundleReport.spectatorStatusJasonView==0);
     CHECK(f.r_.reads<=1536&&!f.calls_&&!f.teleports&&!f.selections);}
    {LabelFixture f;f.r_.cap=100000;auto type=f.r_.Object(),widget=f.r_.Object(type),kind=f.r_.Object(),function=f.r_.Object(kind);
     f.Serial(widget);f.NameForLabel(kind,1,"Function");f.NameForLabel(function,2,"OnSpectatorUpdateStatus");
     f.bundleSpectatorStatusFunction=function;
     f.r_.Put(0x800000+cfgnames::disk::ElementCount,std::array<int,2>{3,1});
     f.r_.Put(type+cfgreflection::disk::Children,function);f.r_.Put(function+0x20,type);
     f.r_.reads=0;f.r_.cap=1536;LabelFixture::NativeLabelEvent event;
     CHECK(f.ResolveNativeLabelEvent(widget,event)&&event.function.serial==0);
     f.r_.reads=0;CHECK(f.NativeLabelEventCurrent(event)&&f.NativeJasonLabelProof()&&f.r_.reads<=1536);
     f.r_.reads=0;f.r_.Put(type+cfgreflection::disk::Children,std::uintptr_t{});
     CHECK(!f.NativeLabelEventCurrent(event));
     f.r_.reads=0;f.r_.Put(type+cfgreflection::disk::Children,function);f.Serial(widget,0);
     CHECK(!f.NativeLabelEventCurrent(event));CHECK(!f.calls_&&!f.teleports&&!f.selections);}
    {LabelFixture f;CHECK(!f.bundleSpectator.Following());CHECK(f.NativeJasonLabelProof());
     CHECK(f.calls_==0&&f.teleports==0&&f.selections==0&&f.r_.reads<=1536);}
     {LabelFixture f;f.r_.Put(f.plan.controller+cfgspectator::disk::SpectatingPlayer,std::uintptr_t{});
      CHECK(!f.bundleSpectator.Following()&&f.NativeJasonLabelProof());CHECK(!f.calls_&&!f.teleports&&!f.selections);}
     {LabelFixture f;f.r_.Put(f.fixtureSpectator+0x3F8,f.r_.bot);
      CHECK(!f.NativeJasonLabelProof());CHECK(!f.calls_&&!f.teleports&&!f.selections);}
    for(unsigned fault=0;fault<8;++fault){LabelFixture f;
        if(fault==0)f.r_.Put(f.fixtureCamera+cfgspectatorview::disk::Active,f.r_.bot);
        if(fault==1)f.r_.Put(f.localState+cfgescape::disk::bDead,std::uint8_t{});
        if(fault==2)f.r_.Put(f.localState+0x88,f.r_.bot);
        if(fault==3)f.Serial(f.localState,0);
        if(fault==4)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==5)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==6)f.r_.cap=1;
        if(fault==7)f.r_.Put(f.plan.controller+cfgprobe::ControllerPlayer,f.r_.bot);
        CHECK(!f.NativeJasonLabelProof());CHECK(!f.calls_&&!f.teleports&&!f.selections);
    }
    std::printf("Native Jason label selection proof: %d failures; no camera writes/calls.\n",failures);return failures?1:0;
}
#endif
