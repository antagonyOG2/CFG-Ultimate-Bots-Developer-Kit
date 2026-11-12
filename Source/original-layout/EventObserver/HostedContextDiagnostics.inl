// Read-only failure evidence, outside the bounded admission. Never a call grant.
template<class R>void Describe(R& r,const cfghosted::native::Admission& a,const cfghosted::native::Trace& trace,ContextTrace& out){
 out={};out.stage=trace.stage;out.object=trace.object;out.expected=trace.expected;
 cfgnames::Observation<R> names(r,a.module);
 const auto copy=[](auto& target,std::u16string_view text){unsigned i=0;for(auto ch:text){if(i+1==target.size())break;target[i++]=ch>0&&ch<128?static_cast<char>(ch):'?';}};
 const auto name=[&](std::uintptr_t object,auto& target){std::uintptr_t type=0;cfgnames::Text text;if(r.Read(object+cfgprobe::ObjectClass,type)&&names.Object(type,text))copy(target,text.View());};
 std::uintptr_t mode=0,state=0,instance=0,player=0,controller=0;cfghosted::listen35::LocalPlayers locals{};
 if(r.Read(a.world+cfgprobe::WorldGameMode,mode)){name(mode,out.mode);cfgnames::Words word;cfgnames::Text text;if(r.Read(mode+cfgescape::disk::MatchState,word)&&names.Decode(word,text))copy(out.match,text.View());}
 if(r.Read(a.world+cfgprobe::WorldGameState,state))name(state,out.state);
 if(r.Read(a.world+cfgprobe::WorldGameInstance,instance)&&r.Read(instance+cfgprobe::GameInstanceLocalPlayers,locals)){
  out.locals=locals.count;out.capacity=locals.capacity;
  if(locals.count>0&&locals.count<=16&&r.Read(locals.data,player)&&r.Read(player+cfghosted::disk35::PlayerController,controller)){
   name(controller,out.controller);(void)r.Read(controller+cfghosted::disk35::ControllerPlayer,out.backlink);
  }
 }
 if(cfgprobe::ObjectPointer(trace.object)){
  name(trace.object,out.failedClass);(void)r.Read(trace.object+cfgprobe::ObjectClass,out.actual);
  cfgprobe::lifetime::ArrayHeader table;cfgprobe::lifetime::SlotObservation slot;
  if(cfgprobe::lifetime::ReadArrayHeader(r,a.module+cfgprobe::lifetime::ObjectArrayRva,table)&&cfgprobe::lifetime::ReadSlot(r,table,trace.object,slot)){
   out.serial=slot.serial;(void)r.Read(table.data+24ULL*static_cast<unsigned>(slot.index)+8,out.internal);(void)r.Read(trace.object+8,out.flags);
  }
 }
}
