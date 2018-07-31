/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/08 [8:9:2010 - 22:13]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\WhiteOrBlack.c
* 
* Description:
*      
*   判定当前字符串的黑白                      
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "GrayList.h"
#include "WhiteOrBlack.h"

//////////////////////////////////////////////////////////////////////////



NTSTATUS
Ioctl_WhiteOrBlack (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	LPIOCTL_WHITEORBLACK_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;
	LPWSTR lpCompareName = NULL ;
	PERESOURCE	QueueLockList = NULL ;
	PLIST_ENTRY ListHeadWhite = NULL, ListHeadBlack = NULL ;
	BOOL bIsBlack = FALSE, bIsWhite = FALSE ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_WhiteOrBlack(); \n" );

	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_WHITEORBLACK_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_WhiteOrBlack() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		ProbeForWrite( Buffer->bIsWhite, 4, sizeof(DWORD) );
		ProbeForWrite( Buffer->bIsBlack, 4, sizeof(DWORD) );

		// 检测数据包的合法性
		if ( NULL == Buffer->szPath || Buffer->PathLength < 2 || Buffer->PathLength >= 0x800 || FALSE == IsWBFlagOK( Buffer->Flag ) )
		{
			dprintf( "error! | Ioctl_WhiteOrBlack() - IsWBFlagOK(); | 数据包不合法 \n" );
			__leave ;
		}

		ProbeForRead( Buffer->szPath, Buffer->PathLength, 2 );

		// 拷贝R3字符串到R0内存池,用于比较
		lpCompareName = (LPWSTR) kmalloc( Buffer->PathLength + 10 );
		if ( NULL == lpCompareName ) 
		{
			dprintf( "error! | Ioctl_WhiteOrBlack(); | malloc Buffer failed,size=0x%x \n", Buffer->PathLength );
			__leave ;
		}

		RtlZeroMemory( lpCompareName, Buffer->PathLength );
		memcpy( lpCompareName, Buffer->szPath, Buffer->PathLength );
		lpCompareName[ Buffer->PathLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		// 决定用哪条链表
		switch ( Buffer->Flag )
		{
		case WhiteOrBlack_Flag_XFilePath :
			{
				QueueLockList	= ProcessNode->XFilePath.pResource ;
				ListHeadWhite	= &ProcessNode->XFilePath.OpenFilePathListHead ;
				ListHeadBlack	= &ProcessNode->XFilePath.ClosedFilePathListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XRegKey :
			{
				QueueLockList	= ProcessNode->XRegKey.pResource ;
				ListHeadWhite	= &ProcessNode->XRegKey.DirectListHead ;
				ListHeadBlack	= &ProcessNode->XRegKey.DennyListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XIpcPath :
			{
				QueueLockList	= ProcessNode->XIpcPath.pResource ;
				ListHeadWhite	= &ProcessNode->XIpcPath.OpenIpcPathListHead ;
				ListHeadBlack	= &ProcessNode->XIpcPath.ClosedIpcPathListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XClassName :
			{
				QueueLockList	= ProcessNode->XWnd.pResource ;
				ListHeadWhite	= &ProcessNode->XWnd.WndListHead ;
				ListHeadBlack	= NULL;
			}
			break ;
		
		default :
			{
				dprintf( "err! | Ioctl_WhiteOrBlack(); | 对当前Flag(0x%x)不予处理 \n", Buffer->Flag );
				__leave ;
			}
			break ;
		}

		// 过黑名单
		if ( ListHeadBlack )
			bIsBlack = GLFindNodeEx( ListHeadBlack, QueueLockList, lpCompareName, NULL );

		if ( FALSE == bIsBlack && ListHeadWhite )
		{
			// 若不是黑的,继续过白名单
			bIsWhite = GLFindNodeEx( ListHeadWhite, QueueLockList, lpCompareName, NULL );
		}

		// 填充R3数据包
		*Buffer->bIsBlack = bIsBlack ;
		*Buffer->bIsWhite = bIsWhite ;

		// 释放内存
		kfree( lpCompareName );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	//	dprintf( "error! | Ioctl_WhiteOrBlack() - __try __except(); | ProbeForXX 对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




///////////////////////////////   END OF FILE   ///////////////////////////////