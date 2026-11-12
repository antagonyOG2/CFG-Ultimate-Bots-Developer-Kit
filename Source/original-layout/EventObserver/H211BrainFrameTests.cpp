#include "../RuntimeProbe/HostedCounselorBrainBindings.generated.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include <cstddef>
#include <cstdio>
struct Frame {std::uintptr_t tree;std::uint8_t returned;std::uint8_t padding[7];};
int main(){unsigned n=0,bad=0;auto check=[&](bool ok){++n;if(!ok)++bad;};
 const auto& b=cfghostedbrain::disk::RunBehaviorTree;
 // Exact disk return setter C6 41 08 01 C3 stores ONE byte at8; pointer BTAsset@0.
 check(offsetof(Frame,returned)==8&&sizeof(Frame)==16);
 check(b.nativeBytes==9&&b.storageBytes==16);
 check(cfgbundle::NativeFrame(9,b.nativeBytes,b.storageBytes));
 check(!cfgbundle::NativeFrame(16,b.nativeBytes,b.storageBytes));
 check(!cfgbundle::NativeFrame(8,b.nativeBytes,b.storageBytes));
 std::printf("H211 RunBT real binding native9/storage16 live-frame replay: %u checks, %u failures.\n",n,bad);return bad?1:0;
}
