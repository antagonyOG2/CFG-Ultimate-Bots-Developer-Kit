#pragma once
#include "NativeSetupRows.hpp"
#include "../Core/HostedOptions.hpp"
namespace cfgmenu::hostedsettings {
#if CFG_H20_MENU
inline constexpr std::array<RowId,4> Ids{{RowId::OnlineJason,RowId::OnlineDifficulty,RowId::OnlineFill,RowId::OnlineJasonCount}};
inline std::array<rows::Spec,4> Specs(const cfghostedoptions::Values& current){
    std::array<rows::Spec,4> result{};
    constexpr std::array<std::u16string_view,4> titles{{u"Online: Jason Bot",u"Online: Jason Bot Difficulty",u"Online: Fill Empty Slots with Counselor Bots",u"Online: Number of Jasons"}};
    const std::array<unsigned,4> selected{{current.jason,current.difficulty,current.fill,current.jasonCount-1}};
    for(unsigned i=0;i<result.size();++i){auto& s=result[i];s.id=Ids[i];s.title=titles[i];s.selected=selected[i];s.count=2;
        s.labels[0]=u"Off";s.labels[1]=u"On";s.preserveNativeValue=true;}
    result[1].count=4;result[1].labels[1]=u"Easy";result[1].labels[2]=u"Normal";result[1].labels[3]=u"Hard";
    result[3].count=2;result[3].labels[0]=u"1";result[3].labels[1]=u"2";return result;
}
template<class Host>bool Save(rows::Runtime& rows,Host& host,std::uint64_t generation,cfghostedoptions::Store& store){
    if(!host.Current(generation))return false;
    std::array<unsigned,rows::Capacity> choices{};
    if(!rows.Choices(host,generation,choices))return false;
    std::array<unsigned,4> values{};unsigned mask=0;
    for(unsigned i=0;i<rows.Count();++i)for(unsigned j=0;j<Ids.size();++j)if(rows.At(i).spec.id==Ids[j]){
        if(mask&(1u<<j))return false;mask|=1u<<j;values[j]=choices[i];}
    return mask==15&&store.Save({values[0],values[1],values[2],values[3]+1});
}
#endif
}
