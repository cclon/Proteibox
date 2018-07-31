#pragma once

//////////////////////////////////////////////////////////////////////////

extern HMODULE g_handle_advapi32dll;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef ULONG (WINAPI* _CryptProtectData_) (
	DATA_BLOB* pDataIn,
	LPCWSTR szDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	);

extern _CryptProtectData_ g_CryptProtectData_addr ;

ULONG WINAPI
fake_CryptProtectData (
	DATA_BLOB* pDataIn,
	LPCWSTR szDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	);


typedef ULONG (WINAPI* _CryptUnprotectData_) (
	DATA_BLOB* pDataIn,
	LPWSTR * ppszDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	);

extern _CryptUnprotectData_	g_CryptUnprotectData_addr ;

ULONG WINAPI
fake_CryptUnprotectData (
	DATA_BLOB* pDataIn,
	LPWSTR * ppszDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	);


typedef ULONG (WINAPI* _RegOpenKeyExA_) (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );

extern _RegOpenKeyExA_ g_RegOpenKeyExA_addr ;


typedef ULONG (WINAPI* _RegOpenKeyExW_) (
	HKEY hKey,
	LPCWSTR lpSubKey,
	DWORD dwOptions,
	REGSAM samDesired,
	PHKEY phkResult
	);

extern _RegOpenKeyExW_ g_RegOpenKeyExW_addr ;


typedef ULONG (WINAPI* _RegCloseKey_) (
    IN HKEY hKey
    );

extern _RegCloseKey_ g_RegCloseKey_addr ;


typedef ULONG (WINAPI* _RegGetKeySecurity_) (
    HKEY hKey,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    );

extern _RegGetKeySecurity_ g_RegGetKeySecurity_addr ;


typedef ULONG (WINAPI* _RegSetKeySecurity_) (
	HKEY hKey,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	);

extern _RegSetKeySecurity_ g_RegSetKeySecurity_addr ;


typedef ULONG (WINAPI* _GetFileSecurityA_) (
	LPCSTR lpFileName,
	SECURITY_INFORMATION RequestedInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	DWORD nLength,
	LPDWORD lpnLengthNeeded
	);

extern _GetFileSecurityA_ g_GetFileSecurityA_addr ;


typedef ULONG (WINAPI* _GetFileSecurityW_) (
	LPCWSTR lpFileName,
	SECURITY_INFORMATION RequestedInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	DWORD nLength,
	LPDWORD lpnLengthNeeded
	);

extern _GetFileSecurityW_ g_GetFileSecurityW_addr ;


typedef ULONG (WINAPI* _SetFileSecurityA_) (
	LPCSTR lpFileName,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	);

extern _SetFileSecurityA_ g_SetFileSecurityA_addr ;


typedef ULONG (WINAPI* _SetFileSecurityW_) (
	LPCWSTR lpFileName,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	);

extern _SetFileSecurityW_ g_SetFileSecurityW_addr ;


typedef ULONG (WINAPI* _LookupAccountNameW_) (
	LPCWSTR lpSystemName,
	LPCWSTR lpAccountName,
	PSID Sid,
	LPDWORD cbSid,
	LPWSTR ReferencedDomainName,
	LPDWORD cbReferencedDomainName,
	PSID_NAME_USE peUse
	);

extern _LookupAccountNameW_ g_LookupAccountNameW_addr ;


typedef ULONG (WINAPI* _LookupPrivilegeValueW_) (
	LPCWSTR lpSystemName,
	LPCWSTR lpName,
	PLUID  lpLuid
	);

extern _LookupPrivilegeValueW_ g_LookupPrivilegeValueW_addr ;


typedef ULONG (WINAPI* _RegConnectRegistryA_) (
	LPCSTR lpMachineName,
	HKEY hKey,
	PHKEY phkResult
	);

extern _RegConnectRegistryA_ g_RegConnectRegistryA_addr ;


typedef ULONG (WINAPI* _RegConnectRegistryW_) (
	IN LPCWSTR lpMachineName OPTIONAL,
	IN HKEY hKey,
	OUT PHKEY phkResult
	);

extern _RegConnectRegistryW_ g_RegConnectRegistryW_addr ;


typedef ULONG (WINAPI* _CryptVerifySignatureA_) (
	__in  HCRYPTHASH hHash,
	__in  BYTE *pbSignature,
	__in  DWORD dwSigLen,
	__in  HCRYPTKEY hPubKey,
	__in  LPCSTR sDescription,
	__in  DWORD dwFlags
	);

extern _CryptVerifySignatureA_ g_CryptVerifySignatureA_addr ;


typedef ULONG (WINAPI* _CryptVerifySignatureW_) (
	__in  HCRYPTHASH hHash,
	__in  BYTE *pbSignature,
	__in  DWORD dwSigLen,
	__in  HCRYPTKEY hPubKey,
	__in  LPCWSTR sDescription,
	__in  DWORD dwFlags
	);

extern _CryptVerifySignatureW_ g_CryptVerifySignatureW_addr ;


typedef ULONG (WINAPI* _WSANSPIoctl_) (HANDLE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD,LPWSACOMPLETION);
extern _WSANSPIoctl_ g_WSANSPIoctl_addr ;


//////////////////////////////////////////////////////////////////////////

HMODULE Get_advapi32dll_Handle();


///////////////////////////////   END OF FILE   ///////////////////////////////
