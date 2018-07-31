/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/08 [8:9:2010 - 22:13]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\GetInjectSaveArea.c
* 
* Description:
*      
*   将ImageNotify中修改的PE数据传给R3                      
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "GetFileName.h"

//////////////////////////////////////////////////////////////////////////


NTSTATUS
Ioctl_GetFileName (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	LPWSTR ptr = NULL, lpData = NULL ;
	BOOL bHasNoName = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	PFILE_OBJECT FileObject = NULL ;
	LPIOCTL_GETFILENAME_BUFFER Buffer = NULL ;
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;
	ULONG Size = 0, Addr = 0, FileNameLength = 0, TotalLength = 0 ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	// 1. 校验参数合法性
	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_GetFileName(); \n" );

	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_GETFILENAME_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_GetFileName() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		status = ObReferenceObjectByHandle( Buffer->HandleValue, 0, (POBJECT_TYPE)IoFileObjectType, KernelMode, (PVOID *)&FileObject, NULL );
		if ( ! NT_SUCCESS(status) ) { return status; }

		status = STATUS_OBJECT_NAME_NOT_FOUND ;

		FileNameLength = FileObject->FileName.Length ;
		if ( FileObject->DeviceObject && FileNameLength && FileObject->FileName.Buffer )
		{
			ObjectNameInfo = SepQueryNameString( FileObject->DeviceObject, &bHasNoName ); // 由调用者释放内存
			if ( ObjectNameInfo && FALSE == bHasNoName )
			{
				TotalLength = FileObject->FileName.Length + ObjectNameInfo->Name.Length + 4 ;
				if ( TotalLength > Buffer->BufferLength )
				{
					status = STATUS_BUFFER_TOO_SMALL ;
				}
				else
				{
					lpData = (LPWSTR) Buffer->lpData ;
					ProbeForWrite( Buffer->lpData, TotalLength, sizeof(WORD) );
					memcpy( lpData, ObjectNameInfo->Name.Buffer, ObjectNameInfo->Name.Length );

					ptr = &lpData[ (ULONG)ObjectNameInfo->Name.Length / sizeof(WCHAR) ];
					memcpy( ptr, FileObject->FileName.Buffer, FileObject->FileName.Length );
					
					ptr[ (ULONG)FileObject->FileName.Length / sizeof(WCHAR) ] = 0 ;
					status = STATUS_SUCCESS;
				}

			}

			kfree( (PVOID)ObjectNameInfo );
		}

		ObfDereferenceObject( FileObject );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_GetFileName() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




///////////////////////////////   END OF FILE   ///////////////////////////////