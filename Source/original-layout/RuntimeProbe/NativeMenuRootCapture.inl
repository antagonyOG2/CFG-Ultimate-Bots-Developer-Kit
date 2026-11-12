// One-shot serialized menu-class candidates; NOT an instruction decoder.
// Only current captured graph literals, never global object/name discovery.
bool ParentMenuClassCandidates(Reader& r,const cfgmenu::active::Context& c,std::uintptr_t menu,
    const cfgmenu::capture::GraphSnapshot& graph,std::array<std::uintptr_t,2>& custom,unsigned& customCount){
    cfgnames::Observation<Reader> names(r,c.module);unsigned count=0;
    std::array<std::uintptr_t,32> seen{};
    for(unsigned at=0;at+9<=static_cast<unsigned>(graph.script.count);++at){
        if(graph.bytes[at]!=0x89)continue;std::uintptr_t candidate=0,kind=0;
        std::memcpy(&candidate,graph.bytes.data()+at+1,8);if(!cfgprobe::ObjectPointer(candidate))continue;
        cfgnames::Text label,kindName;
        if(!r.Read(candidate+cfgprobe::ObjectClass,kind)||!names.Object(kind,kindName)||
           kindName.View()!=u"WidgetBlueprintGeneratedClass"||!names.Object(candidate,label)||!cfgmenu::maps::Subclass(r,candidate,menu))continue;
        bool repeated=false;for(unsigned i=0;i<count;++i)repeated|=seen[i]==candidate;if(repeated)continue;
        if(count==seen.size()){std::puts("NativeParentClassCandidatesRefused Stage=32ClassBound");return false;}seen[count++]=candidate;
        cfgprobe::lifetime::SlotObservation slot;if(!cfgprobe::lifetime::ReadSlot(r,c.registry,candidate,slot))return false;
        std::printf("NativeParentMenuClassCandidate Offset=0x%X Class=0x%llX Name=",at,static_cast<unsigned long long>(candidate));Name(label);
        std::printf(" Serial=%d InstructionDecoded=no CallApproved=no\n",slot.serial);
        if(label.View().find(u"Custom")!=std::u16string_view::npos){
            bool old=false;for(unsigned i=0;i<customCount;++i)old|=custom[i]==candidate;
            if(!old){if(customCount==custom.size()){std::puts("NativeCustomizationDependencyRefused Stage=2ClassBound");return false;}custom[customCount++]=candidate;}}
    }
    return !r.exhausted;
}
// One-shot evidence of retained native menu-stack CLASS sources, never discovery.
int MenuRootCapture(HANDLE process,std::uintptr_t base,Reader& r){
    using namespace cfgmenu;active::Context c;if(!active::Observe(r,base,c))return 5;
    cfgnames::Observation<Reader> names(r,base);cfgreflection::Observation<Reader> reflection(r,base);
    cfgnames::Text label;std::uintptr_t menu=0,ctor=0,stackOwner=0,stackCtor=0,stackType=0;
    if(!names.Object(c.type,label)||!ActualOfflineEntryScreen(label.View())){
        std::puts("NativeMenuRoots=REFUSED_ACTUAL_OFFLINE_ENTRY_SCREEN NoWrites=yes");return 5;
    }
    if(!r.Read(base+transitiondisk::Owner,menu)||!r.Read(base+transitiondisk::Constructor,ctor)||
       !cfgprobe::MatchingClassCache(menu,ctor)||!r.Read(base+parentdisk::Owner,stackOwner)||
       !r.Read(base+parentdisk::Constructor,stackCtor)||!cfgprobe::MatchingClassCache(stackOwner,stackCtor)||
       !r.Read(c.stack+cfgprobe::ObjectClass,stackType)||!maps::Subclass(r,stackType,stackOwner)||!parentcapture::NativeCode(r,base)){
        std::puts("NativeMenuRootsRefused Stage=NativeStackOwnerOrPreviousClassCode");return 6;}
    std::printf("CurrentMenu=");Name(label);std::printf(" NativeStackUsedCount=%d ParentProbeLimit=2 NoGlobalDiscovery=yes\n",c.array.count);
    unsigned sampled=0,customCount=0;std::array<std::uintptr_t,2> types{},custom{};
    std::array<parentcapture::Record,2> records{};std::array<cfgprobe::lifetime::SlotObservation,2> classIdentities{};
    std::vector<std::unique_ptr<capture::GraphSnapshot>> graphs;
    for(int index=c.array.count-2;index>=0&&sampled<types.size();--index){
        parentcapture::Record record;cfgprobe::lifetime::SlotObservation classSlot,widgetSlot;
        if(!parentcapture::Observe(r,c,index,record)){
            std::printf("NativeMenuRootsRefused Stage=RetainedClassRecord StackIndex=%d\n",index);return 6;}
        const auto type=record.menuClass,widget=record.widget;
        if(!names.Object(type,label)||!maps::Subclass(r,type,menu)||!cfgprobe::lifetime::ReadSlot(r,c.registry,type,classSlot)){
            std::printf("NativeMenuRootsRefused Stage=NativeParentClassFamily StackIndex=%d\n",index);return 6;}
        if(widget){std::uintptr_t actual=0;if(!r.Read(widget+cfgprobe::ObjectClass,actual)||actual!=type||
            !cfgprobe::lifetime::ReadSlot(r,c.registry,widget,widgetSlot)){
                std::printf("NativeMenuRootsRefused Stage=OptionalParentWidget StackIndex=%d\n",index);return 6;}}
        bool seen=false;for(unsigned i=0;i<sampled;++i)seen|=types[i]==type;if(seen)continue;
        records[sampled]=record;classIdentities[sampled]=classSlot;types[sampled++]=type;
        std::printf("NativeParentWidget=0x%llX Class=",static_cast<unsigned long long>(widget));Name(label);
        std::printf(" ClassSerial=%d WidgetSerial=%d StackIndex=%d SourceIsNativeRetainedClass=yes WidgetRequired=no CallApproved=no\n",classSlot.serial,widgetSlot.serial,index);
        std::uintptr_t head=0;std::int32_t storage=0;
        if(!r.Read(type+cfgreflection::disk::Children,head)||!r.Read(type+cfgreflection::disk::StorageBytes,storage)||storage<8||storage>65536)return 6;
        std::array<cfgreflection::Field,192> fields{};unsigned count=0,functions=0;
        for(auto child=head;child;){
            if(count==fields.size())return 6;for(unsigned i=0;i<count;++i)if(fields[i].object==child)return 6;
            auto& field=fields[count++];if(!reflection.ReadField(child,type,field))return 6;child=field.next;
            const auto kind=field.kind.View();
            if(kind==u"ClassProperty"||kind==u"SoftClassProperty"||
               (kind==u"ObjectProperty"&&(pickercapture::SelectionPointer(field.label.View())||field.label.View().find(u"Menu")!=std::u16string_view::npos))){
                cfgreflection::Property property;if(!capture::RawProperty(r,field,property))return 6;Property("NativeParentClassSourceField",property);
                if(widget&&(kind==u"ClassProperty"||kind==u"ObjectProperty")&&property.dimension==1&&property.elementBytes==8&&
                   property.offset%8==0&&property.offset<=storage-8){
                    std::uintptr_t value=0,again=0;if(!r.Read(widget+static_cast<unsigned>(property.offset),value)||!PickerReference(r,c,value)||
                       !r.Read(widget+static_cast<unsigned>(property.offset),again)||again!=value)return 6;
                }
            }else if(kind==u"Function"&&field.label.View().substr(0,17)==u"ExecuteUbergraph_"){
                if(functions++)return 6;auto graph=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
                if(!capture::Graph(r,base,field,*graph,diagnostic)){
                    std::printf("NativeParentGraphRefused Stage=%s ScriptBytes=%d\n",diagnostic.stage,diagnostic.scriptBytes);return 6;}
                std::puts("RoutingGraphSnapshotOnly=yes AllLocalLayoutsObserved=no NativeCallsApproved=no");Function(*graph);
                if(!ParentMenuClassCandidates(r,c,menu,*graph,custom,customCount))return 6;
                graphs.push_back(std::move(graph));
            }
        }
        std::uintptr_t again=0;cfgprobe::lifetime::SlotObservation final;
        if(!parentcapture::Current(r,c,record)||!r.Read(type+cfgreflection::disk::Children,again)||again!=head||
           !cfgprobe::lifetime::ReadSlot(r,c.registry,type,final)||!(final==classSlot))return 7;
        if(widget&&(!cfgprobe::lifetime::ReadSlot(r,c.registry,widget,final)||!(final==widgetSlot)))return 7;
        for(unsigned i=0;i<count;++i){cfgreflection::Field fresh;const auto& old=fields[i];if(!reflection.ReadField(old.object,type,fresh)||
            fresh.type!=old.type||fresh.next!=old.next||!(fresh.name==old.name))return 7;}
    }
    // At most two already loaded customization graph dependencies. Native
    // class constants only; no CDO factory, loader, registry or package scan.
    for(unsigned i=0;i<customCount;++i){cfgnames::Text name;
        if(!names.Object(custom[i],name)||name.View().size()<2||name.View().substr(name.View().size()-2)!=u"_C")return 6;
        std::u16string graphName=u"ExecuteUbergraph_";graphName.append(name.View().data(),name.View().size()-2);
        cfgreflection::Field field;auto graph=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
        if(!reflection.Find(custom[i],graphName,field)||!capture::Graph(r,base,field,*graph,diagnostic)){
            std::printf("NativeCustomizationGraphRefused Stage=%s\n",diagnostic.stage);return 6;}
        std::printf("NativeCustomizationDependencyClass=0x%llX Name=",static_cast<unsigned long long>(custom[i]));Name(name);std::puts(" MetadataOnly=yes CallApproved=no");
        Function(*graph);if(!ParentMenuClassCandidates(r,c,menu,*graph,custom,customCount))return 6;
        graphs.push_back(std::move(graph));
    }
    for(const auto& saved:graphs){auto fresh=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
        if(!capture::Graph(r,base,saved->function,*fresh,diagnostic)||!(fresh->script==saved->script)||!(fresh->slot==saved->slot)||
           fresh->flags!=saved->flags||fresh->parameterBytes!=saved->parameterBytes||fresh->storageBytes!=saved->storageBytes||
           fresh->nativeExecutor!=saved->nativeExecutor||fresh->destructorLink!=saved->destructorLink||fresh->bytes!=saved->bytes)return 7;}
    for(unsigned i=0;i<sampled;++i){cfgprobe::lifetime::SlotObservation identity;
        if(!parentcapture::Current(r,c,records[i])||!cfgprobe::lifetime::ReadSlot(r,c.registry,types[i],identity)||!(identity==classIdentities[i]))return 7;}
    if(!active::Current(r,c,c.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    std::printf("NativeParentOwnersCaptured=%u CaptureSampleStable=yes SnapshotIsNotAtomic=yes NoLoadOrCallAuthority=yes\n",sampled);
    return 0;
}
