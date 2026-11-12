#include "../Core/EndingMetadataPolicy.hpp"
#include "EndingRetainedCall.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include "../RuntimeProbe/EndingMetadataBindings.generated.hpp"
#include "../RuntimeProbe/CounselorBirthBindings.generated.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool x){if(!x)++failures;};
    {cfgbundle::Report r;check(cfgbundle::Valid(r));r.endingRepairStage=9;r.endingRepairFailure=26;r.endingRepairCalls=4;
     check(cfgbundle::Valid(r));r.endingRepairStage=10;check(!cfgbundle::Valid(r));
     r.endingRepairStage=9;r.endingRepairFailure=27;check(cfgbundle::Valid(r));
     r.endingRepairFailure=28;check(!cfgbundle::Valid(r));
     r.endingRepairFailure=26;r.endingRepairCalls=5;check(!cfgbundle::Valid(r));}
    check(cfgending::CanRegisterState(0x2000,0x1000,0));
    check(cfgending::CanRegisterState(0x2000,0x1000,0x2000));
    check(!cfgending::CanRegisterState(0,0x1000,0));
    check(!cfgending::CanRegisterState(0x1000,0x1000,0));
    check(!cfgending::CanRegisterState(0x2000,0x1000,0x3000));
    cfgending::Packet a{},b{};check(!cfgending::EmptySubpath(a));a[16]=1;b=a;
    check(cfgending::EmptySubpath(a)&&cfgending::SamePath(a,b));b[0]=42;check(cfgending::SamePath(a,b));
    b[17]=2;check(!cfgending::SamePath(a,b));b=a;b[24]=1;check(!cfgending::EmptySubpath(b));
    b=a;b[32]=1;check(!cfgending::EmptySubpath(b));b=a;b[36]=1;check(!cfgending::EmptySubpath(b));
    cfgending::Transaction e;check(!e.Finished());check(!e.Refuse(2));check(e.Finished()&&e.failure==2&&e.calls==0);
    check(cfgending::disk::NativeSettersVerified&&cfgending::disk::SetClass==0x3FDBE0&&cfgending::disk::SetSkin==0x3FDFF0&&cfgending::disk::SetWeapon==0x3FEC40);
    check(cfgending::disk::SetClassCode.size()==512&&cfgending::disk::SetSkinCode.size()==3147&&cfgending::disk::SetWeaponCode.size()==373);
    check(cfgmenu::birthdisk::ConsumeActive==0x3FAFD0&&cfgmenu::birthdisk::ActiveCharacterClass==0x848&&
        cfgmenu::birthdisk::ConsumeActiveCode.size()==121&&cfgmenu::birthdisk::ActiveNotifyCode.size()==114&&
        cfgmenu::birthdisk::SettingsNotifyCode.size()==64&&cfgmenu::birthdisk::PathCopyCode.size()==37);
    // An AI state may start with an empty soft path (including native weak
    // sentinel bytes) or already have our selected killer, never a foreign role.
    for(unsigned before=0;before<3;++before){cfgending::Transaction t;t.stage=cfgending::Stage::Active;t.selected=a;
        cfgending::Packet prior{};if(before==1)prior[0]=255;if(before==2)prior=a;
        check(t.BeginActive(prior)&&t.stage==cfgending::Stage::ActiveInFlight&&!t.Finished());
        auto observed=a;observed[0]=73;
        check(t.CompleteActive(true,observed)&&t.stage==cfgending::Stage::Skin);
        check(!t.BeginActive(prior)&&t.Finished());}
    for(unsigned fault=0;fault<8;++fault){cfgending::Transaction t;t.stage=cfgending::Stage::Active;t.selected=a;cfgending::Packet prior{};
        if(fault==0){prior=a;prior[16]=2;}
        if(fault==1)prior[24]=1;
        if(fault==2)prior[32]=1;
        if(fault==3)prior[36]=1;
        if(fault==4)t.selected[24]=1;
        if(fault==5)t.selected[32]=1;
        if(fault==6)t.selected={};
        if(fault==7)t.stage=cfgending::Stage::Class;
        check(!t.BeginActive(prior)&&t.stage==cfgending::Stage::Refused&&t.calls==0);}
    for(unsigned fault=0;fault<4;++fault){cfgending::Transaction t;t.stage=cfgending::Stage::Active;t.selected=a;
        check(t.BeginActive({}));auto observed=a;
        if(fault==1)observed[16]=2;
        if(fault==2)observed[24]=1;
        if(fault==3)check(!t.BeginActive({})); // native callback reentry consumes/refuses, never replays
        check(!t.CompleteActive(fault!=0,observed)&&t.Finished());
        check(!t.CompleteActive(true,a)&&t.stage==cfgending::Stage::Refused);}
    // Exercise the SAME retained gateway called by BundleAccess: input refusal,
    // failed entry, exception/ambiguous return, nested reentry and all 4 setters.
    for(unsigned kind=0;kind<4;++kind)for(unsigned fault=0;fault<6;++fault){
        const auto setter=static_cast<cfgending::NativeSetter>(kind);
        cfgending::Transaction t;t.stage=cfgending::Transaction::NativeStage(setter);
        struct Host {
            cfgending::Transaction& t;cfgending::NativeSetter setter;unsigned fault;
            unsigned prepared=0,begun=0,invoked=0,finished=0;bool nestedAdmitted=false;
            bool Prepare(){++prepared;
                nestedAdmitted=t.BeginNative(setter)||t.BeginNative(cfgending::NativeSetter::Weapon);
                return fault!=1;}
            bool Begin(){++begun;return fault!=2;}
            bool Invoke(){++invoked;
                // A synchronous engine callback cannot reenter even another
                // ending setter while these exact roots/native work are held.
                nestedAdmitted=nestedAdmitted||t.BeginNative(setter)||t.BeginNative(cfgending::NativeSetter::Weapon);
                if(fault==5)t.Refuse(8);
                return fault!=3;
            }
            bool Finish(){++finished;return fault!=4;}
        }host{t,setter,fault};
        check(cfgending::RetainedSetter(t,setter,host)==(fault==0));
        check(!host.nestedAdmitted&&!t.nativeBusy&&host.prepared==1);
        check(host.begun==(fault==1?0u:1u));
        check(host.invoked==((fault==1||fault==2)?0u:1u)&&t.calls==host.invoked);
        check(host.finished==((fault==1||fault==2||fault==3)?0u:1u));
        {const auto before=host.invoked;check(!cfgending::RetainedSetter(t,setter,host));check(host.invoked==before);}
    }
    {cfgending::Transaction t;check(!t.BeginNative(cfgending::NativeSetter::Class));
     check(!t.BeginNative(static_cast<cfgending::NativeSetter>(4))&&!t.nativeConsumed&&!t.calls);}
    std::printf("CFG ending owned-input and full native setter extents: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
