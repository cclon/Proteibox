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
#include <WinSock.h>
#include "HookHelper.h"
#include "PBRpcToken.h"
#include "PBRpcWinsock.h"

#pragma comment(lib, "ws2_32.lib") 

//////////////////////////////////////////////////////////////////////////


// Winsock
static HOOKINFOLittleRpcss g_HookInfoWinsock_Array [] = 
{
	{ WS2_32_TAG, bind_TAG, "bind", "", 0, NULL, fake_bind },
	{ WS2_32_TAG, listen_TAG, "listen", "", 0, NULL, fake_listen },
	{ WS2_32_TAG, WSASocketW_TAG, "WSASocketW", "", 0, NULL, fake_WSASocketW },
	{ ADVAPI32_TAG, OpenThreadToken_TAG, "OpenThreadToken", "", 0, NULL, fake_OpenThreadToken },
	{ ADVAPI32_TAG, RegOpenKeyExW_TAG, "RegOpenKeyExW", "", 0, NULL, fake_RegOpenKeyExW },
	{ ADVAPI32_TAG, RegQueryValueExW_TAG, "RegQueryValueExW", "", 0, NULL, fake_RegQueryValueExW },

};

#define GetWinsockFunc( X )	g_HookInfoWinsock_Array[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL InitHook_pharase2_WinSock()
{
	int i=0;
	BOOL bRet = FALSE;
	char* lpFuncName = NULL;
	LPHOOKINFOLittleRpcss pArray = g_HookInfoWinsock_Array ;
	int ArrayCounts = ARRAYSIZEOF( g_HookInfoWinsock_Array );
	HMODULE* phModuleArray = __ProcModulesInfo->hModuleArrays;

	WSADATA wsaData;
	int iErr = WSAStartup( 2, &wsaData );
	if ( iErr != 0 )
	{
		MYTRACE( "err! | InitHook_pharase2_WinSock() - WSAStartup(); | Could not initialize WinSock \n" );
		return FALSE;
	}

	for( i=0; i<ArrayCounts; i++ )
	{
		if ( phModuleArray[KernelBase_TAG] && pArray[i].FunctionNameEx )
			lpFuncName	= pArray[i].FunctionNameEx;
		else
			lpFuncName	= pArray[i].FunctionName;

		pArray[i].OrignalAddress = (PVOID) GetProcAddress( phModuleArray[pArray[i].DllTag], lpFuncName );
	}

	for( i=0; i<ArrayCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( "err! | InitHook_pharase2_WinSock() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
			return FALSE ;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI
fake_bind ( 
	SOCKET s, 
	const struct sockaddr FAR* name,
	int namelen
	)
{
	return 0;
}


ULONG WINAPI
fake_listen(
	SOCKET s,
	int backlog
	)
{
	return 0;
}


ULONG WINAPI
fake_WSASocketW (
	IN  INT af,
    IN  INT type,
    IN  INT protocol,
    IN  LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN  GROUP g,
    IN  DWORD dwFlags
	)
{
	WSASetLastError(WSAEACCES);
	return 0xFFFFFFFF;
}


ULONG WINAPI
fake_OpenThreadToken (
	HANDLE ThreadHandle, 
	int DesiredAccess, 
	BOOL OpenAsSelf, 
	PHANDLE TokenHandle
	)
{
	ULONG ret = 0 ;
	_OpenThreadToken_ OrignalAddr = (_OpenThreadToken_) GetWinsockFunc(OpenThreadToken);
	
	ret = OrignalAddr( ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle );
	if ( ret && GetLastError() == ERROR_BAD_IMPERSONATION_LEVEL && OpenAsSelf )
		ret = OrignalAddr( ThreadHandle, DesiredAccess, 1, TokenHandle );

	return ret;
}


ULONG WINAPI
fake_RegOpenKeyExW (
	HKEY hKey,
	LPCWSTR lpSubKey,
	DWORD ulOptions,
	REGSAM samDesired,
	PHKEY phkResult
	)
{
	ULONG ret = 0 ;
	_RegOpenKeyExW_ OrignalAddr = (_RegOpenKeyExW_) GetWinsockFunc(RegOpenKeyExW);

	ret = OrignalAddr( hKey, lpSubKey, ulOptions, samDesired, phkResult );
	if ( ERROR_BAD_IMPERSONATION_LEVEL != ret ) { return ret; }

	int err = GetLastError();
	HANDLE TokenHandle = NULL;
	if ( OpenThreadToken((HANDLE)0xFFFFFFFE, TOKEN_IMPERSONATE, 1, &TokenHandle) && TokenHandle )
	{
		g_SetThreadToken_addr( 0, 0 );
		ret = OrignalAddr( hKey, lpSubKey, ulOptions, samDesired, (PHKEY)TokenHandle );
		g_SetThreadToken_addr( 0, TokenHandle );
	}

	SetLastError(err);
	return ret;
}


ULONG WINAPI
fake_RegQueryValueExW (
	HKEY hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData
	)
{
	ULONG ret = 0 ;
	_RegQueryValueExW_ OrignalAddr = (_RegQueryValueExW_) GetWinsockFunc(RegQueryValueExW);

	ret = OrignalAddr( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );
	if ( ERROR_BAD_IMPERSONATION_LEVEL != ret ) { return ret; }

	int err = GetLastError();
	HANDLE TokenHandle = NULL;
	if ( OpenThreadToken((HANDLE)0xFFFFFFFE, TOKEN_IMPERSONATE, 1, &TokenHandle) && TokenHandle )
	{
		g_SetThreadToken_addr( 0, 0 );
		ret = OrignalAddr( hKey, lpValueName, lpReserved, lpType, lpData, (LPDWORD)TokenHandle );
		g_SetThreadToken_addr( 0, TokenHandle );
	}

	SetLastError(err);
	return ret;
}



///////////////////////////////   END OF FILE   ///////////////////////////////