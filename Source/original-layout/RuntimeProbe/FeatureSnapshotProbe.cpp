// External entry is finite read-only capture or own-process self-test only.
#define CFG_FEATURE_SNAPSHOT
#define wmain CfgSharedSnapshotMain
#include "OfflineSnapshot.cpp"
#undef wmain
int wmain(int argc,wchar_t** argv) {
    if(argc==2 && std::wcscmp(argv[1],L"--self-test")==0) return CfgSharedSnapshotMain(argc,argv);
    if(argc!=4 || std::wcscmp(argv[1],L"--feature-snapshot")!=0 || std::wcscmp(argv[2],L"--exe")!=0) {
        std::puts("Use --feature-snapshot --exe <exact CFG executable>.\nOffline read-only capture only; no loading, hooks, native calls or writes.");
        return 1;
    }
    wchar_t option[]=L"--offline-feature-snapshot";
    wchar_t* forwarded[]{argv[0],option,argv[2],argv[3]};
    return CfgSharedSnapshotMain(4,forwarded);
}
