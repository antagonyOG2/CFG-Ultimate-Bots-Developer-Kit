#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
namespace cfgroad {
struct Vector { float x=0,y=0,z=0; };
inline bool Finite(Vector v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
inline Vector Sub(Vector a,Vector b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline float Dot(Vector a,Vector b){return a.x*b.x+a.y*b.y;}
inline float Length(Vector a){return std::sqrt(Dot(a,a));}
inline Vector Blend(Vector a,Vector b,float t){return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t};}
constexpr unsigned MaximumPoints=512,MaximumNeighbors=8,NoPoint=~0u;
constexpr std::uint64_t FailedRoadRetry=1379; // donor stock vehicle acquisition cadence
inline bool RequestDue(std::uint64_t now,std::uint64_t morphAt,std::uint64_t attemptAt,bool moving,
    unsigned interactionPhase,float squared,bool prolongedTail=false){
 return moving&&!interactionPhase&&now>=morphAt&&now>=attemptAt&&std::isfinite(squared)&&
    (squared>=600.f*600.f||(prolongedTail&&squared>275.f*275.f));
}
struct Node {Vector position{};std::array<unsigned,MaximumNeighbors> neighbors{};unsigned count=0;};
struct Graph {std::array<Node,MaximumPoints> nodes{};unsigned count=0;};
struct Candidate {
 bool exitBlock=false;unsigned exitIndex=NoPoint;
 Vector location{},direction{};float requestedLead=0,forwardLead=0,laneCorrection=0,sourceAlignment=0,sourceLane=0;
 // Ordered selected route, including its source edge. Used for live proof.
 std::array<unsigned,66> route{};unsigned count=0;
};
inline float RequestedLead(float speed){return std::isfinite(speed)&&speed>=0?(std::max)(7000.f,(std::min)(12000.f,speed*7.5f)):0;}
// Authored graph X/Y must survive projection. This is NOT the generic
// counselor Morph landing which may choose a nearby navigable X/Y island.
// CFG actors use a90cm capsule-centre offset above the proved ground height.
inline bool Landing(Vector road,Vector projected,Vector& output){
 if(!Finite(road)||!Finite(projected))return false;
 const auto d=Sub(road,projected);
 if(Dot(d,d)>600.f*600.f||std::fabs(d.z)>220.f)return false;
 output={road.x,road.y,projected.z+90.f};return Finite(output);
}
inline bool MayMorph(Vector car,Vector jason,Vector direction,bool prolongedTail=false){
 if(!Finite(car)||!Finite(jason)||!Finite(direction))return false;
 const auto delta=Sub(jason,car);const float ahead=Dot(delta,direction);
 const Vector lateral{delta.x-direction.x*ahead,delta.y-direction.y*ahead,0};
 const float minimum=prolongedTail?275.f:600.f;
 return Dot(delta,delta)>minimum*minimum&&!(ahead>250.f&&Dot(lateral,lateral)<=1200.f*1200.f);
}
inline bool Continued(const Graph& g,unsigned at,unsigned previous){
 const auto& n=g.nodes[at];for(unsigned i=0;i<n.count;++i)if(n.neighbors[i]<g.count&&n.neighbors[i]!=previous)return true;return false;
}
// Donor FrozenJasonBridge::ResolveVehicleRoadCenterIntercept, centimeters.
// No tangent/nav projection fallback. The caller may refine only ground Z.
inline bool Resolve(const Graph& g,Vector car,Vector travel,float speed,Candidate& output){
 output={};Candidate c;c.requestedLead=RequestedLead(speed);
 const float directionLength=Length(travel);
 if(!g.count||g.count>MaximumPoints||!Finite(car)||!Finite(travel)||directionLength<.99f||directionLength>1.01f||!c.requestedLead)return false;
 for(unsigned i=0;i<g.count;++i)if(g.nodes[i].count>MaximumNeighbors||!Finite(g.nodes[i].position))return false;
 unsigned a=NoPoint,b=NoPoint;float best=std::numeric_limits<float>::max();
 for(unsigned i=0;i<g.count;++i){const auto& n=g.nodes[i];for(unsigned j=0;j<n.count;++j){
  unsigned other=n.neighbors[j];if(other>=g.count)continue;const auto d=Sub(g.nodes[other].position,n.position);const float length=Length(d);
  if(!std::isfinite(length)||length<1)continue;const float signedAlignment=Dot(d,travel)/length,alignment=std::fabs(signedAlignment);
  if(alignment<.45f)continue;const float t=(std::clamp)(Dot(Sub(car,n.position),d)/(length*length),0.f,1.f);
  const auto delta=Sub(car,Blend(n.position,g.nodes[other].position,t));const float lateral=Dot(delta,delta);
  if(!std::isfinite(lateral)||lateral>700.f*700.f)continue;const float score=lateral+(1-alignment)*40000.f;
  if(score>=best)continue;best=score;a=signedAlignment>=0?i:other;b=signedAlignment>=0?other:i;
 }}
 if(a==NoPoint)return false;c.route[c.count++]=a;c.route[c.count++]=b;
 Vector d=Sub(g.nodes[b].position,g.nodes[a].position);float length=Length(d);
 const float projection=(std::clamp)(Dot(Sub(car,g.nodes[a].position),d)/(length*length),0.f,1.f);
 const Vector projected=Blend(g.nodes[a].position,g.nodes[b].position,projection);
 c.direction={d.x/length,d.y/length,0};const auto fromGraph=Sub(car,projected);
 const float lane=c.direction.x*fromGraph.y-c.direction.y*fromGraph.x;
 // Donor FrozenJasonBridge14837: authored road points can themselves lie
 // on the shoulder. Carry only the modest measured drivable-lane offset
 // when the car's heading nearly agrees with the current source edge.
 // Do not infer a lane from sharp turns, large offsets or a parallel road.
 const float sourceAlignment=Dot(c.direction,travel);
 c.sourceAlignment=sourceAlignment;c.sourceLane=lane;
 // CFG physical55: sourceAlignment=.800, lane=-295cm; exact authored landing
 // was a shoulder point because donor's .85 gate discarded its calibration.
 // Coarser CFG edge chords permit up to41deg (.75), still bounded320cm and
 // proved against the same source edge/route. No nav-island X/Y fallback.
 const float correction=std::isfinite(sourceAlignment)&&sourceAlignment>=.75f&&
     std::isfinite(lane)&&std::fabs(lane)>=65.f&&std::fabs(lane)<=320.f?lane:0.f;
 const auto finish=[&](Vector location){
  c.location=location;c.location.x-=c.direction.y*correction;c.location.y+=c.direction.x*correction;c.laneCorrection=correction;
  c.forwardLead=Dot(Sub(c.location,car),travel);
  if(!Finite(c.location)||!std::isfinite(c.forwardLead)||c.forwardLead<1200.f||c.forwardLead>c.requestedLead+1000.f)return false;
  output=c;return true;
 };
 float remaining=c.requestedLead;const float distance=length*(1-projection);
 if(remaining<=distance){float advance=remaining;if(!Continued(g,b,a))advance=(std::min)(advance,(std::max)(0.f,distance-2200.f));
  return finish(Blend(projected,g.nodes[b].position,distance>1?advance/distance:0));}
 remaining-=distance;float traversed=distance;Vector interior=projected;
 for(unsigned step=0;step<64;++step){unsigned next=NoPoint;float score=-std::numeric_limits<float>::max();const auto& n=g.nodes[b];
  for(unsigned j=0;j<n.count;++j){const unsigned k=n.neighbors[j];if(k>=g.count||k==a)continue;
   const auto segment=Sub(g.nodes[k].position,n.position);const float size=Length(segment);if(!std::isfinite(size)||size<1)continue;
   const float alignment=Dot(segment,c.direction)/size;if(alignment>score){score=alignment;next=k;}}
  if(next==NoPoint)break;
  // Donor bounds the walk; additionally refuse revisiting a node, so a loop
  // cannot manufacture useful lead or overflow the validation route.
  for(unsigned j=0;j<c.count;++j)if(c.route[j]==next)return false;
  c.route[c.count++]=next;d=Sub(g.nodes[next].position,g.nodes[b].position);length=Length(d);c.direction={d.x/length,d.y/length,0};
  if(remaining<=length){float advance=remaining;if(!Continued(g,next,b))advance=(std::min)(advance,(std::max)(0.f,length-2200.f));
   return finish(Blend(g.nodes[b].position,g.nodes[next].position,advance/length));}
  remaining-=length;traversed+=length;interior=g.nodes[b].position;a=b;b=next;
 }
 return traversed>=2500.f&&finish(interior);
}
}
