// Included in the fixed-path read-only helper's private namespace.
bool PickerReference(Reader& r,const cfgmenu::active::Context& c,std::uintptr_t value){
    std::printf("PickerStrongReference=0x%llX",static_cast<unsigned long long>(value));
    if(!value){std::puts(" Name=<unset> CallApproved=no");return true;}
    cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text name,kind;std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation slot;
    if(!cfgprobe::ObjectPointer(value)||!r.Read(value+cfgprobe::ObjectClass,type)||!names.Object(value,name)||
       !names.Object(type,kind)||!cfgprobe::lifetime::ReadSlot(r,c.registry,value,slot))return false;
    std::printf(" Name=");Name(name);std::printf(" Class=");Name(kind);std::printf(" Serial=%d SerialAssigned=%s CallApproved=no\n",slot.serial,slot.HasAssignedSerial()?"yes":"no");return true;
}
bool PickerStruct(Reader& r,const cfgmenu::active::Context& c,const cfgreflection::Property& item,
    const std::array<unsigned char,cfgmenu::pickercapture::MaximumRowBytes>& row){
    using namespace cfgmenu;std::uintptr_t vtable=0,type=0,typeClass=0,head=0;std::int32_t size=0;
    cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text name,kind;cfgreflection::Observation<Reader> reflection(r,c.module);
    if(!r.Read(item.field.object,vtable)||vtable!=c.module+worldmapdisk::StructPropertyVtable||
       !r.Read(item.field.object+worldmapdisk::StructType,type)||!cfgprobe::ObjectPointer(type)||!names.Object(type,name)||
       !r.Read(type+cfgprobe::ObjectClass,typeClass)||!names.Object(typeClass,kind)){
        std::puts("PickerStructRefused Stage=StructDescriptorOrType NoNativeCallApproval=yes");return false;}
    // This diagnostic handles CFG's Blueprint UserDefinedStruct roster rows,
    // not just native ScriptStructs. Confirm actual inheritance, not name alias.
    cfgprobe::Ancestry ancestry;bool scriptStruct=false;
    if(!cfgprobe::CaptureAncestry(r,type,ancestry))return false;
    for(unsigned i=0;i<ancestry.count;++i){cfgnames::Text parent;if(!names.Object(ancestry.classes[i],parent))return false;
        if(parent.View()==u"ScriptStruct")scriptStruct=true;}
    std::printf("PickerSelectedRowStructType Name=");Name(name);std::printf(" Class=");Name(kind);
    std::printf(" NativeScriptStructAncestry=%s CallApproved=no\n",scriptStruct?"yes":"no");
    if(!scriptStruct||!cfgprobe::StableAncestry(r,type,ancestry)||
       !r.Read(type+cfgreflection::disk::StorageBytes,size)||size!=item.elementBytes||!r.Read(type+cfgreflection::disk::Children,head))return false;
    std::printf("PickerSelectedRowStruct=0x%llX Name=",static_cast<unsigned long long>(type));Name(name);std::printf(" StorageBytes=%d\n",size);
    std::array<cfgreflection::Field,32> fields{};unsigned count=0;
    for(auto child=head;child;){
        if(count==fields.size())return false;for(unsigned i=0;i<count;++i)if(fields[i].object==child)return false;
        auto& field=fields[count++];cfgreflection::Property property;
        if(!reflection.ReadField(child,type,field)||!capture::RawProperty(r,field,property)||property.dimension!=1||property.offset>size-property.elementBytes)return false;
        Property("PickerSelectedRowProperty",property);child=field.next;
        if((field.kind.View()==u"ClassProperty"||field.kind.View()==u"ObjectProperty")&&property.elementBytes==8){
            std::uintptr_t value=0;std::memcpy(&value,row.data()+property.offset,8);if(!PickerReference(r,c,value))return false;}
    }
    std::uintptr_t again=0;if(!r.Read(type+cfgreflection::disk::Children,again)||again!=head||
       !r.Read(item.field.object+worldmapdisk::StructType,again)||again!=type)return false;
    for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];
        if(!reflection.ReadField(old.object,type,fresh)||fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return false;}
    return true;
}
int PickerCapture(HANDLE process,std::uintptr_t base,Reader& r,cfgmenu::pickercapture::Kind kind){
    using namespace cfgmenu;active::Context c;cfgnames::Observation<Reader> names(r,base);cfgnames::Text name;
    if(!active::Observe(r,base,c)||!names.Object(c.type,name)){std::puts("PickerCaptureStage=ActiveOwner OwnerUnavailable=yes");return 5;}
    std::printf("ActiveMenuClass=");Name(name);std::printf(" ActiveWidget=0x%llX ActiveType=0x%llX WidgetSerial=%d\n",
        static_cast<unsigned long long>(c.widget),static_cast<unsigned long long>(c.type),c.identities[2].serial);
    const bool exactPicker=pickercapture::Screen(kind,name.View());
    if(!exactPicker){
        std::puts("PickerCapture=ACTUAL_SCREEN_MISMATCH TypedPickerExtractionApproved=no NoRepeatOrEscalation=yes");
        std::uintptr_t native=0,ctor=0;
        if(!r.Read(base+pushdisk::MenuClass,native)||!r.Read(base+pushdisk::MenuConstructor,ctor)||
           !cfgprobe::MatchingClassCache(native,ctor)||!maps::Subclass(r,c.type,native)){
            std::puts("ObservedOwnerOnly=yes NativeMenuFamilyUnproven=yes");return 5;}
        std::puts("UnknownPickerOwnerNativeMenuFamilyObserved=yes PrimaryMetadataOnly=yes NoAliasOrCallApproval=yes");
    }
    cfgreflection::Observation<Reader> reflection(r,base);std::uintptr_t head=0;std::int32_t storage=0,selected=-1;
    if(!r.Read(c.type+cfgreflection::disk::Children,head)||!r.Read(c.type+cfgreflection::disk::StorageBytes,storage)||storage<4||storage>65536)return 6;
    std::array<cfgreflection::Field,pickercapture::MaximumFields> fields{};unsigned count=0;bool indexObserved=false;cfgreflection::Property indexProperty;
    std::puts("PickerCaptureStage=PrimaryFields NoChildWidgetGraphs=yes");
    for(auto child=head;child;){
        if(count==fields.size())return 6;for(unsigned i=0;i<count;++i)if(fields[i].object==child)return 6;
        auto& field=fields[count++];if(!reflection.ReadField(child,c.type,field))return 6;child=field.next;
        std::printf("PickerOwnedField=0x%llX Name=",static_cast<unsigned long long>(field.object));Name(field.label);std::printf(" Kind=");Name(field.kind);std::puts("");
        if(field.kind.View()==u"Function")continue;cfgreflection::Property property;if(!capture::RawProperty(r,field,property))return 6;Property("PickerProperty",property);
        if(exactPicker&&field.label.View()==u"CurrentIndex"){
            if(field.kind.View()!=u"IntProperty"||property.dimension!=1||property.elementBytes!=4||!pickercapture::Index(r,c.widget,property.offset,storage,selected))return 6;
            indexObserved=true;indexProperty=property;std::printf("PickerCurrentIndex=%d IndexSampleStable=yes\n",selected);}
    }
    if(exactPicker&&!indexObserved){
        cfgreflection::Field field;cfgreflection::Property property;
        if(reflection.Find(c.type,u"CurrentIndex",field)&&field.kind.View()==u"IntProperty"&&capture::RawProperty(r,field,property)&&
            property.dimension==1&&property.elementBytes==4&&pickercapture::Index(r,c.widget,property.offset,storage,selected)){
            Property("PickerInheritedCurrentIndex",property);indexObserved=true;indexProperty=property;
            std::printf("PickerCurrentIndex=%d IndexSampleStable=yes NativeHierarchyField=yes\n",selected);
        }
    }
    unsigned functions=0;bool graph=false,accept=false;std::puts("PickerCaptureStage=PrimaryFunctions");
    for(unsigned i=0;i<count;++i){const auto& field=fields[i];
        if(field.kind.View()!=u"Function"||!pickercapture::Function(field.label.View()))continue;if(functions==pickercapture::MaximumFunctions)return 6;
        const auto prefix=std::u16string_view(u"ExecuteUbergraph_");
        const bool isGraph=field.label.View().substr(0,prefix.size())==prefix;
        if(isGraph){
            auto evidence=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
            if(!capture::Graph(r,base,field,*evidence,diagnostic)){
                std::printf("PickerGraphRefused Stage=%s ScriptBytes=%d NoPartialSuccess=yes\n",diagnostic.stage,diagnostic.scriptBytes);return 6;}
            std::puts("RoutingGraphSnapshotOnly=yes AllLocalLayoutsObserved=no NativeCallsApproved=no");
            Function(*evidence);ReferenceCandidates(r,base,*evidence);++functions;graph=true;continue;
        }
        auto evidence=std::make_unique<capture::PickerFunctionEvidence>();capture::FunctionDiagnostic diagnostic;
        if(!capture::KnownFunction(r,base,field,field.label.View(),*evidence,&diagnostic)){
            std::printf("PickerFunctionRefused Name=");Name(field.label);
            std::printf(" Stage=%s LocalFields=%u ScriptBytes=%d LocalFieldLimit=256 ScriptByteLimit=65536 NoPartialSuccess=yes\n",
                diagnostic.stage,diagnostic.fields,diagnostic.scriptBytes);return 6;}
        Function(*evidence);ReferenceCandidates(r,base,*evidence);startbinding::FastCall fast;if(!startbinding::ReadFastCall(r,field.object,fast))return 6;
        std::printf("FastGraph=0x%llX FastEntry=%d\n",static_cast<unsigned long long>(fast.graph),fast.entry);++functions;
        graph|=field.label.View().substr(0,std::u16string_view(u"ExecuteUbergraph_").size())==u"ExecuteUbergraph_";
        accept|=field.label.View()==u"OnClick_Accept"||field.label.View()==u"OfflineBots_AcceptJason";
    }
    if(!exactPicker){
        std::uintptr_t again=0;if(!r.Read(c.type+cfgreflection::disk::Children,again)||again!=head||!active::Current(r,c,c.thread)||
            !ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
        for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];
            if(!reflection.ReadField(old.object,c.type,fresh)||fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return 7;}
        std::puts("ObservedOwnerAndPrimaryMetadataOnly=yes CaptureSampleStable=yes TypedPickerIndexAndSelectionRefused=yes NoPartialPickerSuccess=yes");return 5;
    }
    if(!indexObserved){std::puts("PickerCurrentIndexObserved=no NoSelectionFallback=yes");return 6;}
    std::puts("PickerCaptureStage=SelectionValues");
    for(unsigned i=0;i<count;++i){const auto& field=fields[i];const auto type=field.kind.View();if(type==u"Function")continue;
        cfgreflection::Property property;if(!capture::RawProperty(r,field,property))return 6;
        if(property.dimension!=1||property.offset>storage-property.elementBytes)continue;
        const auto at=c.widget+static_cast<unsigned>(property.offset);
        if((type==u"ClassProperty"||(type==u"ObjectProperty"&&pickercapture::SelectionPointer(field.label.View())))&&property.elementBytes==8&&property.offset%8==0){
            std::uintptr_t value=0,again=0;if(!r.Read(at,value)||!PickerReference(r,c,value)||!r.Read(at,again)||again!=value)return 6;
        }else if(type==u"ArrayProperty"&&property.elementBytes==16&&property.offset%8==0){
            std::uintptr_t inner=0,innerOwner=0,again=0;cfgreflection::Field innerField;cfgreflection::Property item;
            if(!r.Read(field.object+cfgreflection::arraydisk::Inner,inner)||!cfgprobe::ObjectPointer(inner)||!r.Read(inner+0x20,innerOwner)||
               !reflection.ReadField(inner,innerOwner,innerField)||!capture::RawProperty(r,innerField,item)||!r.Read(field.object+cfgreflection::arraydisk::Inner,again)||again!=inner)return 6;
            Property("PickerArrayInner",item);pickercapture::Array header;if(!r.Read(at,header))return 6;
            std::printf("PickerArray Name=");Name(field.label);std::printf(" UsedCount=%d ReservedCapacity=%d SelectedIndex=%d\n",header.count,header.capacity,selected);
            if(item.dimension!=1||item.offset!=0||!pickercapture::ArrayBounds(header,item.elementBytes,selected)){
                std::puts("PickerSelectedArrayRowObserved=no UnsupportedOrUnusedArrayShape=yes NoCapacityReads=yes");continue;}
            std::array<unsigned char,pickercapture::MaximumRowBytes> row{};if(!pickercapture::SelectedRow(r,at,item.elementBytes,selected,header,row))return 6;
            std::puts("PickerSelectedArrayRowHexBegin");for(int byte=0;byte<item.elementBytes;++byte)std::printf("%02X",row[static_cast<unsigned>(byte)]);std::puts("\nPickerSelectedArrayRowHexEnd");
            if((innerField.kind.View()==u"ClassProperty"||innerField.kind.View()==u"ObjectProperty")&&item.elementBytes==8){std::uintptr_t value=0;std::memcpy(&value,row.data(),8);if(!PickerReference(r,c,value))return 6;}
            if(innerField.kind.View()==u"StructProperty"&&!PickerStruct(r,c,item,row))return 6;
            std::puts("OpaqueSelectedRowNotLoadedOrDecoded=yes NoCapacityReads=yes");
        }
    }
    std::puts("PickerCaptureStage=NativeProfileSnapshot");profiles::Selection profile;profiles::Diagnostic diagnostic;unsigned profileReads=0;
    const bool observed=profiles::ObserveSnapshot(r,c,c.thread,true,profile,&profileReads,&diagnostic);
    std::printf("NativeProfileSnapshotObserved=%s Stage=%s Reads=%u SaveSerial=%d CounselorPick=0x%llX KillerPick=0x%llX ProfileNotAcceptedChoiceProof=yes\n",observed?"yes":"no",diagnostic.stage,profileReads,diagnostic.serial,
        static_cast<unsigned long long>(diagnostic.counselor),static_cast<unsigned long long>(diagnostic.jason));
    if(observed&&(!PickerReference(r,c,profile.counselor)||!PickerReference(r,c,profile.jason)))return 6;if(r.exhausted)return 6;
    std::uintptr_t final=0;if(!r.Read(c.type+cfgreflection::disk::Children,final)||final!=head)return 7;
    for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];
        if(!reflection.ReadField(old.object,c.type,fresh)||fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return 7;
        if(old.label.View()==u"CurrentIndex"){cfgreflection::Property property;std::int32_t last=-1;
            if(!capture::RawProperty(r,fresh,property)||!pickercapture::Index(r,c.widget,property.offset,storage,last)||last!=selected)return 7;}}
    if(!active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    cfgreflection::Field indexField;cfgreflection::Property finalIndexProperty;std::int32_t finalIndex=-1;
    if(!reflection.Find(c.type,u"CurrentIndex",indexField)||indexField.object!=indexProperty.field.object||indexField.type!=indexProperty.field.type||
       !capture::RawProperty(r,indexField,finalIndexProperty)||finalIndexProperty.flags!=indexProperty.flags||finalIndexProperty.dimension!=1||
       finalIndexProperty.elementBytes!=4||finalIndexProperty.offset!=indexProperty.offset||
       !pickercapture::Index(r,c.widget,finalIndexProperty.offset,storage,finalIndex)||finalIndex!=selected)return 7;
    std::printf("PickerPrimaryFunctions=%u PickerGraphObserved=%s PickerAcceptObserved=%s CaptureSampleStable=yes SnapshotIsNotAtomic=yes\n",functions,graph?"yes":"no",accept?"yes":"no");
    std::puts("PickerRouteInstalled=no FullDonorMenuParity=no GameplayTestReady=no");return graph&&accept?0:6;
}
