#pragma once
#ifndef CFG_H247_SUMMARY_BOTS
#define CFG_H247_SUMMARY_BOTS 0 // H2.48: off until re-enabled alone after a clean live run (H2.47 driver regression unexplained)
#endif
namespace cfghostedwidget {
enum class Route:unsigned {FullJ219,ForwardOnly,SpectatorStatusOnly};
// `hostedWorld` describes the world, not ownership of the current H1 driver.
// It must remain true through the hosted outro after the driver freezes.
constexpr Route Select(bool hostedWorld,bool statusWidget)noexcept{
    return !hostedWorld?Route::FullJ219:(statusWidget?Route::SpectatorStatusOnly:Route::ForwardOnly);
}
// H2.47: hosted non-status widgets are still forwarded untouched, but the end-of-match
// summary presentation (J2.19 BundleEndingSummaryService) gets its widget opportunity.
// H2.48: only inside the end-frozen summary window (HostedSummaryWindow), never mid-match.
constexpr bool SummaryOpportunity(Route route,bool endWindow)noexcept{return CFG_H247_SUMMARY_BOTS&&endWindow&&route==Route::ForwardOnly;}
}
