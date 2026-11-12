#pragma once
#include "../RuntimeProbe/Compatibility/CfgBuildProfile.hpp"
#include "NativeSetupRows.hpp"
#include "NativeSkillItems.hpp"
#include "../Core/MenuReentryPolicy.hpp"
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/MenuNativeEvidence.hpp"
#include "../RuntimeProbe/MenuOptionsObservation.hpp"
#include "../RuntimeProbe/MenuPanelChildrenObservation.hpp"
#include "SpawnBindings.hpp"
#include "NativeMenuRetention.hpp"
#include "../RuntimeProbe/SpawnBindings.generated.hpp"
#include "../RuntimeProbe/ChallengeMenuCalls.generated.hpp"
namespace cfgmenu::rows {
// Production engine-facing row adapter. Reader is a bounded current-process
// reader; Engine dispatches only on its already established native owner thread.
// No registry lookup, detour installation, DLL load or protection changes here.
template<class Reader,class Engine> class NativeAccess {
    Reader& r_;Engine& engine_;reflection::RetainedMenuText& texts_;
    active::Context context_{};std::uint64_t generation_=0;
    std::uintptr_t donor_=0,donorType_=0,panel_=0;
    capture::FunctionEvidence options_{};capture::OptionsEvidence copy_{};
    std::array<std::uintptr_t,Capacity> owned_{};unsigned ownedCount_=0;
    std::uintptr_t borrowedCount_=0;
    unsigned borrowedCountSelection_=0;
    std::uintptr_t borrowedSkill_=0;unsigned borrowedSkillSelection_=0;
    std::array<std::u16string,32> countLabels_{};unsigned countLabelCount_=0;
    struct MemberBinding {std::uintptr_t type=0,ownerHead=0;std::u16string name{};cfgreflection::Property property{};};
    std::array<MemberBinding,8> members_{};unsigned memberCount_=0;
    cfgreflection::Property panelSlots_{},panelSlotInner_{};
    std::uintptr_t panelType_=0,panelSlotsHead_=0;
    bool sealed_=false,challenges_=false;
    void* hostedProof_=nullptr;bool(*hostedCurrent_)(Reader&,void*)=nullptr;
    bool Type(std::uintptr_t object,std::uintptr_t expected){
        cfgprobe::Ancestry ancestry;
        return cfgprobe::CaptureAncestry(r_,object,ancestry)&&ancestry.Contains(expected)&&cfgprobe::StableAncestry(r_,object,ancestry);
    }
    bool Member(std::uintptr_t object,std::u16string_view name,std::uintptr_t& out){
        out=0;std::uintptr_t type=0;cfgreflection::Observation<Reader> reflection(r_,context_.module);
        if(!cfgprobe::ObjectPointer(object)||!r_.Read(object+cfgprobe::ObjectClass,type))return false;
        MemberBinding* binding=nullptr;
        for(unsigned i=0;i<memberCount_;++i)if(members_[i].type==type&&members_[i].name==name){binding=&members_[i];break;}
        if(!binding){
            if(memberCount_==members_.size())return false;
            cfgreflection::Field field;cfgreflection::Property property;std::uintptr_t head=0;
            if(!reflection.Find(type,name,field)||!reflection.ReadProperty(field,property)||
                field.kind.View()!=u"ObjectProperty"||property.dimension!=1||property.elementBytes!=8||property.offset%8||
                !r_.Read(field.owner+cfgreflection::disk::Children,head))return false;
            binding=&members_[memberCount_++];binding->type=type;binding->name.assign(name);binding->ownerHead=head;binding->property=property;
        }
        // Resolve metadata once for this view/type, not repeatedly traverse the
        // same hundreds of fields for each validation in a row phase. Recheck
        // exact schema, owner ancestry, head and links around every value read.
        cfgreflection::Property before,after;std::uintptr_t head=0,freshType=0,value=0,again=0;
        const auto& property=binding->property;
        if(!Type(object,property.field.owner)||!r_.Read(property.field.owner+cfgreflection::disk::Children,head)||head!=binding->ownerHead||
            !reflection.ReadProperty(property.field,before)||!capture::SameProperty(before,property)||
            !r_.Read(object+static_cast<unsigned>(property.offset),value)||(value&&!cfgprobe::ObjectPointer(value))||
            !r_.Read(object+cfgprobe::ObjectClass,freshType)||freshType!=type||
            !reflection.ReadProperty(property.field,after)||!capture::SameProperty(before,after)||
            !r_.Read(object+static_cast<unsigned>(property.offset),again)||again!=value)return false;
        out=value;return true;
    }
    bool PanelOwns(std::uintptr_t panel,std::uintptr_t slot){
        cfgreflection::Observation<Reader> reflection(r_,context_.module);
        std::uintptr_t type=0,head=0,inner=0;
        if(!r_.Read(panel+cfgprobe::ObjectClass,type))return false;
        if(!panelSlots_.field.object){
            cfgreflection::Field field,element;cfgreflection::Property slots,child;
            if(!reflection.Find(type,u"Slots",field)||field.kind.View()!=u"ArrayProperty"||
               !reflection.ReadProperty(field,slots)||slots.dimension!=1||slots.elementBytes!=16||slots.offset%8||
               !r_.Read(field.object+cfgreflection::arraydisk::Inner,inner)||
               !reflection.ReadField(inner,field.object,element)||element.kind.View()!=u"ObjectProperty"||
               !reflection.ReadProperty(element,child)||child.dimension!=1||child.elementBytes!=8||child.offset||
               !r_.Read(field.owner+cfgreflection::disk::Children,head))return false;
            panelSlots_=slots;panelSlotInner_=child;panelType_=type;panelSlotsHead_=head;
        }
        const auto schema=[&](){std::uintptr_t currentType=0,currentHead=0,currentInner=0;
            return r_.Read(panel+cfgprobe::ObjectClass,currentType)&&currentType==panelType_&&
                Type(panel,panelSlots_.field.owner)&&
                r_.Read(panelSlots_.field.owner+cfgreflection::disk::Children,currentHead)&&currentHead==panelSlotsHead_&&
                reflection.CurrentProperty(panelSlots_)&&reflection.CurrentProperty(panelSlotInner_)&&
                r_.Read(panelSlots_.field.object+cfgreflection::arraydisk::Inner,currentInner)&&currentInner==panelSlotInner_.field.object;};
        panelcapture::Array before{},after{};
        std::array<std::uintptr_t,panelcapture::ChildLimit> first{},second{};
        const auto at=panel+static_cast<unsigned>(panelSlots_.offset);
        if(type!=panelType_||!schema()||!r_.Read(at,before)||!before.Valid()||
           !r_.ReadBytes(before.data,first.data(),8*static_cast<unsigned>(before.count)))return false;
        unsigned matches=0;
        for(int i=0;i<before.count;++i){const auto value=first[static_cast<unsigned>(i)];
            if(!cfgprobe::ObjectPointer(value))return false;
            for(int j=0;j<i;++j)if(first[static_cast<unsigned>(j)]==value)return false;
            if(value==slot)++matches;}
        // Reverse Slot.Parent/Content alone can be stale after removal. Require
        // the real native panel's forward, reflected ObjectProperty array too.
        // Read Num only; no guessed layout/weak reference/standalone ownership.
        return matches==1&&r_.Read(at,after)&&after==before&&
            r_.ReadBytes(before.data,second.data(),8*static_cast<unsigned>(before.count))&&first==second&&
            schema()&&r_.Read(at,after)&&after==before;
    }
    bool Parent(std::uintptr_t widget,std::uintptr_t& panel){
        panel=0;std::uintptr_t slot=0,content=0;
        return Member(widget,u"Slot",slot)&&slot&&Member(slot,u"Parent",panel)&&panel&&
            Member(slot,u"Content",content)&&content==widget&&PanelOwns(panel,slot);
    }
    bool Owned(std::uintptr_t widget){
        bool found=false;for(unsigned i=0;i<ownedCount_;++i)found|=owned_[i]==widget;
        if(!found)return false;std::uintptr_t type=0,parent=0;
        if(widget==borrowedCount_){std::uintptr_t current=0;
            if(!Member(context_.widget,u"CounselorCountComboBox",current)||current!=widget)return false;}
        return r_.Read(widget+cfgprobe::ObjectClass,type)&&type==donorType_&&Parent(widget,parent)&&parent==panel_;
    }
    bool NativeCountOptions(std::uintptr_t widget,unsigned& count){
        count=0;cfgreflection::Observation<Reader> reflection(r_,context_.module);cfgreflection::Property fresh;
        const auto& property=copy_.destinationItems;
        if(!reflection.ReadProperty(property.field,fresh)||!capture::SameProperty(property,fresh))return false;
        struct Header {std::uintptr_t data;std::int32_t count,capacity;};Header before{},after{};
        const auto at=widget+static_cast<unsigned>(property.offset);
        if(!r_.Read(at,before)||!cfgprobe::ObjectPointer(before.data)||before.count<1||
           before.count>static_cast<int>(countLabelCount_)||before.capacity<before.count||before.capacity>32)return false;
        for(unsigned i=0;i<static_cast<unsigned>(before.count);++i){
            reflection::String str{},again{};std::array<char16_t,128> text{};
            const auto element=before.data+16*i;const auto& expected=countLabels_[i];
            if(!r_.Read(element,str)||!cfgprobe::ObjectPointer(reinterpret_cast<std::uintptr_t>(str.data))||
               str.count!=static_cast<int>(expected.size()+1)||str.capacity<str.count||str.capacity>128||
               !r_.ReadBytes(reinterpret_cast<std::uintptr_t>(str.data),text.data(),static_cast<unsigned>(str.count)*2)||
               text[static_cast<unsigned>(str.count)-1]||std::u16string_view(text.data(),expected.size())!=expected||
               !r_.Read(element,again)||str.data!=again.data||str.count!=again.count||str.capacity!=again.capacity)return false;
        }
        if(!r_.Read(at,after)||before.data!=after.data||before.count!=after.count||before.capacity!=after.capacity)return false;
        count=static_cast<unsigned>(before.count);return true;
    }
    template<std::size_t P,std::size_t C> bool Call(const execdisk::Binding& binding,
        const std::array<execdisk::Parameter,P>& parameters,const std::array<unsigned char,C>& code,
        std::uintptr_t target,void* packet){
        if(!Current(generation_))return false;
        capture::FunctionEvidence function;
        if(!native::CachedFunction(r_,context_.module,binding,parameters,code,function)||!Type(target,function.function.owner)||
            !Current(generation_))return false;
        return engine_.Invoke(target,function.function.object,packet);
    }
    template<std::size_t P,std::size_t C> bool StaticCall(const execdisk::Binding& binding,
        const std::array<execdisk::Parameter,P>& parameters,const std::array<unsigned char,C>& code,void* packet){
        if(!Current(generation_))return false;
        capture::FunctionEvidence function;std::uintptr_t target=0;
        operationStage=101;if(!native::CachedFunction(r_,context_.module,binding,parameters,code,function))return false;
        staticOwner=function.function.owner;operationStage=102;
        if(!r_.Read(function.function.owner+cfgspawn::ClassDefaultObject,target)||!cfgprobe::ObjectPointer(target))return false;
        staticDefault=target;operationStage=103;
        if(hostedCurrent_){
            cfgnames::Observation<Reader> names(r_,context_.module);cfgnames::Text label;std::uintptr_t type=0;
            staticOwnerName={};staticClassName={};
            const auto copy=[](auto& to,std::u16string_view from){for(std::size_t i=0;i<from.size()&&i+1<to.size();++i)to[i]=from[i]<128?static_cast<char>(from[i]):'?';};
            if(names.Object(function.function.owner,label))copy(staticOwnerName,label.View());
            if(r_.Read(target+cfgprobe::ObjectClass,type)&&names.Object(type,label))copy(staticClassName,label.View());
        }
        typeReason=0;cfgprobe::Ancestry ancestry;
        if(!cfgprobe::CaptureAncestry(r_,target,ancestry)){typeReason=1;return false;}
        if(!ancestry.Contains(function.function.owner)){typeReason=2;return false;}
        if(!cfgprobe::StableAncestry(r_,target,ancestry)){typeReason=3;return false;}
        operationStage=104;if(!Current(generation_))return false;
        operationStage=105;invokeReason=0;
        if(!engine_.Invoke(target,function.function.object,packet)){invokeReason=EngineFailure(engine_,0);return false;}
        return true;
    }
    template<class E>static auto EngineFailure(E& e,int)->decltype(static_cast<unsigned>(e.LastFailure())){return static_cast<unsigned>(e.LastFailure());}
    template<class E>static unsigned EngineFailure(E&,long){return ~0u;}
public:
    NativeAccess(Reader& reader,Engine& engine,reflection::RetainedMenuText& text):r_(reader),engine_(engine),texts_(text){}
    std::array<char,64> staticOwnerName{},staticClassName{};
    unsigned prepareStage=0,operationStage=0,currentReason=0,typeReason=0,invokeReason=0;std::uintptr_t staticOwner=0,staticDefault=0;
    unsigned OwnedCount()const{return ownedCount_;}
    bool Prepare(const active::Context& context,std::uint64_t generation,bool challenges=false){
        prepareStage=1;
        if(sealed_||!generation||context.thread!=engine_.Thread())return false;
        context_=context;generation_=generation;challenges_=challenges;
        cfgnames::Observation<Reader> names(r_,context_.module);cfgnames::Text name;
        prepareStage=2;if(!active::Current(r_,context_,engine_.Thread())||!names.Object(context_.type,name)||
            name.View()!=(challenges?u"SPChallengesSettingsMenuWidget_C":u"OfflineBotsSettingsMenuWidget_C"))return false;
        prepareStage=3;if(!Member(context_.widget,u"DifficultyComboBox",donor_)||!donor_)return false;
        prepareStage=4;if(!r_.Read(donor_+cfgprobe::ObjectClass,donorType_)||!names.Object(donorType_,name)||name.View()!=u"SettingsOptionComboBoxWidget_C")return false;
        prepareStage=5;if(!Parent(donor_,panel_))return false;
        prepareStage=6;if(!capture::Function(r_,context_.module,donorType_,u"SetOptions",options_))return false;
        prepareStage=7;if(!capture::OptionsDependencies(r_,context_.module,options_,copy_))return false;
        sealed_=true;prepareStage=8;if(!Current(generation))return false;
        prepareStage=0;return true;
    }
    // Separately admitted listen-host lobby adapter; never calls/weakens active::Current.
    bool PrepareHosted(const active::Context& context,std::uint64_t generation,std::uintptr_t donor,std::uintptr_t panel,
        void* proof,bool(*current)(Reader&,void*)){
        if(sealed_||!generation||!proof||!current||context.thread!=engine_.Thread()||!HostedLobbyRetentionScope::Current(context.module))return false;
        context_=context;generation_=generation;hostedProof_=proof;hostedCurrent_=current;donor_=donor;panel_=panel;
        cfgnames::Observation<Reader> names(r_,context.module);cfgnames::Text name;std::uintptr_t parent=0;
        if(!current(r_,proof)||!r_.Read(donor_+cfgprobe::ObjectClass,donorType_)||!names.Object(donorType_,name)||
            name.View()!=u"LobbyOptionComboBoxWidget_C"||!Parent(donor_,parent)||parent!=panel_||
            !capture::Function(r_,context.module,donorType_,u"SetOptions",options_)||
            !capture::OptionsDependencies(r_,context.module,options_,copy_))return false;
        sealed_=true;return Current(generation);
    }
    // H2.69: SetPosition on one proven canvas slot. Executor RVA and bytes come from the live H2.62 capture.
    bool HostedCanvasFunction(std::uintptr_t slot,std::uintptr_t& function){
        static constexpr std::array<unsigned char,24> code{0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x48,0x83,0x7A,0x20,0x00,0x48,0x8B,0xF9,0x48,0x8B,0xCA,0x48,0x8B,0xDA};
        function=0;std::uintptr_t type=0;auto f=std::make_unique<capture::FunctionEvidence>();std::array<unsigned char,24> live{};
        if(!hostedCurrent_||!Current(generation_)||!r_.Read(slot+cfgprobe::ObjectClass,type)||!capture::Function(r_,context_.module,type,u"SetPosition",*f)||
           f->nativeExecutor!=context_.module+0x1090490||f->parameterBytes!=8||f->storageBytes!=8||f->fieldsCount!=1||f->script.count||f->destructorLink||
           !r_.ReadBytes(context_.module+0x1090490,live.data(),live.size())||live!=code)return false;
        function=f->function.object;return true;
    }
    bool MoveHostedCanvasSlot(std::uintptr_t slot,std::uintptr_t content,std::uintptr_t function,float left,float top){
        std::uintptr_t actual=0;struct {float x,y;} packet{left,top};
        return hostedCurrent_&&Current(generation_)&&Member(slot,u"Content",actual)&&actual==content&&engine_.Invoke(slot,function,&packet)&&Current(generation_);
    }
    bool AppendHostedAfter(unsigned index,std::uintptr_t& widget,bool& attached){
        panelcapture::Array list{};
        // Both live lobby layouts have Team Killing last. Do not invent an insert
        // ABI if a future layout has another native row after the anchor.
        if(!hostedCurrent_||!Current(generation_)||!r_.Read(panel_+static_cast<unsigned>(panelSlots_.offset),list)||
            !list.Valid()||index!=static_cast<unsigned>(list.count))return false;
        return Append(widget,attached);
    }
    bool HideDifficulty(){
        if(!Current(generation_))return false;const auto& b=challengecalls::SetVisibility;
        std::uintptr_t owner=0,ctor=0,function=0;
        if(!r_.Read(context_.module+b.owner,owner)||!r_.Read(context_.module+b.ctor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor)||
           !Type(donor_,owner)||!r_.Read(context_.module+b.cache,function))return false;
        if(!function){std::array<unsigned char,challengecalls::SetVisibilityFactory.size()> code{};
            if(!r_.ReadBytes(context_.module+b.factory,code.data(),code.size())||code!=challengecalls::SetVisibilityFactory||
               !FactoryWithOwner(engine_,context_.module+b.factory,owner,function)||!Current(generation_))return false;}
        cfgreflection::Observation<Reader> reflection(r_,context_.module);cfgreflection::Field field;
        auto f=std::make_unique<capture::FunctionEvidence>();std::array<unsigned char,challengecalls::SetVisibilityCode.size()> code{};
        if(!reflection.ReadField(function,owner,field)||!capture::KnownFunction(r_,context_.module,field,b.name,*f)||
           f->nativeExecutor!=context_.module+b.executor||f->flags!=b.flags||f->parameterBytes!=1||f->storageBytes!=1||
           f->script.count||f->destructorLink||f->fieldsCount!=1||
           f->fields[0].field.label.View()!=u"InVisibility"||f->fields[0].field.kind.View()!=u"EnumProperty"||
           f->fields[0].offset||f->fields[0].elementBytes!=1||f->fields[0].dimension!=1||f->fields[0].flags!=0x0/*CFG3.5 address withheld*/40000280ULL||
           !r_.ReadBytes(context_.module+b.executor,code.data(),code.size())||code!=challengecalls::SetVisibilityCode||!Current(generation_))return false;
        std::uint8_t collapsed=1;return engine_.Invoke(donor_,function,&collapsed)&&Current(generation_);
    }
    template<std::size_t P,std::size_t C,std::size_t F> bool WarmBinding(const execdisk::Binding& binding,
        const std::array<execdisk::Parameter,P>& parameters,const std::array<unsigned char,C>& code,
        const std::array<unsigned char,F>& factoryCode){
        if(!Current(generation_))return false;
        std::uintptr_t function=0,owner=0,constructor=0;
        if(!r_.Read(context_.module+binding.reflection,owner)||!r_.Read(context_.module+binding.constructor,constructor)||
            !cfgprobe::MatchingClassCache(owner,constructor)||!r_.Read(context_.module+binding.cache,function))return false;
        if(!function){
            std::array<unsigned char,F> actual{};
            if(!r_.ReadBytes(context_.module+binding.factory,actual.data(),actual.size())||actual!=factoryCode||
                !Current(generation_)||!FactoryWithOwner(engine_,context_.module+binding.factory,owner,function))return false;
        }
        capture::FunctionEvidence evidence;
        return Current(generation_)&&native::CachedFunction(r_,context_.module,binding,parameters,code,evidence)&&evidence.function.object==function;
    }
    bool Rebind(const active::Context& current){
        if(!sealed_||!reentry::SameWidget(context_,current)||!active::Current(r_,current,engine_.Thread()))return false;
        const auto previous=context_;context_=current;
        if(Current(generation_))return true;
        context_=previous;return false;
    }
    bool Current(std::uint64_t generation){
        if(hostedCurrent_){std::uintptr_t type=0,parent=0;
            currentReason=1;if(!sealed_)return false;
            currentReason=2;if(generation!=generation_)return false;
            currentReason=3;if(!HostedLobbyRetentionScope::Current(context_.module))return false;
            currentReason=4;if(!hostedCurrent_(r_,hostedProof_))return false;
            currentReason=5;if(!r_.Read(donor_+cfgprobe::ObjectClass,type)||type!=donorType_)return false;
            currentReason=6;if(!Parent(donor_,parent)||parent!=panel_)return false;
            currentReason=0;return true;}
        if(!sealed_||generation!=generation_||!active::Current(r_,context_,engine_.Thread()))return false;
        std::uintptr_t donor=0,type=0,parent=0,frontend=0,constructor=0;
        return r_.Read(context_.module+execdisk::FrontendClass,frontend)&&
            r_.Read(context_.module+execdisk::FrontendConstructor,constructor)&&cfgprobe::MatchingClassCache(frontend,constructor)&&
            Type(context_.mode,frontend)&&Member(context_.widget,u"DifficultyComboBox",donor)&&donor==donor_&&
            r_.Read(donor_+cfgprobe::ObjectClass,type)&&type==donorType_&&Parent(donor_,parent)&&parent==panel_;
    }
    bool Append(std::uintptr_t& widget,bool& attached){
        widget=0;attached=false;operationStage=10;if(ownedCount_==owned_.size()||!Current(generation_))return false;
        reflection::CreateParameters create;
        if(!reflection::Create(hostedCurrent_?context_.controller:context_.widget,donorType_,context_.controller,create)||
            !StaticCall(execdisk::Create,execdisk::CreateParameters,execdisk::CreateCode,&create))return false;
        operationStage=12;const auto result=create.result;std::uintptr_t type=0,defaults=0;
        if(!cfgprobe::ObjectPointer(result)||result==donor_||!r_.Read(result+cfgprobe::ObjectClass,type)||type!=donorType_||
            !r_.Read(donorType_+cfgspawn::ClassDefaultObject,defaults)||result==defaults)return false;
        // A normally created widget cannot already be an owned child. Refuse
        // an ambiguous/repeated result instead of relabeling an earlier row.
        for(unsigned i=0;i<ownedCount_;++i)if(owned_[i]==result)return false;
        // Never yield with an unowned new widget. Appending establishes native
        // panel ownership even when widget/slot serials have not been assigned.
        owned_[ownedCount_++]=widget=result;operationStage=13;
        reflection::AddParameters add;if(!reflection::Add(result,add))return false;
        const bool returned=Call(execdisk::AddChildToVerticalBox,execdisk::AddChildToVerticalBoxParameters,execdisk::AddChildToVerticalBoxCode,panel_,&add);
        std::uintptr_t parent=0,slot=0;attached=Parent(result,parent)&&parent==panel_;
        const bool okay=returned&&cfgprobe::ObjectPointer(add.result)&&attached&&Member(result,u"Slot",slot)&&slot==add.result&&Owned(result);
        if(okay)operationStage=0;return okay;
    }
    bool BorrowNativeRow(std::uintptr_t& widget,bool& attached,const Spec& spec){
        if(spec.reuseChallengeDonor){
            widget=0;attached=false;
            if(!challenges_||!spec.Valid()||borrowedSkill_||ownedCount_==owned_.size()||!Current(generation_))return false;
            borrowedSkill_=donor_;borrowedSkillSelection_=0;owned_[ownedCount_++]=donor_;
            widget=donor_;attached=true;return true;
        }
        if(spec.reuseNativeSkill){
            widget=0;attached=false;unsigned selected=0;
            if(!spec.Valid()||borrowedSkill_||ownedCount_==owned_.size()||!Current(generation_)||
               spec.labels[0]!=u"Low"||spec.labels[1]!=u"Medium"||spec.labels[2]!=u"High"||
               !NativeSkill(selected)||!Current(generation_))return false;
            borrowedSkill_=donor_;borrowedSkillSelection_=selected;owned_[ownedCount_++]=donor_;
            widget=donor_;attached=true;return true;
        }
        widget=0;attached=false;std::uintptr_t candidate=0,type=0,parent=0;
        if(!spec.Valid()||!spec.reuseNativeCount||borrowedCount_||ownedCount_==owned_.size()||!Current(generation_)||
           !Member(context_.widget,u"CounselorCountComboBox",candidate)||!candidate||candidate==donor_||
           !r_.Read(candidate+cfgprobe::ObjectClass,type)||type!=donorType_||!Parent(candidate,parent)||parent!=panel_)return false;
        for(unsigned i=0;i<spec.count;++i)countLabels_[i].assign(spec.labels[i]);countLabelCount_=spec.count;
        unsigned count=0;if(!NativeCountOptions(candidate,count)||!Current(generation_))return false;
        borrowedCount_=candidate;owned_[ownedCount_++]=candidate;
        unsigned selected=0;if(!Index(candidate,selected)){
            owned_[--ownedCount_]=0;borrowedCount_=0;return false;
        }
        borrowedCountSelection_=selected;widget=candidate;attached=true;return true; // Never created/reparented; optional verified SetOptions expands the existing row.
    }
    bool CopyOptions(std::uintptr_t widget,const Spec& spec){
        operationStage=20;if(!spec.Valid()||!Current(generation_)||!Owned(widget))return false;
        capture::FunctionEvidence function;capture::OptionsEvidence copy;
        if(!capture::Function(r_,context_.module,donorType_,u"SetOptions",function)||
            !capture::OptionsDependencies(r_,context_.module,function,copy)||
            function.function.object!=options_.function.object||function.nativeExecutor!=context_.module+cfgcompat::cfg35::dispatch::BlueprintExecutorRva||
            !capture::SameProperty(copy.destinationItems,copy_.destinationItems)||
            !capture::SameProperty(copy.destinationSelected,copy_.destinationSelected))return false;
        reflection::OptionsSchema schema;schema.frameBytes=function.parameterBytes;
        schema.fields={reflection::Field{"Items",reflection::Kind::ArrayOfString,0,16,1,true},
            reflection::Field{"SelectedIndex",reflection::Kind::Int32,16,4,1,true}};
        reflection::OptionsPacket packet;
        const auto selectedIndex=spec.configureNativeCount&&widget==borrowedCount_?borrowedCountSelection_:
            spec.reuseNativeSkill&&widget==borrowedSkill_?borrowedSkillSelection_:spec.selected;
        if(!packet.Build(schema,spec.labels.data(),spec.count,selectedIndex)||!Current(generation_)||!Owned(widget)||
            !engine_.Invoke(widget,function.function.object,packet.Data())||!Current(generation_)||!Owned(widget))return false;
        reflection::Array borrowed{};std::memcpy(&borrowed,packet.Data(),sizeof(borrowed));
        struct Array {std::uintptr_t data;std::int32_t count,capacity;};Array array{},again{};
        const auto arrayAt=widget+static_cast<unsigned>(copy.destinationItems.offset);
        if(!r_.Read(arrayAt,array)||!cfgprobe::ObjectPointer(array.data)||array.data==reinterpret_cast<std::uintptr_t>(borrowed.data)||
            array.count!=static_cast<int>(spec.count)||array.capacity<array.count||array.capacity>32)return false;
        // Check engine-owned DEEP COPIES, not just counts/index. Never hand our
        // borrowed descriptors to a destructor/allocator or install them raw.
        for(unsigned i=0;i<spec.count;++i){reflection::String str;
            std::array<char16_t,128> text{};
            if(!r_.Read(array.data+16*i,str)||!cfgprobe::ObjectPointer(reinterpret_cast<std::uintptr_t>(str.data))||
                str.data==spec.labels[i].data()||str.data==borrowed.data[i].data||
                str.count!=static_cast<int>(spec.labels[i].size()+1)||str.capacity<str.count||str.capacity>128||
                !r_.ReadBytes(reinterpret_cast<std::uintptr_t>(str.data),text.data(),static_cast<unsigned>(str.count)*2)||
                text[static_cast<unsigned>(str.count)-1]||std::u16string_view(text.data(),spec.labels[i].size())!=spec.labels[i])return false;
        }
        unsigned selected=0;
        return r_.Read(arrayAt,again)&&array.data==again.data&&array.count==again.count&&array.capacity==again.capacity&&
            Index(widget,selected)&&selected==selectedIndex;
    }
    bool Text(std::u16string_view label,reflection::RetainedMenuText::Request& result){
        result={};if(!Current(generation_))return false;
        if(texts_.Find(label,result))return true;
        if(!texts_.Begin(label,result))return false;
        const bool returned=StaticCall(execdisk::Conv_StringToText,execdisk::Conv_StringToTextParameters,execdisk::Conv_StringToTextCode,result.packet);
        return texts_.Complete(result,returned)&&Current(generation_);
    }
    bool SetText(std::uintptr_t widget,std::u16string_view field,const reflection::RetainedMenuText::Request& text){
        if((field!=u"TitleText"&&field!=u"ValueText")||!Current(generation_)||!Owned(widget))return false;
        operationStage=30;std::uintptr_t target=0;const auto* packet=texts_.Borrow(text);
        return packet&&Member(widget,field,target)&&target&&Call(execdisk::SetText,execdisk::SetTextParameters,execdisk::SetTextCode,
            target,const_cast<reflection::OpaqueText*>(packet))&&Current(generation_)&&Owned(widget);
    }
    bool NativeSkill(unsigned& value){
        value=0;if(!Current(generation_))return false;
        cfgreflection::Observation<Reader> reflection(r_,context_.module);cfgreflection::Property items,selected;
        const auto stable=[&](){return reflection.ReadProperty(copy_.destinationItems.field,items)&&
            capture::SameProperty(copy_.destinationItems,items)&&reflection.ReadProperty(copy_.destinationSelected.field,selected)&&
            capture::SameProperty(copy_.destinationSelected,selected);};
        unsigned result=0;
        if(!stable()||!cfgofflinesettings::NativeSkillItems(r_,donor_+static_cast<unsigned>(items.offset),
            donor_+static_cast<unsigned>(selected.offset),result)||!stable()||!Current(generation_))return false;
        value=result;return true;
    }
    bool Index(std::uintptr_t widget,unsigned& value){
        value=0;if(!Current(generation_)||!Owned(widget))return false;
        std::int32_t selected=-1;const auto& p=copy_.destinationSelected;
        cfgreflection::Observation<Reader> reflection(r_,context_.module);cfgreflection::Property fresh;
        if(!reflection.ReadProperty(p.field,fresh)||!capture::SameProperty(p,fresh)||
            !r_.Read(widget+static_cast<unsigned>(p.offset),selected)||selected<0||!Owned(widget))return false;
        if(widget==borrowedCount_){unsigned count=0;
            std::int32_t again=-1;
            if(!NativeCountOptions(widget,count)||static_cast<unsigned>(selected)>=count||!Owned(widget)||
               !r_.Read(widget+static_cast<unsigned>(p.offset),again)||again!=selected)return false;}
        value=static_cast<unsigned>(selected);return true;
    }
    bool RemoveOwned(std::uintptr_t widget){
        if(!Current(generation_)||!Owned(widget))return false;
        if(widget==borrowedCount_||widget==borrowedSkill_){
            for(unsigned i=0;i<ownedCount_;++i)if(owned_[i]==widget){owned_[i]=0;if(widget==borrowedCount_)borrowedCount_=0;else borrowedSkill_=0;return true;}
            return false; // Borrowed native controls belong to CFG, not this transaction.
        }
        struct Packet {std::uintptr_t content;std::uint8_t result=0;std::array<unsigned char,7> pad{};} packet{widget};
        static_assert(sizeof(Packet)==16&&offsetof(Packet,result)==8);
        if(!Call(execdisk::RemoveChild,execdisk::RemoveChildParameters,execdisk::RemoveChildCode,panel_,&packet)||packet.result!=1)return false;
        std::uintptr_t slot=1;if(!Member(widget,u"Slot",slot)||slot)return false;
        for(unsigned i=0;i<ownedCount_;++i)if(owned_[i]==widget){owned_[i]=0;return true;}return false;
    }
};
}
