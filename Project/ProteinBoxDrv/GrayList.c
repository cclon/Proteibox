/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/16 [16:7:2010 - 10:51]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\GrayList.c
* 
* Description:
*      
*   管理灰名单数据,和进程节点相关联                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "Common.h"
#include "Version.h"
#include "Security.h"
#include "Config.h"
#include "GrayList.h"


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


BOOL 
GLAllocateNode(
	OUT PVOID* pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  分配 & 初始化一个链表节点  
    
Arguments:
  _pCurrenList_ - 保存分配好的链表Node的指针

Return Value:
  BOOL
    
--*/
{
	ULONG Length = sizeof( GRAYLIST_INFO );

	if ( NULL == pNode )
	{
		dprintf( "error! | GLAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pNode = kmalloc( Length );
	if ( NULL == *pNode )
	{
		dprintf( "error! | GLAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pNode, Length );
	return TRUE ;
}



PVOID
GLBuildNode (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead,
	IN LPWSTR wszName,
	IN int NameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  注意:@ListHead 链表头中永远不会存储wszName,它的作用不是用作节点. 故遍历该链表查找数据时,无需关注表头中存储的内容
    
Arguments:
  _TotalHead - 配置文件的链表头
  pInBuffer - 要建立的节点信息
    
--*/
{
	BOOL bRet = FALSE ;
	LPGRAYLIST_INFO pNode = NULL ;
	
	// 1. 校验参数合法性
	if ( NULL == QueueLockList || NULL == ListHead || NULL == wszName || NameLength <= 0 )
	{
		dprintf( "error! | GLBuildNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 2. 申请节点
	bRet = GLAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | GLBuildNode() - GLAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	// 3. 填充之
	pNode->wszName = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszName )
	{
		dprintf( "error! | GLBuildNode() - kmalloc(); | 申请内存失败! \n" );
		kfree( (PVOID)pNode ); // 释放内存
		return NULL ;
	}

	RtlZeroMemory( pNode->wszName, NameLength );
	RtlCopyMemory( pNode->wszName, wszName, NameLength );

	pNode->NameLength = NameLength ;

	// 4. 插入链表中
	EnterCrit( QueueLockList );	// 加锁访问
	InsertTailList( ListHead, &pNode->ListEntry );
	LeaveCrit( QueueLockList );	// 释放锁

	return (PVOID)pNode ;
}



BOOL
GLCopyNode (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead,
	IN PVOID KeyHead
	)
{
	LPCDNODEK pKeyHead = (LPCDNODEK) KeyHead ;
	LPCDNODEV pValueHead = NULL, pValueNode = NULL ;

	// 1. 校验参数合法性
	if ( NULL == QueueLockList || NULL == ListHead || NULL == pKeyHead )
	{
		dprintf( "error! | GLCopyNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 得到Key对应的Value表头,遍历Value链表
	pValueNode = pValueHead = &pKeyHead->ValueListHead ;
	if ( NULL == pValueHead || FALSE == MmIsAddressValid( (PVOID)pValueHead ) ) 
	{ 
		dprintf( "error! | GLCopyNode(); | Key对应的Value表头不合法 \n" );
		return FALSE ;
	}

	do
	{
		// 将当前节点拷贝到GL链表中
		GLBuildNode( QueueLockList, ListHead, pValueNode->ValueName, pValueNode->NameLength );

		pValueNode = pValueNode->pBlink ;
	} while( pValueNode != pValueHead );

	return TRUE ;
}



PVOID
GLFindNode (
	IN PLIST_ENTRY ListHead,
	IN PERESOURCE QueueLockList,
	IN LPWSTR wszName
	)
{
	BOOL bRet = FALSE ;
	PVOID pResult = NULL ;
	PLIST_ENTRY Next = NULL ;
	LPGRAYLIST_INFO pNode = NULL ;

	// 1. 校验参数合法性
	if ( NULL == QueueLockList || NULL == ListHead || NULL == wszName )
	{
		dprintf( "error! | GLFindNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 2. 遍历链表
	EnterCrit( QueueLockList );	// 加锁访问
	Next = ListHead->Flink;

	while ( Next != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( Next, GRAYLIST_INFO, ListEntry ));

		if ( 0 == _wcsicmp( wszName, pNode->wszName ) )
		{
			pResult = (PVOID)pNode ;
			break ;
		}
		
		Next = Next->Flink;
	}

	LeaveCrit( QueueLockList );	// 释放锁
	return pResult ;
}



BOOL
GLFindNodeEx (
	IN PLIST_ENTRY ListHead,
	IN PERESOURCE QueueLockList,
	IN LPWSTR Key,
	OUT LPWSTR Name
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/27 [27:7:2010 - 9:53]

Routine Description:
  遍历@ListHead链表.拿@Key进行匹配,支持通配符比较.
  注意:@ListHead 链表头中永远不会存储wszName,它的作用不是用作节点. 故遍历该链表查找数据时,无需关注表头中存储的内容
    
Arguments:
  ListHead - 链表头
  QueueLockList - 链表锁
  Key - 要查找的字符串

Return Value:
  BOOL
    
--*/
{
	BOOL			bResult	= FALSE ;
	PLIST_ENTRY		Next	= NULL ;
	LPGRAYLIST_INFO pNode	= NULL ;

	// 1. 校验参数合法性
	if ( NULL == ListHead || NULL == QueueLockList || NULL == Key )
	{
		dprintf( "error! | GLFindNodeEx(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 遍历链表
	EnterCrit( QueueLockList );	// 加锁访问
	Next = ListHead->Flink;

	while ( Next != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( Next, GRAYLIST_INFO, ListEntry ));

		if ( StringMatchW( pNode->wszName, Key, TRUE ) )
		{
			if ( Name ) 
			{ 
				RtlZeroMemory( Name, MAX_PATH );
				wcsncpy( Name, pNode->wszName, pNode->NameLength );
			}

			bResult = TRUE ;
			break ;
		}

		Next = Next->Flink;
	}

	LeaveCrit( QueueLockList );	// 释放锁
	return bResult ;
}


VOID
GLDistroyAll (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead
	)
{
	PLIST_ENTRY NextEntry = ListHead ;
	LPGRAYLIST_INFO pNode = NULL ;

	if ( NULL == QueueLockList || NULL == ListHead )
	{
		dprintf( "error! | GLDistroyAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

//	GLWalkNodes( QueueLockList, ListHead );

//	dprintf( "*** 开始卸载GrayList节点管理器 *** \n" );
	EnterCrit( QueueLockList );	// 加锁访问


	NextEntry = ListHead->Flink;
	while ( NextEntry != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( NextEntry, GRAYLIST_INFO, ListEntry ));
		NextEntry = NextEntry->Flink;

		RemoveEntryList( NextEntry->Blink );

		kfree( (PVOID)pNode->wszName );
		kfree( (PVOID)pNode );
	}

	LeaveCrit( QueueLockList );	// 释放锁
//	dprintf( "*** 结束卸载GrayList节点管理器 *** \n" );

	return ;
}



VOID
GLWalkNodes (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead
	)
{
	BOOL bRet = FALSE ;
	PLIST_ENTRY Next = NULL ;
	LPGRAYLIST_INFO pNode = NULL ;

#ifdef DBG

	if ( NULL == QueueLockList || NULL == ListHead )
	{
		dprintf( "error! | GLFindNode(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	dprintf( "当前GrayList链表内容如下: \n" );

	EnterCrit( QueueLockList );	// 加锁访问
	Next = ListHead->Flink;

	while ( Next != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( Next, GRAYLIST_INFO, ListEntry ));

		dprintf( "%ws, ", pNode->wszName );

		Next = Next->Flink;
		if ( NULL == Next ) { break ; }
	}

	LeaveCrit( QueueLockList );	// 释放锁

#endif
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


static const LPWSTR g_Array_BaseNamedObjects[] = 
{
	// add by self
	L"*\\BaseNamedObjects*\\shell.{*",
	L"*\\BaseNamedObjects*\\AtlDebugAllocator_*",

	// copy from Sandboxie
	L"\\Windows\\ApiPort",
	L"\\Sessions\\*\\Windows\\ApiPort",
	L"\\Sessions\\*\\Windows\\SharedSection",
	L"\\ThemeApiPort",
	L"\\KnownDlls\\*",
	L"\\NLS\\*",
	L"*\\BaseNamedObjects*\\ShimCacheMutex",
	L"*\\BaseNamedObjects*\\ShimSharedMemory",
	L"*\\BaseNamedObjects*\\SHIMLIB_LOG_MUTEX",
	L"\\Security\\LSA_AUTHENTICATION_INITIALIZED",
	L"\\LsaAuthenticationPort",
	L"\\NlsCacheMutant",
	L"\\KernelObjects\\*",
	L"\\NLAPublicPort",
	L"\\RPC Control\\nlaapi",
	L"\\RPC Control\\tapsrvlpc",
	L"\\RPC Control\\senssvc",
	L"*\\BaseNamedObjects*\\SENS Information Ca",
	L"*\\BaseNamedObjects*\\TabletHardwarePrese",
	L"*\\BaseNamedObjects*\\userenv: * Group Po",
	L"\\RPC Control\\SLCTransportEndpoint-*",
	L"\\RPC Control\\wpcsvc",
	L"*\\BaseNamedObjects*\\TermSrvReadyEvent",
	L"\\RPC Control\\dhcpcsvc",
	L"\\RPC Control\\dhcpcsvc6",
	L"\\RPC Control\\DNSResolver",
	L"\\RPC Control\\RasmanRpc",
	L"*\\BaseNamedObjects*\\WininetStartupMutex",
	L"*\\BaseNamedObjects*\\WininetConnectionMu",
	L"*\\BaseNamedObjects*\\WininetProxyRegistr",
	L"*\\BaseNamedObjects*\\RasPbFile",
	L"*\\BaseNamedObjects*\\CTF.*",
	L"*\\BaseNamedObjects*\\MSCTF.*",
	L"*\\BaseNamedObjects*\\MSUIM.*",
	L"*\\BaseNamedObjects*\\CtfmonInstMutex*",
	L"*\\BaseNamedObjects*\\CiceroSharedMemDefa",
	L"*\\BaseNamedObjects*\\CicLoadWinStaWinSta",
	L"*\\BaseNamedObjects*\\DBWinMutex",
	L"*\\BaseNamedObjects*\\DBWIN_BUFFER",
	L"*\\BaseNamedObjects*\\DBWIN_BUFFER_READY",
	L"*\\BaseNamedObjects*\\DBWIN_DATA_READY",
	L"\\RPC Control\\AudioSrv",
	L"*\\BaseNamedObjects*\\mmGlobalPnpInfo",
	L"*\\BaseNamedObjects*\\Guard*mmGlobalPnpIn",
	L"*\\BaseNamedObjects*\\MidiMapper_modLongM",
	L"*\\BaseNamedObjects*\\MidiMapper_Configur",
	L"*\\BaseNamedObjects*\\SsiMidiDllCs",
	L"iexplore.exe"
	L"*\\BaseNamedObjects*\\StaccatoSynthCore11",
	L"*\\BaseNamedObjects*\\WDMAUD_Callbacks",
	L"*\\BaseNamedObjects*\\DirectSound*",
	L"*\\BaseNamedObjects*\\AMResourceMutex*",
	L"*\\BaseNamedObjects*\\AMResourceMapping*",
	L"*\\BaseNamedObjects*\\VideoRenderer",
	L"*\\BaseNamedObjects*\\VIDEOMEMORY",
	L"*\\BaseNamedObjects*\\mxrapi",
	L"*\\BaseNamedObjects*\\mixercallback",
	L"*\\BaseNamedObjects*\\hardwaremixercallba",
	L"*\\BaseNamedObjects*\\DINPUTWINMM",
	L"*\\BaseNamedObjects*\\DDrawDriverObjectLi",
	L"*\\BaseNamedObjects*\\__DDrawExclMode__",
	L"*\\BaseNamedObjects*\\__DDrawCheckExclMod",
	L"*\\BaseNamedObjects*\\DDrawWindowListMute",
	L"*\\BaseNamedObjects*\\DDrawCheckFullscree",
	L"*\\BaseNamedObjects*\\D3D9CheckFullscreen",
	L"\\RPC Control\\AudioClientRpc",
	L"\\UxSmsApiPort",
	L"*\\BaseNamedObjects*\\Dwm-*-ApiPort-*",
	L"*\\BaseNamedObjects*\\AudioEngineDuplicat",
	L"\\MmcssApiPort",
	L"\\RPC Control\\spoolss",
	L"\\NLAPublicPort",
	L"*\\BaseNamedObjects*\\WinSpl64To32Mutex",
	L"*\\BaseNamedObjects*\\EPSON-PrgMtr-*",
	L"*\\BaseNamedObjects*\\RouterPreInitEvent",
	L"\\RPC Control\\SbieSvcPort",
	L"\\RPC Control\\PBSvcPort",
	L"*\\BaseNamedObjects*\\Proteinbox_DeviceSet",
	L"*\\BaseNamedObjects*\\Proteinbox_DeviceIdL",
	L"*\\BaseNamedObjects*\\Proteinbox_StartMenu",
	L"*\\BaseNamedObjects*\\TermSrvReadyEvent"
	L"*\\BaseNamedObjects*\\ATITRAY_SMEM",
	L"*\\BaseNamedObjects*\\ATITRAY_OSDM",
	L"*\\BaseNamedObjects*\\NOD32*",
	L"*\\BaseNamedObjects*\\NODCOMM*",
	L"*\\BaseNamedObjects*\\SnagPriv*",
	L"*\\BaseNamedObjects*\\__AVG_FW_*__",
	L"*\\BaseNamedObjects*\\CE6383A0-EB13-428c-",
	L"*\\BaseNamedObjects*\\AMCreateListenSock*",
	L"*\\BaseNamedObjects*\\AMIPC_*",
	L"*\\BaseNamedObjects*\\CTF.*",
	L"*\\BaseNamedObjects*\\AVSDA_KERNELOBJECT_",
	L"*\\BaseNamedObjects*\\KERNELOBJECTNAME_*",
	L"*\\BaseNamedObjects*\\WEBGUARD_KERNEL_OBJ",
	L"*\\BaseNamedObjects*\\AVMAILC_ISPOP3ACTIV",
	L"*\\BaseNamedObjects*\\AVMAILC_KERNEL_OBJE",
	L"*\\BaseNamedObjects*\\/NETNANNY5/*",
	L"*\\BaseNamedObjects*\\devldr32",
	L"*\\BaseNamedObjects*\\nView Shared *",
	L"*\\BaseNamedObjects*\\ThreatfireApiHookIp",
	L"*\\BaseNamedObjects*\\00MemoryShareKeylog",
	L"*\\BaseNamedObjects*\\WacomNewFrontAppEve",
	L"*\\BaseNamedObjects*\\WacomTouchingAppNam",
	L"\\...\\*",
}; 


static const LPWSTR g_Array_RPC[] = {
	L"\\ConsoleEvent-0x*",
	L"*\\BaseNamedObjects*\\ConsoleEvent-0x*",
	L"\\RPC Control\\console-0x*-lpc-handle",
	L"\\RPC Control\\lsapolicylookup",
	L"\\RPC Control\\lsasspirpc",
	L"\\RPC Control\\LSARPC_ENDPOINT",
} ;


static const LPWSTR g_Array_Device_1[ ] = // 保存允许直接操作的字符串 (白名单)
{ 
	L"\\Device\\NamedPipe\\",
	L"\\Device\\NamedPipe\\ROUTER",
	L"\\Device\\NamedPipe\\ShimViewer",
	L"\\Device\\Afd",
	L"\\Device\\Afd\\Endpoint",
	L"\\Device\\Afd\\AsyncConnectHlp",
	L"\\Device\\Afd\\AsyncSelectHlp",
	L"\\Device\\Afd\\ROUTER",
	L"\\Device\\WS2IFSL",
	L"\\Device\\WS2IFSL\\NifsPvd",
	L"\\Device\\WS2IFSL\\NifsSct",
	L"\\Device\\Tcp",
	L"\\Device\\Tcp6",
	L"\\Device\\Ip",
	L"\\Device\\Ip6",
	L"\\Device\\Udp",
	L"\\Device\\Udp6",
	L"\\Device\\RawIp",
	L"\\Device\\RawIp6",
	L"\\Device\\NetBT_Tcpip_*",
	L"\\Device\\NamedPipe\\spoolss",
	L"\\Device\\NamedPipe\\spooler*", 
	L"%DefaultSpoolDirectory%\\*", // "\device\harddiskvolume1\windows\system32\spool\printers\*"
	L"\\Device\\NamedPipe\\*_doPDF*",
	L"\\Device\\NamedPipe\\AudioSrv",
	L"\\Device\\NamedPipe\\Adobe LM Service*",
	L"\\Device\\NamedPipe\\SnagPriv.*",
	L"\\Device\\NamedPipe\\XTIERRPCPIPE",
} ;


static const LPWSTR g_Array_Device_2[ ] = // 保存允许直接操作的字符串 (白名单)
{ 
	L"\\Device\\LanmanRedirector",
	L"\\Device\\Mup",
} ;


static const LPWSTR g_Array_Wnd[] = // 保存白名单,即这些特定的沙盘外窗口可以被沙盘内程序访问
{ 
	L"Shell_TrayWnd",
	L"TrayNotifyWnd",
	L"SystemTray_Main",
	L"Connections Tray",
	L"MS_WebcheckMonitor",
	L"PrintTray_Notify_WndClass",
	L"CicLoaderWndClass",
	L"CicMarshalWndClass",
	L"Logitech Wingman Internal Message Router",
	L"devldr",
	L"CTouchPadSynchronizer",
	L"Type32_Main_Window",
	L"TForm_AshampooFirewall",
	L"WinVNC desktop sink",
	L"Afx:400000:0",
	L"NVIDIA TwinView Window",
} ;



//////////////////////////////////////////////////////////////////////////

BOOL
BuildGrayList_for_IopParse (
	IN PVOID ProcessNode
	)
{
	ULONG Index = 0 ;
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPPDNODE pNode = (LPPDNODE) ProcessNode ;
	LPCDNODEK pKey = NULL ;
	PERESOURCE QueueLockList = NULL ;
	PLIST_ENTRY pListHead = NULL ;

	// 1. 校验参数合法性
	if ( NULL == pNode || NULL == (QueueLockList = pNode->XFilePath.pResource) )
	{
		dprintf( "error! | BuildGrayList_for_IopParse(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( pNode->XFilePath.bFlagInited ) { return TRUE ; }

	// 2. 填充 pNode->XFilePath.OpenFilePathListHead
	pListHead = &pNode->XFilePath.OpenFilePathListHead ;
	
	// 2.1 OpenPipePath
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"OpenPipePath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_IopParse() - GLCopyNode(); | Key节点\"OpenPipePath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 2.2 OpenFilePath
	if ( 0 == pNode->bIsAVDefaultBox )
	{
		pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"OpenFilePath" );
		if ( pKey )
		{
			bRet = GLCopyNode( QueueLockList, pListHead, pKey );
			if ( FALSE == bRet ) { 
				dprintf( "Warning! | BuildGrayList_for_IopParse() - GLCopyNode(); | Key节点\"OpenFilePath\"对应的Value链表信息,拷贝失败 \n" );
			}
		}
	}

	// 2.3 OpenProtectedStorage
	bRet = GetConfigurationA( "OpenProtectedStorage" );
	if ( bRet )
	{
		WCHAR lpPS[ MAX_PATH ] = L"\\Device\\NamedPipe\\protected_storage" ;
		ULONG lpPSLength = ( wcslen(lpPS) + 1 ) * sizeof(WCHAR) ;

		GLBuildNode( QueueLockList, pListHead, lpPS, lpPSLength );
	} 

	// 2.4 g_Array_Device_1 数组
	for ( Index = 0; Index < ARRAYSIZEOF(g_Array_Device_1); Index++ )
	{
		GLBuildNode( QueueLockList, pListHead, g_Array_Device_1[ Index ], ( wcslen(g_Array_Device_1[ Index ]) + 1 ) * sizeof(WCHAR) );
	}

	// 3. 填充 pNode->XFilePath.OpenFilePathListHead
	pListHead = &pNode->XFilePath.ClosedFilePathListHead ;

	// 3.1 ClosedFilePath
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ClosedFilePath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_IopParse() - GLCopyNode(); | Key节点\"ClosedFilePath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 3.2 ClosedFilePath
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ClosedFilePath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_IopParse() - GLCopyNode(); | Key节点\"ClosedFilePath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 3.3 g_Array_Device_2数组
	for ( Index = 0; Index < ARRAYSIZEOF(g_Array_Device_2); Index++ )
	{
		GLBuildNode( QueueLockList, pListHead, g_Array_Device_2[ Index ], ( wcslen(g_Array_Device_2[ Index ]) + 1 ) * sizeof(WCHAR) );
	}

	pNode->XFilePath.bFlag_NotifyInternetAccessDenied = (BYTE) GetConfigurationA( "NotifyInternetAccessDenied" );

	// 4. 填充 pNode->XFilePath.OpenFilePathListHead
	pListHead = &pNode->XFilePath.ReadFilePathListHead ;

	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ReadFilePath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_IopParse() - GLCopyNode(); | Key节点\"ReadFilePath\"对应的Value链表信息,拷贝失败 \n" );
		}

		pListHead = &pNode->XFilePath.OpenFilePathListHead ;
		GLCopyNode( QueueLockList, pListHead, pKey );
	}

	pNode->XFilePath.bFlagInited = TRUE ;
	return TRUE ;
}



BOOL
BuildGrayList_for_OpenProcedure (
	IN PVOID ProcessNode
	)
{
	ULONG Index = 0 ;
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPPDNODE pNode = (LPPDNODE) ProcessNode ;
	LPCDNODEK pKey = NULL ;
	PERESOURCE QueueLockList = NULL ;
	PLIST_ENTRY pListHead = NULL ;
	PVOID pResult = NULL ;
	WCHAR lpTmp[ MAX_PATH ] = L"" ;
	ULONG lpTmpLength = 0 ;

	// 1. 校验参数合法性
	if ( NULL == pNode || NULL == (QueueLockList = pNode->XIpcPath.pResource) )
	{
		dprintf( "error! | BuildGrayList_for_OpenProcedure(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( pNode->XIpcPath.bFlagInited ) { return TRUE ; }

	//
	// 2. 填充 pNode->XIpcPath.OpenIpcPathListHead
	// 

	pListHead = &pNode->XIpcPath.OpenIpcPathListHead ;

	// 2.1 OpenIpcPath
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"OpenIpcPath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_OpenProcedure() - GLCopyNode(); | Key节点\"OpenIpcPath\"对应的Value链表信息,拷贝失败 \n" );
		}

		bRet = GetConfigurationA( "OpenProtectedStorage" );
		if ( bRet )
		{
			RtlZeroMemory( lpTmp, MAX_PATH );
			wcscpy( lpTmp, L"*\\BaseNamedObjects*\\PS_SERVICE_STARTED" );
			lpTmpLength = ( wcslen(lpTmp) + 1 ) * sizeof(WCHAR) ;

			GLBuildNode( QueueLockList, pListHead, lpTmp, lpTmpLength );

			RtlZeroMemory( lpTmp, MAX_PATH );
			wcscpy( lpTmp, L"\\RPC Control\\protected_storage" );
			lpTmpLength = ( wcslen(lpTmp) + 1 ) * sizeof(WCHAR) ;

			GLBuildNode( QueueLockList, pListHead, lpTmp, lpTmpLength );
		} 

		if ( IsKnownSID(pNode) )
		{
			RtlZeroMemory( lpTmp, MAX_PATH );
			wcscpy( lpTmp, L"\\RPC Control\\epmapper" );
			lpTmpLength = ( wcslen(lpTmp) + 1 ) * sizeof(WCHAR) ;

			GLBuildNode( QueueLockList, pListHead, lpTmp, lpTmpLength );

			RtlZeroMemory( lpTmp, MAX_PATH );
			wcscpy( lpTmp, L"\\RPC Control\\OLE*" );
			lpTmpLength = ( wcslen(lpTmp) + 1 ) * sizeof(WCHAR) ;

			GLBuildNode( QueueLockList, pListHead, lpTmp, lpTmpLength );

			pNode->bReserved1 = 1;
		}
	}

	// 2.2 g_Array_BaseNamedObjects 数组
	for ( Index = 0; Index < ARRAYSIZEOF(g_Array_BaseNamedObjects); Index++ )
	{
		GLBuildNode( QueueLockList, pListHead, g_Array_BaseNamedObjects[ Index ], ( wcslen(g_Array_BaseNamedObjects[ Index ]) + 1 ) * sizeof(WCHAR) );
	}

	// 2.3 g_Array_RPC 数组
	if ( g_Version_Info.IS___win7 )
	{
		for ( Index = 0; Index < ARRAYSIZEOF(g_Array_RPC); Index++ )
		{
			GLBuildNode( QueueLockList, pListHead, g_Array_RPC[ Index ], ( wcslen(g_Array_RPC[ Index ]) + 1 ) * sizeof(WCHAR) );
		}
	}

	//
	// 3. 填充 pNode->XIpcPath.ClosedIpcPathListHead
	//

	pListHead = &pNode->XIpcPath.ClosedIpcPathListHead ;

	// 3.1 ClosedFilePath
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ClosedIpcPath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_OpenProcedure() - GLCopyNode(); | Key节点\"ClosedIpcPath\"对应的Value链表信息,拷贝失败 \n" );
		}

		pNode->XIpcPath.bFlag_NotifyStartRunAccessDenied = (BYTE) GetConfigurationA( "NotifyStartRunAccessDenied" );
	}

	pNode->XIpcPath.bFlagInited = TRUE ;
	return TRUE ;
}



BOOL
BuildGrayList_for_Wnd (
	IN PVOID ProcessNode
	)
{
	ULONG Index = 0 ;
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPCDNODEK pKey = NULL ;
	LPPDNODE pNode = (LPPDNODE) ProcessNode ;
	PERESOURCE QueueLockList = NULL ;
	PLIST_ENTRY pListHead = NULL ;

	// 1. 校验参数合法性
	if ( NULL == pNode || NULL == (QueueLockList = pNode->XWnd.pResource) )
	{
		dprintf( "error! | BuildGrayList_for_Wnd(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( pNode->XWnd.bFlagInited ) { return TRUE ; }

	// 2.
	pNode->XWnd.bFlag_BlockFakeInput = (BYTE) GetConfigurationA( "BlockFakeInput" );
	pNode->XWnd.bFlag_BlockWinHooks	 = (BYTE) GetConfigurationA( "BlockWinHooks" );
	pNode->XWnd.bFlag_BlockSysParam	 = (BYTE) GetConfigurationA( "BlockSysParam" );

	// 3. 填充 pNode->XWnd.WndListHead
	pListHead = &pNode->XWnd.WndListHead ;

	// 3.1 OpenWinClass 是指可以设置特定的沙盘外窗口被沙盘内程序访问
	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"OpenWinClass" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_Wnd() - GLCopyNode(); | Key节点\"OpenWinClass\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 3.2 g_Array_Wnd 数组
	for ( Index = 0; Index < ARRAYSIZEOF(g_Array_Wnd); Index++ )
	{
		GLBuildNode( QueueLockList, pListHead, g_Array_Wnd[ Index ], ( wcslen(g_Array_Wnd[ Index ]) + 1 ) * sizeof(WCHAR) );
	}

	// 3.3 win7; 一个白名单窗口
	if ( g_Version_Info.IS___win7 )
	{
		WCHAR lpTmp[ MAX_PATH ] = L"" ;
		ULONG lpTmpLength = 0 ;

		wcscpy( lpTmp, L"Sandbox:*:ConsoleWindowClass" );
		lpTmpLength = ( wcslen(lpTmp) + 1 ) * sizeof(WCHAR) ;

		GLBuildNode( QueueLockList, pListHead, lpTmp, lpTmpLength );
	}

	pNode->XWnd.bFlagInited = TRUE ;
	return TRUE ;
}



BOOL
BuildGrayList_for_RegKey (
	IN PVOID ProcessNode
	)
{
	ULONG Index = 0 ;
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPCDNODEK pKey = NULL ;
	LPPDNODE pNode = (LPPDNODE) ProcessNode ;
	PERESOURCE QueueLockList = NULL ;
	PLIST_ENTRY pListHead = NULL ;

	// 1. 校验参数合法性
	if ( NULL == pNode || NULL == (QueueLockList = pNode->XRegKey.pResource) )
	{
		dprintf( "error! | BuildGrayList_for_Wnd(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( pNode->XRegKey.bFlagInited ) { return TRUE ; }

	// 2.1 填充 pNode->XRegKey.DirectListHead
	pListHead = &pNode->XRegKey.DirectListHead ;

	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"OpenKeyPath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_Wnd() - GLCopyNode(); | Key节点\"OpenKeyPath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 2.2 填充 pNode->XRegKey.DennyListHead
	pListHead = &pNode->XRegKey.DennyListHead ;

	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ClosedIpcPath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_Wnd() - GLCopyNode(); | Key节点\"ClosedIpcPath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	// 2.3 填充 pNode->XRegKey.ReadOnlyListHead
	pListHead = &pNode->XRegKey.ReadOnlyListHead ;

	pKey = (LPCDNODEK) kGetConfMulti( SeactionName, L"ReadKeyPath" );
	if ( pKey )
	{
		bRet = GLCopyNode( QueueLockList, pListHead, pKey );
		if ( FALSE == bRet ) { 
			dprintf( "Warning! | BuildGrayList_for_Wnd() - GLCopyNode(); | Key节点\"ReadKeyPath\"对应的Value链表信息,拷贝失败 \n" );
		}
	}

	pNode->XRegKey.bFlagInited = TRUE ;
	return TRUE ;
}



BOOL
IsKnownSID (
	IN PVOID ProcessNode
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG ReturnLength = 0x18 ;
	HANDLE			hProcess ;
	CLIENT_ID		ClientId ;
	OBJECT_ATTRIBUTES  ObjectAttributes ;
	PROCESS_BASIC_INFORMATION ProcessBasicInfo ;
	WCHAR UserSid[ MAX_PATH ] = L"" ;
	LPPDNODE pNode = (LPPDNODE) ProcessNode ;

	// 1. 校验参数合法性
	if ( NULL == pNode )
	{
		dprintf( "error! | IsKnownSID(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( pNode->bIsAVDefaultBox || _wcsicmp( pNode->lpProcessShortName, L"iexplore.exe" ) ) { return FALSE ; }

	status = ZwQueryInformationProcess (
		(HANDLE)0xFFFFFFFF,
		ProcessBasicInformation,
		&ProcessBasicInfo,
		sizeof(ProcessBasicInfo),
		&ReturnLength
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | IsKnownSID() - ZwQueryInformationProcess; | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	if ( kgetnodePD( ProcessBasicInfo.InheritedFromUniqueProcessId ) ) { return FALSE ; }

	InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, 0, 0 );

	ClientId.UniqueProcess = (HANDLE) ProcessBasicInfo.InheritedFromUniqueProcessId ;
	ClientId.UniqueThread  = 0 ;

	// 通过ID得到句柄
	status = ZwOpenProcess( &hProcess, PROCESS_QUERY_INFORMATION, &ObjectAttributes, &ClientId );
	if ( !NT_SUCCESS(status) ) 
	{
		dprintf( "error! | IsKnownSID() - ZwOpenProcess; | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	// 通过句柄得到SID
	status = RtlGetUserSid( hProcess, UserSid );
	ZwClose( hProcess );

	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | IsKnownSID() - RtlGetUserSid(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	// 验证SID
	if (	_wcsicmp( UserSid, L"S-1-5-18" )
		&&	_wcsicmp( UserSid, L"S-1-5-19" )
		&&	_wcsicmp( UserSid, L"S-1-5-20" )
		)
	{
		dprintf( "ko! | IsKnownSID(); | 当前SID不在已知列表中 (%ws) \n", UserSid );
		return FALSE ;
	}

	return TRUE ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////