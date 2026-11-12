        bool TickJasonAIKnifeSequenceOnGameThread(std::uint64_t time){
            if(!KnifeSequenceActive())return false;
            bundleKnife.Tick(a,time);return true;
        }
