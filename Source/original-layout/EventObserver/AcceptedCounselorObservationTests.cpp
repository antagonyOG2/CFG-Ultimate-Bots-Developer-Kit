#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/AcceptedCounselorObservation.hpp"
#include "PickerPrecreationFixture.hpp"
#include "AcceptedCounselorCommitPolicy.hpp"
int main(){using namespace cfgmenu;unsigned checks=0,failures=0,maximum=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned before=0;before<=10;++before)for(unsigned after=0;after<=10;++after)for(unsigned valid=0;valid<2;++valid){
        const bool expected=valid&&(before==3||before==4)&&(after==6||after==7||after==8);
        check(acceptedcommit::Commit(valid!=0,static_cast<counselorroute::State>(before),static_cast<counselorroute::State>(after))==expected,
            "actual first-arrival Accept commits; rejected/retired/cancelled routes never commit");
    }
    auto fixture=std::make_unique<PickerPreparedFixture>(
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-16530-15103-Roots.txt",
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Native-Picker-Chain-3243-25117-Counselor.txt",
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-11776-28527-Jason.txt");
    check(fixture->initialized,"captured CFG native picker metadata fixture");if(!fixture->initialized)return 1;
    auto& f=fixture->fixture;auto& r=f.r;OptionsReader reader{r};pickerprepare::Prepared prepared;
    check(pickerprepare::Prepare(reader,fixture->owner,*fixture->source,pickerprepare::Kind::Counselor,42,true,prepared),"prepare native counselor metadata");
    constexpr std::uintptr_t widget=0x4400000,inner=0x4600000,structType=0x4610000,script=0x4620000,soft=0x4630000,structure=0x4640000,character=0x4650000,transform=0x4660000,rows=0x4700000,pathObject=0x4800000;
    fixture->Register(widget,8100);r.Set(widget+cfgprobe::ObjectClass,prepared.type);
    r.Set(NativeFixture::Records+execdisk::RecordBytes,prepared.type);r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,widget);
    f.Name(structure,"StructProperty");f.Name(soft,"SoftClassProperty");f.Name(script,"ScriptStruct");fixture->Root(script);
    f.Name(structType,"FSCCharacterSelectData");r.Set(structType+cfgprobe::ObjectClass,script);r.Set(structType+cfgprobe::ClassSuper,std::uintptr_t{0});
    r.Set(structType+cfgreflection::disk::StorageBytes,96);r.Set(structType+cfgreflection::disk::Children,character);
    f.Field(inner,prepared.roster.field.object,structure,"Inner",0);f.Property(inner,96,0);
    r.Set(inner,MenuFixture::Base+cfgreflection::structdisk::Vtable);r.Set(inner+cfgreflection::structdisk::Type,structType);
    r.Set(prepared.roster.field.object+cfgreflection::arraydisk::Inner,inner);
    f.Field(character,structType,soft,"CharacterClass_10_29392D434148412C26DBACA4ECDABFEC",transform);f.Property(character,40,0);
    r.Set(character+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008000000000805ULL});
    f.Field(transform,structType,structure,"MenuTransform_9_E53DDB35467D99E1BA3E72AB59737E49",0);f.Property(transform,48,48);
    r.Set(transform+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0000001040000005ULL});
    f.Name(pathObject,"/Game/Counselors/AJ.AJ_C");cfgnames::Words path;r.Read(pathObject+cfgnames::disk::ObjectName,path);
    std::array<unsigned char,96> row{};std::memcpy(row.data()+16,&path,8);r.Set(rows+6*96,row);
    active::Context actual;check(active::Observe(reader,MenuFixture::Base,actual),"real active counselor widget");
    for(unsigned fault=0;fault<11;++fault){
        r.Set(widget+prepared.index.offset,6);r.Set(widget+prepared.roster.offset,pickercapture::Array{rows,18,21});r.Set(rows+6*96,row);
        r.Set(character+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008000000000805ULL});r.Set(transform+cfgreflection::disk::PropertyOffset,48);
        r.Set(inner,MenuFixture::Base+cfgreflection::structdisk::Vtable);
        r.Set(inner+cfgreflection::structdisk::Type,structType);
        r.Set(prepared.roster.field.object+cfgreflection::disk::Next,prepared.roster.field.next);
        r.Set(prepared.index.field.object+cfgreflection::disk::Next,prepared.index.field.next);
        if(fault==1)r.Set(widget+prepared.index.offset,-1);
        if(fault==2)r.Set(widget+prepared.index.offset,18);
        if(fault==3)r.Set(character+cfgreflection::disk::PropertyFlags,std::uint64_t{0});
        if(fault==4)r.Set(transform+cfgreflection::disk::PropertyOffset,40);
        if(fault==5){auto bad=row;bad[24]=1;r.Set(rows+6*96,bad);}
        if(fault==6)r.Set(inner+cfgreflection::structdisk::Type,script);
        if(fault==7)r.Set(widget+prepared.roster.offset,pickercapture::Array{rows,65,65});
        if(fault==8)r.Set(inner,MenuFixture::Base+0x231D6E8); // stale3.4 table, previously accepted by synthetic fixture
        if(fault==9)r.Set(prepared.roster.field.object+cfgreflection::disk::Next,std::uintptr_t{0});
        if(fault==10)r.Set(prepared.index.field.object+cfgreflection::disk::Next,std::uintptr_t{0});
        acceptedcounselor::Choice choice;unsigned stage=0;const auto before=r.reads;
        const bool okay=acceptedcounselor::Observe(reader,actual,prepared,choice,&stage);const auto reads=r.reads-before;maximum=(std::max)(maximum,reads);
        if(!fault&&!okay)std::printf("Capture stage=%u reads=%u\n",stage,reads);
        check(okay==(fault==0||fault>=9),"only validated selected row accepted; unrelated cross-event Next ignored");check(reads<=acceptedcounselor::ReadLimit,"bounded selected-row reads");
        if(okay){check(choice.index==6&&choice.name.View()==u"/Game/Counselors/AJ.AJ_C","highlighted row exact full asset path");
            MenuCatalogueEvidence evidence;evidence.counselor=0xBAD000;check(acceptedcounselor::Apply(evidence,choice.path)&&evidence.counselor==softpath::Token(choice.path)&&evidence.counselorPathSelection,"stale Customize class replaced only by accepted path token");}
        else check(choice.index==-1&&softpath::Empty(choice.path),"no partial choice on refusal");
    }
    std::printf("Accepted CFG counselor row: %s (%u checks, %u failures, max %u/4096 reads). Owned fixtures only.\n",failures?"FAIL":"PASS",checks,failures,maximum);return failures?1:0;
}
