// Included in OfflineSnapshot.cpp's private namespace. Query-only OS metadata;
// no game-memory reads, loading permissions, writes, privileges or policy changes.
struct AccessTokenContext {
    DWORD integrity=0, session=0, elevated=0;
    std::vector<unsigned char> user;
};
bool QueryTokenBuffer(HANDLE token, TOKEN_INFORMATION_CLASS kind,
    std::vector<unsigned char>& output) {
    DWORD required=0;
    SetLastError(ERROR_SUCCESS);
    if (GetTokenInformation(token,kind,nullptr,0,&required) ||
        GetLastError()!=ERROR_INSUFFICIENT_BUFFER || !required || required>65536) return false;
    output.resize(required);
    DWORD obtained=0;
    return GetTokenInformation(token,kind,output.data(),required,&obtained) &&
        obtained && obtained<=required;
}
bool SidInside(const std::vector<unsigned char>& bytes, PSID sid) {
    const auto begin=reinterpret_cast<std::uintptr_t>(bytes.data());
    const auto at=reinterpret_cast<std::uintptr_t>(sid);
    // SID header is eight bytes. Check extent BEFORE invoking SID helper APIs.
    if (bytes.size()<8 || at<begin || at-begin>bytes.size()-8) return false;
    const auto offset=static_cast<std::size_t>(at-begin);
    const auto count=bytes[offset+1];
    if (count>SID_MAX_SUB_AUTHORITIES || static_cast<std::size_t>(count)*4>bytes.size()-offset-8)
        return false;
    return IsValidSid(sid)!=FALSE;
}
bool AccessContextBoundsSelfTest() {
    std::vector<unsigned char> bytes(12,0);
    SID_IDENTIFIER_AUTHORITY authority=SECURITY_MANDATORY_LABEL_AUTHORITY;
    if (!InitializeSid(bytes.data(),&authority,1) || !SidInside(bytes,bytes.data()) ||
        SidInside(bytes,nullptr) || SidInside(bytes,bytes.data()+5)) return false;
    bytes[1]=SID_MAX_SUB_AUTHORITIES+1;
    if (SidInside(bytes,bytes.data())) return false;
    bytes[1]=2;
    if (SidInside(bytes,bytes.data())) return false;
    bytes[1]=1; bytes[0]=0;
    if (SidInside(bytes,bytes.data())) return false;
    bytes.resize(7);
    return !SidInside(bytes,bytes.data());
}
bool QueryAccessToken(HANDLE process, AccessTokenContext& output, const char* label) {
    Handle token;
    if (!OpenProcessToken(process,TOKEN_QUERY,&token.value)) {
        std::printf("%sTokenQueryError=%lu\n",label,GetLastError()); return false;
    }
    std::vector<unsigned char> integrity;
    TOKEN_ELEVATION elevation{};
    DWORD returned=0;
    if (!QueryTokenBuffer(token.value,TokenIntegrityLevel,integrity) ||
        integrity.size()<sizeof(TOKEN_MANDATORY_LABEL) ||
        !QueryTokenBuffer(token.value,TokenUser,output.user) ||
        output.user.size()<sizeof(TOKEN_USER) ||
        !GetTokenInformation(token.value,TokenElevation,&elevation,sizeof(elevation),&returned) ||
        returned!=sizeof(elevation) ||
        !GetTokenInformation(token.value,TokenSessionId,&output.session,sizeof(output.session),&returned) ||
        returned!=sizeof(output.session)) {
        std::printf("%sTokenMetadata=unavailable\n",label); return false;
    }
    const auto labelSid=reinterpret_cast<const TOKEN_MANDATORY_LABEL*>(integrity.data())->Label.Sid;
    const auto userSid=reinterpret_cast<const TOKEN_USER*>(output.user.data())->User.Sid;
    if (!SidInside(integrity,labelSid) || !SidInside(output.user,userSid)) return false;
    SID_IDENTIFIER_AUTHORITY mandatoryAuthority=SECURITY_MANDATORY_LABEL_AUTHORITY;
    const auto count=*GetSidSubAuthorityCount(labelSid);
    if (count!=1 || std::memcmp(GetSidIdentifierAuthority(labelSid),&mandatoryAuthority,
        sizeof(mandatoryAuthority))!=0) return false;
    output.integrity=*GetSidSubAuthority(labelSid,0);
    output.elevated=elevation.TokenIsElevated;
    std::printf("%sIntegrityRid=%lu\n%sElevated=%s\n",label,output.integrity,
        label,output.elevated ? "yes":"no");
    return true;
}
int InspectAccessContext(HANDLE target) {
    std::puts("CFG PROCESS ACCESS CONTEXT -- QUERY-ONLY OS METADATA");
    std::puts("Stage=query_only_access_context\nGameMemoryReads=0\nMemoryWrites=0\nContextLoadingRightsRequested=no\nInjection=none\nPrivilegeChanges=none\nPolicyChanges=none\nAccountNamesAndSidsPrinted=no");
    AccessTokenContext caller{}, game{};
    const bool callerOk=QueryAccessToken(GetCurrentProcess(),caller,"Caller");
    const bool targetOk=QueryAccessToken(target,game,"Target");
    if (callerOk && targetOk) {
        const auto callerSid=reinterpret_cast<const TOKEN_USER*>(caller.user.data())->User.Sid;
        const auto targetSid=reinterpret_cast<const TOKEN_USER*>(game.user.data())->User.Sid;
        std::printf("SameTokenUser=%s\nSameSession=%s\nTargetHigherIntegrity=%s\n",
            EqualSid(callerSid,targetSid) ? "yes":"no",
            caller.session==game.session ? "yes":"no",
            game.integrity>caller.integrity ? "yes":"no");
        if (game.integrity>caller.integrity)
            std::puts("IntegrityMismatchObserved=yes\nMismatchAloneDoesNotIdentifyAllDenialCauses=yes");
    }
    PROCESS_PROTECTION_LEVEL_INFORMATION protection{};
    const bool protectionOk=GetProcessInformation(target,ProcessProtectionLevelInfo,
        &protection,sizeof(protection))!=FALSE;
    if (protectionOk) {
        std::printf("WindowsProtectionLevel=0x%08lX\nWindowsProtectedProcessReported=%s\n",
            protection.ProtectionLevel,protection.ProtectionLevel==PROTECTION_LEVEL_NONE ? "no":"yes_or_other_level");
    } else std::printf("WindowsProtectionLevelQueryError=%lu\n",GetLastError());
    std::puts("SecurityDescriptorNotModified=yes\nAntiCheatCauseProven=no\nNoLoadingRetry=yes");
    return callerOk && targetOk && protectionOk ? 0:11;
}
