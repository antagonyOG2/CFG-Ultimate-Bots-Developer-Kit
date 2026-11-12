#define CFGMENU_OPTIONS_FIXTURE_ONLY
#include "../RuntimeProbe/MenuOptionsScriptTests.cpp"
#undef CFGMENU_OPTIONS_FIXTURE_ONLY
#include "NativeMenuRowAccess.hpp"
#include "NativeSetupRuntime.hpp"
#include <string>
#include <memory>
// Engine-facing adapter tested against a byte-addressable OWNED engine model.
// This exercises its real reflection/ABI/ownership/deep-copy path, not merely
// the scheduler's permissive fake Host. No game process or native code access.
struct NativeFixture : OptionsFixture {
    static constexpr std::uintptr_t Engine=0x1000000,Viewport=0x1001000,World=0x1002000,Instance=0x1003000,
        Players=0x1004000,Player=0x1005000,Controller=0x1006000,HUD=0x1007000,Stack=0x1008000,
        Records=0x1009000,Mode=0x100A000,Frontend=0x100B000,Panel=0x100C000,PanelType=0x100D000,
        SlotType=0x100E000,DonorSlot=0x100F000,Registry=0x1010000,
        SlotField=0x1020000,ParentField=0x1020100,ContentField=0x1020200,TitleField=0x1020300,ValueField=0x1020400,
        NewWidget=0x1030000,NewSlot=0x1031000,Title=0x1032000,Value=0x1033000,TextType=0x1034000,
        CreateOwner=0x1040000,ConvertOwner=0x1041000,RemoveOwner=0x1042000,
        SlotsField=0x1043000,SlotsInner=0x1043100,PanelSlots=0x1044000;
    void Children(std::initializer_list<std::uintptr_t> slots){
        fixture.r.Set(Panel+0x200,cfgmenu::panelcapture::Array{PanelSlots,static_cast<int>(slots.size()),64});
        unsigned i=0;for(const auto slot:slots)fixture.r.Set(PanelSlots+8*i++,slot);
    }
    unsigned slotCount=0;
    std::uintptr_t createFunction=0,addFunction=0,textFunction=0,convertFunction=0,removeFunction=0;
    void Register(std::uintptr_t object,int serial=0){auto& r=fixture.r;
        const auto i=slotCount++;r.Set(object+cfgprobe::lifetime::ObjectIndexOffset,static_cast<int>(i));
        r.Set(Registry+24*i,object);r.Set(Registry+24*i+16,serial);
        r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(slotCount));
    }
    void Root(std::uintptr_t type){fixture.r.Set(type+cfgprobe::ClassSuper,std::uintptr_t{0});}
    void ObjectMember(std::uintptr_t field,std::uintptr_t owner,const char* name,std::uintptr_t next,int offset){
        fixture.Field(field,owner,MenuFixture::ObjectProperty,name,next);fixture.Property(field,8,offset);
    }
    static std::string Narrow(std::u16string_view text){std::string result;for(auto c:text)result.push_back(static_cast<char>(c));return result;}
    template<std::size_t N,std::size_t C> std::uintptr_t Binding(const cfgmenu::execdisk::Binding& binding,
        const std::array<cfgmenu::execdisk::Parameter,N>& params,const std::array<unsigned char,C>& code,std::uintptr_t owner,unsigned index){
        auto& f=fixture;auto& r=f.r;const auto fn=std::uintptr_t{0x1200000}+index*0x10000;
        r.Set(MenuFixture::Base+binding.reflection,owner);r.Set(MenuFixture::Base+binding.constructor,owner);
        r.Set(MenuFixture::Base+binding.cache,fn);Root(owner);
        r.Set(MenuFixture::Base+binding.executor,code);
        const auto name=Narrow(binding.name);f.Field(fn,owner,MenuFixture::FunctionClass,name.c_str(),0);
        r.Set(fn+cfgreflection::disk::Children,N?fn+0x100:std::uintptr_t{0});
        r.Set(fn+cfgreflection::disk::StorageBytes,static_cast<int>(binding.frame));
        r.Set(fn+cfgreflection::disk::FunctionFlags,binding.flags);
        r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(cfgmenu::native::ParameterExtent(binding,params)));
        r.Set(fn+0xB0,MenuFixture::Base+binding.executor);r.Set(fn+0x68,std::uintptr_t{0});
        r.Set(fn+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{});Register(fn);
        for(unsigned i=0;i<N;++i){const auto& p=params[i];const auto field=fn+0x100+0x100*i;
            const auto kind=fn+0x3000+0x100*i;const auto label=cfgmenu::native::Kind(p.kind);
            const auto kindName=Narrow(label),paramName=Narrow(p.name);
            f.Name(kind,kindName.c_str());f.Field(field,fn,kind,paramName.c_str(),i+1<N?field+0x100:0);
            f.Property(field,p.kind==12?1:p.kind==21?16:p.kind==28?24:8,static_cast<int>(p.offset));
            auto actualFlags=cfgmenu::native::RuntimeFlags(binding,p);
            // Independent recorded CFG values, not the validator under test:
            // diagnostics30843 RemoveChild parameters, lines41/42.
            if(binding.name==u"RemoveChild")actualFlags=p.name==u"Content"?0x0018001040080280ULL:0x0018001040000780ULL;
            r.Set(field+cfgreflection::disk::PropertyFlags,actualFlags);
        }
        return fn;
    }
    NativeFixture(){using namespace cfgprobe;using namespace cfgmenu::execdisk;
        auto& f=fixture;auto& r=f.r;f.Name(MenuFixture::DonorType,"SettingsOptionComboBoxWidget_C");
        r.Set(MenuFixture::Table+cfgnames::disk::ElementCount,std::array<int,2>{400,1});
        r.Set(MenuFixture::Base+lifetime::ObjectArrayRva+16,Registry);
        r.Set(MenuFixture::Base+dispatch::ThreadFlagRva,std::uint8_t{1});r.Set(MenuFixture::Base+dispatch::ThreadIdRva,std::uint32_t{42});
        r.Set(MenuFixture::Base+EngineGlobalCandidate,Engine);r.Set(Engine+EngineGameViewport,Viewport);r.Set(Viewport+ViewportWorld,World);
        r.Set(World+WorldNetDriver,std::uintptr_t{0});r.Set(World+WorldGameMode,Mode);r.Set(World+WorldGameInstance,Instance);
        r.Set(Instance+GameInstanceLocalPlayers,Players);r.Set(Instance+GameInstanceLocalPlayers+8,1);r.Set(Instance+GameInstanceLocalPlayers+12,1);
        r.Set(Players,Player);r.Set(Player+cfgprobe::PlayerController,Controller);r.Set(Controller+ControllerHUD,HUD);r.Set(HUD+HUDStack,Stack);
        r.Set(Stack+StackArray,cfgmenu::active::StackArray{Records,1,1});r.Set(Records+RecordWidget,MenuFixture::Widget);
        Register(World,100);Register(Controller,101);Register(MenuFixture::Widget,102);Register(MenuFixture::Options);Register(HUD,103);
        r.Set(MenuFixture::Base+FrontendClass,Frontend);r.Set(MenuFixture::Base+FrontendConstructor,Frontend);r.Set(Mode+ObjectClass,Frontend);
        Root(Frontend);Root(MenuFixture::Type);Root(MenuFixture::DonorType);Root(SlotType);
        r.Set(MenuFixture::DonorType+cfgreflection::disk::Children,SlotField);
        ObjectMember(SlotField,MenuFixture::DonorType,"Slot",TitleField,0x180);
        ObjectMember(TitleField,MenuFixture::DonorType,"TitleText",ValueField,0x188);
        ObjectMember(ValueField,MenuFixture::DonorType,"ValueText",DestinationItems,0x190);
        r.Set(SlotType+cfgreflection::disk::Children,ParentField);
        ObjectMember(ParentField,SlotType,"Parent",ContentField,0x180);ObjectMember(ContentField,SlotType,"Content",0,0x188);
        r.Set(MenuFixture::Donor+0x180,DonorSlot);r.Set(DonorSlot+ObjectClass,SlotType);r.Set(DonorSlot+0x180,Panel);r.Set(DonorSlot+0x188,MenuFixture::Donor);
        r.Set(Panel+ObjectClass,PanelType);
        r.Set(PanelType+cfgreflection::disk::Children,SlotsField);
        f.Field(SlotsField,PanelType,MenuFixture::ArrayProperty,"Slots",0);f.Property(SlotsField,16,0x200);
        ObjectMember(SlotsInner,SlotsField,"Slots",0,0);
        r.Set(SlotsField+cfgreflection::arraydisk::Inner,SlotsInner);Children({DonorSlot});
        r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,20);
        r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{20});
        r.Set(MenuFixture::Options+cfgreflection::disk::FunctionFlags,std::uint32_t{0x0C420000});
        r.Set(MenuFixture::Options+0xB0,MenuFixture::Base+cfgcompat::cfg35::dispatch::BlueprintExecutorRva);r.Set(MenuFixture::Options+0x68,MenuFixture::Items);
        createFunction=Binding(Create,CreateParameters,CreateCode,CreateOwner,0);
        addFunction=Binding(AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode,PanelType,1);
        textFunction=Binding(SetText,SetTextParameters,SetTextCode,TextType,2);
        convertFunction=Binding(Conv_StringToText,Conv_StringToTextParameters,Conv_StringToTextCode,ConvertOwner,3);
        removeFunction=Binding(RemoveChild,RemoveChildParameters,RemoveChildCode,RemoveOwner,4);
        r.Set(PanelType+ClassSuper,RemoveOwner);Root(RemoveOwner);
        for(const auto owner:{CreateOwner,ConvertOwner}){const auto defaults=owner+0x800;
            r.Set(owner+cfgspawn::ClassDefaultObject,defaults);r.Set(defaults+ObjectClass,owner);}
        r.Set(MenuFixture::DonorType+cfgspawn::ClassDefaultObject,std::uintptr_t{0x1090000});
    }
};
struct ModelEngine {
    NativeFixture& model;unsigned calls=0,creates=0,attaches=0,texts=0,removes=0;
    bool shallowDescriptors=false,shallowStrings=false,failAttach=false,reverseOnlyAttach=false;
    std::uintptr_t optionsTarget=NativeFixture::NewWidget,expectedWorld=MenuFixture::Widget;
    unsigned Thread()const{return 42;}
    bool BeginPhase(){return true;}
    bool Factory(std::uintptr_t,std::uintptr_t&){return false;}
    bool Invoke(std::uintptr_t target,std::uintptr_t function,void* packet){
        ++calls;auto& r=model.fixture.r;
        using F=NativeFixture;using M=MenuFixture;
        if(function==model.createFunction){++creates;
            auto& p=*static_cast<cfgmenu::reflection::CreateParameters*>(packet);
            if(p.world!=expectedWorld||p.type!=M::DonorType||p.player!=F::Controller)return false;
            if(creates>1){p.result=F::NewWidget;return true;} // Inject an invalid reused result without changing its existing tree.
            r.Set(F::NewWidget+cfgprobe::ObjectClass,M::DonorType);r.Set(F::NewWidget+0x180,std::uintptr_t{0});
            r.Set(F::NewWidget+0x188,F::Title);r.Set(F::NewWidget+0x190,F::Value);
            r.Set(F::Title+cfgprobe::ObjectClass,F::TextType);r.Set(F::Value+cfgprobe::ObjectClass,F::TextType);p.result=F::NewWidget;return true;
        }
        if(function==model.addFunction){++attaches;if(failAttach)return false;
            auto& p=*static_cast<cfgmenu::reflection::AddParameters*>(packet);if(target!=F::Panel||p.content!=F::NewWidget)return false;
            r.Set(F::NewWidget+0x180,F::NewSlot);r.Set(F::NewSlot+cfgprobe::ObjectClass,F::SlotType);
            r.Set(F::NewSlot+0x180,F::Panel);r.Set(F::NewSlot+0x188,F::NewWidget);
            if(!reverseOnlyAttach){cfgmenu::panelcapture::Array slots{};if(!r.Read(F::Panel+0x200,slots))return false;
                r.Set(slots.data+8*static_cast<unsigned>(slots.count),F::NewSlot);++slots.count;r.Set(F::Panel+0x200,slots);}
            p.result=F::NewSlot;return true;
        }
        if(function==M::Options){
            cfgmenu::reflection::Array input;int selected=0;std::memcpy(&input,packet,16);std::memcpy(&selected,static_cast<unsigned char*>(packet)+16,4);
            if(target!=optionsTarget)return false;const std::uintptr_t array=shallowDescriptors?reinterpret_cast<std::uintptr_t>(input.data):0x1400000;
            struct Header{std::uintptr_t data;int count,capacity;};r.Set(target+0x200,Header{array,input.count,input.count});r.Set(target+0x210,selected);
            for(int i=0;i<input.count;++i){auto str=input.data[i];const auto dest=std::uintptr_t{0x1410000}+0x100*static_cast<unsigned>(i);
                if(!shallowStrings){for(int j=0;j<str.count;++j)r.Set(dest+2*static_cast<unsigned>(j),str.data[j]);str.data=reinterpret_cast<const char16_t*>(dest);}
                r.Set(array+16*static_cast<unsigned>(i),str);
            }return true;
        }
        if(function==model.convertFunction){auto& p=*static_cast<cfgmenu::reflection::TextConversion*>(packet);
            const std::uintptr_t pointers[]{0x1500000,0x1510000};std::memcpy(p.result.bytes.data(),pointers,16);return true;}
        if(function==model.textFunction){++texts;return target==F::Title||target==F::Value;}
        if(function==model.removeFunction){++removes;if(target!=F::Panel)return false;
            std::uintptr_t child=0;std::memcpy(&child,packet,8);if(child!=F::NewWidget)return false;
            cfgmenu::panelcapture::Array slots{};if(!r.Read(F::Panel+0x200,slots))return false;
            unsigned remaining=0;for(int i=0;i<slots.count;++i){std::uintptr_t slot=0;
                if(!r.Read(slots.data+8*static_cast<unsigned>(i),slot))return false;
                if(slot!=F::NewSlot)r.Set(slots.data+8*remaining++,slot);}
            slots.count=static_cast<int>(remaining);r.Set(F::Panel+0x200,slots);
            r.Set(child+0x180,std::uintptr_t{0});*(static_cast<unsigned char*>(packet)+8)=1;return true;}
        return false;
    }
};
struct CountFixture : NativeFixture {
    static constexpr std::uintptr_t Count=0x1600000,CountSlot=0x1601000,CountField=0x1602000,Items=0x1603000;
    CountFixture(){auto& r=fixture.r;std::uintptr_t head=0;
        r.Read(MenuFixture::Type+cfgreflection::disk::Children,head);
        ObjectMember(CountField,MenuFixture::Type,"CounselorCountComboBox",head,0x300);
        r.Set(MenuFixture::Type+cfgreflection::disk::Children,CountField);r.Set(MenuFixture::Widget+0x300,Count);
        r.Set(Count+cfgprobe::ObjectClass,MenuFixture::DonorType);r.Set(Count+0x180,CountSlot);
        r.Set(CountSlot+cfgprobe::ObjectClass,SlotType);r.Set(CountSlot+0x180,Panel);r.Set(CountSlot+0x188,Count);
        Children({DonorSlot,CountSlot});
        struct Header {std::uintptr_t data;int count,capacity;};r.Set(Count+0x200,Header{Items,7,7});r.Set(Count+0x210,6);
        for(unsigned i=0;i<7;++i){const auto data=std::uintptr_t{0x1610000}+0x100*i;
            r.Set(data,static_cast<char16_t>(u'1'+i));r.Set(data+2,char16_t{});
            r.Set(Items+16*i,cfgmenu::reflection::String{reinterpret_cast<const char16_t*>(data),2,2});}
    }
};
struct SkillFixture:NativeFixture {
    SkillFixture(){auto& r=fixture.r;using M=MenuFixture;
        struct Header {std::uintptr_t data;int count,capacity;};r.Set(M::Donor+0x200,Header{0x1670000,3,3});r.Set(M::Donor+0x210,2);
        r.Set(M::Donor+0x188,Title);r.Set(Title+cfgprobe::ObjectClass,TextType);
        const std::array<std::u16string_view,3> labels{{u"Easy",u"Normal",u"Hard"}};
        for(unsigned i=0;i<3;++i){const auto data=std::uintptr_t{0x1680000}+0x100*i;const auto label=labels[i];
            for(unsigned j=0;j<label.size();++j)r.Set(data+2*j,label[j]);r.Set(data+2*label.size(),char16_t{});
            const auto count=static_cast<int>(label.size()+1);r.Set(0x1670000+16*i,cfgmenu::reflection::String{reinterpret_cast<const char16_t*>(data),count,count});}
    }
};
#ifndef CFGMENU_NATIVE_FIXTURE_ONLY
int main(){unsigned failures=0;const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;using M=MenuFixture;using F=NativeFixture;
    {
        NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};
        reflection::RetainedMenuText pool;rows::NativeAccess<OptionsReader,ModelEngine> access(reader,engine,pool);active::Context context;
        fixture.fixture.r.Set(M::Options+0xB0,M::Base+0x7AA0E0);
        check(active::Observe(reader,M::Base,context)&&access.Prepare(context,19)&&engine.calls==0,
            "read-only menu preparation does not dispatch options");
        std::uintptr_t widget=0;bool attached=false;
        check(access.Append(widget,attached)&&attached,"stale options fixture creates owned row normally");
        rows::Spec stale;stale.id=RowId::Duration;stale.title=u"Match Length";
        stale.count=1;stale.labels[0]=u"30 minutes";
        const auto calls=engine.calls;
        check(!access.CopyOptions(widget,stale)&&engine.calls==calls,
            "3.4 Blueprint executor cannot authorize 3.5 options dispatch");
    }
    rows::Spec spec;spec.id=RowId::Duration;spec.title=u"Match Length";spec.labels[0]=u"30 minutes";spec.labels[1]=u"60 minutes";spec.count=2;spec.selected=1;
    for(unsigned fault=0;fault<10;++fault){
        NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};
        reflection::RetainedMenuText text;rows::NativeAccess<OptionsReader,ModelEngine> access(reader,engine,text);active::Context context;
        check(active::Observe(reader,M::Base,context)&&access.Prepare(context,19),"forward panel schema prepares");
        engine.reverseOnlyAttach=fault==1;
        std::uintptr_t widget=0;bool attached=false;
        check(access.Append(widget,attached)==(fault!=1),"reverse-only attachment is not native panel ownership");
        if(fault==1){check(!attached&&engine.creates==1&&engine.attaches==1,"missing strong child fails in creation phase");continue;}
        auto& r=fixture.fixture.r;
        if(fault==2)fixture.Children({F::DonorSlot}); // stale reverse pointers deliberately remain
        if(fault==3)fixture.Children({F::DonorSlot,F::NewSlot,F::NewSlot});
        if(fault==4)r.Set(F::Panel+0x200,panelcapture::Array{F::PanelSlots,65,65});
        if(fault==5)r.Set(F::Panel+0x200,panelcapture::Array{F::PanelSlots,2,1});
        if(fault==6)r.Set(F::SlotsField+cfgreflection::disk::PropertyOffset,0x208);
        if(fault==7)r.Set(F::SlotsField+cfgreflection::arraydisk::Inner,F::SlotField);
        if(fault==8)r.Set(F::SlotsInner+cfgreflection::disk::PropertyElementSize,16);
        if(fault==9)r.Set(F::PanelSlots+8,std::uintptr_t{1});
        const auto calls=engine.calls;
        check(access.CopyOptions(widget,spec)==(fault==0),"later callback revalidates forward membership and metadata");
        if(fault)check(engine.calls==calls&&!access.RemoveOwned(widget)&&engine.calls==calls,
            "unowned/stale settings row gets no setter or rollback native call");
    }
    for(unsigned fault=0;fault<4;++fault){NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};
        engine.shallowDescriptors=fault==1;engine.shallowStrings=fault==2;engine.failAttach=fault==3;
        reflection::RetainedMenuText pool;rows::NativeAccess<OptionsReader,ModelEngine> access(reader,engine,pool);active::Context context;
        check(active::Observe(reader,M::Base,context)&&access.Prepare(context,19),"real adapter native owner preparation");
        std::uintptr_t widget=0;bool attached=false;check(access.Append(widget,attached)==(fault!=3),"native Create then attach");
        check(engine.creates==1&&engine.attaches==1,"no yield between Create and attachment");
        if(fault==3){check(!attached&&!access.RemoveOwned(widget)&&engine.removes==0,"unattached result cannot remove native panel children");continue;}
        check(widget==F::NewWidget&&attached,"native ownership readback");
        check(access.CopyOptions(widget,spec)==(fault==0),"reject shallow descriptor/string borrowing");
        if(fault==0){unsigned index=0;check(access.Index(widget,index)&&index==1,"native selected value");
            reflection::RetainedMenuText::Request text;check(access.Text(spec.title,text)&&access.SetText(widget,u"TitleText",text),"native retained conversion and title");
            check(access.Text(spec.labels[1],text)&&access.SetText(widget,u"ValueText",text)&&engine.texts==2,"native selected label");
            const auto calls=engine.calls;check(access.Text(spec.title,text)&&engine.calls==calls&&pool.Used()==2,"same native FText retained across views");
        }
        const auto attachCount=engine.attaches;std::uintptr_t duplicate=0;bool duplicateAttached=false;
        check(!access.Append(duplicate,duplicateAttached)&&!duplicate&&!duplicateAttached&&engine.attaches==attachCount,"repeated Create result cannot overwrite a previous row");
        check(!access.RemoveOwned(M::Donor)&&engine.removes==0,"native donor is never rollback-owned");
        check(access.RemoveOwned(widget)&&engine.removes==1&&!access.RemoveOwned(widget),"own row detached exactly once");
        const auto calls=engine.calls;fixture.fixture.r.Set(F::Registry+24*2+16,999);
        reflection::RetainedMenuText::Request changed;
        check(!access.Current(19)&&!access.Text(u"Changed owner",changed)&&engine.calls==calls,"owner replacement stops engine calls");
    }
    for(unsigned fault=0;fault<3;++fault){
        CountFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};engine.optionsTarget=CountFixture::Count;
        engine.shallowDescriptors=fault==1;engine.shallowStrings=fault==2;
        auto text=std::make_unique<reflection::RetainedMenuText>();rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,*text);active::Context context;
        rows::Spec count;count.id=RowId::Count;count.title=u"Counselors";count.count=10;count.selected=6;count.reuseNativeCount=true;count.configureNativeCount=true;
        std::array<std::u16string,10> labels;for(unsigned i=0;i<10;++i){labels[i]=i==9?u"10":std::u16string(1,static_cast<char16_t>(u'1'+i));count.labels[i]=labels[i];}
        fixture.fixture.r.Set(CountFixture::Count+0x210,4);
        check(active::Observe(reader,M::Base,context)&&view.Open(context,19,&count,1)&&view.Service(0),"existing native count borrowed before donor expansion");
        check(view.State()==rows::ViewState::Building&&engine.calls==0,"count borrowed in its own native callback");
        check(view.Service(25)==(fault==0),"native SetOptions deep copies donor1-10; shallow copies refuse");
        check(engine.calls==1&&engine.creates==0&&engine.attaches==0&&engine.texts==0,"one original-count setter; no duplicate row");
        if(fault)continue;
        unsigned index=0;check(view.State()==rows::ViewState::Ready&&view.Index(CountFixture::Count,index)&&index==4,"ten-choice count retains actual saved selection");
        struct Header {std::uintptr_t data;int count,capacity;};Header header{};fixture.fixture.r.Read(CountFixture::Count+0x200,header);
        check(header.count==10,"all ten options exist in native owned list");
        fixture.fixture.r.Set(CountFixture::Count+0x210,9);check(view.Index(CountFixture::Count,index)&&index==9,"native option10 accepted for nine paced helpers plus human");
        check(view.Close()&&view.Service(50)&&view.Service(75)&&view.State()==rows::ViewState::Closed&&engine.removes==0,"Back retains native count attachment");
    }
    for(unsigned fault=0;fault<3;++fault){
        SkillFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};engine.optionsTarget=M::Donor;
        engine.shallowDescriptors=fault==1;engine.shallowStrings=fault==2;
        auto text=std::make_unique<reflection::RetainedMenuText>();rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,*text);active::Context context;
        rows::Spec skill;skill.id=RowId::Skill;skill.title=u"Counselor Bot Skill";skill.count=3;skill.selected=1;skill.reuseNativeSkill=true;skill.preserveNativeValue=true;
        skill.labels[0]=u"Low";skill.labels[1]=u"Medium";skill.labels[2]=u"High";
        check(active::Observe(reader,M::Base,context)&&view.Open(context,19,&skill,1),"original skill row opened for donor labels");
        for(unsigned i=0;i<6;++i)check(view.Service(25*i),"warm native bindings then borrow native skill");
        check(view.Service(150)==(fault==0),"native skill SetOptions copies all donor labels; shallow copies refuse");
        check(engine.creates==0&&engine.attaches==0,"original skill row remains native");if(fault)continue;
        check(view.Service(175)&&view.Service(200)&&view.State()==rows::ViewState::Ready,"original native skill title relabeled through retained FText");
        unsigned semantic=0;check(view.NativeSkill(semantic)&&semantic==2,"saved native Hard selection remains semantic High");
        check(engine.calls==3&&engine.texts==1,"one options setter, conversion and title setter only");
        check(view.Close()&&view.Service(225)&&view.Service(250)&&view.State()==rows::ViewState::Closed&&engine.removes==0,"Back preserves native skill row");
    }
    {
        NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};reflection::RetainedMenuText text;
        rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,text);active::Context current;
        check(active::Observe(reader,M::Base,current)&&view.Open(current,19,&spec,1),"combined native view opening");
        check(!view.Open(current,20,&spec,1),"no duplicate opening of active view");
        for(unsigned i=0;i<11;++i){check(view.Service(i*25),"one native warmup/row phase");check(!view.Service(i*25),"same callback cannot bunch work");}
        check(view.State()==rows::ViewState::Ready&&engine.creates==1&&engine.attaches==1,"native view reaches Ready after bounded phases");
        // Native Back/forward may retain this exact tree while reallocating
        // its menu stack. Resume the rows; never append a second copy.
        fixture.fixture.r.Set(F::Stack+execdisk::StackArray,active::StackArray{F::Records+0x100,1,1});
        fixture.fixture.r.Set(F::Records+0x100+execdisk::RecordWidget,M::Widget);
        active::Context returned;check(active::Observe(reader,M::Base,returned)&&view.Rebind(returned),"cached settings rebinds actual current stack");
        check(engine.creates==1&&engine.attaches==1,"cached settings resume never duplicates native rows");
        auto recycled=returned;++recycled.identities[2].serial;
        check(!view.Rebind(recycled),"recycled widget cannot inherit cached settings");
        current=returned;
        const auto reads=fixture.fixture.r.reads,calls=engine.calls;
        for(unsigned i=0;i<1000;++i)check(!view.Service(1000+i*25),"ready view has no ongoing service");
        check(fixture.fixture.r.reads==reads&&engine.calls==calls,"zero idle menu scans/native calls after Ready");
        std::array<unsigned,rows::Capacity> choices;check(view.Rows().Choices(view,19,choices)&&choices[0]==1,"Start reads native choices through combined view");
        check(view.Close()&&view.State()==rows::ViewState::Retiring&&engine.removes==0,"Back requests paced cleanup before native departure");
        check(!view.Open(current,20,&spec,1),"cannot reopen a view with attached rows");
        check(view.Service(30000)&&engine.removes==1&&view.State()==rows::ViewState::Retiring,"remove only owned row while current");
        check(view.Service(30025)&&view.State()==rows::ViewState::Closed,"route may depart after cleanup completes");
        check(view.Open(current,20,&spec,1),"same cached native widget can reopen without duplicate children");
        for(unsigned i=0;i<11;++i)check(view.Service(30100+i*25),"replayed native view phased setup");
        check(view.State()==rows::ViewState::Ready&&engine.creates==2&&engine.attaches==2,"replayed menu built after old row removal");
        fixture.fixture.r.Set(F::Registry+24*2+16,999);const auto before=engine.calls;
        check(!view.Close()&&view.State()==rows::ViewState::Detached&&engine.calls==before,"lost owner is not falsely cleaned or touched");
        check(!view.Open(current,21,&spec,1),"detached cached view cannot be reopened by forgetting ownership");
    }
    for(unsigned fault=0;fault<5;++fault){
        CountFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};reflection::RetainedMenuText text;
        rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,text);active::Context current;
        rows::Spec count;count.id=RowId::Count;count.title=u"Counselors";count.count=9;count.selected=6;count.reuseNativeCount=true;
        std::array<std::u16string,9> labels;for(unsigned i=0;i<9;++i){labels[i].assign(1,static_cast<char16_t>(u'1'+i));count.labels[i]=labels[i];}
        check(active::Observe(reader,M::Base,current)&&view.Open(current,19,&count,1),"borrow original count without warm factories");
        if(fault==1)fixture.fixture.r.Set(CountFixture::Count+0x210,7);
        if(fault==2)fixture.fixture.r.Set(std::uintptr_t{0x1610000},char16_t{u'9'});
        check(view.Service(0)==(fault!=1&&fault!=2),"native count numeric list and selected bounds verified");
        if(fault==1||fault==2){check(engine.calls==0,"refused borrowed row never modifies UI");continue;}
        check(view.State()==rows::ViewState::Ready&&view.WarmStage()==0&&engine.calls==0,"borrowed count no widget/options/text native calls");
        const auto reads=fixture.fixture.r.reads;for(unsigned i=0;i<1000;++i)view.Service(25+i*25);
        check(reads==fixture.fixture.r.reads&&engine.calls==0,"borrowed count has no idle polling");
        fixture.fixture.r.Set(CountFixture::Count+0x210,4);
        if(fault==3)fixture.fixture.r.Set(M::Widget+0x300,F::NewWidget);
        if(fault==4)fixture.fixture.r.Set(CountFixture::CountSlot+0x188,F::NewWidget);
        std::array<unsigned,rows::Capacity> choices;
        check(view.Rows().Choices(view,19,choices)==(fault==0),"Start revalidates original count owner and slot");
        if(fault){check(engine.calls==0,"changed borrowed row never dispatched");continue;}
        check(choices[0]==4,"native count5 yields index4 for total5");
        check(view.Close()&&view.Service(30000)&&view.Service(30025)&&view.State()==rows::ViewState::Closed,"borrowed row releases registration only");
        std::uintptr_t slot=0;fixture.fixture.r.Read(CountFixture::Count+0x180,slot);
        check(slot==CountFixture::CountSlot&&engine.calls==0&&engine.removes==0,"native count remains attached and untouched after Back");
    }
    {
        NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};reflection::RetainedMenuText text;
        rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,text);active::Context current;
        auto bindingSpec=spec;bindingSpec.preserveNativeValue=true;
        check(active::Observe(reader,M::Base,current)&&view.Open(current,19,&bindingSpec,1),"base-style extra row opening");
        for(unsigned i=0;i<9;++i)check(view.Service(25*i),"base-style row warm and four phases");
        check(view.State()==rows::ViewState::Ready&&engine.texts==1,"extra row sets title only, preserving native value binding");
    }
    {
        CountFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};reflection::RetainedMenuText text;
        rows::NativeSetup<OptionsReader,ModelEngine> view(reader,engine,text);active::Context current;
        rows::Spec specs[2];specs[0].id=RowId::Count;specs[0].title=u"Counselors";specs[0].count=7;specs[0].reuseNativeCount=true;
        std::array<std::u16string,7> labels;for(unsigned i=0;i<7;++i){labels[i].assign(1,static_cast<char16_t>(u'1'+i));specs[0].labels[i]=labels[i];}
        specs[1].id=RowId::Weather;specs[1].title=u"Offline Bots - Play as a Counselor";
        specs[1].count=1;specs[1].labels[0]=u"AI Jason - Automatic";specs[1].informationOnly=true;specs[1].preserveNativeValue=true;
        check(active::Observe(reader,M::Base,current)&&view.Open(current,19,specs,2),"actual companion two-row native layout");
        for(unsigned i=0;i<10;++i)check(view.Service(25*i),"paced native count/status composition");
        check(view.State()==rows::ViewState::Ready&&engine.creates==1&&engine.attaches==1&&engine.texts==1,"only owned status row created/title-only native binding");
        std::array<unsigned,rows::Capacity> choices{};
        check(view.Rows().Choices(view,19,choices)&&choices[0]==6&&choices[1]==0,"native count preserved/status readback verified");
        check(view.Close()&&view.Service(30000)&&view.Service(30025)&&view.Service(30050)&&view.State()==rows::ViewState::Closed,"owned status removed while count registration retired");
        std::uintptr_t slot=0;fixture.fixture.r.Read(CountFixture::Count+0x180,slot);
        check(engine.removes==1&&slot==CountFixture::CountSlot,"stock count never removed/reparented");
    }
    {
        NativeFixture fixture;OptionsReader reader{fixture.fixture.r};ModelEngine engine{fixture};reflection::RetainedMenuText text;
        rows::NativeAccess<OptionsReader,ModelEngine> access(reader,engine,text);active::Context context;
        check(active::Observe(reader,M::Base,context)&&access.Prepare(context,19),"skill capture prepares existing native donor");
        struct Header {std::uintptr_t data;int count,capacity;};auto& r=fixture.fixture.r;
        r.Set(M::Donor+0x200,Header{0x1700000,3,3});r.Set(M::Donor+0x210,1);
        const std::array<std::u16string,3> labels{{u"Hard",u"Easy",u"Normal"}};
        for(unsigned i=0;i<3;++i){const auto at=std::uintptr_t{0x1710000}+i*0x100;
            const auto length=static_cast<int>(labels[i].size()+1);r.Set(0x1700000+i*16,Header{at,length,length});
            for(unsigned c=0;c<static_cast<unsigned>(length);++c)r.Set(at+2*c,c<labels[i].size()?labels[i][c]:char16_t{});}
        unsigned value=99;const auto calls=engine.calls;
        check(access.NativeSkill(value)&&value==0&&engine.calls==calls,"native Easy maps to semantic Low without writes/calls");
        r.Set(M::Donor+0x210,2);check(access.NativeSkill(value)&&value==1,"native Normal maps to semantic Medium");
        r.Set(M::Donor+0x210,3);check(!access.NativeSkill(value)&&engine.calls==calls,"invalid native skill refuses without modifying control");
    }
    std::printf("CFG production native menu adapter owned-engine tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif
