#define CFG_RB_MEMBER_FIXTURE_ONLY
#include "RBEmoteRuntimeTests.cpp"
struct ConfirmHost:NativeHost {
    std::uintptr_t query=0,play=0,widgetType=0,userType=0;
    unsigned queries=0,plays=0,eligibility=0,failures=0,mutation=0;bool allowed=true,returns=true;
    explicit ConfirmHost(Fixture& r):NativeHost(r){}
    bool FeatureFunction(const cfgvehicle::disk::Binding& b,std::uintptr_t& fn,std::uintptr_t& type){
        const bool isPlay=b.cache==cfgrbemote::confirmdisk::PlayEmote.cache;
        CHECK(b.nativeBytes==(isPlay?4u:8u)&&b.storageBytes==b.nativeBytes);
        fn=isPlay?play:query;type=isPlay?widgetType:userType;return true;
    }
    bool RBEligible(const cfgrbemote::Owner&,const cfgrbemote::ConfirmationEvidence&){++eligibility;return allowed;}
    bool RBInvoke(const cfgrbemote::Owner& rbOwner,const cfgrbemote::ConfirmationEvidence& e,std::uintptr_t fn,void* packet){
        CHECK(e.widget.object&&e.hud.object&&rbOwner.pawn==pawn);
        if(fn==query){++queries;*static_cast<std::uintptr_t*>(packet)=mutation==1?0:pawn.object;
            if(mutation==2)r.Put(e.widget.object+cfgrbemote::confirmdisk::CurrentHighlightSection,int{7});
            if(mutation==3)r.Put(e.hud.object+cfgrbemote::confirmdisk::EmoteWidgetInstance,std::uintptr_t{});
            return true;}
        CHECK(fn==play&&*static_cast<int*>(packet)==3);++plays;return returns;
    }
    void RBConfirmationFailed(){++failures;}
};
int main(){using namespace cfgrbemote;namespace d=confirmdisk;
    unsigned peak=0;
    for(unsigned fault=0;fault<13;++fault){Fixture r;ConfirmHost h(r);
        const auto controller=r.Object(),pawn=r.Object(),hudType=r.Object(),userType=r.Object(),widgetType=r.Object();
        const auto hud=r.Object(hudType),widget=r.Object(widgetType);
        h.owner=r.Object();r.Put(pawn+cfgprobe::ObjectClass,h.owner);h.userType=userType;h.widgetType=widgetType;
        h.query=r.Object();h.play=r.Object();
        const auto positive=[&](std::uintptr_t object,Slot& out){int index=0;CHECK(r.Read(object+0xC,index));
            r.Put(r.objects.data+24*index+16,index+1);CHECK(cfgspawn::ObjectSlot(r,r.objects,object,out));};
        Slot controllerSlot{};positive(controller,controllerSlot);positive(pawn,h.pawn);
        r.Put(controller+cfgescape::disk::Pawn,pawn);r.Put(pawn+cfgescape::disk::Controller,controller);
        r.Put(controller+d::ControllerHUD,hud);r.Put(hud+d::PlayerOwner,controller);r.Put(hud+d::EmoteWidgetInstance,widget);
        r.Put(widget+d::CurrentHighlightSection,int{3});r.Put(widgetType+cfgprobe::ClassSuper,userType);
        r.Put(r.module+d::SCInGameHUDClass,hudType);r.Put(r.module+d::SCInGameHUDConstructor,hudType);
        r.Put(r.module+d::SCCounselorEmoteSelectWidgetClass,widgetType);r.Put(r.module+d::SCCounselorEmoteSelectWidgetConstructor,widgetType);
        r.Put(r.module+d::GetOwningPlayerPawn.cache,h.query);r.Put(h.query+0xB0,r.module+d::GetOwningPlayerPawnExecutor);
        r.Put(r.module+d::GetOwningPlayerPawnExecutor,d::GetOwningPlayerPawnCode);
        r.Put(r.module+d::PlayEmote.cache,h.play);r.Put(h.play+0xB0,r.module+d::PlayEmoteExecutor);
        r.Put(r.module+d::PlayEmoteExecutor,d::PlayEmoteCode);
        NativeAccess<Fixture,ConfirmHost> access(r,h,r.module,controller,controllerSlot);Owner owner;CHECK(access.Source(owner));
        if(fault==1)h.allowed=false;
        if(fault==2)r.Put(hud+d::PlayerOwner,pawn);
        if(fault==3)r.Put(widget+d::CurrentHighlightSection,int{-1});
        if(fault==4)r.Put(widget+d::CurrentHighlightSection,int{8});
        if(fault==5)r.Put(r.module+d::SCCounselorEmoteSelectWidgetConstructor,userType);
        if(fault==6)h.mutation=1;
        if(fault==7)h.mutation=2;
        if(fault==8)h.mutation=3;
        if(fault==9)h.returns=false;
        if(fault==10)r.Put(r.module+d::PlayEmote.cache,std::uintptr_t{});
        if(fault==11)r.Put(h.query+0xB0,r.module+d::PlayEmoteExecutor);
        if(fault==12)r.Put(controller+cfgescape::disk::Pawn,widget);
        r.reads=0;r.cap=1536;
        CHECK(access.Confirm(owner)==(fault==0));peak=(std::max)(peak,r.reads);CHECK(r.reads<=1536);
        CHECK(h.plays==((fault==0||fault==9)?1u:0u));CHECK(h.failures==(fault==9?1u:0u));
    }
    std::printf("Actual donor RB confirmation adapter: %d failures; peak%u/1536; highlighted native widget, exact owner, mutation and fallback restore checks. Owned native stubs only.\n",failures,peak);
    return failures?1:0;
}
