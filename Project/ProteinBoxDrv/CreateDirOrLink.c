/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/21 [21:12:2010 - 10:55]
* MODULE : e:\Data\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\CreateDirOrLink.c
* 
* Description:
*
*   创建指定的符号链接
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "Security.h"
#include "CreateDirOrLink.h"

//////////////////////////////////////////////////////////////////////////


NTSTATUS
Ioctl_CreateDirOrLink (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName, LinkTarget ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE DirectoryHandle = NULL ;
	LPIOCTL_CREATEDIRORLINK_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;
	LPWSTR lpcFullPath = NULL, lpcFatherPath = NULL ;
	ULONG lpcFullPathLength = 0, lpcFatherPathLength = 0, LpcRootPath1Length = 0 ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_CreateDirOrLink(); \n" );
	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_CREATEDIRORLINK_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_CreateDirOrLink() - getIoctlBufferBody(); | Body地址不合法 \n" );
			return STATUS_UNSUCCESSFUL ;
		}

		// 全路径必须有; 检测数据包的合法性
		lpcFullPathLength = Buffer->lpcFullPathLength ;
		if ( lpcFullPathLength < 2 || lpcFullPathLength >= 0x800 || NULL == Buffer->lpcFullPath )
		{
			dprintf( "error! | Ioctl_CreateDirOrLink(); | 数据包不合法 lpcFullPath \n" );
			return STATUS_INVALID_PARAMETER;
		}

		ProbeForRead( Buffer->lpcFullPath, lpcFullPathLength, 2 );
	
		// 拷贝R3字符串到R0内存池
		lpcFullPath = (LPWSTR) kmalloc( lpcFullPathLength + 2 );
		if ( NULL == lpcFullPath ) 
		{
			dprintf( "error! | Ioctl_CreateDirOrLink(); | malloc Buffer failed,size=0x%x \n", lpcFullPathLength );
			status = STATUS_UNSUCCESSFUL ;
			__leave ;
		}

		RtlZeroMemory( lpcFullPath, lpcFullPathLength );
		memcpy( lpcFullPath, Buffer->lpcFullPath, lpcFullPathLength );

		// 父目录可以没有,需要看标记
		if ( Buffer->bFlag )
		{
			lpcFatherPathLength = Buffer->lpcFatherPathLength ;
			if ( lpcFatherPathLength < 2 || lpcFatherPathLength >= 0x800 || NULL == Buffer->lpcFatherPath )
			{
				dprintf( "error! | Ioctl_CreateDirOrLink(); | 数据包不合法 lpcFatherPath \n" );
				status = STATUS_UNSUCCESSFUL ;
				__leave ;
			}

			ProbeForRead( Buffer->lpcFatherPath, lpcFatherPathLength, 2 );

			// 拷贝R3字符串到R0内存池
			lpcFatherPath = (LPWSTR) kmalloc( lpcFatherPathLength + 2 );
			if ( NULL == lpcFatherPath ) 
			{
				dprintf( "error! | Ioctl_CreateDirOrLink(); | malloc Buffer failed,size=0x%x \n", lpcFatherPathLength );
				status = STATUS_UNSUCCESSFUL ;
				__leave ;
			}

			RtlZeroMemory( lpcFatherPath, lpcFatherPathLength );
			memcpy( lpcFatherPath, Buffer->lpcFatherPath, lpcFatherPathLength );
		}
		
		InitializeObjectAttributes (
			&ObjAtr,
			&uniObjectName,
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
			NULL,
			g_SecurityDescriptor_new 
			);
	
		RtlInitUnicodeString( &uniObjectName, lpcFullPath );

		LpcRootPath1Length = ProcessNode->pNode_C->LpcRootPath1Length / sizeof(WCHAR) - 1;
		if ( _wcsnicmp(lpcFullPath, ProcessNode->pNode_C->LpcRootPath1, LpcRootPath1Length) )
		{
			status = STATUS_ACCESS_DENIED ;
			__leave ;
		}

		if ( NULL == lpcFatherPath )
		{
			status = ZwCreateDirectoryObject( &DirectoryHandle, DIRECTORY_ALL_ACCESS, &ObjAtr );
		}
		else
		{
			if ( _wcsnicmp(lpcFatherPath, ProcessNode->pNode_C->LpcRootPath1, LpcRootPath1Length) )
			{
				status = STATUS_ACCESS_DENIED ;
			}
			else
			{
				RtlInitUnicodeString( &LinkTarget, lpcFatherPath );
				status = ZwCreateSymbolicLinkObject( &DirectoryHandle, 0xF0001, &ObjAtr, &LinkTarget );
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	if ( STATUS_OBJECT_NAME_COLLISION == status) { status = STATUS_SUCCESS ; }

	kfree( lpcFullPath );
	kfree( lpcFatherPath );
	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////