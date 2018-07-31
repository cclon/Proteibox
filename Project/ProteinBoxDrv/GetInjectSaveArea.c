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
#include "GetInjectSaveArea.h"

//////////////////////////////////////////////////////////////////////////



NTSTATUS
Ioctl_GetInjectSaveArea (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	ULONG Size = 0, Addr = 0 ;
	LPIOCTL_GETINJECTSAVEAREA_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	// 1. 校验参数合法性
	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_GetInjectSaveArea(); \n" );

	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	Addr = (ULONG) ProcessNode->XImageNotifyDLL.BeCoveredAddr ;
	Size = (ULONG) ProcessNode->XImageNotifyDLL.BeCoveredSize ;

	if ( 0 == Addr || 0 == Size ) { return STATUS_ALREADY_COMMITTED ; }

	__try
	{
		Buffer = (LPIOCTL_GETINJECTSAVEAREA_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_GetInjectSaveArea() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		ProbeForWrite( Buffer->BeCoveredAddr, 4, sizeof(DWORD) );
		*Buffer->BeCoveredAddr = Addr ; // 保存待修复的PE地址

		ProbeForWrite( Buffer->MaxLength, 4, sizeof(DWORD) );
		if ( *Buffer->MaxLength < Size )
		{
			dprintf( "error! | Ioctl_GetInjectSaveArea(); | R3提供的缓冲区大小不够 \n" );
			return STATUS_BUFFER_TOO_SMALL ;
		}

		*Buffer->MaxLength = Size ; // 保存实际大小

		ProbeForWrite( Buffer->lpData, Size, sizeof(CHAR) );
		memcpy( Buffer->lpData, ProcessNode->XImageNotifyDLL.BeCoveredOldData, Size );

		kfree( ProcessNode->XImageNotifyDLL.BeCoveredOldData ); // 释放ImageNofity中申请的内存

		// 清空数据
		ProcessNode->XImageNotifyDLL.BeCoveredAddr		= NULL	;
		ProcessNode->XImageNotifyDLL.BeCoveredOldData	= NULL	;
		ProcessNode->XImageNotifyDLL.BeCoveredSize		= 0		;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_GetInjectSaveArea() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




///////////////////////////////   END OF FILE   ///////////////////////////////