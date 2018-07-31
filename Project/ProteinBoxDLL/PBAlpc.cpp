/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/03 [3:12:2010 - 17:44]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBAlpc.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "MemoryManager.h"
#include "PBAlpcData.h"
#include "PBAlpc.h"

#include <RpcDce.h>
#pragma comment( lib, "Rpcrt4.lib" )

//////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_NtCreatePort (
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG MaxConnectInfoLength,
	IN ULONG MaxDataLength,
	IN ULONG MaxPoolUsage
	)
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtCreatePort_ OrignalAddress = (_NtCreatePort_) GetAlpcFunc(NtCreatePort) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreatePort(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( PortHandle, &ObjAtr, MaxConnectInfoLength, MaxDataLength, MaxPoolUsage );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( PortHandle, &ObjAtr, MaxConnectInfoLength, MaxDataLength, MaxPoolUsage );
	if ( STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }
	
	status = CreateDirOrLinks( OrignalPath, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	status = OrignalAddress( PortHandle, &ObjAtr, MaxConnectInfoLength, MaxDataLength, MaxPoolUsage );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN PPORT_VIEW ClientView,
	IN PREMOTE_PORT_VIEW ServerView,
	OUT PULONG MaxMessageLength,
	IN PVOID ConnectionInformation,
	OUT PULONG ConnectionInformationLength
	)
{
	UNICODE_STRING uniObjectName ;
	ULONG_PTR RedirectedPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, RedirectedPathDummy = NULL ;
	_NtConnectPort_ OrignalAddress = (_NtConnectPort_) GetAlpcFunc(NtConnectPort) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPathEx( PortName, NULL, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreatePort(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RedirectedPathLength = (wcslen(RedirectedPath) +1) * sizeof(WCHAR) ;
	RedirectedPathDummy = (LPWSTR) kmalloc( RedirectedPathLength );

	memcpy( RedirectedPathDummy, RedirectedPath, RedirectedPathLength );
	RtlInitUnicodeString( &uniObjectName, RedirectedPathDummy );

	if ( PB_StartCOM_Ex( OrignalPath ) )
	{
		do
		{
			Sleep( 250 );
			status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );
		}
		while ( STATUS_OBJECT_NAME_NOT_FOUND == status );
	}

	kfree( RedirectedPathDummy );
	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtSecureConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN OUT PPORT_VIEW ClientView OPTIONAL,
	IN PSID ServerSid OPTIONAL,
	IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
	OUT PULONG MaxMessageLength OPTIONAL,
	IN OUT PVOID ConnectionInformation OPTIONAL,
	IN OUT PULONG ConnectionInformationLength OPTIONAL
	)
{
	UNICODE_STRING uniObjectName ;
	ULONG_PTR RedirectedPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, RedirectedPathDummy = NULL ;
	_NtSecureConnectPort_ OrignalAddress = (_NtSecureConnectPort_) GetAlpcFunc(NtSecureConnectPort) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPathEx( PortName, NULL, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtSecureConnectPort(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerSid, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerSid, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND == status ) 
	{
		RedirectedPathLength = (wcslen(RedirectedPath) +1) * sizeof(WCHAR) ;
		RedirectedPathDummy = (LPWSTR) kmalloc( RedirectedPathLength );

		memcpy( RedirectedPathDummy, RedirectedPath, RedirectedPathLength );
		RtlInitUnicodeString( &uniObjectName, RedirectedPathDummy );

		if ( PB_StartCOM_Ex( OrignalPath ) )
		{
			do
			{
				Sleep( 250 );
				status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerSid, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );
			}
			while ( STATUS_OBJECT_NAME_NOT_FOUND == status );
		}

		kfree( RedirectedPathDummy );
	}

	if ( STATUS_SERVER_SID_MISMATCH == status )
	{
		status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, NULL, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( PortHandle, &uniObjectName, Qos, ClientView, ServerSid, ServerView, MaxMessageLength, ConnectionInformation, ConnectionInformationLength );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtAlpcCreatePort(
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes
	)
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtAlpcCreatePort_ OrignalAddress = (_NtAlpcCreatePort_) GetAlpcFunc(NtAlpcCreatePort) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreatePort(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( PortHandle, &ObjAtr, PortAttributes );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( PortHandle, &ObjAtr, PortAttributes );
	if ( STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }

	status = CreateDirOrLinks( OrignalPath, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	status = OrignalAddress( PortHandle, &ObjAtr, PortAttributes );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}



ULONG WINAPI
fake_NtAlpcConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes,
	IN ULONG Flags,
	IN PVOID RequiredServerSid, 
	IN PORT_MESSAGE *ConnectionMessage,
	IN PULONG BufferLength,
	OUT LPALPC_MESSAGE_ATTRIBUTES OutMessageAttributes,
	IN LPALPC_MESSAGE_ATTRIBUTES InMessageAttributes,
	IN PLARGE_INTEGER Timeout
	)
{
	UNICODE_STRING uniObjectName ;
	ULONG_PTR RedirectedPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, RedirectedPathDummy = NULL ;
	_NtAlpcConnectPort_ OrignalAddress = (_NtAlpcConnectPort_) GetAlpcFunc(NtAlpcConnectPort) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPathEx( PortName, NULL, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreatePort(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( PortHandle, &uniObjectName, ObjectAttributes, PortAttributes, Flags, RequiredServerSid,
			ConnectionMessage, BufferLength, OutMessageAttributes, InMessageAttributes, Timeout );
		goto _over_ ;
	}

	// 灰名单; 过滤
	if ( 0 == wcsicmp(g_BoxInfo.ProcessName, g_PBDcomLaunch_exe) && 0 == wcsicmp(OrignalPath, L"\\RPC Control\\epmapper") )
	{
		status = STATUS_OBJECT_NAME_NOT_FOUND ;
		goto _over_ ;
	}

	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( PortHandle, &uniObjectName, ObjectAttributes, PortAttributes, Flags, RequiredServerSid,
		ConnectionMessage, BufferLength, OutMessageAttributes, InMessageAttributes, Timeout );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND == status ) 
	{
		RedirectedPathLength = (wcslen(RedirectedPath) +1) * sizeof(WCHAR) ;
		RedirectedPathDummy = (LPWSTR) kmalloc( RedirectedPathLength );

		memcpy( RedirectedPathDummy, RedirectedPath, RedirectedPathLength );
		RtlInitUnicodeString( &uniObjectName, RedirectedPathDummy );

		if ( PB_StartCOM_Ex( OrignalPath ) )
		{
			do
			{
				Sleep( 250 );
				status = OrignalAddress( PortHandle, &uniObjectName, ObjectAttributes, PortAttributes, Flags, RequiredServerSid,
					ConnectionMessage, BufferLength, OutMessageAttributes, InMessageAttributes, Timeout );
			}
			while ( STATUS_OBJECT_NAME_NOT_FOUND == status );
		}

		kfree( RedirectedPathDummy );
	}

	if ( STATUS_SERVER_SID_MISMATCH == status )
	{
		RequiredServerSid = NULL ;
		status = OrignalAddress( PortHandle, &uniObjectName, ObjectAttributes, PortAttributes, Flags, NULL,
			ConnectionMessage, BufferLength, OutMessageAttributes, InMessageAttributes, Timeout );
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( PortHandle, &uniObjectName, ObjectAttributes, PortAttributes, Flags, RequiredServerSid,
		ConnectionMessage, BufferLength, OutMessageAttributes, InMessageAttributes, Timeout );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}



ULONG WINAPI
fake_NtAlpcQueryInformation (
	OUT PHANDLE PortHandle,
	IN ALPC_PORT_INFORMATION_CLASS PortInformationClass, 
	OUT PVOID PortInformation, 
	IN ULONG Length, 
	OUT PULONG ReturnLength
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtAlpcQueryInformation_ OrignalAddress = (_NtAlpcQueryInformation_) GetAlpcFunc(NtAlpcQueryInformation) ;

	status = OrignalAddress( PortHandle, PortInformationClass, PortInformation, Length, ReturnLength );

	if ( STATUS_SERVER_SID_MISMATCH == status ) { status = STATUS_SUCCESS; }
	return status ;
}



ULONG WINAPI
fake_NtAlpcQueryInformationMessage (
	OUT PHANDLE PortHandle,
	IN PORT_MESSAGE *PortMessage, 
	IN ALPC_MESSAGE_INFORMATION_CLASS MessageInformationClass,
	OUT PVOID MessageInformation,
	IN ULONG Length, 
	OUT PULONG ReturnLength
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtAlpcQueryInformationMessage_ OrignalAddress = (_NtAlpcQueryInformationMessage_) GetAlpcFunc(NtAlpcQueryInformationMessage) ;

	status = OrignalAddress( PortHandle, PortMessage, MessageInformationClass, MessageInformation, Length, ReturnLength );

	if ( STATUS_BAD_IMPERSONATION_LEVEL == status ) { status = STATUS_SUCCESS; }
	return status ;
}



ULONG WINAPI
fake_NtImpersonateClientOfPort (
    HANDLE PortHandle,
    PPORT_MESSAGE ClientMessage
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtImpersonateClientOfPort_ OrignalAddress = (_NtImpersonateClientOfPort_) GetAlpcFunc(NtImpersonateClientOfPort) ;

	status = OrignalAddress( PortHandle, ClientMessage );

	if ( FALSE == g_bSID_is_S_1_5_18 )
	{
		status = MyRpcImpersonateClient();
	}

	return status;
}



ULONG WINAPI
fake_NtAlpcImpersonateClientOfPort (
    HANDLE PortHandle,
    PPORT_MESSAGE PortMessage,
	PVOID Reserved
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtAlpcImpersonateClientOfPort_ OrignalAddress = (_NtAlpcImpersonateClientOfPort_) GetAlpcFunc(NtAlpcImpersonateClientOfPort) ;

	status = OrignalAddress( PortHandle, PortMessage, Reserved );

	if ( FALSE == g_bSID_is_S_1_5_18 )
	{
		status = MyRpcImpersonateClient();
	}

	return status;
}



ULONG WINAPI
fake_NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtCreateEvent_ OrignalAddress = (_NtCreateEvent_) GetAlpcFunc(NtCreateEvent) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath )
	{
		status = OrignalAddress( EventHandle, DesiredAccess, ObjectAttributes, EventType, InitialState );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreateEvent(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr, EventType, InitialState );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr, EventType, InitialState );

	if ( STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }

	status = CreateDirOrLinks( OrignalPath, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr, EventType, InitialState );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}



ULONG WINAPI
fake_NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtOpenEvent_ OrignalAddress = (_NtOpenEvent_) GetAlpcFunc(NtOpenEvent) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath ) 
	{
		status = OrignalAddress( EventHandle, DesiredAccess, ObjectAttributes );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, NULL );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtOpenEvent(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( EventHandle, DesiredAccess, &ObjAtr );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}



ULONG WINAPI
fake_NtCreateMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtCreateMutant_ OrignalAddress = (_NtCreateMutant_) GetAlpcFunc(NtCreateMutant) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath ) 
	{
		status = OrignalAddress( MutantHandle, DesiredAccess, ObjectAttributes, InitialOwner );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreateMutant(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr, InitialOwner );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr, InitialOwner );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) 
		{ 
			status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr, InitialOwner );
		}
	}

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtOpenMutant_ OrignalAddress = (_NtOpenMutant_) GetAlpcFunc(NtOpenMutant) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath ) 
	{
		status = OrignalAddress( MutantHandle, DesiredAccess, ObjectAttributes );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, NULL );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtOpenMutant(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( MutantHandle, DesiredAccess, &ObjAtr );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtCreateSemaphore (
    IN PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtCreateSemaphore_ OrignalAddress = (_NtCreateSemaphore_) GetAlpcFunc(NtCreateSemaphore) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath )
	{
		status = OrignalAddress( SemaphoreHandle, DesiredAccess, ObjectAttributes, InitialCount, MaximumCount );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreateSemaphore(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr, InitialCount, MaximumCount );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr, InitialCount, MaximumCount );

	if ( STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }

	status = CreateDirOrLinks( OrignalPath, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr, InitialCount, MaximumCount );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}



ULONG WINAPI
fake_NtOpenSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtOpenSemaphore_ OrignalAddress = (_NtOpenSemaphore_) GetAlpcFunc(NtOpenSemaphore) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath )
	{
		status = OrignalAddress( SemaphoreHandle, DesiredAccess, ObjectAttributes );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, NULL );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtOpenSemaphore(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( SemaphoreHandle, DesiredAccess, &ObjAtr );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtCreateSection_ OrignalAddress = (_NtCreateSection_) GetAlpcFunc(NtCreateSection) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath )
	{
		status = OrignalAddress( SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, g_SecurityDescriptor );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreateSection(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle );

	if ( STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }

	status = CreateDirOrLinks( OrignalPath, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtOpenSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtOpenSection_ OrignalAddress = (_NtOpenSection_) GetAlpcFunc(NtOpenSection) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 取原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetAlpcPath( ObjectAttributes, &OrignalPath, &RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( NULL == OrignalPath )
	{
		status = OrignalAddress( SectionHandle, DesiredAccess, ObjectAttributes );
		goto _over_ ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, NULL, NULL, NULL );
	if ( ObjectAttributes ) { ObjAtr.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ; }

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, OrignalPath, &bIsWhite, &bIsBlack );

	// 黑名单; 禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtOpenSection(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 白名单; 放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr );
		goto _over_ ;
	}

	// 灰名单; 过滤
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr );

	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( status >= 0 ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = OrignalAddress( SectionHandle, DesiredAccess, &ObjAtr );

	if ( FALSE == bIsWhite && STATUS_ACCESS_DENIED == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


///////////////////////////////   END OF FILE   ///////////////////////////////