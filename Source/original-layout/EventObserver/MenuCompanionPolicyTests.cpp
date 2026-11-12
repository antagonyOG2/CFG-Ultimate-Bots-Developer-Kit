#include "../Core/MenuCompanionPolicy.hpp"
#include "NativeSetupRows.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    using namespace cfgmenu::companion;unsigned value=0;
    std::string output(65536,'x');
    check(!AppendOutput(output,"AIStartupComplete=yes\n",false)&&output.size()==65536);
    check(AppendOutput(output,"AIStartupComplete=yes\n",true)&&output.find("AIStartupComplete=yes")!=output.npos);
    std::string maximum(OutputBudget(true),'x');
    check(AppendOutput(maximum,"",true)&&!AppendOutput(maximum,"x",true)&&maximum.size()==OutputBudget(true));
    std::string blocks;
    for(unsigned i=0;i<240;++i)check(AppendOutput(blocks,std::string(12000,'x'),true));
    check(AppendOutput(blocks,"\nAIStartupComplete=yes\n",true));
    check(Number("PID=5\nFrontendState=10 (ready)\n","FrontendState",value)&&value==10);
    check(Number("FrontendState=3\r\n","FrontendState",value)&&value==3);
    check(!Number("NotFrontendState=3\n","FrontendState",value));
    check(!Number("FrontendState=\n","FrontendState",value));
    check(!Number("FrontendState=-1\n","FrontendState",value));
    check(!Number("FrontendState=4294967296\n","FrontendState",value));
    check(!Instruction(9).empty()&&!Instruction(3).empty()&&Finished(5)&&!Finished(7)&&!Finished(10));
    check(Instruction(8).find(L"Offline Play")!=std::wstring_view::npos&&
        Instruction(8).find(L"Back out")==std::wstring_view::npos&&
        Instruction(8).find(L"Play as a Counselor")!=std::wstring_view::npos);
    check(Instruction(8,1,7,6).find(L"Wait")!=std::wstring_view::npos);
    check(Instruction(8,1,8,5).find(L"Wait")!=std::wstring_view::npos);
    for(unsigned order=0;order<=9;++order)check((Instruction(8,1,8,6,order).find(L"Native counselor entry ready")!=std::wstring_view::npos)==(order==4||order==9));
    check(Instruction(8,1,8,6,9).find(L"original order preserved")!=std::wstring_view::npos);
    check(Instruction(8,1,8,6).find(L"Wait")!=std::wstring_view::npos);
    for(const auto route:{3u,4u})check(Instruction(8,route,8,6,4).find(L"press X")!=std::wstring_view::npos&&
        Instruction(8,route,8,6,4).find(L"A customizes")!=std::wstring_view::npos);
    check(Instruction(8,7,8,6,4).find(L"map menu ready")!=std::wstring_view::npos);
    Lines stream;unsigned state=0,records=0;
    const auto sink=[&](std::string_view line){unsigned parsed=0;if(Number(line,"FrontendState",parsed)){state=parsed;++records;}};
    check(stream.Feed("FrontendSta",sink)&&!records);
    check(stream.Feed("te=10 (ready)\r\nNoise=5\nFrontendState=",sink)&&records==1&&state==10);
    check(stream.Feed("3\n",sink)&&records==2&&state==3);
    Lines excessive;check(!excessive.Feed(std::string(4097,'x'),sink));
    cfgmenu::rows::Spec status;status.id=cfgmenu::RowId::Weather;status.title=u"Offline Bots - Play as a Counselor";
    status.count=1;status.labels[0]=u"AI Jason - Automatic";status.informationOnly=true;status.preserveNativeValue=true;
    check(status.Valid());status.count=2;status.labels[1]=u"Off";check(!status.Valid());
    status.count=1;status.id=cfgmenu::RowId::Count;check(!status.Valid());
    std::printf("CFG windowed companion/status-row policy: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
