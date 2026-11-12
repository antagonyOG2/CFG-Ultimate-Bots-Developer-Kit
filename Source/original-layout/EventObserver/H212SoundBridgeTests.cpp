#include "NativeSoundBlipSuppression.hpp"
#include <cstdio>
struct Reader {template<class T>bool Read(std::uintptr_t,T&){return false;}};
struct Host {bool retained=false;bool HostedSoundBlipScope(const cfgsoundblip::Scope&){return retained;}};
int main(){Reader r;Host h;cfgsoundblip::Scope s;unsigned n=0,bad=0;auto check=[&](bool v){++n;if(!v)++bad;};
 cfgsoundblip::NativeAccess<Reader,Host> a(r,h,s,true,true);check(!a.Guard());
 h.retained=true;check(a.Guard());
 cfgsoundblip::NativeAccess<Reader,Host> unsafe(r,h,s,false,true);check(!unsafe.Guard());
 cfgsoundblip::NativeAccess<Reader,Host> foreign(r,h,s,true,false);check(!foreign.Guard());
 std::printf("H212 actual sound adapter bridge: %u checks, %u failures; ONLY explicit hosted retained proof, lookup-safe and owner thread admit; offline Suppress unchanged.\n",n,bad);return bad?1:0;
}
