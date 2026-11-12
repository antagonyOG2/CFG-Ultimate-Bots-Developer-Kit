#define CFG_J215_HUD_TEST_MAIN ExistingHudPathFixture
#include "J215PracticeObservationTests.cpp"
#include "NativeChallengeSettingsSpecs.hpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay)++failures;};
 WidgetFixture f;namespace d=cfgpractice::disk;f.Slot(f.WidgetType,21,121);f.Label(f.Child,"UnprovedArc");f.Label(f.WidgetType,"Image");
 f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
 f.r.Set(f.Child+d::Visibility,std::uint8_t{0});cfgprobe::lifetime::ArrayHeader array;
 check(cfgprobe::lifetime::ReadArrayHeader(f.r,f.Module+cfgprobe::lifetime::ObjectArrayRva,array));
 auto node=cfgpracticeobs::Diagnostic(f.r,f.Module,array,f.Child,1,0);
 check(std::string_view(node.name.data())=="UnprovedArc"&&std::string_view(node.type.data())=="Image"&&node.visibility==0);
 cfgpractice::HudDiagnostics report;report.Visible(node);check(report.visibleCount==1&&report.Valid());
 report.Visible(node);check(report.visibleCount==1);
 // Names are evidence, not a target classifier. Existing target set is not changed.
 for(unsigned i=0;i<80;++i){node.object=f.Child+i*16;report.Visible(node);}check(report.visibleCount==64&&report.visibleTruncated&&report.Valid());
 report.complete=1;node.object=0xFFFF;report.Visible(node);check(report.visibleCount==64);
 for(unsigned reason=1;reason<=6;++reason){node.reason=reason;report.Refused(node);report.Refused(node);}
 check(report.refusedCount==6&&report.Valid());
 for(unsigned i=0;i<30;++i){node.object=100+i;report.Refused(node);}check(report.refusedCount==16&&report.refusedTruncated&&report.Valid());
 auto invalid=report;invalid.visible[0].name.back()='x';check(!invalid.Valid());invalid=report;invalid.refused[0].reason=7;check(!invalid.Valid());
 const auto specs=cfgmenu::challengemenu::Specs();check(specs.size()==8);
 for(const auto& s:specs)check(s.id!=cfgmenu::RowId::SkipReplay);
 check(specs[cfgmenu::challengemenu::choice::PreventEscapes].id==cfgmenu::RowId::PreventEscapes&&
  specs[cfgmenu::challengemenu::choice::Boundary].id==cfgmenu::RowId::Boundary&&specs[cfgmenu::challengemenu::choice::EdgeWalls].id==cfgmenu::RowId::EdgeWalls);
 std::array<unsigned,cfgmenu::challengemenu::choice::Count> choices{};choices.fill(1);check(!cfgmenu::challengemenu::ReplayChoice(choices));
 std::printf("J217 actual HUD name/class/visibility, refusal reasons/dedup/64cap/one-shot, removed replay row: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
