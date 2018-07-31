/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/21 [21:1:2012 - 12:47]
* MODULE : \Code\Project\ProteinBoxDLL\PBAdvapi32dll\PBAdvapi32dll.cpp
* 
* Description:
*
*   Hook advapi32.dll 系列函数，处理一些边缘服务
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "../common.h"
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "PBAdvapi32dll.h"


//////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+															  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_LookupAccountNameW (
	LPCWSTR lpSystemName,
	LPCWSTR lpAccountName,
	PSID Sid,
	LPDWORD SidLength,
	LPWSTR ReferencedDomainName,
	LPDWORD hReferencedDomainNameLength, 
	PSID_NAME_USE SidNameUse
	)
{
	int ret = g_LookupAccountNameW_addr( lpSystemName,lpAccountName,Sid,SidLength,ReferencedDomainName,hReferencedDomainNameLength,SidNameUse );

	if ( 0 == ret && ERROR_NONE_MAPPED == GetLastError() )
	{
		if ( NULL == lpSystemName && lpAccountName && 0 == wcsnicmp(lpAccountName, L"NT SERVICE\\", 0xB) )
		{
			ret = g_LookupAccountNameW_addr (
				NULL,
				L"NT AUTHORITY\\SYSTEM",
				Sid,
				SidLength,
				ReferencedDomainName,
				hReferencedDomainNameLength,
				SidNameUse
				);
		}
	}

	return ret;
}


ULONG WINAPI
fake_RegConnectRegistryA (
	LPCSTR lpMachineName, 
	HKEY hKey,
	PHKEY phkResult
	)
{
	int ret ;

	if ( lpMachineName && *lpMachineName )
		ret = ERROR_ACCESS_DENIED ;
	else
		ret = g_RegOpenKeyExA_addr( hKey, 0, 0, 0x2000000, phkResult );

	return ret;
}


ULONG WINAPI
fake_RegConnectRegistryW (
	LPWSTR lpMachineName, 
	HKEY hKey,
	PHKEY phkResult
	)
{
	int ret;

	if ( lpMachineName && *lpMachineName )
		ret = ERROR_ACCESS_DENIED;
	else
		ret = g_RegOpenKeyExW_addr( hKey, 0, 0, 0x2000000, phkResult );

	return ret;
}


ULONG WINAPI fake_CryptVerifySignature( int a1, int a2, int a3, int a4, int a5, int a6 )
{
	SetLastError( NO_ERROR );
	return 1;
}


static BOOL g_bFlagCrypt = FALSE;

ULONG WINAPI
fake_CryptProtectData (
	DATA_BLOB* pDataIn,
	LPCWSTR szDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	)
{
	BOOL bWork = FALSE;

	if ( g_bFlagCrypt )
	{
		bWork = TRUE;
	}
	else
	{
		BOOL bRet = g_CryptProtectData_addr(
			pDataIn,
			szDataDescr,
			pOptionalEntropy,
			pvReserved,
			pPromptStruct,
			dwFlags,
			pDataOut );

		if ( !bRet && RPC_S_SERVER_UNAVAILABLE == GetLastError() )
			bWork = TRUE;
	}
	
	if ( FALSE == bWork ) { return 0; }
	
	g_bFlagCrypt = TRUE;

	DWORD pOptionalEntropy_cbData = 0, DataDescrLength = 0;
	if ( pOptionalEntropy ) { pOptionalEntropy_cbData = pOptionalEntropy->cbData; }
	if ( szDataDescr ) { DataDescrLength = wcslen(szDataDescr); }

	DWORD TotalLength = pOptionalEntropy_cbData + pDataIn->cbData + 2 * DataDescrLength + 0x1E;
	LPRPC_IN_CryptProtectData pRpcInBuffer = (LPRPC_IN_CryptProtectData) kmalloc( TotalLength );

	pRpcInBuffer->Flag = _PBSRV_APINUM_CryptProtectData_;
	pRpcInBuffer->dwFlags					= dwFlags;
	pRpcInBuffer->TotalLength				= TotalLength;
	pRpcInBuffer->pOptionalEntropy_cbData	= pOptionalEntropy_cbData;
	pRpcInBuffer->pDataIn_cbData			= pDataIn->cbData;
	pRpcInBuffer->DataDescrLength			= DataDescrLength;

	memcpy( pRpcInBuffer->Data, pDataIn->pbData, pRpcInBuffer->pDataIn_cbData );

	LPWSTR ptr1 = (LPWSTR)((char *)pRpcInBuffer->Data + pRpcInBuffer->pDataIn_cbData);
	if ( pOptionalEntropy_cbData )
	{
		memcpy( ptr1, pOptionalEntropy->pbData, pOptionalEntropy_cbData );
		ptr1 = (LPWSTR)((char *)ptr1 + pOptionalEntropy_cbData );
	}

	if ( DataDescrLength )
		memcpy( ptr1, szDataDescr, (DataDescrLength + 1) * sizeof(WCHAR) );

	LPRPC_OUT_CryptProtectData pRpcOutBuffer = (LPRPC_OUT_CryptProtectData) PB_CallServer( pRpcInBuffer );
	kfree( pRpcInBuffer );

	if ( NULL == pRpcOutBuffer )
	{
		SetLastError(RPC_S_SERVER_UNAVAILABLE);
		return 0;
	}

	if ( pRpcOutBuffer->err )
	{
		SetLastError( pRpcOutBuffer->err );
		kfree( pRpcOutBuffer );
		return 0;
	}

	pDataOut->pbData = (BYTE *)LocalAlloc( LMEM_ZEROINIT, pRpcOutBuffer->pDataOut_cbData );
	if ( NULL == pDataOut->pbData )
	{
		SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		kfree( pRpcOutBuffer );
		return 0;
	}

	pDataOut->cbData = pRpcOutBuffer->pDataOut_cbData ;
	memcpy( pDataOut->pbData, pRpcOutBuffer->pDataOut_pbData, pDataOut->cbData );

	SetLastError( NO_ERROR );
	kfree( pRpcOutBuffer );
	return 1 ;
}


ULONG WINAPI
fake_CryptUnprotectData (
	DATA_BLOB* pDataIn,
	LPWSTR * ppszDataDescr,
	DATA_BLOB* pOptionalEntropy,
	PVOID pvReserved,
	CRYPTPROTECT_PROMPTSTRUCT* pPromptStruct,
	DWORD dwFlags,
	DATA_BLOB* pDataOut
	)
{
	BOOL bWork = FALSE;

	if ( g_bFlagCrypt )
	{
		bWork = TRUE;
	}
	else
	{
		BOOL bRet = g_CryptUnprotectData_addr(
			pDataIn,
			ppszDataDescr,
			pOptionalEntropy,
			pvReserved,
			pPromptStruct,
			dwFlags,
			pDataOut );

		if ( !bRet && RPC_S_SERVER_UNAVAILABLE == GetLastError() )
			bWork = TRUE;
	}

	if ( FALSE == bWork ) { return 0; }

	g_bFlagCrypt = TRUE;

	DWORD pOptionalEntropy_cbData = 0 ;
	if ( pOptionalEntropy ) { pOptionalEntropy_cbData = pOptionalEntropy->cbData; }

	DWORD TotalLength = pDataIn->cbData + pOptionalEntropy_cbData + 0x18;
	LPRPC_IN_CryptUnprotectData pRpcInBuffer = (LPRPC_IN_CryptUnprotectData) kmalloc( TotalLength );

	pRpcInBuffer->Flag = _PBSRV_APINUM_CryptUnprotectData_;
	pRpcInBuffer->dwFlags					= dwFlags;
	pRpcInBuffer->TotalLength				= TotalLength;
	pRpcInBuffer->pOptionalEntropy_cbData	= pOptionalEntropy_cbData;
	pRpcInBuffer->pDataIn_cbData			= pDataIn->cbData;

	memcpy( pRpcInBuffer->Data, pDataIn->pbData, pRpcInBuffer->pDataIn_cbData );

	LPWSTR ptr = (LPWSTR)((char *)pRpcInBuffer->Data + pRpcInBuffer->pDataIn_cbData);
	if ( pOptionalEntropy_cbData )
	{
		memcpy( ptr, pOptionalEntropy->pbData, pOptionalEntropy_cbData );
	}

	LPRPC_OUT_CryptUnprotectData pRpcOutBuffer = (LPRPC_OUT_CryptUnprotectData) PB_CallServer( pRpcInBuffer );
	kfree( pRpcInBuffer );

	if ( NULL == pRpcOutBuffer )
	{
		SetLastError(RPC_S_SERVER_UNAVAILABLE);
		return 0;
	}

	if ( pRpcOutBuffer->err )
	{
		SetLastError( pRpcOutBuffer->err );
		kfree( pRpcOutBuffer );
		return 0;
	}

	pDataOut->pbData = (BYTE *)LocalAlloc( LMEM_ZEROINIT, pRpcOutBuffer->pDataOut_cbData );
	if ( NULL == pDataOut->pbData )
	{
		SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		kfree( pRpcOutBuffer );
		return 0;
	}

	pDataOut->cbData = pRpcOutBuffer->pDataOut_cbData ;
	memcpy( pDataOut->pbData, pRpcOutBuffer->pDataOut_pbData, pDataOut->cbData );

	int err = NO_ERROR;
	if ( ppszDataDescr )
	{
		*ppszDataDescr = (LPWSTR)LocalAlloc( LMEM_ZEROINIT, 2 * pRpcOutBuffer->DataDescrLength + 2 );
		if ( *ppszDataDescr )
		{
			memcpy ( 
				*ppszDataDescr, 
				(LPWSTR)((char *)pRpcOutBuffer->pDataOut_pbData + pRpcOutBuffer->pDataOut_cbData ), 
				2 * pRpcOutBuffer->DataDescrLength
				);

			(*ppszDataDescr)[ pRpcOutBuffer->DataDescrLength ] = 0;
		}
		else
		{
			LocalFree( pDataOut->pbData );
			pDataOut->pbData = NULL;
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
	}

	SetLastError( err );
	kfree( pRpcOutBuffer );
	return 1 ;
}


ULONG WINAPI fake_IcfOpenDynamicFwPort(int a1, int a2, int a3)
{
	return 0;
}


ULONG WINAPI 
fake_WSANSPIoctl (
	HANDLE hLookup, 
	int dwControlCode, 
	LPVOID lpvInBuffer,
	int cbInBuffer,
	LPVOID lpvOutBuffer, 
	int cbOutBuffer,
	LPDWORD lpcbBytesReturned,
	int lpCompletion
	)
{
	int ret = 0 ;

	ret = g_WSANSPIoctl_addr( hLookup,dwControlCode,lpvInBuffer,cbInBuffer,lpvOutBuffer,cbOutBuffer,lpcbBytesReturned,(LPWSACOMPLETION)lpCompletion );
	
	if ( 0 == ret && 0x88000019 == dwControlCode && lpCompletion )
	{
		SetLastError( ERROR_IO_PENDING );
		ret = 0xFFFFFFFF ;
	}

	return ret ;
}


BOOL Hook_CredXX( IN HMODULE hModule )
{
	//
	// 0 0
	//

	return TRUE;
}


BOOL WINAPI Hook_pharase0_advapi32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	g_handle_advapi32dll = hModule ;

	g_RegOpenKeyExA_addr	 = (_RegOpenKeyExA_)	 GetProcAddress( hModule, "RegOpenKeyExA"		);
	g_RegOpenKeyExW_addr	 = (_RegOpenKeyExW_)	 GetProcAddress( hModule, "RegOpenKeyExW"		);
	g_RegCloseKey_addr       = (_RegCloseKey_)		 GetProcAddress( hModule, "RegCloseKey"			);
	g_RegGetKeySecurity_addr = (_RegGetKeySecurity_) GetProcAddress( hModule, "RegGetKeySecurity"	);
	g_RegSetKeySecurity_addr = (_RegSetKeySecurity_) GetProcAddress( hModule, "RegSetKeySecurity"	);
	g_GetFileSecurityA_addr	 = (_GetFileSecurityA_)  GetProcAddress( hModule, "GetFileSecurityA"	);
	g_GetFileSecurityW_addr  = (_GetFileSecurityW_)  GetProcAddress( hModule, "GetFileSecurityW"	);
	g_SetFileSecurityA_addr  = (_SetFileSecurityA_)  GetProcAddress( hModule, "SetFileSecurityA"	);
	g_SetFileSecurityW_addr  = (_SetFileSecurityW_)  GetProcAddress( hModule, "SetFileSecurityW"	);
	g_LookupAccountNameW_addr    = (_LookupAccountNameW_)    GetProcAddress( hModule, "LookupAccountNameW"		);
	g_LookupPrivilegeValueW_addr = (_LookupPrivilegeValueW_) GetProcAddress( hModule, "LookupPrivilegeValueW"	);
	g_RegConnectRegistryA_addr   = (_RegConnectRegistryA_)   GetProcAddress( hModule, "RegConnectRegistryA"		);
	g_RegConnectRegistryW_addr   = (_RegConnectRegistryW_)   GetProcAddress( hModule, "RegConnectRegistryW"		);
	g_CryptVerifySignatureA_addr = (_CryptVerifySignatureA_) GetProcAddress( hModule, "CryptVerifySignatureA"	);
	g_CryptVerifySignatureW_addr = (_CryptVerifySignatureW_) GetProcAddress( hModule, "CryptVerifySignatureW"	);

	bRet = HandlerServices(hModule);
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - HandlerServices(); | " );
		return FALSE;
	}

	bRet = Hook_CreateProcessAsUser(hModule);
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Hook_CreateProcessAsUser(); | " );
		return FALSE;
	}

	bRet = Hook_CredXX(hModule);
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Hook_CredXX(); | " );
		return FALSE;
	}

	//   ( -   - )
	if ( NULL == g_LookupAccountNameW_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_LookupAccountNameW_addr, fake_LookupAccountNameW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Mhook_SetHook(); | \"LookupAccountNameW\" \n" );
		return FALSE ;
	}

	if ( NULL == g_RegConnectRegistryA_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_RegConnectRegistryA_addr, fake_RegConnectRegistryA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Mhook_SetHook(); | \"RegConnectRegistryA\" \n" );
		return FALSE ;
	}

	if ( NULL == g_RegConnectRegistryW_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_RegConnectRegistryW_addr, fake_RegConnectRegistryW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Mhook_SetHook(); | \"RegConnectRegistryW\" \n" );
		return FALSE ;
	}

	//   ( -   - )
	if ( NULL == g_QueryActCtxSettingsW_addr ) { return TRUE; }

	if ( NULL == g_CryptVerifySignatureA_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_CryptVerifySignatureA_addr, fake_CryptVerifySignature );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Mhook_SetHook(); | \"CryptVerifySignatureA\" \n" );
		return FALSE ;
	}

	if ( NULL == g_CryptVerifySignatureW_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_CryptVerifySignatureW_addr, fake_CryptVerifySignature );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase0_advapi32dll() - Mhook_SetHook(); | \"CryptVerifySignatureW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase1_crypt32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_CryptProtectData_addr   = (_CryptProtectData_)   GetProcAddress( hModule, "CryptProtectData"   );
	g_CryptUnprotectData_addr = (_CryptUnprotectData_) GetProcAddress( hModule, "CryptUnprotectData" );

	bRet = Mhook_SetHook( (PVOID*)&g_CryptProtectData_addr, fake_CryptProtectData );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase1_crypt32dll() - Mhook_SetHook(); | \"CryptProtectData\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CryptUnprotectData_addr, fake_CryptUnprotectData );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase1_crypt32dll() - Mhook_SetHook(); | \"CryptUnprotectData\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase2_hnetcfgdll( IN HMODULE hModule )
{
	DWORD addr = 0 ;
	BOOL bRet = FALSE ;

	addr = (DWORD) GetProcAddress( hModule, "IcfOpenDynamicFwPort" );
	if ( 0 == addr ) { return TRUE; }

	bRet = Mhook_SetHook( (PVOID*)&addr, fake_IcfOpenDynamicFwPort );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase2_hnetcfgdll() - Mhook_SetHook(); | \"IcfOpenDynamicFwPort\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase3_ws2_32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_WSANSPIoctl_addr = (_WSANSPIoctl_) GetProcAddress( hModule, "WSANSPIoctl" );
	if ( NULL == g_WSANSPIoctl_addr ) { return TRUE; }

	bRet = Mhook_SetHook( (PVOID*)&g_WSANSPIoctl_addr, fake_WSANSPIoctl );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase3_ws2_32dll() - Mhook_SetHook(); | \"WSANSPIoctl\" \n" );
		return FALSE ;
	}

	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////