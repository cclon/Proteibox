/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/26 [26:5:2010 - 17:16]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ProcessData.c
* 
* Description:
*      
*   每个"被沙箱"的进程都对应一个进程节点,该节点包含了该进程的所有信息.
*   该模块负责管理这些数据结构
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "RegHiveData.h"
#include "ProcessData.h"
#include "Config.h"
#include "Memory.h"
#include "StartProcess.h"
#include "Security.h"
#include "SecurityData.h"
#include "GrayList.h"
#include "Common.h"
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

LPPROCESS_NODE_INFO_HEAND g_ListHead__ProcessData = NULL ;
static BOOL g_ProcessDataManager_Inited_ok = FALSE ;


// 蛋白盒的路径,前期硬编码,后期动态获取
static WCHAR g_ProteinBox_path[ MAX_PATH ] = L"\\Device\\HarddiskVolume1\\Program Files\\Proteinboxie" ;
#define  g_ProteinBox_path_length sizeof( g_ProteinBox_path )

// 蛋白合重定向目录的提示信息相关变量
LPSTR g_Context_InfoTip = NULL ;
#define g_InfoTip_DESKTOP_icon "\\??\\C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE"


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
InitProcessData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ProcessDataManager_Inited_ok )
	{
		bRet = PDCreateTotalHead( (PVOID) &g_ListHead__ProcessData );
		g_ProcessDataManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitProcessData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
PDCreateTotalHead(
	OUT PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  创建总结构体,初始化之    
    
Arguments:
  _TotalHead - 要初始化的总结构体指针的地址
    
--*/
{
	BOOL bRet = FALSE ;
	LPPDHEAD *pTotalHead = (LPPDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPPDHEAD) kmalloc( sizeof( PDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | PDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( PDHEAD ) );

	// 初始化资源锁
	bRet = InitResource( &((LPPDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPPDHEAD)*pTotalHead )->ListHead );

	((LPPDHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
PDDeleteTotalHead(
	IN PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  删除总结构体    

--*/
{
	if ( NULL ==  *_TotalHead ) { return  ; }

	kfree( *_TotalHead );
	*_TotalHead = NULL ;

	return  ;
}



BOOL 
PDAllocateNode(
	OUT PVOID* _pCurrenList_
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  分配 & 初始化一个链表节点  
    
Arguments:
  _pCurrenList_ - 保存分配好的链表Node的指针

--*/
{
	LPPDNODE* pCurrenList = (LPPDNODE*) _pCurrenList_ ;
	*pCurrenList = (LPPDNODE) kmalloc( sizeof( PDNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | PDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( PDNODE ) );

	return TRUE ;
}



PVOID
PDBuildNode (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN BOOL bInsert
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  新建&填充PD结构体
    
Arguments:
  _TotalHead - 链表头
  PID - 进程ID
  bInsert - 是否插入到链表中; 一般创建完后该节点后,还有后续的填充其中的Node_c操作,故还是最后一切都填充完毕后再插入链表尾较为合适!

Return Value:
  新建的PD结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPPDNODE pNode = NULL ;
	LPPDHEAD pTotalHead	 = (LPPDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _TotalHead || 0 == PID )
	{
		dprintf( "error! | PDBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 为新节点分配内存
	//

	bRet = PDAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNode() - PDAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	//
	// 3. 填充新节点
	//

	status = PDBuildNodeEx( pNode, PID );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | PDBuildNode() - MPBuildNodeEx() | status = 0x%08lx \n", status );
		goto _error_ ;
	}

	//
	// 4. 插入新节点到链表尾
	//

	if ( TRUE == bInsert )
	{
		kInsertTailPD( pNode );
	}

	return (PVOID) pNode ;

_error_ :
	kfree( pNode );
	return NULL ;
}



NTSTATUS
PDBuildNodeEx (
	OUT PVOID _pNode,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 15:33]

Routine Description:
  构造进程总结点的主力军    
    
Arguments:
  pNode - 待填充的节点
  PID - 进程ID
    
--*/
{
	BOOL bRet = FALSE ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | PDBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. 查询配置信息
	//

	pNode->PID = PID ;
	pNode->FileTraceFlag	= GetConfigurationA( "FileTrace" );
	pNode->PipeTraceFlag	= GetConfigurationA( "PipeTrace" );
	pNode->KeyTraceFlag		= GetConfigurationA( "KeyTrace"	);
	pNode->GuiTraceFlag		= GetConfigurationA( "GuiTrace"	);

	pNode->XImageNotifyDLL.IncrementCounts = 0 ;

	//
	// 3. 初始化各种链表头,资源等 
	//

	// 3.1 初始化File
	InitializeListHead( &pNode->XFilePath.ClosedFilePathListHead );
	InitializeListHead( &pNode->XFilePath.OpenFilePathListHead );
	InitializeListHead( &pNode->XFilePath.ReadFilePathListHead );
	bRet = InitResource( &pNode->XFilePath.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XFilePath.pResource ); 申请资源锁内存失败! \n" );
		goto _error_ ;
	}

	// 3.2 初始化IPC
	InitializeListHead( &pNode->XIpcPath.OpenIpcPathListHead );
	InitializeListHead( &pNode->XIpcPath.ClosedIpcPathListHead );
	bRet = InitResource( &pNode->XIpcPath.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XIpcPath.pResource ); 申请资源锁内存失败! \n" );
		goto _error_ ;
	}

	// 3.3 初始化Wnd
	InitializeListHead( &pNode->XWnd.WndListHead );
	bRet = InitResource( &pNode->XWnd.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XWnd.pResource ); 申请资源锁内存失败! \n" );
		goto _error_ ;
	}

	// 3.4 初始化Reg
	InitializeListHead( &pNode->XRegKey.DennyListHead );
	InitializeListHead( &pNode->XRegKey.DirectListHead );
	InitializeListHead( &pNode->XRegKey.ReadOnlyListHead );
	bRet = InitResource( &pNode->XRegKey.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XRegKey.pResource ); 申请资源锁内存失败! \n" );
		goto _error_ ;
	}

	pNode->XWnd.TranshipmentStation = pNode->XWnd.VictimClassName = NULL ;
	return STATUS_SUCCESS ;

_error_ :
	pNode->bDiscard = 1 ;
	return STATUS_UNSUCCESSFUL ;
}



PVOID
PDBuildNodeForce (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN LPWSTR szFullImageName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/10 [10:6:2010 - 15:36]

Routine Description:
  查询配置文件,得到"ForceFolder","ForceProcess"对应的内容. 用@szFullImageName匹配,若在其中,
  表明要强制其运行在沙箱中,则注入*.dll到当前进程中
    
Arguments:
  _TotalHead - 链表头
  PID - 进程ID
  szFullImageName - 模块回调的参数1，模块全路径

Return Value:
  新建的PD结构体指针 | NULL

--*/
{
	BOOL bRet = FALSE, bRunInSandbox = TRUE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	WCHAR ForcePRocessPath[ MAX_PATH ] = L"" ;
	WCHAR ForceFolderPath[ MAX_PATH ] = L"" ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	PNODE_INFO_C pNode_c = NULL ;
	WCHAR UserSid[ MAX_PATH ] = L"" ;
	IOCTL_STARTPROCESS_BUFFER Buffer = { NULL, UserSid, NULL, NULL } ;
	LPPDNODE pNode = NULL ;
	LPPDHEAD pTotalHead	 = (LPPDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _TotalHead || 0 == PID || NULL == szFullImageName )
	{
		dprintf( "error! | PDBuildNodeForce() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 确保当前用户的合法性
	//

	status = RtlGetUserSid( (HANDLE)0xFFFFFFFF, UserSid );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | PDBuildNodeForce() - RtlGetUserSid(); | (status=0x%08lx) \n", status );
		return NULL ;
	}

	if (	0 == _wcsicmp( UserSid, L"S-1-5-18" )
		||	0 == _wcsicmp( UserSid, L"S-1-5-19" )
		||	0 == _wcsicmp( UserSid, L"S-1-5-20" )
		)
	{
		dprintf( "error! | PDBuildNodeForce() - _wcsicmp(); | (UserSid=%ws) \n", UserSid );
		return NULL ;
	}

	//
	// 3. 查询配置文件,得到"ForceFolder","ForceProcess"对应的内容
	//

	bRet = FPLGetState( szFullImageName, &bRunInSandbox );
	if ( FALSE == bRet || FALSE == bRunInSandbox )
	{
	//	dprintf( "error! | PDBuildNodeForce() | 未匹配到强制运行的文件路径,当前模块将不会被SB \n" );
		return NULL ;
	}

	//
	// 4.1 OK! 需强制运行于沙箱中,创建对应的进程节点
	//

	pNode = (LPPDNODE) kbuildnodePD( PID, FALSE );
	if ( NULL == pNode )
	{
		dprintf( "error! | PDBuildNodeForce() - kbuildnodePD(); | 创建进程节点失败 PID=%d \n", PID );
		return NULL ;
	}

	//
	// 4.2 填充C节点
	//

	pNode->pNode_C = pNode_c = PDBuildNodeC( (PVOID)&Buffer );
	if ( NULL == pNode_c )
	{
		dprintf( "error! | PDBuildNodeForce() - PDBuildNodeC(); | 创建进程节点C失败 PID=%d \n", PID );
		kfree( pNode ); // 还未插入链表,初始化失败则释放内存
		return NULL ;
	}

	//
	// 5. 插入新节点到链表尾
	//

	kInsertTailPD( pNode );
	return (PVOID)pNode ;
}



PVOID
PDCopyNodeC (
	IN PVOID Buffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 17:51]

Routine Description:
  复制父进程的节点C给当前子节点    
    
Arguments:
  Buffer - 父进程的节点C指针

Return Value:
  新复制的一份子节点C指针
    
--*/
{
	PNODE_INFO_C pNode_c = NULL ;
	PNODE_INFO_C pNode_c_old = (PNODE_INFO_C) Buffer ;

	// 1. 校验参数合法性
	if ( NULL == Buffer )
	{
		dprintf( "error! | PDCopyNodeC() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 2. 申请C节点内存
	pNode_c = (PNODE_INFO_C) kmalloc( sizeof( NODE_INFO_C ) );
	if ( NULL == pNode_c )
	{
		dprintf( "error! |  PDCopyNodeC() - kmalloc( sizeof( NODE_INFO_C ); | 分配节点C的内存失败! \n" );
		return NULL ;
	}

	// 3. 拷贝之
	memcpy( pNode_c, pNode_c_old, sizeof( NODE_INFO_C ) );

	// 4 操作链表 g_ListHead_RegistryUserSID
	if ( NULL == kbuildnodeSD( pNode_c->RegisterUserID ) )
	{
		dprintf( "error! | PDCopyNodeC() - kbuildnodeSD(); | 建立SID节点失败; RegisterUserID=\"%ws\" \n", pNode_c->RegisterUserID );
		return NULL ;
	}

	return (PVOID) pNode_c ;
}



PVOID
PDBuildNodeC (
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 14:28]

Routine Description:
  创建 & 填充进程总节点中的小节点: C
  结构体NodeC指针
typedef struct _NODE_INFO_C_ {
	WCHAR wszName1[0x40] ; // eg:"DefaultBox"
	ULONG Name1Length ;
	PVOID wszRegisterUserID ; // 调用ZwQueryInformationProcess()传递参数ProcessSessionInformation得到的SID
                                      //eg:"S-1-5-21-583907252-261903793-1177238915-1003"
	ULONG nRegisterUserID_Length ; 
	ULONG SessionId ;
	PVOID pNode_D ;  // 指向一个0x10大小的结构体
	PVOID wszFilePath_BoxRootFolder ;  // eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	ULONG FilePath_Length ;
	PVOID wszRegPath_KeyRootPath ; // eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
	ULONG RegPath_Length ;
	PVOID wszLpcRootPath1 ;   // eg:"\Sandbox\AV\DefaultBox\Session_0"
	ULONG LpcRootPath1Length ;
	PVOID wszLpcRootPath2 ;   // eg:"_Sandbox_AV_DefaultBox_Session_0"
	ULONG LpcRootPath2Length ;
} NODE_INFO_C, *PNODE_INFO_C ;
    
Arguments:
  pInBuffer - LPIOCTL_STARTPROCESS_BUFFER结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	WCHAR BoxName[ MAX_PATH ] = L"DefaultBox" ;
	WCHAR RegisterUserID[ MAX_PATH ] = L"" ;
	PNODE_INFO_C pNode_c = NULL ;
	LPIOCTL_STARTPROCESS_BUFFER Buffer = (LPIOCTL_STARTPROCESS_BUFFER) pInBuffer ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == Buffer )
	{
		dprintf( "error! |  | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. 申请C节点内存
	//

	pNode_c = (PNODE_INFO_C) kmalloc( sizeof( NODE_INFO_C ) );
	if ( NULL == pNode_c )
	{
		dprintf( "error! |  PDBuildNodeC() - kmalloc( sizeof( NODE_INFO_C ); | 分配节点C的内存失败! \n" );
		return NULL ;
	}

	//
	// 3.1 获取各种变量数据
	//

	// 获取沙箱名
	if ( Buffer->wszBoxName )
	{			
		RtlCopyMemory( BoxName, Buffer->wszBoxName, 0x40 );

		if ( FALSE == Check_IsValid_Characters( BoxName ) )
		{
			dprintf( "error! | PDBuildNodeC() - Check_IsValid_Characters(); | R3内存块中当前沙箱名为畸形字符串,则用默认的名字! \n" );
			RtlZeroMemory( BoxName, 0x40 );
			wcscpy( BoxName, L"DefaultBox" );
		}
	}

	// 获取用户SID
	if ( Buffer->wszRegisterUserID )
	{			
		RtlCopyMemory( RegisterUserID, Buffer->wszRegisterUserID, 0x5E );
	}

	if ( *RegisterUserID != 'S' && RegisterUserID[1] != '-' )
	{
		// R3内存块中的SID非法,直接在驱动中获取之
		status = RtlGetUserSid( NtCurrentProcess(), RegisterUserID );
		if ( ! NT_SUCCESS(status) ) 
		{
			dprintf( "error! | PDBuildNodeC() - RtlGetUserSid() | status = 0x%08lx \n", status );
			goto _error_ ;
		}
	}

	//
	// 3.2 填充其中的成员
	//

	wcscpy( pNode_c->BoxName, BoxName );
	pNode_c->BoxNameLength = (wcslen(pNode_c->BoxName) + 1) * sizeof(WCHAR) ;

	wcscpy( pNode_c->RegisterUserID, RegisterUserID );
	pNode_c->RegisterUserIDLength = (wcslen(pNode_c->RegisterUserID) + 1) * sizeof(WCHAR) ;

	bRet = ProcessIdToSessionId( 0, &pNode_c->SessionId );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeC() - ProcessIdToSessionId(); |  \n" );
		goto _error_ ;
	}

	//
	// 4 操作链表 g_ListHead_RegistryUserSID
	//

	if ( NULL == kbuildnodeSD( RegisterUserID ) )
	{
		dprintf( "error! | PDBuildNodeC() - kbuildnodeSD(); | 建立SID节点失败; RegisterUserID=\"%ws\" \n", RegisterUserID );
		goto _error_ ;
	}

	//
	// 5. 填充其他成员,需要根据配置文件获取相关信息 
	//

	bRet = __PDBuildNodeC( (PVOID)pNode_c );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeC() - __PDBuildNodeC(); | \n" );
		goto _error_ ;
	}

	return (PVOID)pNode_c ;

_error_ :
	kfree( pNode_c );
	return NULL ;
}



BOOL
__PDBuildNodeC (
	OUT PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  填充PD结构体的其他成员,需要根据配置文件获取相关信息    

--*/
{
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	WCHAR FileRootPath[ MAX_PATH ] = L"" ;
	WCHAR KeyRootPath[ MAX_PATH ] = L"" ;
	WCHAR IpcRootPath[ MAX_PATH ] = L"" ;
	LPWSTR ptr = NULL ;
	PNODE_INFO_C pNode = (PNODE_INFO_C) _pNode ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pNode )
	{
		dprintf( "error! | __PDBuildNodeC(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.1 填充 FileRootPath 对应路径; "\Device\HarddiskVolume1\ProteinBox\SUDAMI\DefaultBox"
	bRet = kGetConfSingle( SeactionName, L"FileRootPath", FileRootPath );
	if ( FALSE == bRet )
	{
		bRet = kGetConfSingle( SeactionName, L"BoxRootFolder", FileRootPath );
		if ( TRUE == bRet )
		{
			wcscat( FileRootPath, L"\\SUDAMI\\" );
			wcscat( FileRootPath, pNode->BoxName );
		}
		else
		{
		//	wcscpy( FileRootPath, g_default_FileRootPath );
			return FALSE ;
		}
	}

	ptr = ParseCharacters( (PVOID)FileRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | 解析字符串失败:\"%ws\" \n", FileRootPath );
		return FALSE ;
	}

	wcscpy( pNode->FileRootPath, ptr );
	pNode->FileRootPathLength = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.2 填充 KeyRootPath 对应路径; "\REGISTRY\USER\ProteinBox_SUDAMI_DefaultBox"
	bRet = kGetConfSingle( SeactionName, L"KeyRootPath", KeyRootPath );
	if ( FALSE == bRet ) { wcscpy( KeyRootPath, g_default_KeyRootPath ); }

	ptr = ParseCharacters( (PVOID)KeyRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | 解析字符串失败:\"%ws\" \n", KeyRootPath );
		return FALSE ;
	}

	wcscpy( pNode->KeyRootPath, ptr );
	pNode->KeyRootPathLength = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.3 填充 IpcRootPath1 对应路径; "\ProteinBox\SUDAMI\DefaultBox\Session_0"
	bRet = kGetConfSingle( SeactionName, L"IpcRootPath", IpcRootPath );
	if ( FALSE == bRet ) { wcscpy( IpcRootPath, g_default_IpcRootPath ); }

	ptr = ParseCharacters( (PVOID)IpcRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | 解析字符串失败:\"%ws\" \n", IpcRootPath );
		return FALSE ;
	}

	wcscpy( pNode->LpcRootPath1, ptr );
	pNode->LpcRootPath1Length = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.4 填充 IpcRootPath2 对应路径; "_Sandbox_AV_DefaultBox_Session_0"
	{
		LPWSTR pBuffer = (LPWSTR) pNode->LpcRootPath2 ;
		LPWSTR p = NULL ;

		wcscpy( pNode->LpcRootPath2, ptr );

		while ( *pBuffer )
		{
			p = wcschr( pBuffer, '\\' );
			if ( p )
			{
				pBuffer = p ;
				*p = '_';
			}
			else
			{
				pBuffer += wcslen( pBuffer );
			}
		}
	}

	return TRUE ;
}



BOOL 
PDFixupNode(
	OUT PVOID _pNode,
	IN LPCWSTR wszName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 17:59]

Routine Description:
  修正和进程节点相关的数据    
    
Arguments:
  _pNode - 待修正的进程节点

--*/
{
	BOOL bRet = FALSE ;
	ULONG length = 0 ;
	UNICODE_STRING uniBuffer ;
	WCHAR Temp[ MAX_PATH ] = L"unknown executable image" ;
	LPWSTR pBuffer, ptr = NULL ;
	LPWSTR lpImageShortName = (LPWSTR) wszName ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | PDFixupNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 根据进程全路径,填充节点中的标记位,即是否为自身目录项的程序 or 是否为重定向目录下的程序
	//

	if ( NULL == wszName ) 
	{
		lpImageShortName = Temp ;
	}
	else 
	{
		ptr = wcsrchr(wszName, '\\');
		if ( ptr )
		{
			lpImageShortName = ptr + 1;

			if ( 0 == wcsncmp( wszName, g_ProteinBox_path, g_ProteinBox_path_length ) )
			{
				pNode->bSelfExe = 1 ; // 表明是自己目录下的程序
			}

			if ( 0 == wcsncmp( wszName, pNode->pNode_C->FileRootPath, pNode->pNode_C->FileRootPathLength ) )
			{
				pNode->bIsAVDefaultBox = 1 ; // 表明是重定向目录下的文件被启动
			}
		}
	}

	//  
	// 3. 申请内存,拷贝进程全路径,填充至进程节点中
	//

	length = (wcslen (lpImageShortName) + 1) * sizeof(WCHAR) ;
	pBuffer = (LPWSTR) kmalloc( length );
	if ( NULL == pBuffer )
	{
		dprintf( "error! | PDFixupNode() - kmallocMM(); NULL == pBuffer \n" );
		pNode->bDiscard = 1 ; // 表明要销毁当前结点
		return FALSE ;
	}

	RtlCopyMemory( pBuffer, lpImageShortName, length );

	pNode->lpProcessShortName	= pBuffer ;
	pNode->ImageNameLength		= length  ;

	//  
	// 4. 降权等操作
	// 

	if (   FALSE == DropAdminRights( _pNode )
		|| FALSE == CreateRootBox( _pNode )
		|| FALSE == CreateSessionDirectoryObject( _pNode )
		|| FALSE == HandlerRegHive( _pNode )
		)
	{
		pNode->bDiscard = 1 ; // 表明要销毁当前结点
		return FALSE ;
	}

	return TRUE ;
}



NTSTATUS
PDEnumProcessEx(
	IN LPWSTR szBoxName,
	IN BOOL bFlag,
	OUT int *pArrays
	)
{
	ULONG pAddress = 0, SessionId = 0, nIndex = 0 ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) g_ListHead__ProcessData ;

	// 1. 校验参数合法性
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList )
	{
		return STATUS_INVALID_PARAMETER ;
	}

	if ( bFlag ) { GetSessionId( &SessionId ); }
	 
	// 2. 遍历链表,查找指定节点
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( pCurrentNode->pNode_C )
		{
			if (   (*szBoxName && 0 == _wcsicmp(szBoxName, pCurrentNode->pNode_C->BoxName))
				|| (SessionId && SessionId == pCurrentNode->pNode_C->SessionId)
				)
			{
				++ nIndex;
				if ( nIndex == 0x200 ) { break; }

				pArrays[ nIndex ] = pCurrentNode->PID ;
			}
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	*pArrays = nIndex;

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return STATUS_SUCCESS;
}



PVOID
PDFindNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  在链表中查找指定内存地址,返回之
    
Arguments:
  _TotalHead - 总的链表头
  PID - 待匹配的进程ID, 为0 表明要查找当前PID,通过PsGetCurrentProcessId获取

Return Value:
  已找到的进程节点
    
--*/
{
	KIRQL OldIrql ;
	ULONG pAddress	 = 0 ;
	PVOID pResult = NULL ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || (-1 == PID) )
	{
	//	dprintf( "error! | PDFindNode() | Invalid Parameters \n" );
		return NULL ;
	}

	if ( 0 == PID )
	{
		// 表明要查找当前PID,通过PsGetCurrentProcessId获取
		PID = (ULONG) PsGetCurrentProcessId();
		if ( KernelMode == ExGetPreviousMode() || PID <= 8 ) { return NULL ; }
	}

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
		return NULL ;
	}

	//
	// 2. 遍历链表,查找指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( PID == pCurrentNode->PID ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



VOID
PDConfResetAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/27 [27:12:2011 - 10:39]

Routine Description:
  重置所有沙箱进程节点中的配置文件相关的数据，仅在Reload Configuration时调用
    
--*/
{
	int nTotalCounts = 0 ;
	LPPDNODE pCurrentNode = NULL, pNodeHead = NULL;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	// 1. 参数检查
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDConfResetAll() | Invalid ListHead \n" );
		return ;
	}

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	nTotalCounts = pTotalHead->nTotalCounts;
	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		// 2.1 Reset Files
		if ( pCurrentNode->XFilePath.bFlagInited && pCurrentNode->XFilePath.pResource )
		{
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.OpenFilePathListHead );
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ClosedFilePathListHead );
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ReadFilePathListHead );

			pCurrentNode->XFilePath.bFlagInited = FALSE;
		}

		// 2.2 Reset Ipcs
		if ( pCurrentNode->XIpcPath.bFlagInited && pCurrentNode->XIpcPath.pResource )
		{
			GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.OpenIpcPathListHead );
			GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.ClosedIpcPathListHead );

			pCurrentNode->XIpcPath.bFlagInited = FALSE;
		}

		// 2.3 Reset Wnds
		if ( pCurrentNode->XWnd.bFlagInited && pCurrentNode->XWnd.pResource )
		{
			GLDistroyAll( pCurrentNode->XWnd.pResource, &pCurrentNode->XWnd.WndListHead );

			pCurrentNode->XWnd.bFlagInited = FALSE;
		}

		// 2.4 Reset Regs
		if ( pCurrentNode->XRegKey.bFlagInited && pCurrentNode->XRegKey.pResource )
		{
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DennyListHead );
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DirectListHead );
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.ReadOnlyListHead );

			pCurrentNode->XRegKey.bFlagInited = FALSE;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	if ( nTotalCounts ) { dprintf( "*** 所有沙箱进程节点(%d个)中的配置数据已重置 *** \n", nTotalCounts ); }
	return ;
}



VOID
PDClearNode (
	IN 	PVOID pNode
	)
{
	LPPDNODE pCurrentNode = (LPPDNODE)pNode ;

	// 1. 校验参数合法性
	if ( NULL == pCurrentNode ) { return ; }

	// 2. 清理当前节点的各种资源
	kfree( pCurrentNode->pNode_C );
	kfree( pCurrentNode->lpProcessShortName );

	// 2.1 Release Files
	if ( pCurrentNode->XFilePath.pResource )
	{
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.OpenFilePathListHead );
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ClosedFilePathListHead );
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ReadFilePathListHead );

		ExDeleteResource( pCurrentNode->XFilePath.pResource );
		kfree( pCurrentNode->XFilePath.pResource );
	}
	
	// 2.2 Release Ipcs
	if ( pCurrentNode->XIpcPath.pResource )
	{
		GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.OpenIpcPathListHead );
		GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.ClosedIpcPathListHead );

		ExDeleteResource( pCurrentNode->XIpcPath.pResource );
		kfree( pCurrentNode->XIpcPath.pResource );
	}

	// 2.3 Release Wnds
	if ( pCurrentNode->XWnd.pResource )
	{
		GLDistroyAll( pCurrentNode->XWnd.pResource, &pCurrentNode->XWnd.WndListHead );

		ExDeleteResource( pCurrentNode->XWnd.pResource );
		kfree( pCurrentNode->XWnd.pResource );
	}

	// 2.4 Release Regs
	if ( pCurrentNode->XRegKey.pResource )
	{
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DennyListHead );
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DirectListHead );
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.ReadOnlyListHead );

		ExDeleteResource( pCurrentNode->XRegKey.pResource );
		kfree( pCurrentNode->XRegKey.pResource );
	}

	kfree( pCurrentNode->XImageNotifyDLL.BeCoveredOldData ); // 该内存在Ioctl_GetInjectSaveArea()调用成功时会被释放; 一般情况下不会再次处理,但要防止内存泄露
	kfree( pCurrentNode->XWnd.VictimClassName );
	kfree( pCurrentNode );
	return ;
}



BOOL
PDDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 10:51]

Routine Description:
  当沙箱中的进程销毁时,同样要删除进程链表中对应的节点
    
Arguments:
  _TotalHead - 总的链表头
  PID - 待匹配的进程ID
 
--*/
{
	PVOID pResult = NULL ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	// 1. 校验参数合法性
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || ( 0 == PID) ) { return FALSE ; }

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
//		dprintf( "error! | PDDeleteNode(); | 链表为空,无法查找删除指定节点 PID=%d \n", PID );
		return FALSE ;
	}

	// 2. 遍历链表,查找指定节点
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( PID == pCurrentNode->PID ) 
		{
			RemoveEntryList( (PLIST_ENTRY) pCurrentNode ); // 从链表中移除
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	// 3. 清除当前节点的内存
	if ( NULL == pResult )
	{ 
	//	dprintf( "error! | PDDeleteNode(); | 未找到指定节点 PID=%d \n", PID );
		return FALSE ;
	}

	PDClearNode( (PVOID)pCurrentNode );
	--pTotalHead->nTotalCounts;
	
//	dprintf( "ok! | PDDeleteNode(); | 已移除当前进程节点(PID=%d) \n", PID );
	return TRUE ;
}



VOID
PDDistroyAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 11:08]

Routine Description:
  销毁整个进程链
    
--*/
{
	int i = 0 ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPPDNODE pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. 删除所有的子节点
	//

	PDWalkNodes( pTotalHead );

	dprintf( "*** 开始卸载进程节点管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPPDNODE) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx \n", i++, (PVOID)pCurrentNode );
		PDClearNode( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. 删除总节点
	//

	PDDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载进程节点管理器 *** \n" );
	return ;
}



VOID
PDCheckOut(
	IN PVOID _TotalHead,
	IN ULONG InvalidPID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 11:11]

Routine Description:
  该函数在进程结束notify里面被调用,作用是校验进程链中PID的合法性    
    
Arguments:
  InvalidPID - 对于进程链,除了该PID(因为其对应的进程已经结束),其他的都需要校验

--*/
{
	PVOID pBuffer = NULL, ptr = NULL ;
	PEPROCESS Eprocess = NULL ;
	ULONG i = 0, CurrentPID = 0, Length = 0x1000 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == _TotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDCheckOut(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	//
	// 2. 遍历链表,存储所有PID供后面验证用
	//

	pBuffer = kmalloc( Length );
	if ( NULL == pBuffer )
	{
		dprintf( "error! | PDCheckOut() - kmalloc(); | 分配内存失败, Length=%d \n", Length );
		return ;
	}

	RtlZeroMemory( pBuffer, Length );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		*((DWORD *)pBuffer + i) = pCurrentNode->PID ; // 挨个存储每个PID
		pCurrentNode = pCurrentNode->pBlink ;

		i ++ ;
		if ( i >= 0x3FE ) { break ; }
	}

	*((DWORD *)pBuffer + i) = 0 ;

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	//  
	// 3. 验证进程链表中PID的合法性
	//

	if ( 0 == *(DWORD*) pBuffer )
	{
	//	dprintf( "error! | PDCheckOut(); | 遍历进程链后,无任何PID,说明进程链是空的 \n" );
		goto _CLEAR_UP_ ;
	}

	ptr = pBuffer ;
	while ( TRUE )
	{
		CurrentPID = *(DWORD*) ptr ;
		ptr = (PCHAR)ptr + 4 ;

		if ( 0 == CurrentPID ) { break ; }
		if ( CurrentPID == InvalidPID ) { continue ; }

		status = PsLookupProcessByProcessId( (HANDLE)CurrentPID, &Eprocess );
		if ( ! NT_SUCCESS(status) )
		{
			kRemovePD( CurrentPID );    // 不合法则删除当前PID对应的节点
		}
		else
		{
			ObfDereferenceObject( (PVOID)Eprocess );  // 合法则继续校验下一个
		}
	}

_CLEAR_UP_ :
	kfree( pBuffer );
	return ;
}



VOID
PDWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+          "被沙箱"的进程创建初始化过程中的一些函数         +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
CreateRootBox (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/04 [4:6:2010 - 11:52]

Routine Description:
  创建"\Device\HarddiskVolume1\Sandbox\AV"目录 & "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"目录.
							   ------  --											 ----------
						   沙箱总目录  当前用户名                                    当前沙箱名 

  并在当前目录下创建2个文件:"desktop.ini","DONT-USE.TXT"
    
--*/
{
	ULONG n = 0 ;
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPWSTR ptrOld = NULL, ptrCurrent = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	IO_STATUS_BLOCK IoStatusBlock ;
	UNICODE_STRING uniBuffer ;
	LPWSTR FileRootPath = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 循环创建当前沙箱的目录
	//

	FileRootPath = pNode->pNode_C->FileRootPath ;
	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, g_SecurityDescriptor_new );

	do
	{
		++ n ;

		RtlInitUnicodeString( &uniBuffer, FileRootPath );
		status = ZwCreateFile (
			(PHANDLE) &hFile,
			FILE_GENERIC_READ | FILE_GENERIC_WRITE, // 0x120189, // 
			&ObjectAttributes,
			&IoStatusBlock,
			0,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_OPEN_IF,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
			0,
			0
			);

		if ( STATUS_OBJECT_PATH_NOT_FOUND == status/* || STATUS_OBJECT_PATH_SYNTAX_BAD == status*/ )
		{
			//
			// 逆行(从右到左) 依次截断,尝试打开目录,看是否存在
			// eg: "c:\Test\1\2\3" 是要建立的文件夹,但是系统当前只有"c:\Test"
			// 则依次尝试打开(不存在则新建) "c:\Test\1\2\3" --> "c:\Test\1\2" --> "c:\Test\1"
			// 当出现 "c:\Test\1" 时,会创建成功
			//

			ptrCurrent = wcsrchr( FileRootPath, '\\' );
			if ( ptrOld ) { *ptrOld = '\\' ; } // 恢复旧的被截断的路径

			ptrOld = ptrCurrent ; // 赋予旧的路径当前值
			if ( ptrCurrent ) { *ptrCurrent = 0 ; } // 截断当前路径

			continue ;
		}

		if( NT_SUCCESS( status ) )
		{
			if ( FILE_CREATED == IoStatusBlock.Information  )
			{
				// 表明该目录是新创建的,而非已经打开过的,所以要往里面写子文件
				CreateRootBoxEx( hFile );
			}

			ZwClose( hFile );
		}

		if ( NULL == ptrOld ) { break ; } // 此时的ptrOld 即为当前的ptrCurrent,若为空则跳出循环

		*ptrOld = '\\' ;
		ptrOld = NULL ;     

	} while ( n < 0x40 );

	if ( n >= 0x40 ) { return FALSE ; }
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( " \n" );
		return status ;
	}

	return TRUE ;
}



NTSTATUS
CreateRootBoxEx (
	IN HANDLE hDirectory
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/04 [4:6:2010 - 16:15]

Routine Description:
  在当前对应的目录下创建&填充2个文件"desktop.ini" 和 "README.TXT"

  // desktop.ini的内容如下
  [.ShellClassInfo]
  IconFile=C:\Program Files\Internet Explorer\IEXPLORE.EXE
  IconIndex=0
  IconResource=C:\Program Files\Internet Explorer\IEXPLORE.EXE,0
  InfoTip=This folder is a work area created by the program ProteinBox.  This folder might be 
  deleted at any time.  Use at your own risk.

  // README.TXT 即为上面InfoTip的内容
    
Arguments:
  hDirectory - 对应待操作的文件目录

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	HANDLE hFile = NULL ;
	FILE_BASIC_INFORMATION FileInformation ; 
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	UNICODE_STRING uniBuffer; 
	IO_STATUS_BLOCK IoStatusBlock ;
	CHAR README[ MAX_PATH ] = "folder is a work area created by the program ProteinBox.  This folder might be deleted at any time.  Use at your own risk." ;

	//
	// 1. 分配内存
	//

	if ( NULL == g_Context_InfoTip )
	{
		g_Context_InfoTip = (LPSTR) kmallocMM( 0x300, MTAG___Context_InfoTip );
		if ( NULL == g_Context_InfoTip )
		{
			dprintf( "error! | CreateRootBoxEx() - kmallocMM( 0x300, MTAG___Context_InfoTip ) | 分配内存失败 \n" );
			return status ;
		}
		
		sprintf(
			g_Context_InfoTip ,
			"[.ShellClassInfo]\r\nIconFile=%s\r\nIconIndex=0\r\nIconResource=%s,0\r\nInfoTip=%s",
			g_InfoTip_DESKTOP_icon + 4 ,
			g_InfoTip_DESKTOP_icon + 4 ,
			README
			);
	}

	//
	// 2. 设置当前文件夹的属性
	//

	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, hDirectory, g_SecurityDescriptor_new );

	status = ZwQueryInformationFile( 
		hDirectory,
		&IoStatusBlock, 
		&FileInformation, 
		sizeof(FILE_BASIC_INFORMATION), 
		FileBasicInformation
		);

	if( NT_SUCCESS( status ) )
	{
		FileInformation.FileAttributes = FileInformation.FileAttributes & 0xFFFFFF79 | 1 ;
		ZwSetInformationFile( hDirectory, &IoStatusBlock, &FileInformation, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	}

	//
	// 3.1 写入文件 desktop.ini
	//

	RtlInitUnicodeString( &uniBuffer, L"desktop.ini" );

	status = ZwCreateFile(
		&hFile,
		0x12019F, 
		&ObjectAttributes, 
		&IoStatusBlock, 
		NULL, 
		FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE ,
		NULL, 0
		);

	if( NT_SUCCESS( status ) )
	{
		if ( FILE_CREATED == IoStatusBlock.Information  )
		{
			ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, g_Context_InfoTip, strlen(g_Context_InfoTip), 0, 0 );
		}

		ZwClose( hFile );
	}
	
	//
	// 3.2 写入文件 README.TXT
	//

	RtlInitUnicodeString( &uniBuffer, L"README.TXT" );

	status = ZwCreateFile(
		&hFile, 
		0x12019F,
		&ObjectAttributes,
		&IoStatusBlock,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE ,
		NULL, 0
		);

	if( NT_SUCCESS( status ) )
	{
		if ( FILE_CREATED == IoStatusBlock.Information )
		{
			ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, README, strlen(README), 0, 0 );
		}

		status = ZwClose( hFile );
	}

	return status ;
}



BOOL
CreateSessionDirectoryObject (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/07 [7:6:2010 - 10:21]

Routine Description:
  创建"\\ProteinBox\\SUDAMI\\DefaultBox\\Session_0"对象  
    
--*/
{
	ULONG n = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPWSTR ptrOld = NULL, ptrCurrent = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	UNICODE_STRING uniBuffer ;
	SECURITY_DESCRIPTOR SecurityDescriptor;
	LPWSTR wszLpcRootPath1 = NULL ;
	HANDLE hDirectory  = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | CreateSessionDirectoryObject(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 循环创建当前沙箱的Session目录
	//

	wszLpcRootPath1 = pNode->pNode_C->LpcRootPath1 ;
	
	RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION1 );
	RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, 0, FALSE );

	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_PERMANENT, 0, &SecurityDescriptor );

	do
	{
		++ n ;

		RtlInitUnicodeString( &uniBuffer, wszLpcRootPath1 );
		
		status = ZwCreateDirectoryObject( &hDirectory, 0xF000F, &ObjectAttributes );
		if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
		{
			//
			// 逆行(从右到左) 依次截断,尝试打开目录,看是否存在
			// eg: "c:\Test\1\2\3" 是要建立的文件夹,但是系统当前只有"c:\Test"
			// 则依次尝试打开(不存在则新建) "c:\Test\1\2\3" --> "c:\Test\1\2" --> "c:\Test\1"
			// 当出现 "c:\Test\1" 时,会创建成功
			//

			ptrCurrent = wcsrchr( wszLpcRootPath1, '\\' );
			if ( ptrOld ) { *ptrOld = '\\' ; } // 恢复旧的被截断的路径

			ptrOld = ptrCurrent ; // 赋予旧的路径当前值
			if ( ptrCurrent ) { *ptrCurrent = 0 ; } // 截断当前路径

			continue ;
		}

		if( NT_SUCCESS( status ) ) { ZwClose( hDirectory ); }
		if ( STATUS_OBJECT_NAME_COLLISION == status ) { status = STATUS_SUCCESS ; }

		if ( NULL == ptrOld ) { break ; } // 此时的ptrOld 即为当前的ptrCurrent,若为空则跳出循环

		*ptrOld = '\\' ;
		ptrOld = NULL ;     

	} while ( n < 0x40 );

	if ( n >= 0x40 ) { return FALSE ; }
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( " \n" );
		return status ;
	}

	return TRUE ;
}



BOOL
HandlerRegHive (
	IN PVOID _pNode
	)
{
	LPRHNODE pCurrentNode = NULL ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 建立RegHive节点
	//

	pCurrentNode = kbuildnodeRH( _pNode );
	if ( NULL == pCurrentNode )
	{
		dprintf( "error! | HandlerRegHive() - kbuildnodeRH(); | 建立RegHive节点失败 \n" );
		return FALSE ;
	}

// 	dprintf( 
// 		"ok! | HandlerRegHive() | 新建立的RegHive节点信息如下:"
// 		"\nHiveRegPath:\"%ws\" \nHiveFilePath:\"%ws\" \n",
// 		pCurrentNode->HiveRegPath, pCurrentNode->HiveFilePath
// 		);

	return TRUE ;
}



BOOL 
Check_IsValid_Characters(
	IN LPWSTR pBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 14:49]

Routine Description:
  判断字符串的合法性,比如R3传进来默认的沙箱名,若是畸形字符串,则无法创建对应的文件夹  
  必须介于0~9 | a~z | A~Z 之间

Arguments:
  pBuffer - 待检测的字符串

Return Value:
  TRUE - 合法; FALSE - 非法

--*/
{
	ULONG n ;
	unsigned __int16 tmp ; 

	n = 0;
	do
	{
		tmp = pBuffer[n];
		if ( !tmp )
			break;

		if ( (tmp < '0' || tmp > '9') && (tmp < 'A' || tmp > 'Z') && (tmp < 'a' || tmp > 'z') && tmp != '_' )
			return FALSE ;

		++ n ;
	}
	while ( n < 0x20 );

	if ( n && !pBuffer[n] )
		return TRUE ;

	return FALSE ;
}



PVOID
ParseCharacters (
	IN PVOID pInBuffer
	)
{

	return pInBuffer ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////