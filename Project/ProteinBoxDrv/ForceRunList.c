/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/16 [16:7:2010 - 10:51]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ForceRunList.c
* 
* Description:
*      
*   管理被强制运行于沙箱中的进程                       
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
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

LPFORCEPROC_NODE_INFO_HEAND g_ListHead__ForceProc ;
static BOOL g_ForceProcLists_Inited_ok = FALSE ;

BOOL g_bForceAll2RunInSandbox = TRUE ; // 初始状态为让配置文件中指定的所有进程强制运行在沙箱中
BOOL g_bAllRunOutofSandbox = FALSE ;  // 置TRUE表示让配置文件中指定的所有进程运行在沙箱外


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
InitForceProcData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ForceProcLists_Inited_ok )
	{
		bRet = FPLCreateTotalHead( (PVOID) &g_ListHead__ForceProc );
		g_ForceProcLists_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitForceProcData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
FPLCreateTotalHead(
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
	LPFORCEPROC_NODE_INFO_HEAND *pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) kmalloc( sizeof( FORCEPROC_NODE_INFO_HEAND ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | FPLCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( FORCEPROC_NODE_INFO_HEAND ) );

	// 初始化资源锁
	bRet = InitResource( &((LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | FPLCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead )->ListHead );

	((LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead)->nTotalCounts = 0 ;
	return TRUE ;
}



VOID 
FPLDeleteTotalHead(
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
FPLAllocateNode(
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
	ULONG Length = sizeof( FORCEPROC_NODE_INFO );

	if ( NULL == pNode )
	{
		dprintf( "error! | FPLAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pNode = kmalloc( Length );
	if ( NULL == *pNode )
	{
		dprintf( "error! | FPLAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pNode, Length );
	return TRUE ;
}



PVOID
FPLBuildNode (
	IN PVOID _TotalHead,
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
  wszName - 进程全路径
  NameLength - 进程全路径的长度
    
--*/
{
	BOOL bRet = FALSE ;
	LPFORCEPROC_NODE_INFO pNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;
	
	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 || NameLength > MAX_PATH )
	{
		dprintf( "error! | FPLBuildNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	pNode = kgetnodeFPL( wszName );
	if ( pNode ) 
	{ 
		dprintf( "ok! | FPLBuildNode(); | 该沙箱已存在,无需加入链表 \n" );
		return pNode; 
	}

	// 2. 申请节点
	bRet = FPLAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | FPLBuildNode() - SBLAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	// 3. 填充之
	pNode->wszProcFullPath = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszProcFullPath )
	{
		dprintf( "error! | FPLBuildNode() - kmalloc(); | 申请内存失败! \n" );
		kfree( (PVOID)pNode ); // 释放内存
		return NULL ;
	}

	RtlZeroMemory( pNode->wszProcFullPath, NameLength );
	RtlCopyMemory( pNode->wszProcFullPath, wszName, NameLength );

	pNode->NameLength = NameLength ;
	pNode->bRunInSandbox = FALSE; // 新建节点时,置FALSE表示该节点不会被强制运行于沙箱中

	// 4. 插入链表中
	kInsertTailFP( pNode );
	return (PVOID)pNode ;
}



PVOID
FPLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  在链表中查找指定沙箱名,返回对应节点
    
Arguments:
  _TotalHead - 总的链表头
  wszName - 待匹配的沙箱名

Return Value:
  已找到的节点
    
--*/
{
	PVOID pResult = NULL ;
	LPFORCEPROC_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || NULL == wszName )
	{
	//	dprintf( "error! | PDFindNode() | Invalid Parameters \n" );
		return NULL ;
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
	pCurrentNode = (LPFORCEPROC_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( 0 == _wcsicmp(wszName, pCurrentNode->wszProcFullPath) ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPFORCEPROC_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



BOOL
FPLGetState (
	IN LPWSTR wszName,
	IN BOOL* bRunInSandbox
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/14 [14:7:2011 - 16:36]

Routine Description:
  查询强制功能的状态
    
Arguments:
  wszName - 当前被创建的进程全路径
  bRunInSandbox - 保存状态值; 为TRUE表明允许强制功能
 
--*/
{
	BOOL bRet = FALSE ;
	LPFORCEPROC_NODE_INFO pNode = NULL ;

	*bRunInSandbox = FALSE ;

	// 在配置文件中匹配路径
	bRet = kIsValueNameExist( L"GlobalSetting", L"ForceProcess", wszName );
	if ( FALSE == bRet )
	{
		bRet = kIsValueNameExist( L"GlobalSetting", L"ForceFolder", wszName );
	}

	if ( FALSE == bRet ) { return FALSE ; }

	// 在配置文件中,进一步验证是否需要"被sb"
	*bRunInSandbox = TRUE;

	if ( g_bForceAll2RunInSandbox ) { return TRUE; }
	if ( g_bAllRunOutofSandbox ) { return FALSE; }
	
	// 不是允许所有,遍历链表,找到对应节点,返回当前状态
	pNode = kgetnodeFPL( wszName );
	if ( pNode ) 
	{ 
		*bRunInSandbox = pNode->bRunInSandbox ; 
	}
	
	return TRUE;
}



VOID
FPLSetStateAll (
	IN PVOID _TotalHead,
	IN BOOL bRunInSandbox
	)
{
	LPFORCEPROC_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | FPLSetStateAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = (LPFORCEPROC_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		pCurrentNode->bRunInSandbox = bRunInSandbox;
		pCurrentNode = (LPFORCEPROC_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return ;
}




VOID
FPLDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPFORCEPROC_NODE_INFO pCurrentNode = NULL ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | FPLDistroyAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	// 1. 删除所有的子节点
	dprintf( "*** 开始卸载SandboxsList节点管理器 *** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPFORCEPROC_NODE_INFO) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx, ForceProcName:%ws, state:%n \n", i++, (PVOID)pCurrentNode, pCurrentNode->wszProcFullPath, pCurrentNode->bRunInSandbox );
		kfree( (PVOID)pCurrentNode->wszProcFullPath );
		kfree( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 2. 删除总节点
	FPLDeleteTotalHead( &_TotalHead );

//	dprintf( "*** 结束卸载SandboxsList节点管理器 *** \n" );
	return ;
}



VOID
FPLWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////