#pragma once
#include "ChallengeEdgeWallsPolicy.hpp"
namespace cfgj215walls {
struct Box {cfgj36walls::Vec minimum{},maximum{};std::uint8_t valid=0;std::array<std::uint8_t,3> pad{};};
struct Transform {std::array<float,4> rotation{},translation{},scale{};};
static_assert(sizeof(Box)==28&&sizeof(Transform)==48);
inline bool TransformBox(const Box& box,const Transform& t,cfgj36walls::Bounds& out){
 out={};if(box.valid!=1)return false;
 for(const float v:{box.minimum.x,box.minimum.y,box.minimum.z,box.maximum.x,box.maximum.y,box.maximum.z})if(!std::isfinite(v)||std::fabs(v)>10000000)return false;
 if(box.minimum.x>box.maximum.x||box.minimum.y>box.maximum.y||box.minimum.z>box.maximum.z)return false;
 for(const auto& group:{t.rotation,t.translation,t.scale})for(const float v:group)if(!std::isfinite(v)||std::fabs(v)>10000000)return false;
 const auto& q=t.rotation;const double norm=double(q[0])*q[0]+double(q[1])*q[1]+double(q[2])*q[2]+double(q[3])*q[3];
 if(std::fabs(norm-1.)>.001)return false;
 cfgj36walls::Vec low{INFINITY,INFINITY,INFINITY},high{-INFINITY,-INFINITY,-INFINITY};
 for(unsigned n=0;n<8;++n){
  const double x=(n&1?box.maximum.x:box.minimum.x)*double(t.scale[0]);
  const double y=(n&2?box.maximum.y:box.minimum.y)*double(t.scale[1]);
  const double z=(n&4?box.maximum.z:box.minimum.z)*double(t.scale[2]);
  const double a=2*(q[1]*z-q[2]*y),b=2*(q[2]*x-q[0]*z),c=2*(q[0]*y-q[1]*x);
  cfgj36walls::Vec v{static_cast<float>(x+q[3]*a+q[1]*c-q[2]*b+t.translation[0]),
   static_cast<float>(y+q[3]*b+q[2]*a-q[0]*c+t.translation[1]),static_cast<float>(z+q[3]*c+q[0]*b-q[1]*a+t.translation[2])};
  low.x=(std::min)(low.x,v.x);low.y=(std::min)(low.y,v.y);low.z=(std::min)(low.z,v.z);
  high.x=(std::max)(high.x,v.x);high.y=(std::max)(high.y,v.y);high.z=(std::max)(high.z,v.z);
 }
 out={{(low.x+high.x)*.5f,(low.y+high.y)*.5f,(low.z+high.z)*.5f},{(high.x-low.x)*.5f,(high.y-low.y)*.5f,(high.z-low.z)*.5f}};
 return cfgj36walls::Finite(out);
}
inline void Union(cfgj36walls::Bounds& out,const cfgj36walls::Bounds& b){
 const auto lo=[](float c,float e){return c-e;};const auto hi=[](float c,float e){return c+e;};
 const cfgj36walls::Vec low{(std::min)(lo(out.center.x,out.extent.x),lo(b.center.x,b.extent.x)),(std::min)(lo(out.center.y,out.extent.y),lo(b.center.y,b.extent.y)),(std::min)(lo(out.center.z,out.extent.z),lo(b.center.z,b.extent.z))};
 const cfgj36walls::Vec high{(std::max)(hi(out.center.x,out.extent.x),hi(b.center.x,b.extent.x)),(std::max)(hi(out.center.y,out.extent.y),hi(b.center.y,b.extent.y)),(std::max)(hi(out.center.z,out.extent.z),hi(b.center.z,b.extent.z))};
 out={{(low.x+high.x)*.5f,(low.y+high.y)*.5f,(low.z+high.z)*.5f},{(high.x-low.x)*.5f,(high.y-low.y)*.5f,(high.z-low.z)*.5f}};
}
}
