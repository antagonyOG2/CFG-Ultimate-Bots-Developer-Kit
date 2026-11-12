// Read-only stock-bot observations also work when a full stock roster prevents
// Jason adoption. Running/Paused=2 means not queried: no native getter is called.
#ifdef CFG_H1_JASON
std::array<cfgh1::BotTrace,7> hostedStockBaseline{};
std::uintptr_t hostedStockWorld=0;unsigned hostedStockCursor=0;
void ObserveHostedStockBaseline(const cfghosted::takeover35::Frame& frame,std::uintptr_t module){
    if(!frame.count)return;
    const auto world=frame.host.session.World();
    if(world!=hostedStockWorld){hostedStockWorld=world;hostedStockBaseline={};hostedStockCursor=0;}
    const auto index=hostedStockCursor++%frame.count;const auto state=frame.players[index].object;
    Reader r;r.cap=1024;std::uint32_t flags=0;std::uintptr_t controller=0,pawn=0,ai=0,type=0,brain=0;
    const auto& cache=cfgcompat::cfg35::ai::Classes[2];
    const auto& table=frame.host.session.table;
    cfgprobe::lifetime::SlotObservation pc{},p{};
    if(!cfgjason::SameSlot(r,table,frame.players[index])||!r.Read(state+cfghosted::birth35::BotFlags,flags)||
       !(flags&cfghosted::birth35::BotMask)||!r.Read(state+cfghosted::adoption::StateOwner,controller)||
       !cfghosted::native::Class(r,module,cache.reflection,cache.constructor,ai)||!cfgspawn::ObjectIs(r,controller,ai)||
       !cfgspawn::ObjectSlot(r,table,controller,pc)||!pc.HasAssignedSerial()||
       !r.Read(controller+cfgprobe::ControllerPawn,pawn)||!cfgspawn::ObjectSlot(r,table,pawn,p)||!p.HasAssignedSerial()||
       !cfgobserver::Same(r,pawn+cfgescape::disk::Controller,controller)||
       !cfgobserver::Same(r,pawn+cfgescape::disk::PlayerState,state)||
       !cfgobserver::Same(r,controller+cfgescape::disk::PlayerState,state)||
       !r.Read(controller+cfgprobe::ObjectClass,type)||!r.Read(controller+0x418,brain))return;
    auto it=std::find_if(hostedStockBaseline.begin(),hostedStockBaseline.end(),[&](const auto& d){return d.pawn==pawn;});
    if(it==hostedStockBaseline.end())it=std::find_if(hostedStockBaseline.begin(),hostedStockBaseline.end(),[](const auto& d){return !d.pawn;});
    if(it==hostedStockBaseline.end())return;
    auto next=*it;next.pawn=pawn;next.controller=controller;next.controllerClass=type;next.brain=brain;
    next.tree=frame.counselorTree;next.registered=1;next.source=2;++next.samples;
    std::uintptr_t root=0,scene=0;cfgspawn::Vec3 position{},again{};
    if(r.Read(pawn+cfgposition::disk::ActorRootComponent,root)&&
       cfghosted::native::Class(r,module,cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,scene)&&
       cfgspawn::ObjectIs(r,root,scene)&&cfgobserver::Same(r,root+0x20,pawn)&&cfgobserver::Same(r,root+0xE0,pawn)&&
       r.Read(root+cfgposition::disk::SceneWorldTranslation,position)&&cfgspawn::Finite(position)&&
       r.Read(root+cfgposition::disk::SceneWorldTranslation,again)&&position.x==again.x&&position.y==again.y&&position.z==again.z&&
       cfgobserver::Same(r,pawn+cfgposition::disk::ActorRootComponent,root)){
        if(next.samples>1&&(std::fabs(position.x-next.x)>2||std::fabs(position.y-next.y)>2||std::fabs(position.z-next.z)>2))++next.moves;
        next.x=position.x;next.y=position.y;next.z=position.z;
    }
    if(!cfgjason::SameSlot(r,table,frame.players[index])||!cfgjason::SameSlot(r,table,pc)||!cfgjason::SameSlot(r,table,p)||
       !cfgobserver::Same(r,controller+0x418,brain)||!cfgobserver::Same(r,controller+cfgprobe::ControllerPawn,pawn))return;
    next.reads=r.reads;*it=next;
}
#endif
