/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/11 [11:1:2012 - 12:23]
* MODULE : \PBRpcSs\PBRpcSrv.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "HookHelper.h"
#include "PBRpcToken.h"

//////////////////////////////////////////////////////////////////////////

static DWORD g_dwTlsIndex_TokenHandle = 0 ;

// Token
static HOOKINFOLittleRpcss g_HookInfoToken_Array [] = 
{
	{ Ntdll_TAG, RtlAdjustPrivilege_TAG, "RtlAdjustPrivilege", "", 0, NULL, fake_RtlAdjustPrivilege },
	{ Ntdll_TAG, NtSetInformationProcess_TAG, "NtSetInformationProcess", "", 0, NULL, fake_NtSetInformationProcess },
	{ ADVAPI32_TAG, AccessCheckByType_TAG, "AccessCheckByType", "", 0, NULL, fake_AccessCheckByType },
	{ ADVAPI32_TAG, SetThreadToken_TAG, "SetThreadToken", "", 0, NULL, fake_SetThreadToken },
	{ ADVAPI32_TAG, GetTokenInformation_TAG, "GetTokenInformation", "", 0, NULL, fake_GetTokenInformation },

};

#define GetTokenFunc( X )	g_HookInfoToken_Array[ X##_TAG ].OrignalAddress

_SetThreadToken_ g_SetThreadToken_addr = NULL;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL InitHook_pharase1_Token()
{
	int i=0;
	BOOL bRet = FALSE;
	char* lpFuncName = NULL;
	LPHOOKINFOLittleRpcss pArray = g_HookInfoToken_Array ;
	int ArrayCounts = ARRAYSIZEOF( g_HookInfoToken_Array );
	HMODULE* phModuleArray = __ProcModulesInfo->hModuleArrays;

	for( i=0; i<ArrayCounts; i++ )
	{
		if ( phModuleArray[KernelBase_TAG] && pArray[i].FunctionNameEx )
			lpFuncName	= pArray[i].FunctionNameEx;
		else
			lpFuncName	= pArray[i].FunctionName;

		pArray[i].OrignalAddress = (PVOID) GetProcAddress( phModuleArray[pArray[i].DllTag], lpFuncName );
	}

	g_SetThreadToken_addr = (_SetThreadToken_) GetTokenFunc(SetThreadToken);

	for( i=0; i<ArrayCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( "err! | InitHook_pharase1_Token() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
			return FALSE ;
		}
	}

	g_dwTlsIndex_TokenHandle = TlsAlloc();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI
fake_RtlAdjustPrivilege (
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    )
{
	_RtlAdjustPrivilege_ OrignalAddr = (_RtlAdjustPrivilege_) GetTokenFunc(RtlAdjustPrivilege);

	NTSTATUS status = OrignalAddr( Privilege, Enable, Client, WasEnabled );
	if ( STATUS_PRIVILEGE_NOT_HELD == status && SE_ASSIGNPRIMARYTOKEN_PRIVILEGE == Privilege )
	{
		status = STATUS_SUCCESS;
	}

	return (ULONG)status;
}


ULONG WINAPI
fake_NtSetInformationProcess (
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
    )
{
	_NtSetInformationProcess_ OrignalAddr = (_NtSetInformationProcess_) GetTokenFunc(NtSetInformationProcess);

	NTSTATUS status = OrignalAddr( ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength );
	if ( STATUS_PRIVILEGE_NOT_HELD == status && ProcessAccessToken == ProcessInformationClass )
	{
		status = STATUS_SUCCESS;
	}

	return (ULONG)status;
}


ULONG WINAPI
fake_AccessCheckByType(
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	PSID PrincipalSelfSid,
	HANDLE ClientToken,
	DWORD DesiredAccess,
	POBJECT_TYPE_LIST ObjectTypeList,
	DWORD ObjectTypeListLength,
	PGENERIC_MAPPING GenericMapping,
	PPRIVILEGE_SET PrivilegeSet,
	LPDWORD PrivilegeSetLength,
	LPDWORD GrantedAccess,
	LPBOOL AccessStatus
	)
{
	*GrantedAccess = 0xFFFFFFFF;
	*AccessStatus = 1;
	SetLastError(NO_ERROR);
	return 1;
}


ULONG WINAPI
fake_SetThreadToken (
	PHANDLE ThreadHandle, 
	LPVOID TokenHandle
	)
{
	ULONG ret = 0;
	BOOL bRet = FALSE;
	HANDLE DuplicateTokenHandle = NULL;
	_SetThreadToken_ OrignalAddr = (_SetThreadToken_) GetTokenFunc(SetThreadToken);

	TlsSetValue( g_dwTlsIndex_TokenHandle, 0 );
	ret = OrignalAddr( ThreadHandle, TokenHandle );

	do 
	{
		if ( ret || GetLastError() != ERROR_ACCESS_DENIED || ThreadHandle )
			break;

		bRet = OpenProcessToken( (HANDLE)0xFFFFFFFF, TOKEN_ALL_ACCESS, (PHANDLE)&ThreadHandle );
		if ( FALSE == bRet )
			break;

		bRet = DuplicateToken( ThreadHandle, SecurityImpersonation, &DuplicateTokenHandle );
		CloseHandle(ThreadHandle);
		if ( FALSE == bRet )
			break;

		ret = OrignalAddr( 0, DuplicateTokenHandle );
		CloseHandle(DuplicateTokenHandle);
		if ( FALSE == bRet )
			break;

		TlsSetValue( g_dwTlsIndex_TokenHandle, TokenHandle );
		SetLastError(NO_ERROR);
		return 1;

	} while (FALSE);
	
	return ret;
}


ULONG WINAPI
fake_GetTokenInformation (
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	LPVOID TokenInformation,
	DWORD TokenInformationLength,
	PDWORD ReturnLength
	)
{
	_GetTokenInformation_ OrignalAddr = (_GetTokenInformation_) GetTokenFunc(GetTokenInformation);

	LPVOID TokenHandle_new = TlsGetValue(g_dwTlsIndex_TokenHandle);
	if ( !TokenHandle_new )
		TokenHandle_new = TokenHandle;

	return (ULONG)OrignalAddr( TokenHandle_new, TokenInformationClass, TokenInformation, TokenInformationLength, ReturnLength );
}


///////////////////////////////   END OF FILE   ///////////////////////////////