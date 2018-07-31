/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 17:42]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\StartProcess.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "StartProcess.h"
#include "DispatchIoctl.h"
#include "ProcessData.h"
#include "SdtData.h"
#include "SandboxsList.h"

//////////////////////////////////////////////////////////////////////////

extern BOOL g_bMonitor_Notify ;
extern HANDLE g_NewTokenHandle_system ;

_ZwSetInformationToken_ g_ZwSetInformationToken_addr = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



NTSTATUS
Ioctl_StartProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  Shit,进程启动的源泉啊. 
  从这个函数可以知道要"被沙箱"的程序是在哪个沙箱中启动,故在此记录沙箱的个数和名称
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	BOOL bRet = FALSE, bOverload = FALSE ;
	ULONG PID = 0 ;
	ULONG SessionId = 0 ;
	LPPDNODE pNodeNew = NULL ;
	PNODE_INFO_C pNode_c = NULL ;
	LPIOCTL_STARTPROCESS_BUFFER Buffer = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	SECURITY_QUALITY_OF_SERVICE SecurityQos ;
	HANDLE new_hToken, new_ImpersonationToken ;
	KPROCESSOR_MODE PreviousMode = ExGetPreviousMode();
	
	//
	// 1. 校验参数合法性
	//

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_StartProcess(); \n" );
	if ( FALSE == g_bMonitor_Notify ) { return STATUS_SERVER_DISABLED ; }
	
	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Ioctl_StartProcess(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	__try
	{
		Buffer = (LPIOCTL_STARTPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_StartProcess() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		ProbeForWrite( Buffer->new_hToken, 4, 4 );
		ProbeForWrite( Buffer->new_ImpersonationToken, 4, 4 );		
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_StartProcess() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return status ;
	}

	//
	// 2. 检查是否是已创建的进程节点
	//

	PID = (ULONG) PsGetCurrentProcessId();

	if ( kgetnodePD( PID ) )
	{
		dprintf( "error! | Ioctl_StartProcess() - kbuildnodePD(); | 当期进程节点已存在于链表中 PID=%d \n", PID );
		return STATUS_ALREADY_COMMITTED ;
	}

	//
	// 3.1 创建进程节点
	//

	pNodeNew = (LPPDNODE) kbuildnodePD( PID, FALSE );
	if ( NULL == pNodeNew )
	{
		dprintf( "error! | Ioctl_StartProcess() - kbuildnodePD(); | 创建进程节点失败 PID=%d \n", PID );
		return status ;
	}

	//
	// 3.2 填充C节点
	//

	__try
	{
		if ( UserMode == PreviousMode ) 
		{ 
			if ( Buffer->wszBoxName ) { ProbeForRead( Buffer->wszBoxName, 0x40, 1 ); }
			if ( Buffer->wszRegisterUserID ) { ProbeForRead( Buffer->wszRegisterUserID, 0xC0, 1 ); }
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		PDClearNode( (PVOID)pNodeNew ); // 还未插入链表,初始化失败则释放内存
		dprintf( "error! | Ioctl_StartProcess() - ProbeForRead(); | R3传进来的Buffer不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	pNode_c = PDBuildNodeC( Buffer );
	if ( NULL == pNode_c )
	{
		PDClearNode( (PVOID)pNodeNew ); // 还未插入链表,初始化失败则释放内存
		dprintf( "error! | Ioctl_StartProcess() - PDBuildNodeC(); | 创建进程节点C失败 PID=%d \n", PID );
		return status ;
	}

	//
	// 4. 修正当前进程节点
	//

	pNodeNew->bSelfExe = 1 ;
	pNodeNew->pNode_C = pNode_c ;

	bRet = PDFixupNode( pNodeNew, L"PBStart.exe" );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_StartProcess() - PDFixupNode(); | 修正进程节点失败 PID=%d \n", PID );
		PDClearNode( (PVOID)pNodeNew ); // 还未插入链表,初始化失败则释放内存
		return status ;
	}

	kInsertTailPD( pNodeNew );

	//
	// 5. 修正当前进程的权限; R3的PBStart.exe调用Start_sun_process(),其内部会根据命令行参数决定是否以系统权限启动"被SB"的进程
	//    CreateProcessAsUserW赋予指定的权限创建 或者 ShellExecuteExW 以普通权限创建
	//

	// 5.0 若要降权,则不会将带有系统权限的句柄填充到Buffer中
	if ( pNodeNew->bDropAdminRights )
	{
		*(Buffer->new_hToken) = 0 ;
		*(Buffer->new_ImpersonationToken) = 0 ;
	}

	// 5.1 复制g_NewTokenHandle_system的句柄
	SecurityQos.Length					= sizeof( SecurityQos );
	SecurityQos.ImpersonationLevel		= SecurityImpersonation ;
	SecurityQos.ContextTrackingMode		= FALSE ;
	SecurityQos.EffectiveOnly			= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &SecurityQos ;

	InitializeObjectAttributes (
		&ObjectAttributes,
		NULL,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	status = ZwDuplicateToken( g_NewTokenHandle_system, 0, &ObjectAttributes, 0, TokenPrimary, &new_hToken );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwDuplicateToken(阶段1) | status == 0x%08lx \n", status );
		return status ;
	}

	// 5.2 获取 ZwCreateToken 的原始地址,调用之
	bRet = kgetaddrSDT( ZwSetInformationToken );
	if ( FALSE == bRet )
	{ 
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | 无法获取ZwSetInformationToken地址 \n" );
		return STATUS_UNSUCCESSFUL ; 
	}

	SessionId = pNodeNew->pNode_C->SessionId ;
	status = g_ZwSetInformationToken_addr( new_hToken, TokenSessionId, &SessionId, sizeof(DWORD) );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwSetInformationToken() | status == 0x%08lx \n", status );
		ZwClose( new_hToken );
		return status ;
	}

	// 5.3 复制new_hToken的句柄
	status = ZwDuplicateToken (
		new_hToken,
		0,
		&ObjectAttributes,
		0,
		TokenImpersonation,
		&new_ImpersonationToken
		);

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwDuplicateToken(阶段2) | status == 0x%08lx \n", status );
		ZwClose( new_hToken );
		return status ;
	}

	// 5.4 复制成功,将得到的新句柄抛给R3
	*(Buffer->new_hToken) = new_hToken ;
	*(Buffer->new_ImpersonationToken) = new_ImpersonationToken ;

	pNodeNew->bProcessNodeInitOK = TRUE ;

	// 6. 记录当前沙箱名到链表
	kbuildnodeSBL( pNode_c->BoxName, pNode_c->BoxNameLength, &bOverload );
	if ( bOverload ) { return STATUS_UNSUCCESSFUL ; }

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_QueryProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  查询指定PID对应的 沙箱名/进程短名/ SID / 用户名 等信息 
    
--*/
{
	ULONG PID = 0, length = 0 ;
	LPIOCTL_QUERYPROCESS_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryProcess(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryProcess() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		PID = Buffer->PID ;
		if ( ProcessNode )
		{
			if ( PID == ProcessNode->PID || 0xFFFFFFFF == PID )
			{
				PID = 0 ; // 表明待查询的进程对应的即是当前的ProcessNode节点
			}
		}
		else
		{
			if ( 0 == PID || 0xFFFFFFFF == PID )
				return STATUS_INVALID_CID;
		}

		if ( PID )
		{
			ProcessNode = (LPPDNODE) kgetnodePD( PID );
			if ( NULL == ProcessNode || ProcessNode->bDiscard ) { return STATUS_INVALID_CID ; }
		}

		// 拷贝沙箱名
		length = ProcessNode->pNode_C->BoxNameLength ;
		if ( Buffer->BoxNameMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpBoxName, length, sizeof(DWORD) );
		memcpy( Buffer->lpBoxName, ProcessNode->pNode_C->BoxName, length );

		// 拷贝进程短名
		length = ProcessNode->ImageNameLength ;
		if ( Buffer->ProcessShortNameMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpCurrentProcessShortName, length, sizeof(DWORD) );
		memcpy( Buffer->lpCurrentProcessShortName, ProcessNode->lpProcessShortName, length );

		// 拷贝SID
		length = ProcessNode->pNode_C->RegisterUserIDLength ;
		if ( Buffer->RegisterUserIDMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpRegisterUserID, length, sizeof(DWORD) );
		memcpy( Buffer->lpRegisterUserID, ProcessNode->pNode_C->RegisterUserID, length );

		// 拷贝SessionId
		ProbeForWrite( Buffer->SessionId, 4, sizeof(DWORD) );
		*Buffer->SessionId = ProcessNode->pNode_C->SessionId ; 

		// 拷贝FilePath
		length = ProcessNode->pNode_C->FileRootPathLength ;
		if ( Buffer->FileRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->FileRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->FileRootPath, ProcessNode->pNode_C->FileRootPath, length );

		// 拷贝RegPath
		length = ProcessNode->pNode_C->KeyRootPathLength ;
		if ( Buffer->KeyRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->KeyRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->KeyRootPath, ProcessNode->pNode_C->KeyRootPath, length );

		// 拷贝LpcPath
		length = ProcessNode->pNode_C->LpcRootPath1Length ;
		if ( Buffer->LpcRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->LpcRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->LpcRootPath, ProcessNode->pNode_C->LpcRootPath1, length );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryProcess() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




NTSTATUS
Ioctl_QueryProcessPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  查询指定PID对应的 沙箱文件夹根目录

--*/
{
	ULONG PID = 0, length = 0 ;
	LPIOCTL_QUERYPROCESSPATH_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryProcessPath(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYPROCESSPATH_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryProcess() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		PID = Buffer->PID ;
		if ( ProcessNode )
		{
			if ( PID == ProcessNode->PID || 0xFFFFFFFF == PID )
			{
				PID = 0 ; // 表明待查询的进程对应的即是当前的ProcessNode节点
			}
		}
		else
		{
			if ( 0 == PID || 0xFFFFFFFF == PID )
				return STATUS_INVALID_CID;
		}

		if ( PID )
		{
			ProcessNode = (LPPDNODE) kgetnodePD( PID );
			if ( NULL == ProcessNode || ProcessNode->bDiscard ) { return STATUS_INVALID_CID ; }
		}

		// 拷贝FilePath
		length = ProcessNode->pNode_C->FileRootPathLength ;
		if ( Buffer->FileRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->FileRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->FileRootPath, ProcessNode->pNode_C->FileRootPath, length );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryProcessPath() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_QueryBoxPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  查询指定的沙箱名全路径
    
--*/
{
	BOOL bRet = FALSE ;
	WCHAR FileRootPath[ MAX_PATH ] = L"" ;
	LPIOCTL_QUERYBOXPATH_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryBoxPath(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYBOXPATH_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		// 拷贝FilePath
		bRet = kGetConfSingle( L"GlobalSetting", L"BoxRootFolder", FileRootPath );
		if ( FALSE == bRet || wcslen(FileRootPath) > MAX_PATH - 0x10 - Buffer->BoxNamLength )
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - kGetConfSingle(); |  \n" );
			__leave ;
		}

		wcscat( FileRootPath, L"\\SUDAMI\\" );
		wcscat( FileRootPath, Buffer->lpBoxName );

		if ( Buffer->BoxPathMaxLength < (wcslen(FileRootPath)+1) * sizeof(WCHAR) )  { return STATUS_BUFFER_TOO_SMALL ; }

		if ( IsWrittenKernelAddr(Buffer->lpBoxPath, (ULONG_PTR)Buffer->BoxPathMaxLength) ) 
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - IsWrittenKernelAddr(); | 参数不合法,待写入的地址:0x%08lx, 长度:0x%08lx \n", Buffer->lpBoxPath, Buffer->BoxPathMaxLength );
			return STATUS_INVALID_PARAMETER_4 ;
		}

		ProbeForWrite( Buffer->lpBoxPath, Buffer->BoxPathMaxLength, 2 );
		wcscpy( Buffer->lpBoxPath, FileRootPath );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryBoxPath() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_EnumProcessEx (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  枚举指定沙箱中包含的所有进程
    
--*/
{
	BOOL bRet = FALSE ;
	WCHAR szBoxName[ MAX_PATH ] = L"" ;
	LPIOCTL_ENUMPROCESSEX_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_EnumProcessEx(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_ENUMPROCESSEX_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_EnumProcessEx() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		if ( NULL == Buffer->pArray ) { return STATUS_INVALID_PARAMETER; }

		if ( ProcessNode )
		{
			wcscpy( szBoxName, ProcessNode->pNode_C->BoxName );
		}

		if ( !*szBoxName && Buffer->lpBoxName )
		{
			ProbeForRead( Buffer->lpBoxName, 0x40, 1 );
			if ( *(Buffer->lpBoxName) )
			{
				wcsncpy( szBoxName, Buffer->lpBoxName, 0x20 );
			}
		}

		ProbeForWrite( Buffer->pArray, 0x800, sizeof(ULONG) );
		return PDEnumProcessEx( szBoxName, TRUE, Buffer->pArray );
		
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_EnumProcessEx() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////