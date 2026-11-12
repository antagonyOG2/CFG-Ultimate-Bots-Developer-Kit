#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "NativeEmoteMirror.hpp"
using namespace cfgemotemirror;
static Slot Identity(std::uintptr_t object){Slot s{};s.object=object;s.serial=1;return s;}
struct Access {
    Inputs input;Slot human=Identity(0x100000);Catalogue catalogue{};
    std::map<std::uintptr_t,std::uintptr_t> montage;
    bool guard=true,source=true,assets=true,functions=true,dispatch=true,recipient=true,updateMontage=true;
    unsigned calls=0,cancelCalls=0,binds=0;Runtime* runtime=nullptr;
    Access(){input.world=0x300000;input.count=Queue::Capacity;
        catalogue.source=human;catalogue.selected=0x400000;catalogue.type=Identity(0x400000);
        catalogue.defaults=Identity(0x410000);catalogue.male=Identity(0x420000);catalogue.female=Identity(0x430000);
        for(unsigned i=0;i<input.count;++i)input.bots[i]={Identity(0x500000+0x10000*i),Identity(0x600000+0x10000*i)};
    }
    const Inputs& Input()const{return input;}
    bool Guard()const{return guard;}
    bool Source(Slot& s){s=human;return guard&&source;}
    bool Bind(unsigned id,Function& f,bool){++binds;f.object=0x800000+id*0x10000;return functions;}
    bool FunctionCurrent(unsigned,const Function&)const{return functions;}
    bool Montage(std::uintptr_t p,std::uintptr_t& out){out=montage[p];return true;}
    bool Parameter(std::uintptr_t p,std::uintptr_t& out){out=p;return p!=0;}
    bool CatalogueFor(const Slot&,std::uintptr_t selected,Catalogue& c){c=catalogue;return assets&&selected==catalogue.selected;}
    bool RecipientCurrent(const Recipient&)const{return recipient;}
    bool Dispatch(const Recipient& r,const Function& f,std::uintptr_t selected){
        ++calls;if(runtime){CHECK(!runtime->Candidate(r.pawn.object,f.object,false));runtime->Before(*this,r.pawn.object,f.object,selected,false);}
        if(dispatch&&updateMontage)montage[r.pawn.object]=(calls%2)?catalogue.male.object:catalogue.female.object;return dispatch;
    }
    bool DispatchCancel(const Recipient& r,const Function& f){
        ++cancelCalls;if(runtime){CHECK(!runtime->Candidate(r.pawn.object,f.object,false));runtime->Before(*this,r.pawn.object,f.object,0,false);}
        if(dispatch)montage[r.pawn.object]=0;return dispatch;
    }
};
template<class A>static void Prepare(Runtime& runtime,A& access){CHECK(!runtime.Prepare(access));CHECK(runtime.Prepare(access));access.runtime=&runtime;}
template<class A>static void Start(Runtime& runtime,A& access,std::uint64_t now=0){
    access.montage[access.human.object]=0;
    runtime.Before(access,access.human.object,0x800000,access.catalogue.selected,false);
    CHECK(!runtime.Pending());access.montage[access.human.object]=access.catalogue.male.object;
    runtime.After(access,access.human.object,0x800000,false,true,now);
}
template<class A>static void Cancel(Runtime& runtime,A& access,std::uint64_t now){
    runtime.Before(access,access.human.object,0x810000,0,false);access.montage[access.human.object]=0;
    runtime.After(access,access.human.object,0x810000,false,true,now);
}
struct Host {
    struct {cfgprobe::lifetime::ArrayHeader objectArray{};std::uintptr_t world=0;}context;
    unsigned calls=0;std::uintptr_t receiver=0,function=0,selected=0;bool actor=false,nullPacket=false;
    bool Context(){return true;}
    bool Invoke(std::uintptr_t object,std::uintptr_t fn,void* packet,bool isActor){
        ++calls;receiver=object;function=fn;actor=isActor;nullPacket=packet==nullptr;
        selected=packet?*static_cast<std::uintptr_t*>(packet):0;return true;
    }
};
static Slot Assigned(Fixture& fixture,std::uintptr_t object){int index=0;CHECK(fixture.Read(object+0xC,index));
    fixture.Put(fixture.objects.data+24*static_cast<unsigned>(index)+16,std::int32_t{7});
    Slot result;CHECK(cfgspawn::ObjectSlot(fixture,fixture.objects,object,result));return result;
}
static void NativeCatalogue(){
    Fixture fixture;Host host;host.context.objectArray=fixture.objects;
    Inputs input;input.module=fixture.module;input.counselorClass=fixture.counselorBase;
    input.counselorAIClass=fixture.classes[cfgspawn::CounselorAI];input.controller=0x900000;
    const auto emoteType=fixture.Object(),montageType=fixture.Object(),type=fixture.Object();
    fixture.Put(type+0x30,emoteType);
    const auto defaults=fixture.Object(type),male=fixture.Object(montageType),female=fixture.Object(montageType);
    fixture.Put(type+cfgspawn::ClassDefaultObject,defaults);
    fixture.Put(defaults+disk::MaleMontage,male);fixture.Put(defaults+disk::FemaleMontage,female);
    fixture.Put(fixture.module+disk::SCEmoteDataReflection,emoteType);fixture.Put(fixture.module+disk::SCEmoteDataConstructor,emoteType);
    fixture.Put(fixture.module+disk::AnimMontageReflection,montageType);fixture.Put(fixture.module+disk::AnimMontageConstructor,montageType);
    fixture.Put(fixture.bot+disk::DefaultEmotes,cfgspawn::PlayerArray{0x910000,1,1});fixture.Put(0x910000,type);
    const auto source=Assigned(fixture,fixture.bot);
    input.controller=fixture.Object();input.controllerSlot=Assigned(fixture,input.controller);
    NativeAccess<Fixture,Host> access(fixture,host,input);Catalogue result;
    CHECK(access.CatalogueFor(source,type,result));CHECK(result.Contains(male)&&result.Contains(female));CHECK(result.type.serial==0);
    const auto original=result;Catalogue identity;
    CHECK(access.IdentityFor(original,identity)&&access.proofStage==0);
    Assigned(fixture,male);Assigned(fixture,female);
    CHECK(access.IdentityFor(original,identity)); // recipient-gender montage serials do not change selected class identity
    const auto replacement=fixture.Object(type);fixture.Put(type+cfgspawn::ClassDefaultObject,replacement);
    CHECK(!access.IdentityFor(original,identity)&&access.proofDifference==2);
    fixture.Put(type+cfgspawn::ClassDefaultObject,defaults);
    // Equipped/customized native selections need not be in pawn defaults.
    fixture.reads=0;fixture.Put(0x910000,std::uintptr_t{0});CHECK(access.CatalogueFor(source,type,result));
    fixture.Put(fixture.bot+disk::DefaultEmotes,cfgspawn::PlayerArray{});CHECK(access.CatalogueFor(source,type,result));
    CHECK(!access.CatalogueFor(source,0,result));CHECK(!access.CatalogueFor(source,male,result));
    auto staleSource=source;++staleSource.serial;CHECK(!access.CatalogueFor(staleSource,type,result));
    fixture.Put(type+0x30,montageType);CHECK(!access.CatalogueFor(source,type,result));fixture.Put(type+0x30,emoteType);
    fixture.Put(defaults+cfgprobe::ObjectClass,emoteType);CHECK(!access.CatalogueFor(source,type,result));fixture.Put(defaults+cfgprobe::ObjectClass,type);
    fixture.Put(male+cfgprobe::ObjectClass,emoteType);CHECK(!access.CatalogueFor(source,type,result));fixture.Put(male+cfgprobe::ObjectClass,montageType);
    fixture.Put(female+cfgprobe::ObjectClass,emoteType);CHECK(!access.CatalogueFor(source,type,result));fixture.Put(female+cfgprobe::ObjectClass,montageType);
    fixture.reads=0;
    fixture.trigger=type+cfgspawn::ClassDefaultObject;fixture.mutation=fixture.trigger;fixture.newPointer=0;
    CHECK(!access.CatalogueFor(source,type,result));fixture.trigger=0;fixture.Put(type+cfgspawn::ClassDefaultObject,defaults);
    fixture.trigger=defaults+disk::FemaleMontage;fixture.mutation=fixture.trigger;fixture.triggerCount=0;fixture.newPointer=male;
    CHECK(!access.CatalogueFor(source,type,result));fixture.trigger=0;fixture.Put(defaults+disk::FemaleMontage,female);
    fixture.trigger=male+cfgprobe::ObjectClass;fixture.mutation=fixture.trigger;fixture.triggerCount=0;fixture.newPointer=emoteType;
    CHECK(!access.CatalogueFor(source,type,result));fixture.trigger=0;fixture.Put(male+cfgprobe::ObjectClass,montageType);
    fixture.reads=0;CHECK(access.CatalogueFor(source,type,result));
    // Exercise the actual native catalogue with an absent default list, then
    // confirm the human must start a valid male or female native montage.
    struct NativeSelectionAccess:Access {
        NativeAccess<Fixture,Host>& native;
        explicit NativeSelectionAccess(NativeAccess<Fixture,Host>& value):native(value){}
        bool CatalogueFor(const Slot& s,std::uintptr_t selected,Catalogue& c){return native.CatalogueFor(s,selected,c);}
    } selectedAccess(access);
    selectedAccess.human=source;selectedAccess.catalogue=result;
    for(const auto humanMontage:{male,female}){
        selectedAccess.montage.clear();selectedAccess.calls=0;
        fixture.reads=0;Runtime runtime;CHECK(!runtime.Prepare(selectedAccess));CHECK(runtime.Prepare(selectedAccess));
        selectedAccess.montage[source.object]=0;
        runtime.Before(selectedAccess,source.object,0x800000,type,false);CHECK(!runtime.Pending());
        selectedAccess.montage[source.object]=humanMontage;
        runtime.After(selectedAccess,source.object,0x800000,false,true,0);CHECK(runtime.Pending()&&runtime.report.starts==1);
        fixture.reads=0;runtime.Service(selectedAccess,24);CHECK(runtime.report.attempts==0);
        runtime.Service(selectedAccess,25);fixture.reads=0;runtime.Service(selectedAccess,50);
        CHECK(runtime.report.started==2&&runtime.report.returns==2);
    }
    fixture.reads=0;
    Recipient target{source,Assigned(fixture,fixture.botController)};
    fixture.Put(fixture.bot+cfghunter::disk::Health,100.0f);
    fixture.Put(fixture.module+cfgescape::disk::SCPlayerStateReflection,fixture.classes[cfgspawn::PlayerState]);
    fixture.Put(fixture.module+cfgescape::disk::SCPlayerStateConstructor,fixture.classes[cfgspawn::PlayerState]);
    CHECK(access.RecipientCurrent(target));
    fixture.Put(fixture.bot+cfgescape::disk::bHasEscaped,std::uint8_t{1});CHECK(!access.RecipientCurrent(target));
    fixture.Put(fixture.bot+cfgescape::disk::bHasEscaped,std::uint8_t{0});fixture.Put(fixture.botState+cfgescape::disk::bDead,std::uint8_t{1});CHECK(!access.RecipientCurrent(target));
    fixture.Put(fixture.botState+cfgescape::disk::bDead,std::uint8_t{0});fixture.Put(fixture.botController+cfgprobe::ObjectClass,fixture.classes[cfgspawn::BaseAI]);CHECK(!access.RecipientCurrent(target));
    fixture.Put(fixture.botController+cfgprobe::ObjectClass,fixture.classes[cfgspawn::CounselorAI]);
    auto stale=target;++stale.pawn.serial;CHECK(!access.RecipientCurrent(stale));stale=target;++stale.controller.serial;CHECK(!access.RecipientCurrent(stale));
    Function function;function.object=0xA00000;
    CHECK(access.DispatchCancel(target,function)&&host.calls==1&&host.nullPacket&&host.actor&&host.receiver==target.pawn.object&&host.function==function.object);
    CHECK(access.Dispatch(target,function,type)&&host.calls==2&&!host.nullPacket&&host.selected==type);
}
#ifndef CFG_EMOTE_MIRROR_MAIN
#define CFG_EMOTE_MIRROR_MAIN main
#endif
int CFG_EMOTE_MIRROR_MAIN(){
    for(const auto ended:{std::uintptr_t{0},std::uintptr_t{0xDEAD0000}}){
     struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;a.input.count=1;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);r.Service(a,75);
     CHECK(!r.Pending()&&a.calls==1);
     const auto target=a.input.bots[0];a.montage[target.pawn.object]=a.catalogue.female.object;
     CHECK(r.OwnsMontage(target,a.catalogue.female.object));
     a.montage[a.human.object]=ended;CHECK(r.Due(300));r.Service(a,300);CHECK(r.Pending());
     r.Service(a,325);CHECK(a.cancelCalls==1&&r.report.stopped==1&&!r.Due(600));
    }
    {struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;a.input.count=1;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);r.Service(a,75);
     r.Before(a,a.input.bots[0].pawn.object,0x800000,a.catalogue.selected,true);
     a.montage[a.human.object]=0;CHECK(!r.Due(300));r.Service(a,300);CHECK(!a.cancelCalls);
    }
    for(unsigned fault=0;fault<3;++fault){
     struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;a.input.count=1;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);r.Service(a,75);
     if(fault==0)++a.human.serial;if(fault==1)++a.input.world;if(fault==2)++a.catalogue.defaults.serial;
     a.montage[a.human.object]=0;r.Service(a,300);
     CHECK(!a.cancelCalls&&!r.Pending()&&!r.Due(600));
    }
    {struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);
     a.montage[a.human.object]=a.catalogue.female.object; // same verified catalogue, different montage
     r.Service(a,50);r.Service(a,75);CHECK(a.calls==1&&r.report.started==1);
     for(unsigned i=1;i<a.input.count;++i){const auto at=25+250*i;r.Service(a,at);r.Service(a,at+25);r.Service(a,at+50);}
     CHECK(!r.Pending()&&a.calls==a.input.count&&r.report.started==a.input.count);
     Cancel(r,a,5000);for(unsigned i=0;i<a.input.count;++i)r.Service(a,5025+25*i);
     CHECK(a.cancelCalls==a.input.count&&r.report.stopped==a.input.count);
    }
    {struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);a.montage[a.human.object]=0xDEAD0000;r.Service(a,25);
     CHECK(!r.Pending()&&!a.calls);
    }
    for(std::uintptr_t ended:{std::uintptr_t{0},std::uintptr_t{0xDEAD0000}}){
     struct LoopAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);r.Service(a,75);CHECK(a.calls==1);
     a.montage[a.human.object]=ended;r.Service(a,275);CHECK(r.Pending());r.Service(a,300);
     CHECK(a.cancelCalls==1&&r.report.stopped==1&&!r.Pending());
    }
    {Access a;a.input.count=1;a.updateMontage=false;Runtime r;Prepare(r,a);Start(r,a);
     r.Service(a,25);r.Service(a,50);r.Service(a,75);
     CHECK(a.calls==1&&r.report.returns==1&&!r.report.started&&r.report.startedUnchanged==1);
    }
    NativeCatalogue();
    {struct InterruptedAccess:Access{unsigned Interrupt(const Recipient&){return 1u;}}a;
     a.input.count=1;Runtime r;Prepare(r,a);Start(r,a);
     // The publication/scheduler caller queries Pending before servicing. Taking
     // the final queue item must not hide its multi-callback interruption.
     for(std::uint64_t now=25;now<=150;now+=25)if(r.Pending()&&r.Due(now))r.Service(a,now);
     CHECK(a.calls==1&&r.report.started==1&&!r.Pending());}
    {struct InterruptedAccess:Access{unsigned Interrupt(const Recipient&){return 0u;}}a;
     a.input.count=1;Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);
     CHECK(r.InterruptPending(a.input.bots[0]));
     r.Before(a,a.human.object,0x810000,0,false);
     r.After(a,a.human.object,0x810000,false,false,30);
     CHECK(!r.Pending()&&!r.Due(50)&&!r.InterruptPending(a.input.bots[0]));
     r.Service(a,50);CHECK(!a.calls);}
    {struct InterruptedAccess:Access{unsigned stops=0;unsigned Interrupt(const Recipient&){return ++stops<3?0u:1u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);CHECK(!a.calls&&!a.stops);
     r.Service(a,50);r.Service(a,75);r.Service(a,100);CHECK(!a.calls&&a.stops==3);
     r.Service(a,125);CHECK(a.calls==1&&r.report.started==1);}
    {struct RefusedInterruption:Access{unsigned Interrupt(const Recipient&){return 2u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);CHECK(!a.calls&&r.report.skipped==1);}
    {struct InterruptedAccess:Access{unsigned Interrupt(const Recipient&){return 0u;}}a;
     Runtime r;Prepare(r,a);Start(r,a);r.Service(a,25);CHECK(r.InterruptPending(a.input.bots[0]));
     CHECK(!r.InterruptPending(a.input.bots[1]));Cancel(r,a,30);CHECK(!r.InterruptPending(a.input.bots[0]));}
    {Queue q;unsigned i=99;CHECK(!q.Start(Queue::Capacity+1,0));CHECK(q.Start(9,0));CHECK(!q.Take(24,i));
        CHECK(q.Take(25,i)&&i==0);CHECK(!q.Take(25,i));CHECK(q.Take(1000,i)&&i==1);CHECK(!q.Take(1000,i));
        CHECK(!q.Take(999,i)&&!q.Pending());CHECK(q.Start(9,0));CHECK(!q.Take(3001,i)&&!q.Pending());}
    {Runtime runtime;Access access;Prepare(runtime,access);Start(runtime,access);CHECK(runtime.Pending());
        runtime.Service(access,24);CHECK(access.calls==0);
        for(unsigned i=1;i<=Queue::Capacity;++i){runtime.Service(access,25*i);runtime.Service(access,25*i);CHECK(access.calls==i);}
        CHECK(!runtime.Pending()&&runtime.report.started==Queue::Capacity&&runtime.report.returns==Queue::Capacity);}
    {Runtime r;Access a;Prepare(r,a);r.Before(a,a.input.bots[0].pawn.object,0x800000,a.catalogue.selected,false);
        r.After(a,a.input.bots[0].pawn.object,0x800000,false,true,0);CHECK(!r.Pending());
        r.Before(a,a.human.object,0x800000,a.catalogue.selected,true);r.After(a,a.human.object,0x800000,true,true,0);CHECK(!r.Pending());
        r.Before(a,a.human.object,0x800000,a.catalogue.selected,false);r.After(a,a.human.object,0x800000,false,true,0);CHECK(!r.Pending());}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);a.recipient=false;r.Service(a,50);CHECK(a.calls==1&&r.report.skipped==1);
        a.recipient=true;r.Service(a,75);CHECK(a.calls==2);a.source=false;r.Service(a,100);CHECK(!r.Pending()&&a.calls==2);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Before(a,a.human.object,0x810000,0,false);a.montage[a.human.object]=0;
        r.After(a,a.human.object,0x810000,false,true,5);CHECK(!r.Pending()&&r.report.cancels==1);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);a.catalogue.defaults.serial++;r.Service(a,25);CHECK(!r.Pending()&&!a.calls);}
    {Runtime r;Access a;Prepare(r,a);r.Before(a,a.human.object,0x800000,a.catalogue.selected,false);
        a.montage[a.human.object]=0xDEAD0000;r.After(a,a.human.object,0x800000,false,true,0);
        CHECK(!r.Pending()&&!r.report.starts&&r.report.failure==4);}
    {Runtime r;Access a;Prepare(r,a);r.Before(a,a.human.object,0x800000,a.catalogue.selected,false);
        a.montage[a.human.object]=a.catalogue.male.object;++a.catalogue.type.serial;
        r.After(a,a.human.object,0x800000,false,true,0);CHECK(!r.Pending()&&!r.report.starts);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);a.catalogue.female=Identity(0x440000);
        r.Service(a,25);CHECK(!r.Pending()&&!a.calls&&r.report.failure==3);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);a.dispatch=false;r.Service(a,25);r.Service(a,50);CHECK(!r.Pending()&&a.calls==1&&r.report.failure==6);}
    {Runtime r;Access a;Prepare(r,a);a.assets=false;Start(r,a);CHECK(!r.Pending()&&r.report.failure==3);}
    {Runtime r;Access a;a.functions=false;CHECK(!r.Prepare(a));CHECK(!r.Prepare(a));CHECK(a.binds==1);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);Cancel(r,a,60);
        CHECK(r.Pending());r.Service(a,84);CHECK(!a.cancelCalls);r.Service(a,85);CHECK(a.cancelCalls==1&&r.report.stopped==1);
        r.Service(a,85);CHECK(a.cancelCalls==1);r.Service(a,110);CHECK(a.cancelCalls==2&&r.report.stopped==2&&!r.Pending());
        CHECK(a.calls==2);Cancel(r,a,120);CHECK(!r.Pending());}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);r.Service(a,50);
        a.montage[a.input.bots[0].pawn.object]=0xDEAD0000;Cancel(r,a,60);r.Service(a,85);r.Service(a,110);
        CHECK(a.cancelCalls==1&&a.montage[a.input.bots[0].pawn.object]==0xDEAD0000);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);
        CHECK(r.Candidate(a.input.bots[0].pawn.object,0x800000,true));
        r.Before(a,a.input.bots[0].pawn.object,0x800000,a.catalogue.selected,true);
        Cancel(r,a,30);CHECK(!r.Pending()&&r.report.ownershipLost==1&&!a.cancelCalls);}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);Cancel(r,a,30);
        ++a.input.bots[0].pawn.serial;r.Service(a,55);CHECK(!a.cancelCalls&&!r.Pending());}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);Cancel(r,a,30);
        a.dispatch=false;r.Service(a,55);CHECK(a.cancelCalls==1&&!r.Pending()&&!r.report.stopped);
        Cancel(r,a,60);r.Service(a,85);CHECK(a.cancelCalls==1&&!r.Pending());}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);Cancel(r,a,30);
        a.montage[a.human.object]=a.catalogue.female.object;r.Service(a,55);CHECK(!a.cancelCalls&&!r.Pending());}
    {Runtime r;Access a;Prepare(r,a);Start(r,a);r.Service(a,25);Cancel(r,a,30);
        a.assets=false;r.Service(a,55);CHECK(!a.cancelCalls&&!r.Pending());}
    std::printf("Native emote mirror tests: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
