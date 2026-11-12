#pragma once
#include <cstdint>
#include "J215FeatureFlags.hpp"
namespace cfgpractice {
enum Option : std::uint32_t { PamelaMute=1,GrabMute=2,MinimalHud=4,HardMovement=8,BotSweater=16 };
constexpr std::uint32_t KnownMask=31;
constexpr std::uint32_t SupportedMask() noexcept {
    return (CFG_J215_PAMELA_MUTE?PamelaMute:0u)|(CFG_J215_GRAB_MUTE?GrabMute:0u)|
        (CFG_J215_MINIMAL_HUD?MinimalHud:0u)|(!CFG_J216_FINISHING&&CFG_J215_HARD_MOVEMENT?HardMovement:0u)|
        (!CFG_J216_FINISHING&&CFG_J215_BOT_SWEATER?BotSweater:0u);
}
constexpr bool Valid(std::uint32_t value) noexcept {return (value&~SupportedMask())==0;}
constexpr bool Enabled(std::uint32_t value,Option option) noexcept {return (value&option)!=0;}
constexpr bool HardAutomaticOrSelected(std::uint32_t value) noexcept {
    return CFG_J216_FINISHING||Enabled(value,HardMovement);
}
constexpr bool BotSweaterAutomaticOrSelected(std::uint32_t value) noexcept {
    return CFG_J216_FINISHING||Enabled(value,BotSweater);
}
}
