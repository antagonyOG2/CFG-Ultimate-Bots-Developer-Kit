#pragma once
#include <cstdint>
namespace cfgsweaterowner {
// A changed local counselor is an independent owner and may wake an otherwise
// retired previous-owner scan. Do not migrate during publication or an
// exclusive kill transaction. Ordinary lifecycle polls retain their cadence.
inline bool PollDue(bool lifecycleCanPoll,bool lateMaskRetryPending,bool ownerChanged,
    bool transactionExclusive,bool publicationStarted,std::uint64_t next,std::uint64_t now){
    const bool ordinaryDue=lifecycleCanPoll&&now>=next;
    const bool lateRetryDue=lateMaskRetryPending&&!transactionExclusive&&!publicationStarted&&now>=next;
    const bool ownerRefreshDue=ownerChanged&&!transactionExclusive&&!publicationStarted;
    return ordinaryDue||lateRetryDue||ownerRefreshDue;
}
}
