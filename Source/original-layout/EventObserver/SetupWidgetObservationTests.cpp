#include "../RuntimeProbe/SetupWidgetObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
struct MenuFixture {
    static constexpr std::uintptr_t Base=0x100000,Table=0x200000,Chunk=0x300000,
        Type=0x500000,Widget=0x510000,Donor=0x520000,DonorType=0x530000,
        Member=0x540000,Start=0x550000,Options=0x560000,Items=0x570000,Selected=0x580000,
        ObjectProperty=0x600000,FunctionClass=0x610000,ArrayProperty=0x620000,IntProperty=0x630000,StrProperty=0x640000,Inner=0x650000;
    OwnedByteReader r;unsigned nameIndex=0;
    void Name(std::uintptr_t object,const char* label){
        const auto index=nameIndex++,entry=0x400000+index*256;
        r.Set(Chunk+8*index,std::uintptr_t{entry});r.Set(entry,std::uint32_t{0});
        std::array<unsigned char,128> text{};for(unsigned i=0;label[i]&&i<127;++i)text[i]=static_cast<unsigned char>(label[i]);
        r.Set(entry+16,text);r.Set(object+24,cfgnames::Words{static_cast<int>(index),0});
    }
    void Field(std::uintptr_t object,std::uintptr_t owner,std::uintptr_t type,const char* name,std::uintptr_t next){
        Name(object,name);r.Set(object+16,type);r.Set(object+32,owner);r.Set(object+40,next);
    }
    void Property(std::uintptr_t object,int size,int offset){
        r.Set(object+cfgreflection::disk::PropertyDimensions,1);r.Set(object+cfgreflection::disk::PropertyElementSize,size);
        r.Set(object+cfgreflection::disk::PropertyFlags,std::uint64_t{0x80});r.Set(object+cfgreflection::disk::PropertyOffset,offset);
    }
    MenuFixture(){
        r.Set(Base+cfgnames::disk::Global,Table);r.Set(Table,Chunk);r.Set(Table+0x400,std::array<int,2>{100,1});
        Name(Type,"OfflineBotsSettingsMenuWidget_C");Name(DonorType,"NativeDonorCombo_C");
        Name(ObjectProperty,"ObjectProperty");Name(FunctionClass,"Function");Name(ArrayProperty,"ArrayProperty");Name(IntProperty,"IntProperty");
        Name(StrProperty,"StrProperty");
        r.Set(Widget+16,Type);r.Set(Widget+0x180,Donor);r.Set(Donor+16,DonorType);
        r.Set(Type+cfgreflection::disk::Children,Member);r.Set(DonorType+cfgreflection::disk::Children,Options);
        Field(Member,Type,ObjectProperty,"DifficultyComboBox",Start);Property(Member,8,0x180);
        Field(Start,Type,FunctionClass,"OnClicked_Start",0);r.Set(Start+cfgreflection::disk::Children,std::uintptr_t{0});
        r.Set(Start+cfgreflection::disk::StorageBytes,0);r.Set(Start+cfgreflection::disk::FunctionFlags,std::uint32_t{0});
        r.Set(Start+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{0});
        Field(Options,DonorType,FunctionClass,"SetOptions",0);r.Set(Options+cfgreflection::disk::Children,Items);
        r.Set(Options+cfgreflection::disk::StorageBytes,24);r.Set(Options+cfgreflection::disk::FunctionFlags,std::uint32_t{0});
        r.Set(Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{24});
        Field(Items,Options,ArrayProperty,"Items",Selected);Property(Items,16,0);
        Field(Selected,Options,IntProperty,"SelectedIndex",0);Property(Selected,4,16);
        r.Set(Items+cfgreflection::arraydisk::Inner,Inner);Field(Inner,Items,StrProperty,"Items",0);Property(Inner,16,0);
    }
};
int main(){
    using namespace cfgmenu::observation;unsigned failures=0;auto check=[&](bool value){if(!value)++failures;};Report report;
    for(unsigned fault=0;fault<9;++fault){MenuFixture f;
        switch(fault){
        case 1:f.r.Set(MenuFixture::Inner+16,MenuFixture::IntProperty);break;
        case 2:f.Property(MenuFixture::Inner,8,0);break;
        case 3:f.Property(MenuFixture::Inner,16,4);break;
        case 4:f.r.Set(MenuFixture::Inner+cfgreflection::disk::PropertyDimensions,2);break;
        case 5:f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyFlags,std::uint64_t{0});break;
        case 6:f.r.Set(MenuFixture::Selected+cfgreflection::disk::PropertyFlags,std::uint64_t{0x180});break;
        case 7:f.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{16});break;
        case 8:f.r.changeAt=MenuFixture::Inner+24;f.r.changeSlot=MenuFixture::Items+cfgreflection::arraydisk::Inner;f.r.newPointer=MenuFixture::Selected;break;
        }
        SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base,true);
        const bool ok=observer.Capture(MenuFixture::Widget,report);check(ok==(fault==0));
        if(ok)check(report.stringArrayShape&&!report.callable&&report.options.inner.owner==MenuFixture::Items);
        else check(!report.widget&&!report.callable);
    }
    {MenuFixture f;f.r.Set(MenuFixture::Inner+32,MenuFixture::Options);
        SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base,true);
        check(observer.Capture(MenuFixture::Widget,report)&&report.options.inner.owner==MenuFixture::Options&&!report.callable);}
    for(unsigned fault=0;fault<4;++fault){MenuFixture f;
        f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyFlags,std::uint64_t{0x08000394});
        f.r.Set(MenuFixture::Selected+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294});
        f.r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,20);
        f.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{20});
        if(fault==1)f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyFlags,std::uint64_t{0x180});
        if(fault==2)f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyFlags,std::uint64_t{0x08000314});
        if(fault==3)f.r.Set(MenuFixture::Selected+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000394});
        SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base,true);
        const bool ok=observer.Capture(MenuFixture::Widget,report);check(ok==(fault==0));
        if(ok)check(report.stringArrayShape&&!report.callable&&report.options.items.flags==0x08000394&&report.options.parameterBytes==20);
        else check(!report.widget&&!report.callable);
    }
    {MenuFixture f;SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base);
        check(observer.Capture(MenuFixture::Widget,report));check(report.settingsSchema&&!report.callable&&report.modes==3);
        check(report.options.items.offset==0&&report.options.selected.offset==16&&report.reads<=ReadLimit);
        check(report.start.object==MenuFixture::Start);}
    for(unsigned fault=0;fault<10;++fault){MenuFixture f;
        switch(fault){
        case 0:f.r.Set(MenuFixture::Items+32,MenuFixture::Type);break; // wrong parameter owner
        case 1:f.Property(MenuFixture::Selected,4,12);break; // overlapping fields
        case 2:f.Property(MenuFixture::Selected,4,24);break; // outside storage
        case 3:f.r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,513);break;
        case 4:f.r.Set(MenuFixture::Selected+40,MenuFixture::Items);break; // cycle/extra parameter
        case 5:f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyDimensions,2);break;
        case 6:f.Name(MenuFixture::Items,"Other");break;
        case 7:f.r.Set(MenuFixture::Widget+0x180,std::uintptr_t{0});break;
        case 8:f.r.Set(MenuFixture::Start+cfgreflection::disk::StorageBytes,8);break;
        case 9:f.r.changeAt=MenuFixture::Items+24;f.r.changeSlot=MenuFixture::Widget+16;f.r.newPointer=MenuFixture::DonorType;break;
        }
        SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base);check(!observer.Capture(MenuFixture::Widget,report));
        check(!report.widget&&!report.callable&&observer.Reads()<=ReadLimit);
    }
    const char* labels[]{"OfflinePlayMenuWidget_C","Counselor_Menu_C","PickOfflineBotsMapMenuWidget_C","SPChallengesSettingsMenuWidget_C"};
    const Route routes[]{Route::OfflineEntry,Route::CounselorPicker,Route::MapPicker,Route::ChallengeSettings};
    for(unsigned i=0;i<4;++i){MenuFixture f;f.Name(MenuFixture::Type,labels[i]);SetupWidget<OwnedByteReader> observer(f.r,MenuFixture::Base);
        check(observer.Capture(MenuFixture::Widget,report)&&report.route==routes[i]&&!report.settingsSchema&&!report.callable);}
    {MenuFixture f;Bounded<OwnedByteReader> reader(f.r);std::uintptr_t value=0;
        for(unsigned i=0;i<ReadLimit;++i)reader.Read(MenuFixture::Widget+16,value);
        check(!reader.Read(MenuFixture::Widget+16,value)&&reader.exhausted&&f.r.reads==ReadLimit);}
    std::printf("SetupWidgetObservationTests: %u failures; readonly synthetic widget metadata, no callable UI\n",failures);
    return failures?1:0;
}
