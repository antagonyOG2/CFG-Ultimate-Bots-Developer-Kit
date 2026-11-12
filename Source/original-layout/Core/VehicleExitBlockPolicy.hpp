#pragma once
#include "VehicleRoadChasePolicy.hpp"
namespace cfgroad {
constexpr float ExitBlockRange=5000.f; // 50 metres, UE centimetres
// A typed native escape-volume location must coincide with a terminal road
// segment. No arbitrary roadside/tangent fallback or guessed map coordinates.
inline bool ResolveExit(const Graph& g,Vector car,Vector travel,Vector escape,Candidate& output){
 output={};ChaseEdge from;if(!ChaseNearest(g,car,from)||!Finite(travel)||!Finite(escape))return false;
 const float speed=Length(travel);if(speed<.99f||speed>1.01f)return false;
 const auto delta=Sub(escape,car);if(Dot(delta,delta)>ExitBlockRange*ExitBlockRange||Dot(delta,travel)<=250.f)return false;
 unsigned end=NoPoint,inside=NoPoint;Vector point{};float best=1000.f*1000.f;
 for(unsigned i=0;i<g.count;++i){const auto& n=g.nodes[i];if(n.count!=1||n.neighbors[0]>=g.count)continue;
  const unsigned k=n.neighbors[0];const auto d=Sub(n.position,g.nodes[k].position);const float sq=Dot(d,d);if(sq<1)continue;
  const float t=(std::clamp)(Dot(Sub(escape,g.nodes[k].position),d)/sq,0.f,1.f);
  const auto projection=Blend(g.nodes[k].position,n.position,t),offset=Sub(escape,projection);
  if(Dot(offset,offset)>700.f*700.f||std::fabs(offset.z)>400.f||Length(Sub(projection,n.position))>1000.f)continue;
  const float score=Dot(offset,offset);if(score>=best)continue;best=score;end=i;inside=k;point=projection;
 }
 if(end==NoPoint)return false;
 Candidate c;c.exitBlock=true;c.location=point;c.requestedLead=ExitBlockRange;
 unsigned a=from.a,b=from.b;auto d=Sub(g.nodes[b].position,g.nodes[a].position);
 if(Dot(d,travel)<0)std::swap(a,b);
 if(Dot(Sub(g.nodes[b].position,g.nodes[a].position),travel)/Length(Sub(g.nodes[b].position,g.nodes[a].position))<.45f)return false;
 c.route[c.count++]=a;c.route[c.count++]=b;float walked=Length(Sub(g.nodes[b].position,from.point));
 for(unsigned step=0;step<64;++step){
  if(b==end&&a==inside){
   walked-=Length(Sub(g.nodes[b].position,point));
   const auto lead=Sub(point,car);c.forwardLead=Dot(lead,travel);
   d=Sub(g.nodes[b].position,g.nodes[a].position);const float length=Length(d);
   if(walked<250.f||walked>ExitBlockRange||c.forwardLead<=250.f||length<1)return false;
   c.direction={d.x/length,d.y/length,0};output=c;return true;
  }
  if(walked>ExitBlockRange+1000.f||c.count>=c.route.size())return false;
  const auto previous=Sub(g.nodes[b].position,g.nodes[a].position);const float size=Length(previous);
  if(size<1)return false;unsigned next=NoPoint;float alignment=-2.f;
  for(unsigned j=0;j<g.nodes[b].count;++j){const unsigned k=g.nodes[b].neighbors[j];if(k>=g.count||k==a)continue;
   const auto segment=Sub(g.nodes[k].position,g.nodes[b].position);const float length=Length(segment);if(length<1)continue;
   const float score=Dot(previous,segment)/(size*length);if(score>alignment){alignment=score;next=k;}}
  if(next==NoPoint)return false;
  for(unsigned j=0;j<c.count;++j)if(c.route[j]==next)return false;
  c.route[c.count++]=next;walked+=Length(Sub(g.nodes[next].position,g.nodes[b].position));a=b;b=next;
 }
 return false;
}
}
