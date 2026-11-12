#pragma once
namespace cfglimits {
// Match the donor's ten-total counselor option. One is the native local human;
// extra helpers use the same paced birth pipeline and never a burst spawn.
constexpr unsigned MaximumHelpers=9;
constexpr unsigned MaximumCounselors=MaximumHelpers+1;
constexpr unsigned TerminalMaskLimit=1u<<MaximumCounselors;
}
