// Isolated read-only entry: no loading/integration command is accepted here.
#define CFG_PRIMARY_RESOURCES
#define wmain CfgSharedSnapshotMain
#include "OfflineSnapshot.cpp"
#undef wmain
int wmain(int argc,wchar_t** argv) {
    if (argc==2 && std::wcscmp(argv[1],L"--self-test")==0)
        return CfgSharedSnapshotMain(argc,argv);
    if (argc!=4 || std::wcscmp(argv[1],L"--primary-jason-resources")!=0 ||
        std::wcscmp(argv[2],L"--exe")!=0) {
        std::puts("Use --primary-jason-resources --exe <exact CFG executable>.\nRead-only Sandbox counselor resource check; no loading, hooks or writes.");
        return 1;
    }
    wchar_t primaryOption[]=L"--offline-primary-resources";
    wchar_t* forwarded[]{argv[0],primaryOption,argv[2],argv[3]};
    return CfgSharedSnapshotMain(4,forwarded);
}
