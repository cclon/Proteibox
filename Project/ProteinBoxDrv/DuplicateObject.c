/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/28 [28:12:2010 - 16:09]
* MODULE : e:\Data\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\DuplicateObject.c
* 
* Description:
*
*   处理复制句柄的操作  
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "Security.h"
#include "ObjectHook.h"
#include "DuplicateObject.h"

//////////////////////////////////////////////////////////////////////////


NTSTATUS
Ioctl_DuplicateObject (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	LUID SeDebugPrivilege ;
	CLIENT_ID ClientId ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	PROCESS_BASIC_INFORMATION ProcessBasicInfo ;
	PHANDLE pTargetHandle = NULL ;
	WCHAR UserSid[ MAX_PATH ] = L"" ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;
	LPIOCTL_DUPLICATEOBJECT_BUFFER Buffer = NULL ;
	ULONG Options = 0, HandleAttributes = 0, ReturnLength = 0 ;
	HANDLE SourceHandle = NULL, FuckedHandle = NULL, SourceProcessHandle = NULL ;
	HANDLE TargetProcessHandle = NULL, TempHandle = NULL, TargetHandle = NULL ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_DuplicateObject(); \n" );
	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_DUPLICATEOBJECT_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_DuplicateObject() - getIoctlBufferBody(); | Body地址不合法 \n" );
			return STATUS_UNSUCCESSFUL ;
		}

		SourceHandle	= Buffer->SourceHandle ;
		pTargetHandle	= Buffer->pTargetHandle ;
		FuckedHandle	= Buffer->FuckedHandle ;
		Options			= Buffer->Options ;

		ProbeForWrite( pTargetHandle, 4, 4 );
	
		if ( Options & 0x40000 )
		{
			HandleAttributes = OBJ_INHERIT ;
			Options &= 0xFFFBFFFF ;
		}

		status = ZwQueryInformationProcess( FuckedHandle, ProcessBasicInformation, &ProcessBasicInfo, 0x18, &ReturnLength );
		if ( status < 0 ) { return status; }

		if ( kgetnodePD( ProcessBasicInfo.UniqueProcessId ) ) { return STATUS_ACCESS_DENIED; }

		SeDebugPrivilege.HighPart = 0;
		SeDebugPrivilege.LowPart = 0x14 ;

		if ( FALSE == SeSinglePrivilegeCheck( SeDebugPrivilege, ExGetPreviousMode() ) )
		{
			status = RtlGetUserSid( FuckedHandle, UserSid );
			if ( status < 0 ) { return status; }

			if ( _wcsicmp( ProcessNode->pNode_C->RegisterUserID, UserSid ) ) { return STATUS_ACCESS_DENIED; }
		}

		ClientId.UniqueProcess = (HANDLE) ProcessBasicInfo.UniqueProcessId ;
		ClientId.UniqueThread = 0;

		InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

		status = ZwOpenProcess( &FuckedHandle, PROCESS_DUP_HANDLE, &ObjectAttributes, &ClientId );
		if ( status < 0 ) { return status; }

		if ( Options & 0x80000 )
		{
			SourceProcessHandle = TempHandle = INVALID_HANDLE_VALUE;
			TargetProcessHandle = FuckedHandle;
			Options &= 0xFFF7FFFF ;
		}
		else
		{
			SourceProcessHandle = FuckedHandle ;
			TargetProcessHandle = (HANDLE)0xFFFFFFFF;
			TempHandle = FuckedHandle ;
		}

		if ( (HANDLE)0xFFFFFFFF == TempHandle )
		{
			status = Is_ObjectTypeName_Allowed( SourceHandle );
			if ( status >= 0 )
			{
				status = ZwDuplicateObject( (HANDLE)0xFFFFFFFF, SourceHandle, TargetProcessHandle, pTargetHandle, Buffer->DesiredAccess, HandleAttributes, Options );
			}
		}
		else
		{
			if ( (HANDLE)0xFFFFFFFF != TargetProcessHandle )
			{
				status = STATUS_INVALID_HANDLE;
				goto _over_ ;
			}
			
			status = ZwDuplicateObject( TempHandle, SourceHandle, (HANDLE)0xFFFFFFFF, pTargetHandle, Buffer->DesiredAccess, HandleAttributes, Options & 0xFFFFFFFE );

			TargetHandle = *pTargetHandle;
			*pTargetHandle = 0;

			if ( ! NT_SUCCESS(status) ) { goto _over_ ; }
			
			status = Is_ObjectTypeName_Allowed( TargetHandle );
			if ( status >= 0 || (ZwClose(TargetHandle), status >= 0) )
			{
				if ( Options & DUPLICATE_CLOSE_SOURCE )
				{
					ZwClose( TargetHandle );
					status = ZwDuplicateObject( SourceProcessHandle,SourceHandle,(HANDLE)0xFFFFFFFF,pTargetHandle,Buffer->DesiredAccess,HandleAttributes,Options );

					TargetHandle = *pTargetHandle ;
				}

				if ( status >= 0 ) { *pTargetHandle = TargetHandle ; }
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

_over_ :
	ZwClose( FuckedHandle );
	return status ;
}



NTSTATUS 
Is_ObjectTypeName_Allowed (
	IN HANDLE Handle
	)
{
	PVOID Object = NULL ;
	ULONG FuckedObject = 0, ObjectHeader = 0, Type = 0, Size = 0 ;
	WORD Length = 0 ;
	BYTE NameInfoOffset = 0 ;
	LPWSTR szObjectName = NULL, szTemp = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	status = ObReferenceObjectByHandle( Handle, 0, 0, 0, &Object, 0 );
	if ( ! NT_SUCCESS(status) ) { return status; }

	if ( g_Version_Info.IS___win7 )
	{
		if ( NULL == g_ObQueryNameInfo_addr(Object) )
		{
			FuckedObject = (ULONG) g_ObGetObjectType_addr( Object );
			Length = *(WORD *)( FuckedObject + 8 );
			szObjectName = *(LPWSTR *)( FuckedObject + 0xC );
		}
	}
	else
	{
		NameInfoOffset = *(BYTE *)( (int)Object - 0xC );
		ObjectHeader = (int)Object - 0x18 ;

		if ( 0 == NameInfoOffset || ObjectHeader == NameInfoOffset )
		{
			Type = *(ULONG *)(ObjectHeader + 8);

			if ( TRUE == g_Version_Info.IS_before_vista )
			{
				Length = *(WORD *)(Type + 0x40);
				szObjectName = *(LPWSTR *)(Type + 0x44);
			}
			else
			{
				Length = *(WORD *)(Type + 8);
				szObjectName = *(LPWSTR *)(Type + 0xC);
			}
		}
	}

	status = STATUS_ACCESS_DENIED;
	if ( 0 == Length || NULL == szObjectName ) { goto _over_ ; }

	switch ( Length )
	{
	case 0xA :
		Size = 5 ;
		szTemp = L"Event" ;
		break;

	case 0xC :
		Size = 6 ;
		szTemp = L"Mutant" ;
		break;

	case 0xE :
		Size = 7 ;
		szTemp = L"Section" ;
		break;

	case 0x12 :
		Size = 9 ;
		szTemp = L"Semaphore" ;
		break;

	default:
		Size = 0;
		break;
	}
	
	if ( Size && 0 == _wcsnicmp(szObjectName, szTemp, Size) ) { status = STATUS_SUCCESS ; }

_over_ :
	ObfDereferenceObject( Object );
	return status;
}

///////////////////////////////   END OF FILE   ///////////////////////////////