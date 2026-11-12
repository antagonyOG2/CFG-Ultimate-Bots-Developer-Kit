#pragma once
#include "VehicleRoadInterceptPolicy.hpp"
namespace cfgroad {
struct ChaseEdge {unsigned a=NoPoint,b=NoPoint;Vector point{};float t=0,squared=std::numeric_limits<float>::max();};
inline bool ChaseNearest(const Graph& g,Vector p,ChaseEdge& output){
    output={};if(!Finite(p)||!g.count||g.count>MaximumPoints)return false;
    for(unsigned i=0;i<g.count;++i){const auto& node=g.nodes[i];
        if(node.count>MaximumNeighbors||!Finite(node.position))return false;
        for(unsigned j=0;j<node.count;++j){const auto k=node.neighbors[j];if(k>=g.count)continue;
            if(!Finite(g.nodes[k].position))return false;
            const auto d=Sub(g.nodes[k].position,node.position);const float length=Dot(d,d);if(length<1.f)continue;
            const float t=(std::clamp)(Dot(Sub(p,node.position),d)/length,0.f,1.f);
            const auto point=Blend(node.position,g.nodes[k].position,t);const auto delta=Sub(p,point);const float squared=Dot(delta,delta);
            if(std::fabs(delta.z)>400.f||squared>=output.squared)continue;
            output={i,k,point,t,squared};
        }
    }
    return output.a!=NoPoint&&output.squared<=700.f*700.f;
}
// Follow connected authored road segments while far from the moving car.
// A single direct request to a distant hood cuts across bends/fences even
// when the preceding Morph landed exactly on its requested road segment.
inline bool Chase(const Graph& g,Vector jason,Vector car,Candidate& output){
    output={};ChaseEdge from,to;if(!ChaseNearest(g,jason,from)||!ChaseNearest(g,car,to))return false;
    std::array<float,MaximumPoints> distance;distance.fill(std::numeric_limits<float>::max());
    std::array<bool,MaximumPoints> visited{};std::array<unsigned,MaximumPoints> next;next.fill(NoPoint);
    distance[to.a]=Length(Sub(to.point,g.nodes[to.a].position));distance[to.b]=Length(Sub(to.point,g.nodes[to.b].position));
    for(unsigned step=0;step<g.count;++step){unsigned at=NoPoint;float best=std::numeric_limits<float>::max();
        for(unsigned i=0;i<g.count;++i)if(!visited[i]&&distance[i]<best){best=distance[i];at=i;}
        if(at==NoPoint)break;visited[at]=true;
        // Reverse relaxation also supports an authored directed neighbor list.
        for(unsigned i=0;i<g.count;++i){const auto& n=g.nodes[i];
            for(unsigned k=0;k<n.count;++k)if(n.neighbors[k]==at){
                const float length=Length(Sub(n.position,g.nodes[at].position));if(length<1.f||!std::isfinite(length))continue;
                if(best+length<distance[i]){distance[i]=best+length;next[i]=at;}
            }
        }
    }
    Candidate c;
    const auto add=[&](unsigned i){for(unsigned j=0;j<c.count;++j)if(c.route[j]==i)return;c.route[c.count++]=i;};
    add(from.a);add(from.b);add(to.a);add(to.b);
    const float a=Length(Sub(from.point,g.nodes[from.a].position))+distance[from.a];
    const float b=Length(Sub(from.point,g.nodes[from.b].position))+distance[from.b];
    const unsigned endpoint=a<=b?from.a:from.b;
    if(!std::isfinite((std::min)(a,b))||(std::min)(a,b)==std::numeric_limits<float>::max())return false;
    Vector goal{};
    if(from.squared>120.f*120.f)goal=from.point; // first return to the road, not diagonally across the bend
    else if((from.a==to.a&&from.b==to.b)||(from.a==to.b&&from.b==to.a))goal=to.point;
    else if(Length(Sub(jason,g.nodes[endpoint].position))>100.f)goal=g.nodes[endpoint].position;
    else if(next[endpoint]!=NoPoint){add(next[endpoint]);goal=g.nodes[next[endpoint]].position;}
    else goal=to.point;
    if(!Finite(goal)||Length(Sub(goal,jason))<30.f)return false;
    c.location=goal;output=c;return true;
}
}
