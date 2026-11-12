// Included inside the observer namespace. This is an independent listen-host
// UI provider; frontend/offline/Challenges admission remains unchanged.
#include "HostedLobbyOutputSlotObservation.hpp"
#include "../RuntimeProbe/MenuNativeEvidence.hpp"
#include "../RuntimeProbe/MenuOptionsObservation.hpp"
struct HostedLobbyRowReceipt {
    std::uintptr_t slot=0,row=0;
    cfgprobe::lifetime::SlotObservation slotIdentity{},rowIdentity{};
    std::uint32_t slotFlags=0,slotObjectFlags=0,rowFlags=0,rowObjectFlags=0;
};
struct HostedLobbyRowReceiptStore {std::array<HostedLobbyRowReceipt,3> rows{};};
struct HostedLobbyRowFunctionDiagnostic {unsigned clause=0,reads=0,cap=0;};
template<class Trace> bool FlushHostedLobbyRowFunctionDiagnostic(const HostedLobbyRowFunctionDiagnostic& failure,Trace&& trace){
    if(!failure.clause)return false;
    trace("hosted-row-function-refused",failure.clause,(static_cast<std::uint64_t>(failure.reads)<<32)|failure.cap);
    return true;
}
class HostedLobbyRootProvider {
    cfghostedlobby::Discovery& discovery_;cfghostedlobby::RowLayout layout_;
    std::uintptr_t actor_,function_,pc_;std::uint32_t thread_;
    cfgretain::CallbackRootLease<decltype(hostedLobbyRootBank)> lease_;
    unsigned maximum_=0,stage_=0,role_=0,inputAttempts_=0,inputFailures_=0;bool admitted_=false;
    cfghostedlobbystatic::Failure failure_{};
    std::array<cfghostedlobbystatic::Proof,4> staticProofs_{};unsigned staticCount_=0;
    bool stackRequired_=false,panelRequired_=false;
    std::uintptr_t panelFunction_=0;
    HostedLobbyRowReceiptStore localRows_{};HostedLobbyRowReceiptStore* rowStore_=&localRows_;
    struct RowFunctionProof {
        std::uintptr_t target=0,function=0,owner=0;
        unsigned kind=0;
        cfgprobe::lifetime::SlotObservation ownerSlot{};
        std::uint32_t ownerInternal=0,ownerFlags=0;
    };
    std::array<RowFunctionProof,3> rowFunctions_{};unsigned rowFunctionCount_=0;
    struct RowFunctionFailure {unsigned clause=0,entryReads=0,reads=0,cap=0,error=0;std::uintptr_t address=0,target=0,supplied=0;};
    RowFunctionFailure firstRowFunctionFailure_{};
    RowFunctionFailure lastRowFunctionFailure_{};
    void* rowFailureContext_=nullptr;void(*rowFailureSink_)(void*,unsigned,unsigned,unsigned)=nullptr;
    std::array<cfghostedlobby::OwnerEvidence,13> owners_{};
    bool StrongStack(Reader& r,std::uintptr_t value){
        const auto& c=discovery_.context;std::uintptr_t type=0,ctor=0,stack=0,outer=0;
        // EXACT typed ILL menu-stack default subobject, not arbitrary serial0 UI.
        // LayoutCurrent already reproves HUD -> current stack -> top menu -> anchor.
        return value==c.stack&&r.Read(c.module+cfgcompat::cfg35::menuStack::OwnerRva,type)&&type&&
            r.Read(c.module+cfgcompat::cfg35::menuStack::ConstructorRva,ctor)&&ctor==type&&
            cfgspawn::ObjectIs(r,value,type)&&r.Read(c.hud+cfghosted::lobbydisk::HudStack,stack)&&stack==value&&
            r.Read(value+0x20,outer)&&outer==c.hud;
    }
    bool StrongPanel(Reader& r,std::uintptr_t value){
        const auto& c=discovery_.context;std::uintptr_t type=0,panelType=0;
        cfgnames::Text name,kind;cfgnames::Observation<Reader> names(r,c.module);
        // #516 identifies this EXACT authored panel. LayoutCurrent reproves its
        // menu-owned widget tree and native indexed TeamKilling child every time.
        return value==layout_.panel&&r.Read(c.module+cfgpractice::disk::PanelWidgetReflection,panelType)&&panelType&&
            cfgspawn::ObjectIs(r,value,panelType)&&r.Read(value+cfgprobe::ObjectClass,type)&&
            names.Object(value,name)&&name.View()==u"CustomServerSettings"&&
            names.Object(type,kind)&&kind.View()==u"VerticalBox";
    }
    // Serial-zero admission is limited to the authored destination of the two
    // exact row mutation functions. The panel's identity and full current
    // menu/tree path are independently re-proved by Context/StrongPanel.
    template<std::size_t P,std::size_t C> bool StrongPanelCall(Reader& r,std::uintptr_t value,
        std::uintptr_t supplied,const cfgmenu::execdisk::Binding& binding,
        const std::array<cfgmenu::execdisk::Parameter,P>& parameters,
        const std::array<unsigned char,C>& code){
        if(value!=layout_.panel||!StrongPanel(r,value))return false;
        cfgmenu::capture::FunctionEvidence evidence;
        return cfgmenu::native::CachedFunction(r,discovery_.context.module,binding,parameters,code,evidence)&&
            evidence.function.object==supplied&&cfgspawn::ObjectIs(r,value,evidence.function.owner);
    }
    template<std::size_t P,std::size_t C> bool StrongPanelFunction(Reader& r,std::uintptr_t value,
        const cfgmenu::execdisk::Binding& binding,const std::array<cfgmenu::execdisk::Parameter,P>& parameters,
        const std::array<unsigned char,C>& code){
        if(!StrongPanel(r,layout_.panel))return false;
        cfgmenu::capture::FunctionEvidence evidence;
        return cfgmenu::native::CachedFunction(r,discovery_.context.module,binding,parameters,code,evidence)&&
            evidence.function.object==value&&cfgspawn::ObjectIs(r,layout_.panel,evidence.function.owner);
    }
    bool StrongPanelOutputSlot(Reader& r,std::uintptr_t value){
        std::uintptr_t content=0;const HostedLobbyRowReceipt* proof=nullptr;
        for(const auto& row:rowStore_->rows)if(row.slot==value&&row.row){content=row.row;proof=&row;break;}
        const bool known=proof&&proof->slotIdentity.object==value&&proof->rowIdentity.object==content;
        if(!value||!known||!StrongPanel(r,layout_.panel))return false;
        cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot{},row{};
        std::uint32_t slotFlags=0,rowFlags=0,slotObjectFlags=0,rowObjectFlags=0;
        return cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgprobe::lifetime::ReadSlot(r,registry,value,slot)&&slot==proof->slotIdentity&&
            cfgprobe::lifetime::ReadSlot(r,registry,content,row)&&row==proof->rowIdentity&&
            r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,slotFlags)&&slotFlags==proof->slotFlags&&!(slotFlags&0x30000000u)&&!(slotFlags&0x30u)&&
            r.Read(value+8,slotObjectFlags)&&slotObjectFlags==proof->slotObjectFlags&&!(slotObjectFlags&0x30u)&&
            r.Read(registry.data+24ULL*static_cast<unsigned>(row.index)+8,rowFlags)&&rowFlags==proof->rowFlags&&!(rowFlags&0x30000000u)&&!(rowFlags&0x30u)&&
            r.Read(content+8,rowObjectFlags)&&rowObjectFlags==proof->rowObjectFlags&&!(rowObjectFlags&0x30u)&&
            cfgspawn::ObjectIs(r,content,layout_.rowClass)&&
            cfghostedlobbyoutput::Current(r,discovery_.context.module,registry,layout_.panel,value,content);
    }
    bool AttachedTextTarget(Reader& r,std::uintptr_t target){
        if(!target||!layout_.rowClass)return false;
        cfgprobe::lifetime::ArrayHeader registry;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        for(const auto& row:rowStore_->rows){
            if(!row.row||!StrongPanelOutputSlot(r,row.slot))continue;
            cfgreflection::Observation<Reader> reflection(r,discovery_.context.module);
            std::uintptr_t title=0,value=0;
            if((reflection.ReadObjectMember(row.row,layout_.rowClass,u"TitleText",title)&&title==target)||
               (reflection.ReadObjectMember(row.row,layout_.rowClass,u"ValueText",value)&&value==target)){
                cfgprobe::lifetime::SlotObservation slot{};cfghostedlobbystatic::Failure why;
                return cfghostedlobbystatic::Slot(r,registry,target,false,slot,why);
            }
        }
        return false;
    }
    bool RowFunction(Reader& r,std::uintptr_t target,std::uintptr_t supplied,unsigned& kind,std::uintptr_t& owner){
        using namespace cfgmenu;using namespace cfgmenu::execdisk;kind=0;owner=0;
        const unsigned startReads=r.reads;
        const auto refuse=[&](unsigned clause,std::uintptr_t address=0){
            lastRowFunctionFailure_={clause,startReads,r.reads,r.cap,0,address,target,supplied};
            if(!firstRowFunctionFailure_.clause)firstRowFunctionFailure_={clause,startReads,r.reads,r.cap,0,address,target,supplied};
            return false;
        };
        capture::FunctionEvidence evidence;capture::OptionsEvidence options;
        std::uintptr_t textFunction=0;
        if(!r.Read(discovery_.context.module+SetText.cache,textFunction))return refuse(1,discovery_.context.module+SetText.cache);
        if(supplied==textFunction){
            if(!AttachedTextTarget(r,target))return refuse(2,target);
            if(!native::CachedFunction(r,discovery_.context.module,SetText,SetTextParameters,SetTextCode,evidence))return refuse(7,supplied);
            if(evidence.function.object!=supplied)return refuse(8,evidence.function.object);
            if(!cfgspawn::ObjectIs(r,target,evidence.function.owner))return refuse(9,target);
            kind=2;owner=evidence.function.owner;return true;
        }
        if(!capture::Function(r,discovery_.context.module,layout_.rowClass,u"SetOptions",evidence))return refuse(3,layout_.rowClass);
        if(evidence.function.object!=supplied)return refuse(4,evidence.function.object);
        if(!capture::OptionsDependencies(r,discovery_.context.module,evidence,options))return refuse(5,evidence.function.object);
        if(!cfgspawn::ObjectIs(r,target,evidence.function.owner))return refuse(6,target);
        kind=1;owner=evidence.function.owner;return true;
    }
    bool RowOwner(Reader& r,RowFunctionProof& proof){
        cfgprobe::lifetime::ArrayHeader registry{};cfghostedlobbystatic::Failure why;
        // The exact pinned native SetText owner is already established by
        // RowFunction. Register its UClass like the native static owners.
        // Blueprint SetOptions still requires a positive owner serial.
        if(!cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfghostedlobbystatic::Slot(r,registry,proof.owner,proof.kind!=2,proof.ownerSlot,why))return false;
        proof.ownerInternal=why.internal;proof.ownerFlags=why.flags;return true;
    }
    static bool SameRowOwner(const RowFunctionProof& a,const RowFunctionProof& b){
        return a.kind==b.kind&&a.owner==b.owner&&a.ownerSlot==b.ownerSlot&&
            a.ownerInternal==b.ownerInternal&&a.ownerFlags==b.ownerFlags;
    }
    bool RegisterRowFunction(Reader& r,std::uintptr_t target,std::uintptr_t supplied){
        RowFunctionProof fresh;fresh.target=target;fresh.function=supplied;
        if(!RowFunction(r,target,supplied,fresh.kind,fresh.owner)||!RowOwner(r,fresh))return false;
        for(unsigned i=0;i<rowFunctionCount_;++i)if(rowFunctions_[i].function==supplied&&rowFunctions_[i].target==target)
            return SameRowOwner(rowFunctions_[i],fresh);
        if(rowFunctionCount_==rowFunctions_.size())return false;
        rowFunctions_[rowFunctionCount_++]=fresh;return true;
    }
    bool Refuse(unsigned stage,unsigned reason,std::uintptr_t value=0){
        if(!failure_.reason){stage_=stage;failure_.reason=reason;failure_.value=value;}return false;
    }
    bool Refuse(unsigned stage,const cfghostedlobbystatic::Failure& why){
        if(!failure_.reason){stage_=stage;failure_=why;}return false;
    }
    bool RegisteredCurrent(Reader& r){
        lastRowFunctionFailure_={};
        for(unsigned i=0;i<staticCount_;++i){cfghostedlobbystatic::Failure why;
            if(!cfghostedlobbystatic::Current(r,discovery_.context.module,staticProofs_[i],why))return Refuse(5,why);}
        if(panelFunction_){using namespace cfgmenu::execdisk;
            if(!StrongPanelFunction(r,panelFunction_,AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode)&&
               !StrongPanelFunction(r,panelFunction_,RemoveChild,RemoveChildParameters,RemoveChildCode))return Refuse(5,39,panelFunction_);}
        for(const auto& row:rowStore_->rows)if(row.slot&&!StrongPanelOutputSlot(r,row.slot))return Refuse(5,40,row.slot);
        for(unsigned i=0;i<rowFunctionCount_;++i){const auto& proof=rowFunctions_[i];RowFunctionProof fresh;
            Reader rowProof;rowProof.cap=32768;
            if(!RowFunction(rowProof,proof.target,proof.function,fresh.kind,fresh.owner)||!RowOwner(rowProof,fresh)||
               !SameRowOwner(proof,fresh))return Refuse(5,46,proof.function);}
        return true;
    }
    std::uintptr_t chatSlot_=0,chatWidget_=0,chatPanel_=0,chatFunction_=0;
    bool ChatSlotStrong(Reader& r){
        cfgpracticeobs::WidgetNode::List list;std::uintptr_t content=0,tree=0,outer=0;bool found=false;
        if(!chatSlot_||!r.Read(chatSlot_+cfgpractice::disk::Content,content)||content!=chatWidget_||!r.Read(chatPanel_+cfgpractice::disk::Slots,list)||!list.Sane()||
           !r.Read(discovery_.context.menu+cfgpractice::disk::WidgetTree,tree)||!tree||!r.Read(chatPanel_+0x20,outer)||outer!=tree)return false;
        for(unsigned i=0;i<static_cast<unsigned>(list.count)&&!found;++i){std::uintptr_t entry=0;if(!r.Read(list.data+8ULL*i,entry))return false;found=entry==chatSlot_;}
        return found;
    }
    bool Registered(std::uintptr_t value)const{
        for(unsigned i=0;i<staticCount_;++i)if(staticProofs_[i].Contains(value))return true;return false;
    }
    static HostedLobbyRootProvider& Self(void* p){return *static_cast<HostedLobbyRootProvider*>(p);}
    bool Context(Reader& r){
        if(!admitted_)return Refuse(1,20);
        if(GetCurrentThreadId()!=thread_)return Refuse(1,21);
        if(!cfgprimary::LookupVetoClearForObservation(r,discovery_.context.module))return Refuse(1,22);
        if(!cfghostedlobby::LayoutCurrent(r,discovery_,layout_))return Refuse(1,23);
        return RegisteredCurrent(r)&&(!stackRequired_||StrongStack(r,discovery_.context.stack))&&
            (!panelRequired_||StrongPanel(r,layout_.panel));
    }
    bool Callback(Reader& r,std::uintptr_t function,std::uintptr_t pc){
        const auto& c=discovery_.context;
        return function==function_&&pc==pc_&&(actor_==c.controller||actor_==c.mode)&&
            cfgtick::Observe(r,c.module,actor_,function,pc);
    }
    GcRegistryAccess::NativeContextProof Proof(){return {this,
        [](Reader& r,void* p){return Self(p).Context(r);},
        [](Reader& r,void* p,std::uintptr_t f,std::uintptr_t pc){return Self(p).Callback(r,f,pc);}};}
    template<class Work>bool Window(Work&& work){
        Reader r;r.cap=32768;if(!lease_.Current())return Refuse(1,24);
        if(!Context(r))return false;
        GcRegistryAccess access(Proof(),131072);bool okay=false;
        __try{if(access.Begin(function_,pc_)){okay=work(access);if(!okay&&!failure_.reason)Refuse(3,26);}
              else Refuse(3,25);}
        __finally{access.End();}
        return okay;
    }
    template<unsigned N>bool Values(Reader& r,const std::array<std::uintptr_t,N>& values,unsigned role=1,
        std::uintptr_t callFunction=0){
        cfgprobe::lifetime::ArrayHeader registry;role_=role;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry))return Refuse(4,27);
        bool cachedPanelFunction=false;if(role==2&&callFunction){using namespace cfgmenu::execdisk;
            std::uintptr_t add=0,remove=0;
            cachedPanelFunction=(r.Read(discovery_.context.module+AddChildToVerticalBox.cache,add)&&callFunction==add)||
                (r.Read(discovery_.context.module+RemoveChild.cache,remove)&&callFunction==remove);}
        for(auto value:values)if(value){cfgprobe::lifetime::SlotObservation slot;cfghostedlobbystatic::Failure why;
            bool defaultObject=false,staticFunction=false;for(unsigned i=0;i<staticCount_;++i){
                defaultObject=defaultObject||staticProofs_[i].target==value;
                staticFunction=staticFunction||(role==2&&callFunction&&staticProofs_[i].function==callFunction);}
            const bool stack=role==1&&value==discovery_.context.stack&&StrongStack(r,value);
            const bool panel=role==1&&value==layout_.panel&&StrongPanel(r,value);
            bool panelTarget=false;
            if(role==2&&value==layout_.panel){using namespace cfgmenu::execdisk;
                panelTarget=StrongPanelCall(r,value,callFunction,AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode)||
                    StrongPanelCall(r,value,callFunction,RemoveChild,RemoveChildParameters,RemoveChildCode);}
            bool panelFunction=false;
            if(role==2&&callFunction&&(value==callFunction)){using namespace cfgmenu::execdisk;
                panelFunction=StrongPanelFunction(r,value,AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode)||
                    StrongPanelFunction(r,value,RemoveChild,RemoveChildParameters,RemoveChildCode);}
            const bool rowFunction=role==2&&callFunction&&!staticFunction&&!panelFunction&&!cachedPanelFunction&&
                ((value==values[0]&&RegisterRowFunction(r,value,callFunction))||
                 (value==callFunction&&RegisterRowFunction(r,values[0],value)));
            const bool panelOutput=role==3&&callFunction==panelFunction_&&StrongPanelOutputSlot(r,value);
            bool panelRow=false;if(role==3)for(const auto& row:rowStore_->rows)if(value==row.row&&row.slot&&StrongPanelOutputSlot(r,row.slot))panelRow=true;
            if(panelFunction){panelFunction_=value;panelRequired_=true;}
            const bool chat=role==2&&chatSlot_&&((value==chatSlot_&&callFunction==chatFunction_&&ChatSlotStrong(r))||(value==chatFunction_&&callFunction==chatFunction_));
            const bool strong=stack||panel||panelTarget||panelFunction||rowFunction||panelOutput||panelRow||chat;
            if(stack)stackRequired_=true;if(panel||panelTarget)panelRequired_=true;
            if(!cfghostedlobbystatic::Slot(r,registry,value,!Registered(value)&&!strong,slot,why,defaultObject))return Refuse(4,why);}
        return true;
    }
    bool StaticInputs(Reader& r,std::uintptr_t actor,std::uintptr_t function){
        std::uintptr_t create=0,text=0;using namespace cfgmenu::execdisk;
        if(!r.Read(discovery_.context.module+Create.cache,create)||!r.Read(discovery_.context.module+Conv_StringToText.cache,text))return Refuse(5,28,function);
        const unsigned kind=function==create?1u:function==text?2u:0u;if(!kind)return true;
        for(unsigned i=0;i<staticCount_;++i)if(staticProofs_[i].target==actor&&staticProofs_[i].function==function)return RegisteredCurrent(r);
        if(staticCount_==staticProofs_.size())return Refuse(5,29,function);
        cfgprobe::lifetime::ArrayHeader registry;cfghostedlobbystatic::Proof p;cfghostedlobbystatic::Failure why;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry))return Refuse(5,27);
        if(!cfghostedlobbystatic::Observe(r,discovery_.context.module,registry,kind,actor,function,p,why))return Refuse(5,why);
        staticProofs_[staticCount_++]=p;return true;
    }
    bool InputsBody(std::uintptr_t actor,std::uintptr_t function,void* packet){
        if(!packet||!actor||!function)return Refuse(2,30,!actor?actor:function);
        Reader r;r.cap=32768;cfgretainframe::RuntimeHost<Reader> host(r,discovery_.context.module);
        // Keep the attached output's receipt through every label/value
        // window. Before removing THIS row, release its claims while the
        // strong native panel graph is still intact; never bless a
        // detached serial-zero slot after RemoveChild.
        if(StrongPanelCall(r,actor,function,cfgmenu::execdisk::RemoveChild,
            cfgmenu::execdisk::RemoveChildParameters,cfgmenu::execdisk::RemoveChildCode)){
                std::uintptr_t child=0;unsigned rowIndex=static_cast<unsigned>(rowStore_->rows.size());
                if(!r.Read(reinterpret_cast<std::uintptr_t>(packet),child))return Refuse(6,44);
                for(unsigned i=0;i<rowStore_->rows.size();++i)if(rowStore_->rows[i].row==child&&rowStore_->rows[i].slot&&StrongPanelOutputSlot(r,rowStore_->rows[i].slot)){rowIndex=i;break;}
                if(rowIndex==rowStore_->rows.size()||
                   lease_.NativePending()||!Window([&](GcRegistryAccess& a){return lease_.Close(a);})||!lease_.EmptyIdle())return Refuse(6,44);
                rowFunctions_={};rowFunctionCount_=0;
                using Lease=cfgretain::CallbackRootLease<decltype(hostedLobbyRootBank)>;
                lease_.~Lease();::new(static_cast<void*>(&lease_)) Lease(hostedLobbyRootBank,discovery_.context.module);
                rowStore_->rows[rowIndex]={};
        }
        const auto& c=discovery_.context;
        // Retain the positive callback actor, not its borrowed ReceiveTick
        // UFunction. That callback witness is independently reproved at Begin.
        const std::array<std::uintptr_t,11> owners{c.world,c.mode,c.gameState,c.controller,c.hud,c.stack,c.menu,layout_.panel,layout_.anchor,layout_.rowClass,actor_};
        const std::array<std::uintptr_t,2> targets{actor,function};std::array<std::uintptr_t,32> values{};
        if(!StaticInputs(r,actor,function))return false;
        if(!Window([&](GcRegistryAccess& a){
            if(!RegisteredCurrent(r)||!Values(r,owners,1)||!Values(r,targets,2,function))return false;
            if(!lease_.Retain(a,owners)||!lease_.Retain(a,targets))return Refuse(6,31);
            for(const auto& row:rowStore_->rows)if(row.slot){
                const std::array<std::uintptr_t,2> proof{row.slot,row.row};
                if(!Values(r,proof,3,panelFunction_)||!lease_.Retain(a,proof))return Refuse(6,48,row.slot);
            }
            // Register the static UClass too; its CDO/function ownership and
            // slots/flags/code are reproved in EVERY subsequent Window.
            for(unsigned i=0;i<staticCount_;++i){const std::array<std::uintptr_t,1> owner{staticProofs_[i].owner};
                if(!Values(r,owner,2)||!lease_.Retain(a,owner))return Refuse(6,32,owner[0]);}
            for(unsigned i=0;i<rowFunctionCount_;++i){const auto& proof=rowFunctions_[i];auto fresh=proof;
                const std::array<std::uintptr_t,1> owner{proof.owner};
                if(!RowOwner(r,fresh)||!SameRowOwner(proof,fresh)||!lease_.Retain(a,owner))return Refuse(6,47,owner[0]);}
            return true;}))return false;
        if(!cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(packet),values))return Refuse(7,33,function);
        if(!cfgretainframe::MaximumOutputRoots(host,function,maximum_))return Refuse(8,34,function);
        if(!Window([&](GcRegistryAccess& a){
            if(!Values(r,values,3))return false;
            if(!lease_.Retain(a,values))return Refuse(9,35);
            if(!lease_.PrepareCall(a,maximum_))return Refuse(9,36);return true;}))return false;
        if(!failure_.reason)stage_=0;return true;
    }
    bool Inputs(std::uintptr_t actor,std::uintptr_t function,void* packet){
        ++inputAttempts_;const bool okay=InputsBody(actor,function,packet);
        if(!okay){++inputFailures_;if(!failure_.reason)Refuse(10,38,function);
            // One bounded read-only snapshot of ALL owners/targets, after the
            // registry window closed. Names never confer call/retention rights.
            Reader r;r.cap=16384;const auto& c=discovery_.context;
            const std::array<std::uintptr_t,13> values{c.world,c.mode,c.gameState,c.controller,c.hud,c.stack,c.menu,layout_.panel,layout_.anchor,layout_.rowClass,actor_,actor,function};
            cfgprobe::lifetime::ArrayHeader registry;cfgnames::Observation<Reader> names(r,c.module);
            if(cfgprobe::lifetime::ReadArrayHeader(r,c.module+cfgprobe::lifetime::ObjectArrayRva,registry)){
                for(unsigned i=0;i<values.size();++i){auto& d=owners_[i];d.value=values[i];d.role=i<11?1u:2u;d.index=i<11?i:i-11;
                    cfghostedlobbystatic::Failure why;cfgprobe::lifetime::SlotObservation slot;std::uintptr_t type=0;cfgnames::Text text;
                    (void)cfghostedlobbystatic::Slot(r,registry,values[i],false,slot,why);d.serial=why.slot.serial;d.internal=why.internal;d.flags=why.flags;
                    if(names.Object(values[i],text))cfghostedlobby::Copy(d.name,text.View());
                    if(r.Read(values[i]+cfgprobe::ObjectClass,type)&&names.Object(type,text))cfghostedlobby::Copy(d.className,text.View());
                }
            }
        }return okay;
    }
    bool Outputs(std::uintptr_t function,void* packet){
        Reader r;r.cap=16384;cfgretainframe::RuntimeHost<Reader> host(r,discovery_.context.module);std::array<std::uintptr_t,32> values{};
        if(!packet||!cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(packet),values,cfgretainframe::Phase::Outputs))return false;
        if(StrongPanelFunction(r,function,cfgmenu::execdisk::AddChildToVerticalBox,
            cfgmenu::execdisk::AddChildToVerticalBoxParameters,cfgmenu::execdisk::AddChildToVerticalBoxCode)){
            const auto& add=*static_cast<cfgmenu::reflection::AddParameters*>(packet);
            if(!add.content||!add.result)return Refuse(7,42,function);
            unsigned rowIndex=static_cast<unsigned>(rowStore_->rows.size());
            for(unsigned i=0;i<rowStore_->rows.size();++i)if(!rowStore_->rows[i].slot){rowIndex=i;break;}
            if(rowIndex==rowStore_->rows.size())return Refuse(7,43,add.result);
            cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slotIdentity{},rowIdentity{};
            std::uint32_t slotFlags=0,slotObjectFlags=0,rowFlags=0,rowObjectFlags=0;
            if(!cfgprobe::lifetime::ReadArrayHeader(r,discovery_.context.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
               !cfgprobe::lifetime::ReadSlot(r,registry,add.result,slotIdentity)||
               !cfgprobe::lifetime::ReadSlot(r,registry,add.content,rowIdentity)||!rowIdentity.HasAssignedSerial()||
               !r.Read(registry.data+24ULL*static_cast<unsigned>(slotIdentity.index)+8,slotFlags)||
               !r.Read(add.result+8,slotObjectFlags)||!r.Read(registry.data+24ULL*static_cast<unsigned>(rowIdentity.index)+8,rowFlags)||
               !r.Read(add.content+8,rowObjectFlags)||
               !cfgspawn::ObjectIs(r,add.content,layout_.rowClass))return Refuse(7,43,add.result);
            rowStore_->rows[rowIndex]={add.result,add.content,slotIdentity,rowIdentity,slotFlags,slotObjectFlags,rowFlags,rowObjectFlags};
            if(!StrongPanelOutputSlot(r,add.result)){rowStore_->rows[rowIndex]={};return Refuse(7,43,add.result);}
        }
        const bool okay=Window([&](GcRegistryAccess& a){return Values(r,values,3,function)&&lease_.CompleteCall(a,values,maximum_);});
        // CompleteCall retains its output claim until Close. The receipt must
        // outlive it too; only pre-Remove verified Close clears this receipt.
        return okay;
    }
    bool FactoryInput(std::uintptr_t owner){
        Reader r;r.cap=16384;const std::array<std::uintptr_t,3> values{discovery_.context.world,discovery_.context.menu,owner};
        return Window([&](GcRegistryAccess& a){return Values(r,values)&&lease_.Retain(a,values)&&lease_.PrepareCall(a,1);});
    }
    bool FactoryOutput(std::uintptr_t object){
        Reader r;r.cap=16384;const std::array<std::uintptr_t,1> values{object};
        return Window([&](GcRegistryAccess& a){return Values(r,values)&&lease_.CompleteCall(a,values,1);});
    }
    void Cancel(){if(!lease_.EmptyIdle()&&!Window([&](GcRegistryAccess& a){return lease_.CancelCall(a);}))lease_.Uncertain();}
public:
    RowFunctionFailure FirstRowFunctionFailure()const{return firstRowFunctionFailure_;}
    RowFunctionFailure LastRowFunctionFailure()const{return lastRowFunctionFailure_;}
    void SetRowFunctionFailureSink(void* context,void(*sink)(void*,unsigned,unsigned,unsigned)){rowFailureContext_=context;rowFailureSink_=sink;}
    HostedLobbyRootProvider(cfghostedlobby::Discovery& discovery,const cfghostedlobby::RowLayout& layout,
        std::uintptr_t actor,std::uintptr_t function,std::uintptr_t pc,HostedLobbyRowReceiptStore* rows=nullptr):discovery_(discovery),layout_(layout),
        actor_(actor),function_(function),pc_(pc),thread_(discovery.context.thread),lease_(hostedLobbyRootBank,discovery.context.module){
        if(rows)rowStore_=rows;
        Reader r;r.cap=16384;
        if(cfgmenu::rows::NativeRetentionScope::Occupied()||cfgmenu::rows::HostedLobbyRetentionScope::Occupied()||GetCurrentThreadId()!=thread_||!Callback(r,function,pc))return;
        for(const auto& receipt:rowStore_->rows)if(receipt.slot){
            std::uintptr_t current=0;using namespace cfgmenu::execdisk;
            if(r.Read(discovery_.context.module+AddChildToVerticalBox.cache,current)&&
               StrongPanelFunction(r,current,AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode))panelFunction_=current;
            break;
        }
        admitted_=true;if(!Context(r)||!lease_.Current())admitted_=false;
    }
    ~HostedLobbyRootProvider(){
        if(admitted_&&!lease_.EmptyIdle()&&!Window([&](GcRegistryAccess& a){return lease_.Close(a);}))lease_.Uncertain();
        if(rowFailureSink_&&firstRowFunctionFailure_.clause)
            rowFailureSink_(rowFailureContext_,firstRowFunctionFailure_.clause,firstRowFunctionFailure_.reads,firstRowFunctionFailure_.cap);
    }
    unsigned InputAttempts()const{return inputAttempts_;}
    unsigned InputFailures()const{return inputFailures_;}
    const auto& Owners()const{return owners_;}
    unsigned Stage()const{return stage_;}
    unsigned Role()const{return role_;}
    const cfghostedlobbystatic::Failure& Failure()const{return failure_;}
    bool Admitted()const{return admitted_;}
    void AllowChatSlot(std::uintptr_t slot,std::uintptr_t widget,std::uintptr_t panel,std::uintptr_t function){chatSlot_=slot;chatWidget_=widget;chatPanel_=panel;chatFunction_=function;}
    cfgmenu::rows::NativeRetentionHooks Hooks(){return {this,
        [](void* p,std::uintptr_t module){auto& s=Self(p);Reader r;r.cap=16384;if(module!=s.discovery_.context.module)return s.Refuse(1,37,module);
            if(!s.lease_.Current())return s.Refuse(1,24);return s.Context(r);},
        [](void* p,std::uintptr_t,std::uintptr_t object,std::uintptr_t function,void* packet){return Self(p).Inputs(object,function,packet);},
        [](void* p,std::uintptr_t,std::uintptr_t function,void* packet){return Self(p).Outputs(function,packet);},
        [](void* p,std::uintptr_t,std::uintptr_t,std::uintptr_t owner){return Self(p).FactoryInput(owner);},
        [](void* p,std::uintptr_t,std::uintptr_t object){return Self(p).FactoryOutput(object);},
        [](void* p){Self(p).lease_.Uncertain();},[](void* p){Self(p).Cancel();},
        [](void* p){auto& s=Self(p);Reader r;r.cap=16384;GcRegistryAccess unlocked(s.Proof());return s.Context(r)&&s.lease_.EnterNative(unlocked);},nullptr};}
};
