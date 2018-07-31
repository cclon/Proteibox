/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/04 [4:7:2011 - 10:33]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\ExportFuncData.cpp
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
#include "MemoryManager.h"
#include "Exportfunc.h"
#include "ExportFuncData.h"

#pragma warning(disable: 4995)

//////////////////////////////////////////////////////////////////////////


_OpenSCManagerW_ g_OpenSCManagerW_addr = NULL ;
_OpenServiceW_ g_OpenServiceW_addr = NULL ;
_StartServiceW_ g_StartServiceW_addr = NULL ;
_CloseServiceHandle_ g_CloseServiceHandle_addr = NULL ;
_CreateServiceW_ g_CreateServiceW_addr = NULL ;


RPCGLOBALINFO g_RpcGlobalInfo = {} ;

#define __RpcPortName	L"\\RPC Control\\PBSvcPort"


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


LPWSTR GetRegShellCommandString( LPWSTR lpValueName1, LPWSTR lpValueName2 )
{
	ULONG nCounts = 1, ResultLength = 0, cchExpandedSz = 0 ;
	HANDLE hRootKey = NULL, hKey = NULL ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szSubKey[ MAX_PATH ] = L"" ;
	PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = NULL ;
	LPWSTR pValueData = NULL ;

	hRootKey = OpenSpecRootKey( lpValueName1 );
	if ( NULL == hRootKey ) { return NULL ; }

	wcscpy( szSubKey, L"shell\\" );
	wcscat( szSubKey, lpValueName2 );
	wcscat( szSubKey, L"\\command" );

	KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) kmalloc( 0x400 );

	while ( nCounts != 8 )
	{
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hRootKey, NULL );
		RtlInitUnicodeString( &KeyName, szSubKey );

		status = ZwOpenKey( &hKey, KEY_READ, &ObjAtr );
		if ( NT_SUCCESS(status) )
		{
			ZwClose( hRootKey );
			hRootKey = hKey;
			hKey = NULL ;

			RtlInitUnicodeString( &KeyName, NULL );
			status = ZwQueryValueKey( hRootKey, &KeyName, KeyValuePartialInformation, KeyInfo, 0x3E0, &ResultLength );
			if ( ! NT_SUCCESS(status) || 0 == KeyInfo->DataLength ) { break; }

			pValueData = (LPWSTR) kmalloc( KeyInfo->DataLength + 4 );	
			memcpy( pValueData, KeyInfo->Data, KeyInfo->DataLength );

			if ( REG_EXPAND_SZ == KeyInfo->Type )
			{
				cchExpandedSz = ExpandEnvironmentStringsW( pValueData, 0, 0 );
				if ( cchExpandedSz )
				{
					LPWSTR pTemp = (LPWSTR) kmalloc( 2 * cchExpandedSz + 0x10 );
					ExpandEnvironmentStringsW( pValueData, pTemp, cchExpandedSz );
					pTemp[ cchExpandedSz ] = 0;

					kfree( pValueData );
					pValueData = pTemp;
				}
				else
				{
					kfree( pValueData );
					pValueData = NULL ;
				}
			}

			break;
		}

		if ( status != STATUS_OBJECT_NAME_NOT_FOUND && status != STATUS_OBJECT_PATH_NOT_FOUND ) { break; }

		RtlInitUnicodeString( &KeyName, NULL );
		RtlZeroMemory( KeyInfo, 0x400 );

		status = ZwQueryValueKey( hRootKey, &KeyName, KeyValuePartialInformation, KeyInfo, 0x3E0, &ResultLength );
		if ( ! NT_SUCCESS(status) || 0 == KeyInfo->DataLength ) { break; }

		ZwClose( hRootKey );

		hRootKey = OpenSpecRootKey( (LPWSTR)KeyInfo->Data );
		++ nCounts ;
		if ( NULL == hRootKey )
		{
			kfree( KeyInfo );
			return pValueData ;
		}

	} // end-of-while

	if ( hRootKey ) { ZwClose( hRootKey ); }

	kfree( KeyInfo );
	return pValueData ;
}



HANDLE OpenSpecRootKey( LPWSTR ValueName )
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	WCHAR szName[ MAX_PATH ] = L"" ;
	HKEY Type = HKEY_CURRENT_USER ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE KeyHandle = NULL ;
	LPWSTR ptr = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	while ( TRUE )
	{
		wcscpy( szName, L"\\REGISTRY\\" );

		if ( Type == HKEY_CURRENT_USER )
		{
			wcscat( szName, L"USER\\CURRENT" );
		}
		else if ( Type == HKEY_LOCAL_MACHINE )
		{
			wcscat( szName, L"MACHINE" );
		}

		wcscat( szName, L"\\SOFTWARE\\CLASSES\\" );

		ptr = &szName[ wcslen(szName) ];
		wcscpy( ptr, ValueName );

		RtlInitUnicodeString( &KeyName, szName );
		status = ZwOpenKey( &KeyHandle, KEY_READ, &ObjAtr );
		if ( NT_SUCCESS(status) ) { return KeyHandle; }

		if ( PB_IsWow64() )
		{
			wcscpy( ptr, L"Wow6432Node\\" );
			wcscat( ptr, ValueName );

			RtlInitUnicodeString( &KeyName, szName );
			status = ZwOpenKey( &KeyHandle, KEY_READ, &ObjAtr );
			if ( NT_SUCCESS(status) ) { return KeyHandle; }
		}

		if ( Type != HKEY_CURRENT_USER ) { return NULL; }
		Type = HKEY_LOCAL_MACHINE ;
	}

	return NULL;
}



BOOL ConnectRpcPort()
{
	NTSTATUS status = STATUS_SUCCESS ;
	UNICODE_STRING uniPortName ;
	SECURITY_QUALITY_OF_SERVICE SecurityQos ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( pNode->sLPC.PortHandle )
		return TRUE;

	SecurityQos.Length = 0xC ;
	SecurityQos.ImpersonationLevel	= SecurityImpersonation ;
	SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING ;
	SecurityQos.EffectiveOnly		= TRUE ;

	RtlInitUnicodeString( &uniPortName, __RpcPortName );
	status = ZwConnectPort( 
		&pNode->sLPC.PortHandle,
		&uniPortName,
		&SecurityQos,
		NULL,
		NULL,
		&pNode->sLPC.MaxMessageLength,
		NULL,
		NULL 
		);

	if ( ! NT_SUCCESS(status) ) 
	{
		MYTRACE( L"error! | ConnectRpcPort() - ZwConnectPort(); | status=0x%08lx \n", status );
		return FALSE; 
	}

	ZwRegisterThreadTerminatePort( pNode->sLPC.PortHandle );
	
	pNode->sLPC.LpcHeaderLength = 0x18;
	if ( PB_IsWow64() ) { pNode->sLPC.LpcHeaderLength += 0x10; }

	pNode->sLPC.MaxMessageLength -= pNode->sLPC.LpcHeaderLength ;
	return TRUE;
}



///////////////////////////////   END OF FILE   ///////////////////////////////