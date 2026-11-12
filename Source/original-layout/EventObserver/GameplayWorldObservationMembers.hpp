    bool GameplayWorld(cfgobserver::Plan& out){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            // HostedContext already performs the fresh retained-world proof and
            // stores its result in context. Reuse that exact proof for the
            // immediately following caller instead of spending the same
            // registry/world walk a second time in one callback.
            if(!HostedContext(false))return false;
            out=context;return true;
        }
#endif
        return cfgprimary::RetainedOfflineWorld(r_,plan,out);
    }

