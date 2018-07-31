#include "stdafx.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBFilesData.h"
#include "HookHelper.h"
#include "PBAlpcData.h"
#include "PBToken.h"
#include "PBServicesData.h"
#include "ExportFuncData.h"
#include "PBDynamicData.h"
#include "Config.h"
#include "MemoryManager.h"
#include "PBAdvapi32dll/PBAdvapi32dllData.h"
#include <Windows.h>

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////

BOOL g_IsWow64 = FALSE ;
BOOL g_Function_PB_IsWow64_is_Called = FALSE ;
BOOL g_bIsOpenCOM = FALSE ;
BOOL g_b_Function_SbieDll_IsOpenCOM_is_Called = FALSE ;

LONG g_RefCount_rpc = 0 ;

static int* g_CLSIDs_WhiteArray = NULL;
static ULONG g_CLSIDs_WhiteArray_nCounts = 0xFFFFFFFF;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



// This is an example of an exported variable
PROTEINBOXDLL_API int nProteinBoxDLL=0;

// This is an example of an exported function.
PROTEINBOXDLL_API int fnProteinBoxDLL(void)
{
	return 42;
}



PROTEINBOXDLL_API void Test(void)
{
	return ;
}



PROTEINBOXDLL_API BOOL PB_IsWow64(void)
{
	BOOL bIsWow64 = FALSE;
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	if ( g_Function_PB_IsWow64_is_Called ) { return g_IsWow64 ; }
	
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress( __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG], "IsWow64Process" );
	if ( fnIsWow64Process )
	{
		if ( fnIsWow64Process(GetCurrentProcess(),&bIsWow64) )
		{
			g_IsWow64 = bIsWow64 ;
		}
	}
	
	g_Function_PB_IsWow64_is_Called = TRUE ;
	return g_IsWow64;
}



PROTEINBOXDLL_API BOOL 
PB_QueryProcess (
	IN ULONG PID,
	OUT PVOID Data
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/13 [13:9:2010 - 14:47]

Routine Description:
    
    
Arguments:
  PID - 待查询的进程ID 
  Data - struct _PB_BOX_INFO_ 结构体指针,保存查询到的信息
    
--*/
{
	IOCTL_PROTEINBOX_BUFFER buffer ;
	LPPB_BOX_INFO pInfo = (LPPB_BOX_INFO) Data ;

	if ( NULL == Data ) { return FALSE ; }
	
	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );
		
	buffer.Head.Flag = FLAG_Ioctl_QueryProcess ;
	buffer.QueryProcessBuffer.PID						= PID;
	buffer.QueryProcessBuffer.lpBoxName					= pInfo->BoxName ;
	buffer.QueryProcessBuffer.lpCurrentProcessShortName = pInfo->ProcessName ;
	buffer.QueryProcessBuffer.lpRegisterUserID			= pInfo->SID ;
	buffer.QueryProcessBuffer.SessionId					= &pInfo->SessionId ;
	buffer.QueryProcessBuffer.FileRootPath				= pInfo->FileRootPath ;
	buffer.QueryProcessBuffer.KeyRootPath				= pInfo->KeyRootPath ;
	buffer.QueryProcessBuffer.LpcRootPath				= pInfo->LpcRootPath ;
	buffer.QueryProcessBuffer.BoxNameMaxLength			= MAX_PATH ;
	buffer.QueryProcessBuffer.ProcessShortNameMaxLength = MAX_PATH ;
	buffer.QueryProcessBuffer.RegisterUserIDMaxLength	= MAX_PATH ;
	buffer.QueryProcessBuffer.FileRootPathLength		= MAX_PATH ;
	buffer.QueryProcessBuffer.KeyRootPathLength			= MAX_PATH ;
	buffer.QueryProcessBuffer.LpcRootPathLength			= MAX_PATH ;

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	if ( bRet )
	{
		pInfo->BoxNameLength = ( wcslen(pInfo->BoxName) + 1 ) * sizeof(WCHAR) ; 
		pInfo->ProcessNameLength = ( wcslen(pInfo->ProcessName) + 1 ) * sizeof(WCHAR) ; 
		pInfo->SIDLength = ( wcslen(pInfo->SID) + 1 ) * sizeof(WCHAR) ; 
		pInfo->FileRootPathLength = ( wcslen(pInfo->FileRootPath) + 1 ) * sizeof(WCHAR) ; 
		pInfo->KeyRootPathLength = ( wcslen(pInfo->KeyRootPath) + 1 ) * sizeof(WCHAR) ; 
		pInfo->LpcRootPathLength = ( wcslen(pInfo->LpcRootPath) + 1 ) * sizeof(WCHAR) ; 
	}

	return bRet ;
}



PROTEINBOXDLL_API BOOL 
PB_QueryProcessPath (
	IN ULONG PID,
	OUT LPWSTR szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/13 [13:9:2010 - 14:47]

Routine Description:
    
Arguments:
  PID - 待查询的进程ID 
  szPath - 保存查询到的沙箱根目录,长度限定为MAX_PATH
    
--*/
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	if ( NULL == szPath ) { return FALSE ; }
	
	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );
		
	buffer.Head.Flag = FLAG_Ioctl_QueryProcessPath ;
	buffer.QueryProcessPathBuffer.PID					= PID ;
	buffer.QueryProcessPathBuffer.FileRootPath			= szPath ;	
	buffer.QueryProcessPathBuffer.FileRootPathLength	= MAX_PATH ;

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}



PROTEINBOXDLL_API NTSTATUS PB_GetHandlePath( IN HANDLE FileHandle, IN LPWSTR outSzPath, OUT BOOL *bIsHandlerSelfFilePath )
{
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath, RedirectedPath, szPath ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( NULL == g_SymbolicLinkObjectName_Array )
		HandlerPanfu( 0xFFFFFFFF );

	// 2. 通过句柄得到文件的 原始路径 & 重定位路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	RtlInitUnicodeString( &uniObjectName, NULL );
	status = GetFilePath ( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, bIsHandlerSelfFilePath );
	if ( ! NT_SUCCESS(status) )
	{
		if ( STATUS_BAD_INITIAL_PC != status ) { goto _over_ ; }

		if ( bIsHandlerSelfFilePath ) { goto _over_ ; }
		
		status = STATUS_SUCCESS ;
	}

	if ( g_BoxInfo.BoxName && bIsHandlerSelfFilePath && *bIsHandlerSelfFilePath )
	{
		szPath = RedirectedPath ;
	}
	else
	{
		szPath = OrignalPath ;
	}

	wcscpy( outSzPath, szPath );

_over_:
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status ;
}



PROTEINBOXDLL_API BOOL PB_TranslateNtToDosPath( IN LPWSTR szName )
{
	int VolumeNumber = 0, Length = 0 ;
	BOOL bResult = FALSE ;
	int v4 ;
	LPWSTR pName, ptr1 ;

	if ( 0 == wcsnicmp( szName, L"\\device\\mup\\", 0xC ) )
	{
		Length = wcslen( szName + 11 );
		memmove( szName + 1, szName + 11, 2 * Length + 2 );
		return TRUE ;
	}

	while ( TRUE )
	{
		if ( g_SymbolicLinkObjectName_Array || HandlerPanfu(0xFFFFFFFF) )
		{
			if ( g_SymbolicLinkObjectName_Array )
			{
				v4 = *(DWORD *)((int)g_SymbolicLinkObjectName_Array + 4 * VolumeNumber);
				if ( v4 && v4 != -8 )
				{
					pName = (LPWSTR)(v4 + 8);

					if ( 0 == wcsnicmp(pName, szName, wcslen(pName)) )
					{
						ptr1 = &szName[ wcslen(pName) ] ;
						if ( '\\' == *ptr1 || NULL == *ptr1 )
						{
							Length = wcslen(ptr1);
							memmove(szName + 2, ptr1, 2 * Length + 2);
							*szName = VolumeNumber + 'A';
							szName[1] = ':';
							return TRUE ;
						}
					}
				}
			}
		}

		++ VolumeNumber ;
		if ( VolumeNumber >= 0x1A ) { break; }
	}

	return bResult ;
}



PROTEINBOXDLL_API PVOID PB_CallServer( IN PVOID pBuffer )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/04 [4:8:2011 - 17:06]

Routine Description:
  PBStart.exe 跟PBSrv.exe通信的主要函数,你妹啊(+﹏+)~
    
Arguments:
  pBuffer - 各种纠结的结构体指针

Return Value:

    
--*/
{
	BOOL bFlag = FALSE ;
	PVOID MsgBody = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	PB_PORT_MESSAGE RequestMessage, ReplyMessage ;
	LPPB_PORT_MESSAGE pRequestMessage = NULL, pReplyMessage = NULL ;
	PRPC_IN_HEADER pRpcInBuffer = (PRPC_IN_HEADER) pBuffer ;
	LPRPC_OUT_HEADER pOutBuffer = (LPRPC_OUT_HEADER) kmalloc( sizeof(RPC_OUT_HEADER) );
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( NULL == pBuffer ) { return NULL; }
	bFlag = pRpcInBuffer->Flag == _PBSRV_APINUM_GetAppVersion_ || pRpcInBuffer->Flag == _PBSRV_APINUM_INIGetCurUserSection_ ;

	// 1. 连接沙箱的LPC端口,获得句柄
	if ( NULL == pNode->sLPC.PortHandle && FALSE == ConnectRpcPort() ) { return NULL; }

	LONG refCount = InterlockedIncrement( &g_RefCount_rpc );
	PVOID pBufferCur = pRpcInBuffer;
	ULONG DataLength = (ULONG)pRpcInBuffer->DataLength ;
	ULONG nSize_little = 0, LpcHeaderLength = pNode->sLPC.LpcHeaderLength ;

	RtlZeroMemory( &RequestMessage, sizeof(PB_PORT_MESSAGE) );

	// 2. 将@Buffer的内容全部通过LPC通信传输到沙箱的LPC服务端
	while ( DataLength )
	{
		pRequestMessage = &RequestMessage ;
		RtlZeroMemory( pRequestMessage, LpcHeaderLength );

		// 2.2 因为每次LPC通信的内存块大小有限,若@Buffer的内容过大,就得分多次通信传输; 这里是将@Buffer的内容(可能是部分内容)填充到MsgBody中
		MsgBody = (char *)pRequestMessage + LpcHeaderLength ;
		nSize_little = DataLength <= pNode->sLPC.MaxMessageLength ? DataLength : pNode->sLPC.MaxMessageLength ;
		
		pRequestMessage->Header.u1.s1.DataLength = (short)nSize_little ;
		pRequestMessage->Header.u1.s1.TotalLength = (short)(nSize_little + LpcHeaderLength) ;

		memcpy( MsgBody, pBufferCur, nSize_little );

		// 2.3 这句代码在大循环中只会执行一次
		if ( pBufferCur == pBuffer ) { *((BYTE *)MsgBody + 3) = (BYTE)refCount; }

		// 2.4 正如2.2所说.这里定位到下次LPC通信需要传输的剩余数据的地址 & 剩余数据大小
		pBufferCur = (PVOID)((char *)pBufferCur + nSize_little) ;
		DataLength -= nSize_little ;

		// 2.5 发送LPC消息到沙箱LPC服务端
		status = ZwRequestWaitReplyPort( pNode->sLPC.PortHandle, (PPORT_MESSAGE)&RequestMessage, (PPORT_MESSAGE)&ReplyMessage );
		if ( ! NT_SUCCESS(status) ) { break; }

		// 2.6 若Buffer是分多次发送的,那么在发送完之前,是不应该收到返回消息的.若收到则表明是过早的回复,通信失败
		pRequestMessage = &ReplyMessage;
		if ( DataLength && pRequestMessage->Header.u1.s1.DataLength ) { return NULL; }
	}

	// 3. 处理LPC通信失败的情况
	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( pNode->sLPC.PortHandle );
		pNode->sLPC.PortHandle = NULL ;
		return NULL;
	}

	// 4. LPC通信成功,应该收到返回的消息.处理该消息
	pRequestMessage = &ReplyMessage;
	MsgBody = (char *)pRequestMessage + LpcHeaderLength ;
	if ( ReplyMessage.Header.u1.s1.DataLength < 8 )
	{
		DataLength = 0 ;
	}
	else
	{
		// mismatched reply
		if ( *((BYTE *)MsgBody + 3) != refCount ) { return NULL; }
		
		// 得到返回消息数据的总大小
		*((BYTE *)MsgBody + 3) = 0 ;
		DataLength = *(DWORD *) MsgBody ; 
	}

	// 5. 接收服务端返回的消息,也可能是多次接收,因为一次传送的数据量有限
	if ( 0 == DataLength )
	{
		// null reply
		return NULL;
	}

	// 5.1 分配相同大小的内存,存放返回的数据
	PVOID pBufferNew = kmalloc( DataLength + 8 );
	pBufferCur = pBufferNew ;

	do
	{
		if ( pRequestMessage->Header.u1.s1.DataLength + (ULONG)pBufferCur - (ULONG)pBufferNew > DataLength ) { break; }

		// 5.2 按照每次返回的数据大小,拷贝内容
		MsgBody = (char *)pRequestMessage + LpcHeaderLength ;
		memcpy( pBufferCur, MsgBody, pRequestMessage->Header.u1.s1.DataLength );

		// 5.3 定位到下次要存放数据的地址,清空当前数据包,准备接收下次数据
		pBufferCur = (char *)pBufferCur + pRequestMessage->Header.u1.s1.DataLength ;
		if ( (ULONG)pBufferCur - (ULONG)pBufferNew >= DataLength )
		{
			*(DWORD *)pBufferCur = 0;
			*(DWORD *)((ULONG)pBufferCur + 4) = 0;

			return pBufferNew;
		}

		pRequestMessage = &RequestMessage;
		RtlZeroMemory( pRequestMessage, LpcHeaderLength );

		// 5.4 再次接收数据
		pRequestMessage->Header.u1.s1.TotalLength = (CSHORT)LpcHeaderLength ;
		status = ZwRequestWaitReplyPort( pNode->sLPC.PortHandle, (PPORT_MESSAGE)&RequestMessage, (PPORT_MESSAGE)&ReplyMessage );

		pRequestMessage = &ReplyMessage;
		
	}
	while ( status >= 0 );

	// 6. 收尾工作
	kfree( pBufferNew );
	ZwClose( pNode->sLPC.PortHandle );
	pNode->sLPC.PortHandle = NULL ;

	return NULL ;
}



PROTEINBOXDLL_API NTSTATUS 
PB_RenameFile (
	IN HANDLE hFile,
	IN LPWSTR szPathFather,
	IN LPWSTR szPathSun,
	IN BOOLEAN Replace
	)
{
	WCHAR OldData ;
	HANDLE hDirectory = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR szFullPath = NULL, ptr = NULL ;
	ULONG TotalLength, PathFatherLength, PathSunLength, FileInfoLength ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	PFILE_RENAME_INFORMATION FileInfo = NULL ;

	if ( NULL == hFile || NULL == szPathFather || NULL == szPathSun ) { return STATUS_INVALID_PARAMETER; }

	// 1. 拼接成完整的路径
	PathFatherLength = wcslen(szPathFather) * sizeof(WCHAR) ;
	PathSunLength = wcslen(szPathSun) * sizeof(WCHAR) ;
	TotalLength = PathFatherLength + PathSunLength + 0x10 ;

	szFullPath = (LPWSTR) kmalloc( TotalLength );
	if ( NULL == szFullPath ) { return STATUS_INSUFFICIENT_RESOURCES; }

	memcpy( szFullPath, szPathFather, PathFatherLength );
	ptr = &szFullPath[ wcslen(szFullPath) ];
	*ptr = '\\' ;
	memcpy( ptr + 1, szPathSun, PathSunLength );
	szFullPath[ wcslen(szFullPath) ] = UNICODE_NULL ;

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XFilePath, szFullPath, &bIsWhite, &bIsBlack );

	// 若重命名后的原始文件全路径,不在白名单中,或者在黑名单中,则直接禁止掉.
	if ( bIsBlack || FALSE == bIsWhite )
	{
		kfree( (PVOID)szFullPath );
		return STATUS_BAD_INITIAL_PC ;
	}

	*ptr = 0; // 得到其父目录
	WhiteOrBlack( WhiteOrBlack_Flag_XFilePath, szFullPath, &bIsWhite, &bIsBlack );

	// 若重命名后的原始文件父路径,在黑名单中,直接禁止掉
	if ( bIsBlack )
	{
		kfree( (PVOID)szFullPath );
		return STATUS_BAD_INITIAL_PC ;
	}

	// 一切正常,创建重命名后的原始文件
	*ptr = '\\' ;
	++ ptr ;
	OldData = *ptr;
	*ptr = 0 ;

	// 打开文件的父目录,获得句柄
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniObjectName, szFullPath );
	
	status = g_NtCreateFile_addr( &hDirectory, 0x120116, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x20, 0, 0 );
	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( hDirectory );
		kfree( (PVOID)szFullPath );
		return status ;
	}

	*ptr = OldData ;

	FileInfoLength = PathSunLength + 0x18 ;
	FileInfo = (PFILE_RENAME_INFORMATION) kmalloc( FileInfoLength );
	if ( NULL == FileInfo ) 
	{ 
		ZwClose( hDirectory );
		kfree( (PVOID)szFullPath );
		return STATUS_INSUFFICIENT_RESOURCES; 
	}

	FileInfo->Replace = Replace ;
	FileInfo->FileNameLength = PathSunLength ;
	FileInfo->RootDir = hDirectory ;

	memcpy( FileInfo->FileName, szPathSun, PathSunLength );

	// OK,重命名文件
	status = g_NtSetInformationFile_addr ( hFile, &IoStatusBlock, &FileInfo, FileInfoLength, FileRenameInformation );

	ZwClose( hDirectory );
	kfree( (PVOID)szFullPath );
	kfree( (PVOID)FileInfo );
	return status ;
}



PROTEINBOXDLL_API NTSTATUS PB_CreateDirOrLink( IN LPWSTR lpcFullPath, IN LPWSTR lpcFatherPath )
{
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;

	if ( NULL == lpcFullPath ) { return STATUS_INVALID_PARAMETER ; }

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_CreateDirOrLink ;
	buffer.CreateDirOrLinkBuffer.bFlag = FALSE ;
	buffer.CreateDirOrLinkBuffer.lpcFullPath = lpcFullPath ;
	buffer.CreateDirOrLinkBuffer.lpcFullPathLength = ( wcslen(lpcFullPath) + 1 ) * sizeof(WCHAR) ;

	if ( lpcFatherPath )
	{ 
		buffer.CreateDirOrLinkBuffer.bFlag = TRUE;
		buffer.CreateDirOrLinkBuffer.lpcFatherPath = lpcFatherPath ;
		buffer.CreateDirOrLinkBuffer.lpcFatherPathLength = ( wcslen(lpcFatherPath) + 1 ) * sizeof(WCHAR) ;
	}

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	return status ;
}



PROTEINBOXDLL_API BOOL PB_StartCOM()
{
	return PB_StartCOM_Ex( L"\\RPC Control\\epmapper" );
}



PROTEINBOXDLL_API BOOL PB_IsOpenCOM()
{
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;

	if ( FALSE == g_b_Function_SbieDll_IsOpenCOM_is_Called )
	{
		g_b_Function_SbieDll_IsOpenCOM_is_Called = TRUE ;

		WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, L"\\RPC Control\\epmapper", &bIsWhite, &bIsBlack );
		if ( bIsWhite ) { g_bIsOpenCOM = TRUE ; }
	}

	return g_bIsOpenCOM ;
}



PROTEINBOXDLL_API BOOL 
PB_RunFromHome (
	IN LPWSTR szProcessName,
	IN LPWSTR szCommandLine,
	IN LPSTARTUPINFOW lpStartupInfo,
	OUT LPPROCESS_INFORMATION lpProcessInfo
	)
{
	HMODULE hModule = NULL ;
	HANDLE Heap = NULL ;
	BOOL bRet = FALSE, bInheritHandles = FALSE ;
	LPWSTR lpCommandLine = NULL, ptr1 = NULL, ptr2 = NULL ; 
	ULONG_PTR Size = 0, offset = 0 ;

	if ( NULL == szProcessName ) { return FALSE; }

	// 1. 拼接得到szProcessName的全路径,比如start.exe对应的全路径为"c:\Program Files\sandboxie\start.exe"
	Size = wcslen( szProcessName ) + 0x208;
	if ( szCommandLine ) { Size += wcslen( szCommandLine ); }

	lpCommandLine = (LPWSTR) kmalloc( 2 * Size );
	ptr1 = wcsrchr(szProcessName, '.');
	if ( NULL == ptr1 || wcsicmp(ptr1, L".exe") )
	{
		offset = 0 ;
	}
	else
	{
		*lpCommandLine = '\"' ;
		offset = 1 ;
	}

	// 2. 拼接得到完整的命令行内容
	hModule = GetPBDllHandle();
	GetModuleFileNameW( hModule, &lpCommandLine[offset], 0x104 );
	
	ptr2 = wcsrchr(lpCommandLine, '\\');
	if ( ptr2 ) { ptr2[1] = 0 ; }
	
	wcscat(lpCommandLine, szProcessName);

	if ( offset )
	{
		if ( szCommandLine )
		{
			wcscat( lpCommandLine, L"\" " );
			wcscat( lpCommandLine, szCommandLine );
		}
		else
		{
			wcscat( lpCommandLine, L"\"" );
		}
	}

	// 3. 根据拼接得到的命令行内容,启动进程
	if ( lpProcessInfo )
	{
		if ( lpStartupInfo->lpReserved )
		{
			bInheritHandles = TRUE ;
			lpStartupInfo->lpReserved = NULL ;
		}
		else
		{
			bInheritHandles = FALSE ;
		}

		lpProcessInfo->hProcess = 0 ;
		lpProcessInfo->hThread = 0 ;
		lpProcessInfo->dwProcessId = 0 ;
		lpProcessInfo->dwThreadId = 0 ;

		bRet = CreateProcessW( NULL, lpCommandLine, NULL, NULL, bInheritHandles, 0, 0, 0, lpStartupInfo, lpProcessInfo );
	}
	else
	{
		Heap = GetProcessHeap();
		lpStartupInfo->lpReserved = (LPWSTR) HeapAlloc( Heap, 0, 2 * Size );
		if ( lpStartupInfo->lpReserved )
		{
			memcpy( lpStartupInfo->lpReserved, lpCommandLine, 2 * Size );
			bRet = TRUE ;
		}
		else
		{
			bRet = FALSE ;
		}
	}

	kfree( lpCommandLine );
	return bRet;
}



PROTEINBOXDLL_API NTSTATUS
PB_DuplicateObject( 
	IN PHANDLE TargetHandle,
	IN HANDLE FuckedHandle,
	IN HANDLE SourceHandle,
	IN ACCESS_MASK DesiredAccess,
	IN ULONG_PTR Options
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_DuplicateObject ;
	buffer.DuplicateObjectBuffer.pTargetHandle = TargetHandle ; 
	buffer.DuplicateObjectBuffer.FuckedHandle  = FuckedHandle ;
	buffer.DuplicateObjectBuffer.SourceHandle  = SourceHandle ;
	buffer.DuplicateObjectBuffer.DesiredAccess = DesiredAccess ;
	buffer.DuplicateObjectBuffer.Options	   = Options ;	

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	return status ;
}



PROTEINBOXDLL_API void 
PB_DeviceChange (
	int wParam,
	int lParam
	)
{
	if ( wParam == WM_APP || wParam == 0x8004 )
	{
		if ( *(DWORD *)(lParam + 4) == 2 )
		{
			if ( !(*(BYTE *)(lParam + 0x10) & 1) )
			{
				HandlerPanfu( *(DWORD *)(lParam + 0xC) );
				//	Ioctl_ClearUpNode();
			}
		}
	}
}



PROTEINBOXDLL_API int 
PB_StartBoxedService (
	IN LPWSTR ServiceName
	)
{
	ULONG NameLength = 0, DataLength = 0, ErrorCode = 0, nCounts = 0 ;
	BOOL bRet = FALSE, bAreadyHaveName = FALSE, bNeedFree = FALSE ;
	HANDLE hKey1 = NULL, hKey2 = NULL ;
	LPWSTR szExePath = NULL, Name = NULL ;
	LPFuckService_Info lpBuffer = NULL, Data = NULL ;
	LPRPC_OUT_HEADER pRpcOutBuffer = NULL ;
	LPRPC_IN_StartBoxedService pRpcInBuffer = NULL ;
	SERVICE_DATA ServiceData ;
	STARTUPINFOW StartupInfo ;
	PROCESS_INFORMATION ProcessInfo ;

	lpBuffer = (LPFuckService_Info) FuckService( ServiceName, 1, 0xFFFFFFFF );
	if ( 0 == lpBuffer ) { return 0; }

	if ( lpBuffer->SBIE_CurrentState != 1 && lpBuffer->SBIE_CurrentState != 2 )
	{
		kfree( lpBuffer );
		SetLastError( ERROR_SERVICE_ALREADY_RUNNING );
		return 0;
	}

	if ( !(lpBuffer->ServiceType & 0x30) )
	{
		kfree( lpBuffer );
		SetLastError( ERROR_SERVICE_LOGON_FAILED );
		return 0;
	}

	hKey1 = Open_Services_RegKey( ServiceName, TRUE );
	if ( NULL == hKey1 )
	{
		kfree( lpBuffer );
		SetLastError( ERROR_SERVICE_DOES_NOT_EXIST );
		return 0;
	}

	ServiceData.SBIE_ProcessId = 0;
	ServiceData.SBIE_Win32ExitCode = 0;
	ServiceData.SBIE_ServiceSpecificExitCode = 0;
	ServiceData.SBIE_CheckPoint = 0;
	ServiceData.SBIE_CurrentState = 2;
	ServiceData.SBIE_ControlsAccepted = 1;
	ServiceData.SBIE_WaitHint = 5000 ;

	Set_service_data( (SC_HANDLE)0x12340002, &ServiceData, hKey1 );
	CloseHandle( hKey1 );

	if ( 0 == wcsicmp(ServiceName, L"bits") )
	{
		szExePath = L"PBBITS.exe" ;
	}
	else if ( 0 == wcsicmp(ServiceName, L"wuauserv") )
	{
		szExePath = L"PBWUAU.exe" ;
	}
	else if ( 0 == wcsicmp(ServiceName, L"cryptsvc") )
	{
		memset( &StartupInfo, 0, sizeof(StartupInfo) );
		StartupInfo.cb = 0x44 ;
		
		bRet = PB_RunFromHome( L"PBCrypto.exe", NULL, &StartupInfo, &ProcessInfo );
		if ( bRet )
		{
			CloseHandle( ProcessInfo.hThread );
			CloseHandle( ProcessInfo.hProcess );
			SetLastError( NO_ERROR );
		}
		else
		{
			SetLastError( ERROR_SERVICE_LOGON_FAILED );
		}

		return bRet ;
	}
	else
	{
		bAreadyHaveName = TRUE ;
	}

	if ( FALSE == bAreadyHaveName )
	{
		StartupInfo.lpReserved = 0 ;

		bRet = PB_RunFromHome( szExePath, NULL, &StartupInfo, NULL );
		if ( FALSE == bRet || (Name = StartupInfo.lpReserved) == NULL )
		{
			bAreadyHaveName = TRUE ;
		}
		else
		{
			bNeedFree = TRUE ;
		}
	}

	if ( bAreadyHaveName ) { Name = (LPWSTR)((char *)&lpBuffer->Separatrix + lpBuffer->szImagePathOffset); }

	NameLength = 2 * wcslen(Name) + 2;
	DataLength = 2 * wcslen(Name) + 0x12;

	pRpcInBuffer = (LPRPC_IN_StartBoxedService) kmalloc( DataLength );

	pRpcInBuffer->RpcHeader.Flag		= _PBSRV_APINUM_StartBoxedService_ ;
	pRpcInBuffer->RpcHeader.DataLength	= DataLength ;
	pRpcInBuffer->NameLength			= NameLength ;
	
	pRpcInBuffer->szName = (LPWSTR) kmalloc( NameLength );
	memcpy( pRpcInBuffer->szName, Name, NameLength );
	kfree( lpBuffer );

	pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( pRpcInBuffer );

	kfree(pRpcInBuffer->szName);
	kfree(pRpcInBuffer);
	if ( pRpcOutBuffer )
	{
		ErrorCode = pRpcOutBuffer->u.ErrorCode ;
		PB_FreeReply( pRpcOutBuffer );
	}
	else
	{
		ErrorCode = ERROR_NOT_ENOUGH_MEMORY ;
	}

	if ( bNeedFree ) { HeapFree( GetProcessHeap(), 4, Name ); }

	if ( !ErrorCode )
	{
		while ( TRUE )
		{
			Sleep( 500 );

			Data = (LPFuckService_Info) FuckService( ServiceName, 1, 0 );
			if ( NULL == Data ) { return 0; }

			if ( 4 == Data->SBIE_CurrentState )
			{
				SetLastError( NO_ERROR );
				return 1;
			}

			kfree( Data );
			++ nCounts;
			if ( nCounts >= 0x14 )
			{
				ErrorCode = ERROR_SERVICE_LOGON_FAILED ;
				goto _error_ ;
			}
		}
	}

_error_ :
	hKey2 = Open_Services_RegKey( ServiceName, TRUE );
	if ( hKey2 )
	{
		ServiceData.SBIE_ProcessId = 0;
		ServiceData.SBIE_Win32ExitCode = 0;
		ServiceData.SBIE_ServiceSpecificExitCode = 0;
		ServiceData.SBIE_CheckPoint = 0;
		ServiceData.SBIE_ControlsAccepted = 0;
		ServiceData.SBIE_WaitHint = 0;
		ServiceData.SBIE_CurrentState = 1;

		Set_service_data( (SC_HANDLE)0x12340002, &ServiceData, hKey2 );
		CloseHandle( hKey2 );
	}

	if ( ErrorCode == ERROR_SPECIAL_ACCOUNT ) { ErrorCode = ERROR_ACCESS_DENIED ; }

	SetLastError( ErrorCode );
	return 0;
}



PROTEINBOXDLL_API BOOL PB_CanElevateOnVista()
{
	BOOL bRet = FALSE ;
	HANDLE hToken = NULL;
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG ReturnLength = 0, info = 0 ;

	status = ZwOpenProcessToken( NtCurrentProcess(), TOKEN_QUERY, &hToken );
	if (!NT_SUCCESS(status)) { return FALSE; }
	
	if ( g_NtQueryInformationToken_addr ) 
	{
		status = g_NtQueryInformationToken_addr( hToken, (TOKEN_INFORMATION_CLASS)0x12, &info, 4, &ReturnLength );
	}
	else
	{ 
		status = ZwQueryInformationToken( hToken, (TOKEN_INFORMATION_CLASS)0x12, &info, 4, &ReturnLength );
	}

	if ( NT_SUCCESS(status) && 3 == info )
	{
		bRet = TRUE ;
	}

	ZwClose( hToken );
	return bRet;
}


PROTEINBOXDLL_API BOOL
PB_IsOpenClsid (
	LPCLSID clsid,
	DWORD dwClsContext,
	BOOL bDestroy
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/30 [30:1:2012 - 9:47]

Routine Description:
  判断当前CLSID是否属于白名单。
  沙盘内的程序默认是不能访问沙盘外程序提供的COM组件服务，允许访问特定的COM组件，也是为了解决在使用中的问题，提供例外设置。
  COM 是有可能造成信息泄露的，比如PCFlank测试。eg：OpenClsid={D713F357-7920-4B91-9EB6-49054709EC7A}
    
Arguments:
  clsid - 待验证的CLSID
  dwClsContext - CoGetClassObject的参数
  bDestroy - 是否销毁数组

--*/
{
	// 1. 仅处理本地服务
	if ( IS_FLAG_OFF(dwClsContext, CLSCTX_LOCAL_SERVER) )
		return FALSE;

	const CLSID CLSID_nothing = /*{056CDF58-CD7A-41D2-AF75-7CA1F44B84E9}*/ { 0x56CDF58, 0xCD7A, 0x41D2, {0xAF, 0x75, 0x7c, 0xA1, 0xF4, 0x4B, 0x84, 0xE9} } ;
	if ( IsEqualCLSID((const GUID &)clsid, (const GUID &)CLSID_InProcFreeMarshaler) )
		return TRUE;

	BOOL bGotoCheck = FALSE;
	if (bDestroy)
	{
		if ( g_CLSIDs_WhiteArray )
		{
			kfree(g_CLSIDs_WhiteArray);
			g_CLSIDs_WhiteArray = NULL;
		}

		g_CLSIDs_WhiteArray_nCounts = 0xFFFFFFFF;
	} 
	else
	{
		if ( g_CLSIDs_WhiteArray_nCounts != 0xFFFFFFFF )
			bGotoCheck = TRUE;
	}

	typedef HRESULT (WINAPI* _IIDFromString_)(LPOLESTR, LPIID );
	_IIDFromString_ IIDFromString_addr = NULL;

	do 
	{
		if ( bGotoCheck ) { break; }

		//
		// 2. 查询配置文件中的"Openclsid",将白名单CLSID全部保存至数组中。
		// TODO:{sudami}需调用PB_QueryConf函数; 我们先简化一下儿,后期调试时再修补  - -|
		//

		HMODULE hModule = LoadLibraryW(L"Ole32.dll");
		if ( NULL == hModule ) { break; }

		IIDFromString_addr = (_IIDFromString_) GetProcAddress(hModule, "IIDFromString");

	} while (FALSE);


	//
	// {TODO}sudami:白名单匹配，后期再做吧。
	//


	return FALSE;
}



PROTEINBOXDLL_API BOOL PB_IsDirectory ( LPCWSTR lpFileName )
{	
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;
	ULONG n = 0, length = 0 ;
	WCHAR data1, data2 ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	LPWSTR lpBuffer = NULL, lp = NULL ;
	BOOL bIsDirectory = FALSE ;

	lpBuffer = (LPWSTR) kmalloc( 0x120 );
	n = GetFullPathNameW( lpFileName, 0x80, lpBuffer, &lp );
	
	if (  0 == n ) { return FALSE; }
	
	if ( n >= 0x80 )
	{
		kfree( lpBuffer );
		lpBuffer = (LPWSTR) kmalloc( 2 * n + 0x20 );
		n = GetFullPathNameW( lpFileName, n, lpBuffer, &lp );
	}

	if ( 2 == n || 3 == n )
	{
		// 处理目录
		data1 = towlower( *lpFileName );
		data2 = lpFileName[2] ;

		if ( data1 >= 'a' && data1 <= 'z' && ':' == lpFileName[1] && ( '\\' == data2 || NULL == data2 ) ) { return TRUE; }
	}

	length = wcslen( lpBuffer );
	memmove( lpBuffer + 4, lpBuffer, 2 * length + 2 );
	memcpy( lpBuffer, L"\\??\\", 4 * sizeof(WCHAR) );
	
	RtlInitUnicodeString( &uniBuffer, lpBuffer );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwOpenFile( &hFile, FILE_GENERIC_READ, &ObjAtr, &IoStatusBlock, 7, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT );
	if ( NT_SUCCESS(status) || STATUS_NO_MEDIA_IN_DEVICE == status )
	{
		bIsDirectory = TRUE ;
		ZwClose( hFile );
	}

	kfree( lpBuffer );
	return bIsDirectory ;
}



PROTEINBOXDLL_API NTSTATUS PB_MonitorPut( IN ULONG Flag, IN LPWSTR szInfo )
{
	return STATUS_SUCCESS;
}



PROTEINBOXDLL_API void PB_Log( IN LPCWSTR szInfo )
{
	return;
}



PROTEINBOXDLL_API NTSTATUS PB_GetFileName( IN HANDLE HandleValue, IN ULONG BufferLength, IN LPWSTR lpData )
{
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_GetFileName ;
	buffer.GetFileNameBuffer.HandleValue  = HandleValue ;
	buffer.GetFileNameBuffer.BufferLength = BufferLength ;
	buffer.GetFileNameBuffer.lpData		  = (PVOID)	lpData ;

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	return status ;
}



PROTEINBOXDLL_API NTSTATUS PB_EnumProcessEx( IN LPWSTR szBoxName, OUT int* pArrays )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/05/06 [6:5:2011 - 14:54]

Routine Description:
  统计沙箱中运行的进程个数 & PID,格式如下
   _________
  | nCounts |
  | PID1    |  
  | PID2    |
  | PIDN    |
   ---------
  一般情况下会有至少3个进程: PBRpcSs.exe / PBDcomLaunch.exe / 1.exe(在沙箱中被运行的测试程序)
    
Arguments:
  Buffer - 保存查询到的内容 

    
--*/
{

	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;

	if ( NULL == szBoxName ) { return STATUS_INVALID_PARAMETER; }

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_EnumProcessEx ;
	buffer.EnumProcessExBuffer.lpBoxName = szBoxName ;
	buffer.EnumProcessExBuffer.pArray    = pArrays ;

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	return status ;
}



PROTEINBOXDLL_API LPWSTR PB_AssocQueryProgram( IN LPWSTR lpValueName )
{
	WORD data = 0 ;
	DWORD length = 0 ;
	wchar_t string ;
	LPWSTR pData = NULL, ptr1 = NULL, ptr2 = NULL, pBuffer = NULL ;

	pData = (LPWSTR) GetRegShellCommandString( lpValueName, L"open" );
	if ( NULL == pData )
	{
		pData = GetRegShellCommandString( lpValueName, L"cplopen" );
		if ( NULL == pData ) { return NULL; }
	}

	data = (WORD) *pData ;
	for ( ptr1 = pData; data; ++ptr1 )
	{
		if ( data != ' ' ) { break; }
		data = ptr1[1];
	}

	if ( *ptr1 == '\"' )
	{
		string = '\"';
		++ptr1;
	}
	else
	{
		string = ' ';
	}

	ptr2 = wcschr(ptr1, string);
	if ( ptr2 ) { *ptr2 = 0; }

	length = GetLongPathNameW(ptr1, 0, 0);
	if ( length )
	{
		pBuffer = (LPWSTR) kmalloc( 2 * length + 0x10 );
		GetLongPathNameW( ptr1, pBuffer, length );
		pBuffer[ length ] = 0;
	}

	kfree( pData );
	return pBuffer;
}



PROTEINBOXDLL_API BOOL PB_QueryBoxPath( IN LPWSTR szBoxName, OUT LPWSTR szBoxPath )
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	if ( NULL == szBoxName ) { return FALSE ; }

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_QueryBoxPath ;
	buffer.QueryBoxPathBuffer.lpBoxName = szBoxName ;
	buffer.QueryBoxPathBuffer.BoxNamLength = (wcslen(szBoxName) + 1) * sizeof(WCHAR) ;
	buffer.QueryBoxPathBuffer.lpBoxPath = szBoxPath ; // 硬编码,大小不超过MAX_PATH
	buffer.QueryBoxPathBuffer.BoxPathMaxLength = MAX_PATH ;

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}



PROTEINBOXDLL_API BOOL PB_EnumBoxes( IN PVOID pData )
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	if ( NULL == pData ) { return FALSE ; }

	buffer.Head.Flag = FLAG_Ioctl_EnumBoxs ;
	memcpy( &buffer.EnumBoxsBuffer, pData, sizeof(IOCTL_ENUMBOXS_BUFFER) );

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}




PROTEINBOXDLL_API BOOL PB_DisableForceProcess( IN int Flag, IN LPWSTR szForcePath )
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_DisableForceProcess ;
	buffer.DisableForceProcessBuffer.Flag = Flag;

	if ( szForcePath )
	{
		buffer.DisableForceProcessBuffer.szProcName = szForcePath ;
		buffer.DisableForceProcessBuffer.NameLength = (wcslen(szForcePath) + 1) * sizeof(WCHAR);
	}

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}



PROTEINBOXDLL_API BOOL PB_KillAll( IN int PID, IN LPWSTR szBoxName )
{
	BOOL bRet = FALSE ;
	LPRPC_OUT_HEADER pRpcOutBuffer = NULL ;
	RPC_PROCTERMINATEBOX_INFO pRpcInBuffer ; 

	pRpcInBuffer.RpcHeader.DataLength = sizeof( RPC_PROCTERMINATEBOX_INFO ) ;
	pRpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_TerminateBox_ ;
	pRpcInBuffer.PID = PID;
	wcscpy( pRpcInBuffer.szBoxName, szBoxName );

	pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pRpcInBuffer );
	if ( pRpcOutBuffer )
	{
		if ( !pRpcOutBuffer->u.Status )
			bRet = TRUE;

		PB_FreeReply( pRpcOutBuffer );
	}

	return bRet;
}



PROTEINBOXDLL_API BOOL PB_KillOne( IN int PID )
{
	BOOL bRet = FALSE ;
	LPRPC_OUT_HEADER pRpcOutBuffer = NULL ;
	RPC_PROCKILLPROCESS_INFO pRpcInBuffer ; 

	pRpcInBuffer.RpcHeader.DataLength = sizeof( RPC_PROCKILLPROCESS_INFO ) ;
	pRpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_KillProcess_ ;
	pRpcInBuffer.PID = PID;

	pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pRpcInBuffer );
	if ( pRpcOutBuffer )
	{
		if ( !pRpcOutBuffer->u.Status )
			bRet = TRUE;

		PB_FreeReply( pRpcOutBuffer );
	}

	return bRet;
}



PROTEINBOXDLL_API BOOL PB_GetVersion()
{
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;
	WCHAR szVersion[ 0x20 ] = L"" ;

	if ( NULL == szVersion ) { return FALSE ; }

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_GetSysVersion ;
	buffer.GetSysVersion.wszVersion = szVersion ;

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	if ( ! NT_SUCCESS(status) ) { return FALSE; }
	return TRUE;
}



PROTEINBOXDLL_API BOOL PB_StartPBSvc( IN BOOL bRetry )
{
	ULONG nIndex = 0 ;
	HMODULE hModule = NULL ;
	SC_HANDLE hSCManager = NULL, hService = NULL ;

	if ( NULL == g_OpenSCManagerW_addr )
	{
		hModule = Get_advapi32dll_Handle();
		if ( hModule )
		{
			g_OpenSCManagerW_addr		= (_OpenSCManagerW_) GetProcAddress( hModule, "OpenSCManagerW" );
			g_OpenServiceW_addr			= (_OpenServiceW_) GetProcAddress( hModule, "OpenServiceW" );
			g_StartServiceW_addr		= (_StartServiceW_) GetProcAddress( hModule, "StartServiceW" );
			g_CloseServiceHandle_addr	= (_CloseServiceHandle_) GetProcAddress( hModule, "CloseServiceHandle" );
			g_CreateServiceW_addr		= (_CreateServiceW_) GetProcAddress( hModule, "CreateServiceW" );
		}
	}

	TCHAR szModuleFile[ MAX_PATH ] = L"" ;
	RtlZeroMemory( szModuleFile, MAX_PATH*sizeof(WCHAR) );

	GetModuleFileNameW( NULL, szModuleFile, 500 );
	PathRemoveFileSpec( szModuleFile );
	PathAppend( szModuleFile, L"PBSrv.exe" );

	PVOID pRpcOutBuffer = NULL ;
	RPC_IN_HEADER pRpcInBuffer ;

	while ( TRUE )
	{
		pRpcInBuffer.DataLength = 8;
		pRpcInBuffer.Flag = _PBSRV_APINUM_GetAppVersion_ ;

		pRpcOutBuffer = PB_CallServer( &pRpcInBuffer );
		if ( pRpcOutBuffer ) { break; }

		if ( nIndex )
		{
			if ( FALSE == bRetry ) { return FALSE; }
			goto _while_next_ ;
		}

		if ( g_OpenSCManagerW_addr )
		{
			hSCManager = g_OpenSCManagerW_addr( NULL, NULL, 0x80000000 | SC_MANAGER_CREATE_SERVICE );
			if ( hSCManager )
			{
				if ( NULL == g_OpenServiceW_addr )
				{
					g_CloseServiceHandle_addr( hSCManager );
					goto _while_next_;
				}

				hService = (SC_HANDLE)g_OpenServiceW_addr( hSCManager, L"PBSvc", SERVICE_START );
				if ( NULL == hService )
				{
					hService = g_CreateServiceW_addr( 
						hSCManager, L"PBSvc", L"PBSvc",
						SERVICE_ALL_ACCESS,
						SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS ,
						SERVICE_AUTO_START,
						SERVICE_ERROR_NORMAL,
						szModuleFile,
						NULL, NULL, NULL, NULL, NULL 
						);
				}
				
				if ( hService )
				{
					if ( g_StartServiceW_addr ) { g_StartServiceW_addr( hService, 0, 0 ); }
					g_CloseServiceHandle_addr( hService );
				}

				g_CloseServiceHandle_addr( hSCManager );
			}
		}

_while_next_:
		Sleep( 200 );
		++ nIndex;
		if ( nIndex >= 3 ) { return FALSE; }
	}

	if ( pRpcOutBuffer ) { PB_FreeReply(pRpcOutBuffer); }
	return TRUE;
}



PROTEINBOXDLL_API BOOL PB_StartPBDrv( IN BOOL bWait )
{
	BOOL bRet = FALSE ;
	ULONG nIndex = 0 ;
	PVOID pRpcOutBuffer = NULL ;
	RPC_IN_HEADER pRpcInBuffer ;

	if ( PB_GetVersion() ) { return TRUE; }

	// 1. 初始化R3的配置文件读取模块
	CConfigEx *pConf = new CConfigEx();
	if ( NULL == pConf )
	{
		MYTRACE( L"error! | PB_StartPBDrv() - new CConfig(); | NULL == pConf \n" );
		return FALSE ;
	}

	// 2. 加载ProteinBox.sys
	while ( FALSE == PB_GetVersion() )
	{
		if ( nIndex )
		{
			if ( FALSE == bWait ) { return FALSE; }
		}
		else
		{
			pRpcInBuffer.DataLength = 8 ;
			pRpcInBuffer.Flag = _PBSRV_APINUM_StartPBDrv_ ;

			pRpcOutBuffer = PB_CallServer( &pRpcInBuffer );
			if ( pRpcOutBuffer ) { PB_FreeReply(pRpcOutBuffer); }
		}

		Sleep( 200 );
		++ nIndex;
		if ( nIndex >= 50 ) { return FALSE; }
	}

	// 3.1 开始传输数据
	bRet = pConf->Wakeup_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		MYTRACE( L"error!  | PB_StartPBDrv() - pConf->Wakeup_R0_InitConfigData() |  \n" );
		goto _over_ ;
	}

	bRet = pConf->Waitfor_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		MYTRACE( L"error!  | PB_StartPBDrv() - pConf->Waitfor_R0_InitConfigData() |  \n" );
		goto _over_ ;
	}

	// 3.2 告诉驱动开始Hook
	PB_HookObject( TRUE ) ;
	PB_HookShadow( TRUE ) ;

	MYTRACE( L"OK! | PB_StartPBDrv(); \n" );
	bRet = TRUE;

_over_:
	if ( pConf )
	{
		pConf->UnLoad();
		delete pConf ;
		pConf = NULL;
	}

	return bRet;
}



PROTEINBOXDLL_API BOOL PB_StopPBDrv()
{
	PVOID pRpcOutBuffer = NULL ;
	RPC_IN_HEADER pRpcInBuffer ;

	// 1. 卸载Shadow ssdt的Inline Hook
	if ( FALSE == PB_HookShadow(FALSE) )
		return FALSE;

	// 2. 卸载驱动
	pRpcInBuffer.DataLength = 8 ;
	pRpcInBuffer.Flag = _PBSRV_APINUM_StopPBDrv_ ;

	PB_CallServer( &pRpcInBuffer );
	return TRUE;
}



PROTEINBOXDLL_API BOOL PB_QueryConf( IN LPWSTR section, IN LPWSTR key, IN ULONG MaxLength, OUT PVOID pBuffer )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 11:34]

Routine Description:
  查询配置文件,需要和驱动通信    
    
Arguments:
  section - eg:"[GlobalSetting]"
  key - eg:"ForceProcess"
  MaxLength - @pBuffer缓冲区的最大长度
  pBuffer - 保存查询到的信息

Return Value:
  TRUE / FALSE
    
--*/
{
	if ( NULL == section || NULL == key || MaxLength <= 0 || NULL == pBuffer ) { return FALSE; }

	return TRUE;
}



PROTEINBOXDLL_API BOOL PB_ReloadConf()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 14:42]

Routine Description:
  重新加载配置文件
    
--*/
{
	BOOL bRet = FALSE;
	ULONG Reserved = 0x119 ;
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER Buffer ;

	// 1. 初始化R3的配置文件读取模块
	CConfigEx* pConf = new CConfigEx();
	if ( NULL == pConf )
	{
		MYTRACE( L"error! | PB_ReloadConf() - new CConfig(); | NULL == pConf \n" );
		return FALSE ;
	}

	// 2. 通知驱动Reload Config,驱动便会创建一个工作线程进行等待
	RtlZeroMemory( &Buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	Buffer.Head.Flag = FLAG_Ioctl_HandlerConf ;
	Buffer.Head.LittleIoctlCode = _Ioctl_Conf_function_Reload_ ;
	Buffer.ConfigBuffer.u.ReloadData.Reserved = Reserved;

	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&Buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&Buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	// 3. 开始传输数据
	bRet = pConf->Wakeup_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		MYTRACE( L"error!  | PB_ReloadConf() - pConf->Wakeup_R0_InitConfigData() |  \n" );
		goto _over_ ;
	}

	bRet = pConf->Waitfor_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		MYTRACE( L"error!  | PB_ReloadConf() - pConf->Waitfor_R0_InitConfigData() |  \n" );
		goto _over_ ;
	}

	MYTRACE( L"OK! | PB_ReloadConf(); \n" );
	bRet = TRUE;
_over_:
	pConf->UnLoad();
	delete pConf ;
	pConf = NULL;
	return bRet;
}



PROTEINBOXDLL_API VOID PB_FreeReply( IN PVOID pBuffer )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/10 [10:8:2011 - 17:21]

Routine Description:
  释放RPC返回的内存    

--*/
{
	int lpAddress = (int) pBuffer ;


	return;




	//
	// 调用内存管理器去释放内存 [PBSrv.exe 和 ProteinBoxDLL.dll 公用一套内存管理代码]
	// 后期实现
	//

	if ( 0 == lpAddress ) { return; }

	CMemoryManager::GetInstance().ExFreePool( lpAddress - 4, *(int *)(lpAddress - 4));
	return;
}



PROTEINBOXDLL_API NTSTATUS PB_StartProcess( IN HANDLE new_hToken, IN HANDLE new_ImpersonationToken, OUT BOOL* pbDriverNotLoad )
{
	IOCTL_PROTEINBOX_BUFFER Buffer ;
	HANDLE hTemp1, hTemp2 ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR wszBoxName[ MAX_PATH ] = L"DefaultBox" ;

	// 初始化InputBuffer
	Buffer.Head.Flag = FLAG_Ioctl_StartProcess ;
	Buffer.StartProcessBuffer.wszRegisterUserID = NULL ;
	Buffer.StartProcessBuffer.wszBoxName = wszBoxName ;

	if ( new_hToken && new_ImpersonationToken )
	{
		Buffer.StartProcessBuffer.new_hToken = &new_hToken ;
		Buffer.StartProcessBuffer.new_ImpersonationToken = &new_ImpersonationToken ;
	}
	else
	{
		Buffer.StartProcessBuffer.new_hToken = &hTemp1 ;
		Buffer.StartProcessBuffer.new_ImpersonationToken = &hTemp2 ;
	}

	// 关键一句
	status = g_ProteinBoxDLL.IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)&Buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&Buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	if ( ! NT_SUCCESS(status) ) 
	{ 
		MYTRACE( L"error!  | PB_StartProcess() - IoControlEx() |  \n" );

		if ( STATUS_UNSUCCESSFUL == status && pbDriverNotLoad ) { *pbDriverNotLoad = TRUE ; }
	}

	return status ;
}



PROTEINBOXDLL_API BOOL PB_InitProcess()
{
	g_ProteinBoxDLL.DoWork( GetModuleHandleW(L"ProteinBoxDLL.dll") );
	return TRUE;
}



PROTEINBOXDLL_API BOOL PB_HookShadow( IN BOOL bHook )
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_HookShadowSSDT ;
	buffer.HookShadowBuffer.bHook = bHook ;
	
	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}



PROTEINBOXDLL_API BOOL PB_HookObject( IN BOOL bHook )
{
	IOCTL_PROTEINBOX_BUFFER buffer ;

	// 1. 初始化InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	buffer.Head.Flag = FLAG_Ioctl_HookObject ;
	buffer.HookObjectBuffer.bHook = bHook ;

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	return bRet ;
}


PROTEINBOXDLL_API BOOL PB_IsBoxedService( IN SC_HANDLE hService )
{	
	BOOL bRet = FALSE;
	LPWSTR szServiceName = NULL;

	szServiceName = Get_service_name_from_handle(hService);
	if ( szServiceName )
		bRet = Is_white_service(szServiceName);
	else
		bRet = 0;

	return bRet;
}


///////////////////////////////   END OF FILE   ///////////////////////////////