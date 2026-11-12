#include "MenuReflectionParameters.hpp"
#include <cstdio>
#include <limits>
#include <type_traits>
using namespace cfgmenu::reflection;
int main(){
    unsigned failures=0;
    const auto check=[&](bool ok){if(!ok)++failures;};
    CreateParameters create;check(Create(0x10000,0x20000,0x30000,create)&&create.result==0);
    check(!Create(1,0x20000,0x30000,create)&&create.world==0);
    AddParameters add;check(Add(0x10000,add)&&add.result==0);check(!Add(3,add)&&!add.content);
    OptionsSchema schema{24,{{{"Items",Kind::ArrayOfString,0,16,1,true},{"SelectedIndex",Kind::Int32,16,4,1,true}}}};
    const std::u16string_view labels[]={u"Off",u"Rain",u"Random"};OptionsPacket packet;
    check(packet.Build(schema,labels,3,2)&&packet.Size()==24);
    Array array{};std::int32_t selected=0;
    std::memcpy(&array,packet.Data(),16);std::memcpy(&selected,static_cast<unsigned char*>(packet.Data())+16,4);
    check(array.count==3&&array.capacity==3&&selected==2&&array.data[1].count==5&&array.data[1].data[4]==0);
    check(std::u16string_view(array.data[2].data)==u"Random");
    auto bad=schema;bad.fields[1].offset=8;check(!packet.Build(bad,labels,3,0)&&!packet.Data()&&!packet.Size());
    bad=schema;bad.fields[0].offset=std::numeric_limits<unsigned>::max();check(!packet.Build(bad,labels,3,0));
    bad=schema;bad.fields[1].name="Items";check(!packet.Build(bad,labels,3,0));
    bad=schema;bad.fields[0].input=false;check(!packet.Build(bad,labels,3,0));
    bad=schema;bad.frameBytes=513;check(!packet.Build(bad,labels,3,0));
    check(!packet.Build(schema,labels,3,3));check(!packet.Build(schema,nullptr,3,0));check(!packet.Build(schema,labels,33,0));
    const char16_t embedded[]={u'A',0,u'B'};const std::u16string_view invalid[]={std::u16string_view(embedded,3)};
    check(!packet.Build(schema,invalid,1,0));
    check(packet.Build(schema,labels,1,0));
    static_assert(!std::is_move_constructible_v<OptionsPacket> && !std::is_copy_constructible_v<OptionsPacket>);
    std::printf("MenuReflectionTests: %u failures; packet-only, no engine access\n",failures);
    return failures?1:0;
}
