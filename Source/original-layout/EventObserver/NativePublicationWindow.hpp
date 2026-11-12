#pragma once
#include <cstdint>
namespace cfgretain {
struct PublicationLimits {unsigned attempts=8;std::uint64_t milliseconds=8;};
// Startup runs expensive independent owner proofs and Windows scheduling/time
// granularity can consume the former8ms limit before a second lock attempt.
// Keep eight nonblocking attempts, with a separate finite startup-only cutoff.
constexpr PublicationLimits StartupPublicationLimits{8,64};
// Retry only failed NONBLOCKING engine-lock acquisition, before publication
// or native work. Each attempt must release all locks before yielding. A
// refused context, code pin, claim or entered work is terminal, never replayed.
template<class Context,class Attempt,class Clock,class Yield>
bool PublicationWindow(Context&& context,Attempt&& attempt,Clock&& clock,Yield&& yield,
    unsigned& attempts,bool* preWorkContention=nullptr,PublicationLimits limits={})noexcept{
    attempts=0;if(preWorkContention)*preWorkContention=false;const auto started=clock();
    if(!limits.attempts||limits.attempts>8||!limits.milliseconds||limits.milliseconds>64)return false;
    while(attempts<limits.attempts){
        if(attempts&&clock()-started>=limits.milliseconds)return false;
        if(!context()){if(preWorkContention)*preWorkContention=false;return false;}
        bool acquired=false,contended=false;++attempts;
        const bool result=attempt(acquired,contended);
        // Begin may have acquired/released the hash lock while refusing the
        // list lock. Only an explicitly contended, fully unwound PRE-WORK
        // attempt qualifies. Any guard or acquired-operation failure does not.
        if(preWorkContention)*preWorkContention=!result&&!acquired&&contended;
        if(result||acquired||!contended||attempts==limits.attempts||clock()-started>=limits.milliseconds)return result;
        yield();
    }
    return false;
}
}
