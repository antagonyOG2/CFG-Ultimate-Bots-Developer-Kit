#pragma once
#include "ReflectionObservation.hpp"
#include "ArrayReflection.generated.hpp"
#include "../Core/SetupMenuModel.hpp"
namespace cfgmenu::observation {
constexpr unsigned ReadLimit=4096;
enum class Route { Unknown,OfflineEntry,OfflineSettings,CounselorPicker,MapPicker,ChallengeSettings };
inline Route Classify(std::u16string_view name)noexcept{
    if(name==u"OfflinePlayMenuWidget_C")return Route::OfflineEntry;
    if(name==u"OfflineBotsSettingsMenuWidget_C")return Route::OfflineSettings;
    if(name==u"Counselor_Menu_C")return Route::CounselorPicker;
    if(name==u"PickOfflineBotsMapMenuWidget_C")return Route::MapPicker;
    if(name==u"SPChallengesSettingsMenuWidget_C")return Route::ChallengeSettings;
    return Route::Unknown;
}
inline unsigned Modes(Route route)noexcept{
    if(route==Route::CounselorPicker)return static_cast<unsigned>(Mode::Counselor);
    if(route==Route::ChallengeSettings)return static_cast<unsigned>(Mode::Challenges);
    if(route==Route::OfflineSettings||route==Route::MapPicker)return 3;
    if(route==Route::OfflineEntry)return 7;
    return 0;
}
struct Parameter {
    std::uintptr_t field=0;std::int32_t offset=0,bytes=0;std::uint64_t flags=0;
    std::uintptr_t owner=0,type=0;
    bool operator==(const Parameter& b)const{return field==b.field&&offset==b.offset&&bytes==b.bytes&&flags==b.flags&&owner==b.owner&&type==b.type;}
};
struct Function {
    std::uintptr_t object=0,owner=0,head=0;
    std::int32_t storage=0;std::uint32_t flags=0;
    std::uint16_t parameterBytes=0;Parameter items{},selected{},inner{};
    bool operator==(const Function& b)const{return object==b.object&&owner==b.owner&&head==b.head&&storage==b.storage&&flags==b.flags&&parameterBytes==b.parameterBytes&&items==b.items&&selected==b.selected&&inner==b.inner;}
};
struct Report {
    Route route=Route::Unknown;unsigned modes=0,reads=0;
    std::uintptr_t widget=0,type=0,donor=0,donorType=0;
    Function start{},options{};
    // String shape is optional. Inner Outer is observed, not assumed to be the
    // function or array. Ownership/allocator/reference flags are NOT proved.
    bool settingsSchema=false,stringArrayShape=false,callable=false;
};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;bool exhausted=false;
    explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t address,T& output){
        if(reads==ReadLimit){exhausted=true;return false;}++reads;
        return cfgprobe::lifetime::Extent(address,sizeof(T))&&reader_.Read(address,output);
    }
};
template<class Reader> class SetupWidget {
    Bounded<Reader> r_;std::uintptr_t module_;bool exactStringArray_;
    bool FunctionSchema(std::uintptr_t type,std::u16string_view name,bool options,Function& out){
        cfgreflection::Observation<Bounded<Reader>> reflection(r_,module_);
        cfgreflection::Field f;
        if(!reflection.Find(type,name,f)||f.label.View()!=name||f.kind.View()!=u"Function")return false;
        Function candidate;candidate.object=f.object;candidate.owner=f.owner;
        if(!r_.Read(f.object+cfgreflection::disk::StorageBytes,candidate.storage)||candidate.storage<0||candidate.storage>512||
           !r_.Read(f.object+cfgreflection::disk::FunctionFlags,candidate.flags)||
           !r_.Read(f.object+cfgreflection::arraydisk::FunctionParameterBytes,candidate.parameterBytes)||candidate.parameterBytes>candidate.storage||
           !r_.Read(f.object+cfgreflection::disk::Children,candidate.head))return false;
        auto child=candidate.head;unsigned count=0;
        while(child){
            if(!options||count==2)return false;
            cfgreflection::Field field;cfgreflection::Property p;
            if(!reflection.ReadField(child,f.object,field)||!reflection.ReadProperty(field,p)||p.dimension!=1||
               p.offset>candidate.parameterBytes||p.elementBytes>candidate.parameterBytes-p.offset)return false;
            Parameter value{field.object,p.offset,p.elementBytes,p.flags};
            if(field.label.View()==u"Items"&&field.kind.View()==u"ArrayProperty"&&p.elementBytes==16&&p.offset%8==0&&!candidate.items.field)candidate.items=value;
            else if(field.label.View()==u"SelectedIndex"&&field.kind.View()==u"IntProperty"&&p.elementBytes==4&&p.offset%4==0&&!candidate.selected.field)candidate.selected=value;
            else return false;
            child=field.next;++count;
        }
        if(options&&(!candidate.items.field||!candidate.selected.field||
            !(candidate.items.offset+16<=candidate.selected.offset||candidate.selected.offset+4<=candidate.items.offset)))return false;
        if(options&&exactStringArray_){
            std::uintptr_t inner=0,owner=0,again=0;cfgreflection::Field field;cfgreflection::Property p;
            if(!r_.Read(candidate.items.field+cfgreflection::arraydisk::Inner,inner)||!cfgprobe::ObjectPointer(inner)||
               !r_.Read(inner+0x20,owner)||!cfgprobe::ObjectPointer(owner)||
               !reflection.ReadField(inner,owner,field)||field.kind.View()!=u"StrProperty"||
               !reflection.ReadProperty(field,p)||p.dimension!=1||p.elementBytes!=cfgreflection::arraydisk::StringValueBytes||p.offset!=0||
               !r_.Read(candidate.items.field+cfgreflection::arraydisk::Inner,again)||again!=inner)return false;
            candidate.inner={inner,p.offset,p.elementBytes,p.flags,owner,field.type};
            // Only the dispatcher parameter/output-chain masks are proven.
            // This is NOT a complete input/reference/return ownership check.
            if(!(candidate.items.flags&cfgreflection::arraydisk::ParameterMask)||
               !(candidate.selected.flags&cfgreflection::arraydisk::ParameterMask)||
               (candidate.selected.flags&cfgreflection::arraydisk::OutputChainMask))return false;
            // CFG's captured Items input is placed on the dispatcher's output
            // chain (08000394). Do not confuse chain membership with an output
            // assignment. Accept only this observed reference shape here; it
            // still grants NO call authority or ownership transfer. The script
            // and destination copy pipeline require separate validation.
            if((candidate.items.flags&cfgreflection::arraydisk::OutputChainMask)&&
               candidate.items.flags!=0x08000394ULL)return false;
        }
        // Start is a zero-field, zero-storage candidate, not proof of safe execution.
        if(!options&&candidate.storage!=0)return false;
        std::uintptr_t head=0;std::int32_t storage=0;std::uint32_t flags=0;std::uint16_t parameterBytes=0;
        if(!r_.Read(f.object+cfgreflection::disk::Children,head)||head!=candidate.head||
           !r_.Read(f.object+cfgreflection::disk::StorageBytes,storage)||storage!=candidate.storage||
           !r_.Read(f.object+cfgreflection::arraydisk::FunctionParameterBytes,parameterBytes)||parameterBytes!=candidate.parameterBytes||
           !r_.Read(f.object+cfgreflection::disk::FunctionFlags,flags)||flags!=candidate.flags)return false;
        cfgreflection::Field finalField;
        if(!reflection.ReadField(f.object,f.owner,finalField)||finalField.type!=f.type||
           !(finalField.name==f.name)||finalField.next!=f.next||finalField.kind.View()!=u"Function")return false;
        out=candidate;return true;
    }
    bool Sample(std::uintptr_t widget,Report& out){
        cfgnames::Observation<Bounded<Reader>> names(r_,module_);
        cfgreflection::Observation<Bounded<Reader>> reflection(r_,module_);
        Report candidate;candidate.widget=widget;cfgnames::Text name;
        if(!cfgprobe::ObjectPointer(widget)||!r_.Read(widget+cfgprobe::ObjectClass,candidate.type)||
            !names.Object(candidate.type,name)||(candidate.route=Classify(name.View()))==Route::Unknown)return false;
        candidate.modes=Modes(candidate.route);
        if(candidate.route==Route::OfflineSettings){
            if(!reflection.ReadObjectMember(widget,candidate.type,u"DifficultyComboBox",candidate.donor)||!candidate.donor||
                !r_.Read(candidate.donor+cfgprobe::ObjectClass,candidate.donorType)||
                !FunctionSchema(candidate.type,u"OnClicked_Start",false,candidate.start)||
                !FunctionSchema(candidate.donorType,u"SetOptions",true,candidate.options))return false;
            candidate.settingsSchema=true;
            candidate.stringArrayShape=exactStringArray_;
        }
        std::uintptr_t again=0;
        if(!r_.Read(widget+cfgprobe::ObjectClass,again)||again!=candidate.type)return false;
        if(candidate.settingsSchema&&(!reflection.ReadObjectMember(widget,candidate.type,u"DifficultyComboBox",again)||again!=candidate.donor||
            !r_.Read(candidate.donor+cfgprobe::ObjectClass,again)||again!=candidate.donorType))return false;
        out=candidate;return true;
    }
public:
    SetupWidget(Reader& reader,std::uintptr_t module,bool exactStringArray=false):r_(reader),module_(module),exactStringArray_(exactStringArray){}
    // One bounded observation object per supplied widget event, never a scan.
    bool Capture(std::uintptr_t widget,Report& output){
        output={};Report a,b;
        const bool ok=Sample(widget,a)&&Sample(widget,b)&&a.route==b.route&&a.type==b.type&&
            a.donor==b.donor&&a.donorType==b.donorType&&a.start==b.start&&a.options==b.options;
        if(ok){a.reads=r_.reads;output=a;}return ok;
    }
    unsigned Reads()const{return r_.reads;}
};
}
