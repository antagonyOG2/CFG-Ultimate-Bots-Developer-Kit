// Read-only strong ownership evidence for this native AddChild return only.
// Caller separately proves exact function/frame and authored destination panel.
namespace cfghostedlobbyoutput {
template<class R>bool Current(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t panel,std::uintptr_t value,std::uintptr_t expectedContent){
    std::uintptr_t type=0,content=0,parent=0;cfgpracticeobs::WidgetNode::List list{},again{};
    if(!cfgpracticeobs::Typed(r,registry,module,value,cfgpractice::disk::PanelSlotReflection)||
       !r.Read(value+cfgprobe::ObjectClass,type)||!r.Read(value+cfgpractice::disk::Content,content)||content!=expectedContent)return false;
    cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field;cfgreflection::Property property;
    if(!reflection.Find(type,u"Parent",field)||!reflection.ReadProperty(field,property)||field.kind.View()!=u"ObjectProperty"||
       property.dimension!=1||property.elementBytes!=8||property.offset%8||!r.Read(value+static_cast<unsigned>(property.offset),parent)||
       parent!=panel||!r.Read(panel+cfgpractice::disk::Slots,list)||!list.Sane())return false;
    unsigned matches=0;for(int i=0;i<list.count;++i){std::uintptr_t slot=0;
        if(!r.Read(list.data+8ULL*static_cast<unsigned>(i),slot))return false;if(slot==value)++matches;}
    std::uintptr_t contentAgain=0,parentAgain=0;
    return matches==1&&r.Read(panel+cfgpractice::disk::Slots,again)&&again==list&&
        r.Read(value+cfgpractice::disk::Content,contentAgain)&&contentAgain==expectedContent&&
        r.Read(value+static_cast<unsigned>(property.offset),parentAgain)&&parentAgain==panel;
}
}
