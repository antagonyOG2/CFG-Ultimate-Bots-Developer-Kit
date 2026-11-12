#pragma once
#include <memory>
#include <utility>
namespace cfgstartupreuse {
template<class Lease>bool Retirable(bool finished,bool callback,bool admitted,bool healthy,const Lease& lease)noexcept{
    return finished&&!callback&&!admitted&&healthy&&lease.EmptyIdle();
}
// Called only after the actual native Intro/Match entry and current owner were
// independently proved. Renewal additionally requires a freshly Armed startup.
// Keep the resident collector bank; replace only a completed empty local lease.
template<class Session,class Factory>bool Prepare(std::unique_ptr<Session>& session,
    bool intro,bool freshlyArmed,bool occupied,Factory&& factory){
    if(occupied)return false;
    if(session&&intro&&freshlyArmed){
        if(!session->Retirable())return false;
        auto replacement=factory();if(!replacement)return false;session=std::move(replacement);
    }
    if(!session){if(!intro||!freshlyArmed)return false;session=factory();}
    return session!=nullptr;
}
}
