/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcApiNum0x1200.cpp
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
#include "RpcProcess.h"


//////////////////////////////////////////////////////////////////////////

#define ProteinBoxDrv_LinkName	"ProteinBoxDrv"
#define SUBKEY_PROTEINBOX _T("SOFTWARE\\Proteinbox\\Config")
#define SUBVALUE_ROOTFOLDER _T("RootFolder")

CRpcProcess g_CRpcProcess ;


//////////////////////////////////////////////////////////////////////////


CDriver* 
InitDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName
	) 
{
	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == szDriverPath || NULL == szDriverLinkName )
	{
		// printf( "error! | InitDriver() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 加载驱动
	CDriver* drv = new CDriver( szDriverPath, szDriverLinkName );
	if(!drv->StartDriver() || !drv->OpenDevice())
	{
		char sz[8] = "";
		wsprintf(sz, "%d", ::GetLastError());
		//	MessageBox("Load Driver Failed",sz);
	}
	
	return drv ;
}



BOOL
LoadDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName,
	CDriver** drv
	)
{
	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == szDriverPath || NULL == szDriverLinkName )
	{
		// printf( "error! | LoadDriver() | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 加载 & 打开驱动设备
	//
	
	*drv = InitDriver( szDriverPath, szDriverLinkName );
	
	if ( NULL == *drv ) 
	{
		// printf( "error! | LoadDriver() | Can't Load Driver:\"%ws\" \n", szDriverPath );
		return FALSE ;
	}

	return TRUE ;
}



VOID 
UnloadDriver(
	IN CDriver* drv
	)
{
	if ( drv )
	{
		delete drv;
	}
	
	return ;
}




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcProcess( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcProcess.ProcProcess( pInfo, pRpcBuffer, Msg );
}



CRpcProcess::CRpcProcess(void)
:m_Context_LoadDrv(NULL),
m_drv(NULL)
{

}

CRpcProcess::~CRpcProcess(void)
{
}



VOID CRpcProcess::HandlerProcess( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	LPLOADDRVCONTEXT pTemp = (LPLOADDRVCONTEXT) pBuffer;

	if ( NULL == pNode || NULL == pBuffer ) { return; }

	pTemp->unknown1 = 0 ;
	pTemp->unknown2 = 0x1000 ;

	_InsertList( pNode, 0x1200, pBuffer, (_PROCAPINUMBERFUNC_)__ProcProcess, FALSE );

	InitializeCriticalSection( &m_LoadDrv_cs );
	InitializeCriticalSection( &m_UnmontRegHive_cs );
	m_Context_LoadDrv = pBuffer ;

	return;
}



PVOID 
CRpcProcess::ProcProcess(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	LPPOSTPROCRPCINFO ret = NULL ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_StartPBDrv_ :
		RpcLoadDrv();
		ret = PostProcRPC( g_CGlobal.GetTotalData(), NO_ERROR );
		break;

	case 0x1202 :
		break;

	case _PBSRV_APINUM_KillProcess_ :
		ret = RpcKillProcess( (LPRPC_PROCKILLPROCESS_INFO)pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_TerminateBox_ :
		ret = RpcTerminateBox( (LPRPC_PROCTERMINATEBOX_INFO)pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_StopPBDrv_ :
		RpcUnLoadDrv();
		ret = PostProcRPC( g_CGlobal.GetTotalData(), NO_ERROR );
		break;

	default:
		break;
	}

	 return (PVOID)ret;
}



VOID CRpcProcess::RpcLoadDrv()
{
	MYTRACE( "RpcLoadDrv \n" );

	CHAR szValue[MAX_PATH] = {0};
	CHAR szPath[MAX_PATH + 0x20 ] = {0};
	DWORD dwSize = sizeof(szValue);
	DWORD dwType = REG_SZ;

	LONG ret = SHGetValue (
		HKEY_LOCAL_MACHINE,
		SUBKEY_PROTEINBOX,
		SUBVALUE_ROOTFOLDER,
		&dwType, 
		szValue, 
		&dwSize 
		); 

	if( ERROR_SUCCESS != ret || NULL == szValue[0] ) 
	{ 
		MYTRACE( "error! | RpcLoadDrv() - SHGetValue(); | 读取注册表沙箱根目录失败!  \n" );
		return; 
	}

	int Size = MAX_PATH + 0x20;
	StringCbPrintf( szPath, Size, "%s\\%s", szValue, "ProteinBoxDrv.sys" );
// 	CHAR buffer[MAX_PATH];
// 	GetCurrentDirectory( MAX_PATH, buffer) ;
// 	sprintf( buffer + strlen(buffer), "\\ProteinBoxDrv.sys" );

	BOOL bRet = LoadDriver( szPath, ProteinBoxDrv_LinkName, &m_drv );
	if ( FALSE == bRet )
	{
		MYTRACE( "error! | RpcLoadDrv() - LoadDriver() | 加载ProteinBox.sys失败 \n" );
		return;
	}

	return;
}



VOID CRpcProcess::RpcUnLoadDrv()
{
	MYTRACE( "RpcUnLoadDrv \n" );

	// 创建一个线程去卸载驱动,不要让客户端等待;在线程中等到客户端进程结束后卸载驱动.
	HANDLE 	hThread = CreateThread( NULL, 0, UnloadDrvThread, this, 0, NULL );	
	if ( hThread )
		CloseHandle(hThread);

	return;
}


DWORD WINAPI CRpcProcess::UnloadDrvThread(LPVOID lpParameter)
{
	CRpcProcess* pThis = (CRpcProcess*)lpParameter;
	if( NULL != pThis )
	{
		pThis->UnloadDrvThreadProc();
	}

	return 0;
}


void CRpcProcess::UnloadDrvThreadProc()
{
	MYTRACE( "UnloadDrvThreadProc(); | I'm waiting just for 1 second,and then I'll unload your honey driver... \n" );
	Sleep(1000); // 等待1秒钟,让PBStart.exe退出
	
	MYTRACE( "UnloadDrvThreadProc(); | here we go, unloading \"proteinbox.sys\" \n" );
	if ( m_drv )
	{
		delete m_drv ;
		m_drv = NULL ;
	}

	MYTRACE( "UnloadDrvThreadProc(); | Done! Exit \n" );
	ExitProcess(0xFFFFFFFF);
	return;
}



LPPOSTPROCRPCINFO 
CRpcProcess::RpcKillProcess (
	IN LPRPC_PROCKILLPROCESS_INFO pRpcInBuffer, 
	IN int dwProcessId 
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL bRet = FALSE ;
	PB_BOX_INFO BoxInfo1, BoxInfo2 ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcKillProcess \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_PROCKILLPROCESS_INFO) )
	{
		status = STATUS_INVALID_PARAMETER;
		goto _over_;
	}

	// 确定在同一个沙箱内
	bRet = PB_QueryProcess( pRpcInBuffer->PID, &BoxInfo1 );
	if ( FALSE == bRet ) { goto _over_; }

	bRet = PB_QueryProcess( dwProcessId, &BoxInfo2 );
	if ( FALSE == bRet ) { goto _over_; }

	if ( wcsicmp(BoxInfo1.BoxName, BoxInfo2.BoxName) )
	{
		status = STATUS_ACCESS_DENIED;
		goto _over_;
	}

	bRet = KillProcessByID( pRpcInBuffer->PID );

_over_ :
	return PostProcRPC( pTotalData, status );
}



LPPOSTPROCRPCINFO 
CRpcProcess::RpcTerminateBox (
	IN LPRPC_PROCTERMINATEBOX_INFO pRpcInBuffer,
	IN int dwProcessId 
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL bRet = FALSE ;
	PB_BOX_INFO BoxInfo ;
	HANDLE hProcess = NULL;
	ULONG pBuffer = 0, TotalProcCounts = 0, n = 0, PIDCur = 0 ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcTerminateBox \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_PROCTERMINATEBOX_INFO) || NULL == pRpcInBuffer->szBoxName )
	{
		status = STATUS_INVALID_PARAMETER;
		goto _over_;
	}

	// 确定在同一个沙箱内
	bRet = PB_QueryProcess( dwProcessId, &BoxInfo );
	if ( FALSE == bRet ) { goto _over_; }

	DWORD PID = pRpcInBuffer->PID ;
	if ( 0xFFFFFFFF == PID )
	{
		PID = BoxInfo.SessionId;
	}
	else
	{
		if ( PID != BoxInfo.SessionId )
		{
			status = STATUS_ACCESS_DENIED;
			goto _over_;
		}
	}

	if ( wcsicmp(BoxInfo.BoxName, pRpcInBuffer->szBoxName) )
	{
		status = STATUS_ACCESS_DENIED;
		goto _over_;
	}

	pBuffer = (ULONG) kmalloc( 0x800 );
	status = PB_EnumProcessEx( pRpcInBuffer->szBoxName, (int*)pBuffer );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	TotalProcCounts = *(ULONG *)pBuffer ;
	if ( TotalProcCounts < 1 ) { goto _over_ ; }

	do
	{
		PIDCur = *(ULONG *)( pBuffer + 4 * n );
		hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, PIDCur );
		if ( hProcess )
		{
			TerminateProcess( hProcess, STATUS_UNSUCCESSFUL );
			CloseHandle(hProcess);
		}
		++n;
	}
	while( n <= TotalProcCounts );

	status = STATUS_SUCCESS;

_over_ :
	kfree( (PVOID)pBuffer );
	return PostProcRPC( pTotalData, status );
}


//////////////////////////////////////////////////////////////////////////

BOOL CRpcProcess::KillProcessByID( int dwProcessId )
{
	HANDLE hProcess = NULL;
	BOOL bRet = FALSE;

	hProcess = OpenProcess( 1, 0, dwProcessId );
	if ( hProcess )
	{
		bRet = TerminateProcess( hProcess, STATUS_UNSUCCESSFUL );
		CloseHandle(hProcess);
	}

	return bRet;
}



///////////////////////////////   END OF FILE   ///////////////////////////////