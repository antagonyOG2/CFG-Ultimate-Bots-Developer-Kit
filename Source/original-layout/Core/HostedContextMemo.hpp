#pragma once

namespace cfghostedcontext {
// A positive proof may be reused only for the lifetime of its owning callback.
// The owner invalidates it before every native callback that could change the
// adopted world or actor links.
class CallbackMemo {
    bool proven_=false;
public:
    template<class Prove> bool Get(Prove&& prove){
        if(proven_)return true;
        if(!prove())return false;
        proven_=true;return true;
    }
    void Invalidate()noexcept{proven_=false;}
    bool Proven()const noexcept{return proven_;}
};
}
