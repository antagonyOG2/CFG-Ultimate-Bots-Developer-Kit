// Include inside BundleAccess. Only our confirmed spawned helper roster is
// eligible. Service one pending slot once, then never scan/reapply its name.
    bool BotNamePhase(){
        auto& q=bundleBotNames;if(q.cursor>=bundleReport.bots)return false;
        const auto index=q.cursor++;q.failure=0;
        const auto refuse=[&](unsigned why){q.failure=why;++q.skipped;return true;};
        if(!cfgbotname::disk::SetterVerified)return refuse(9);
        if(!Context()||index>=cfgbotname::Maximum)return refuse(1);
        const auto& bot=bundleReport.roster[index];std::uintptr_t state=0,humanState=0,type=0,table=0;
        cfgprobe::lifetime::SlotObservation stateSlot{},fresh{};
        const auto identity=[&](){return bot.pawn&&bot.controller&&bot.pawn!=plan.pawn&&bot.controller!=plan.controller&&
            cfgjason::SameSlot(r_,context.objectArray,bundlePawnSlots[index])&&
            cfgjason::SameSlot(r_,context.objectArray,bundleControllerSlots[index])&&
            cfgspawn::ObjectIs(r_,bot.pawn,bundleCounselorClass)&&
            cfgspawn::ObjectIs(r_,bot.controller,spawnAssets.resources.classes[cfgspawn::CounselorAI])&&
            cfgobserver::Same(r_,bot.pawn+0x3A0,bot.controller)&&cfgobserver::Same(r_,bot.controller+0x370,bot.pawn);};
        if(!identity()||!r_.Read(bot.controller+0x388,state)||!state||
           !r_.Read(plan.controller+0x388,humanState)||state==humanState||
           !cfgbotname::OwnedState(r_,context.objectArray,bot.pawn,bot.controller,state,
                spawnAssets.resources.classes[cfgspawn::PlayerState],plan.pawn,plan.controller,humanState,stateSlot))return refuse(2);
        for(const auto claimed:q.claimedStates)if(claimed==state)return refuse(8);
        if(!r_.Read(plan.module+cfgbotname::disk::StateClass,type)||!type||
           !cfgobserver::Same(r_,plan.module+cfgbotname::disk::StateConstructor,type)||!cfgspawn::ObjectIs(r_,state,type)||
           !r_.Read(state,table)||!cfgobserver::Same(r_,table+cfgbotname::disk::SetterSlot,plan.module+cfgbotname::disk::Setter)||
           !cfgbundle::Code(r_,plan.module,cfgbotname::disk::Setter,cfgbotname::disk::SetterCode)||
           !cfgbundle::Code(r_,plan.module,cfgbotname::disk::BaseSetter,cfgbotname::disk::BaseSetterCode)||
           !cfgbundle::Code(r_,plan.module,cfgbotname::disk::StringCopy,cfgbotname::disk::StringCopyCode))return refuse(3);
        auto name=cfgbotname::Make(index,q.seed);cfgbotname::String packet{name.text.data(),name.count,name.count};
        if(name.count<2||!Context()||!identity()||!cfgobserver::Same(r_,bot.controller+0x388,state)||
           !cfgbotname::OwnedState(r_,context.objectArray,bot.pawn,bot.controller,state,type,
                plan.pawn,plan.controller,humanState,fresh)||!(fresh==stateSlot)||
           !cfgobserver::Same(r_,state,table)||calls_>=cfgbundle::CallCap||!NativeEnvironmentAllowed())return refuse(4);
        q.claimedStates[index]=state;++q.attempts;
        if(!BotNameNative(state,packet,bot.pawn,bot.controller))return refuse(5);
        cfgbotname::String actual{};std::array<wchar_t,32> text{};
        if(!identity()||!cfgbotname::OwnedState(r_,context.objectArray,bot.pawn,bot.controller,state,type,
                plan.pawn,plan.controller,humanState,fresh)||!(fresh==stateSlot)||
           !r_.Read(state+cfgbotname::disk::PlayerName,actual)||!actual.data||actual.count!=name.count||actual.capacity<actual.count||actual.capacity>4096||
           !r_.ReadBytes(reinterpret_cast<std::uintptr_t>(actual.data),text.data(),static_cast<std::size_t>(actual.count)*sizeof(wchar_t)))return refuse(6);
        for(int i=0;i<name.count;++i)if(text[static_cast<unsigned>(i)]!=name.text[static_cast<unsigned>(i)])return refuse(7);
        ++q.confirmed;return true;
    }
#include "BotNameNativeMembers.hpp"
