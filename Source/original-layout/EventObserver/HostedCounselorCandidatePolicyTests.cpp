#include "../Core/HostedCounselorCandidatePolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;auto c=[&](bool ok){++checks;if(!ok)++failures;};
    c(cfghostedcounselor::SupportedKind(u"SoftClassProperty"));c(cfghostedcounselor::SupportedKind(u"ClassProperty"));
    c(cfghostedcounselor::SupportedKind(u"NameProperty"));c(cfghostedcounselor::SupportedKind(u"FNameProperty"));
    c(cfghostedcounselor::SupportedKind(u"StrProperty"));c(!cfghostedcounselor::SupportedKind(u"ObjectProperty"));
    c(cfghostedcounselor::PathLike(u"/Game/Counselors/AJ"));c(cfghostedcounselor::PathLike(u"/Counselors/Vanessa"));
    c(cfghostedcounselor::PathLike(u"PickedCounselorClass"));c(!cfghostedcounselor::PathLike(u"/Game/Items/Knife"));
    std::printf("Hosted counselor candidate filter: %u checks, %u failures (read-only).\n",checks,failures);
    return failures?1:0;
}
