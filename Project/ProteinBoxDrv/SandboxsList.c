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
#include "SandboxsList.h"

//////////////////////////////////////////////////////////////////////////

LPSANDBOX_NODE_INFO_HEAND g_ListHead__Sandboxs = NULL ;
static BOOL g_SandboxLists_Inited_ok = FALSE ;

static const ULONG g_nSandboxMaxCouts = 5 ; // 限制仅能创建<=5个沙箱


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
InitSandboxsData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_SandboxLists_Inited_ok )
	{
		bRet = SBLCreateTotalHead( (PVOID) &g_ListHead__Sandboxs );
		g_SandboxLists_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitProcessData(); \n" );
		}
	}

	return bRet ;
}



int SBLGetSandboxsCounts()
{
	int nTotalCounts = 0 ;

	if ( g_ListHead__Sandboxs )
	{
		EnterCrit( g_ListHead__Sandboxs->QueueLockList );	// 加锁访问

		nTotalCounts = g_ListHead__Sandboxs->nTotalCounts ;

		LeaveCrit( g_ListHead__Sandboxs->QueueLockList );	// 加锁访问 
	}

	return nTotalCounts;
}



BOOL 
SBLCreateTotalHead(
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
	LPSANDBOX_NODE_INFO_HEAND *pTotalHead = (LPSANDBOX_NODE_INFO_HEAND*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) kmalloc( sizeof( SANDBOX_NODE_INFO_HEAND ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | SBLCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( SANDBOX_NODE_INFO_HEAND ) );

	// 初始化资源锁
	bRet = InitResource( &((LPSANDBOX_NODE_INFO_HEAND)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SBLCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPSANDBOX_NODE_INFO_HEAND)*pTotalHead )->ListHead );

	((LPSANDBOX_NODE_INFO_HEAND)*pTotalHead)->nTotalCounts = 0 ;
	return TRUE ;
}



VOID 
SBLDeleteTotalHead(
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
SBLAllocateNode(
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
	ULONG Length = sizeof( SANDBOX_NODE_INFO );

	if ( NULL == pNode )
	{
		dprintf( "error! | SBLAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pNode = kmalloc( Length );
	if ( NULL == *pNode )
	{
		dprintf( "error! | SBLAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pNode, Length );
	return TRUE ;
}



PVOID
SBLBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR wszName,
	IN ULONG NameLength,
	OUT BOOL* bOverload
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  注意:@ListHead 链表头中永远不会存储wszName,它的作用不是用作节点. 故遍历该链表查找数据时,无需关注表头中存储的内容
    
Arguments:
  _TotalHead - 配置文件的链表头
  wszName - 沙箱名
  NameLength - 沙箱名长度
  bOverload - 若沙箱数量超过极限,置TRUE	
    
--*/
{
	BOOL bRet = FALSE ;
	LPSANDBOX_NODE_INFO pNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;
	
	// 1. 校验参数合法性
	*bOverload = FALSE;
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 || NameLength > MAX_PATH )
	{
		dprintf( "error! | SBLBuildNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	if ( pTotalHead->nTotalCounts > g_nSandboxMaxCouts )
	{
		*bOverload = TRUE;
		dprintf( "error! | SBLBuildNode(); | 沙箱数量超过极限,现不予创建新沙箱 \n" );
		return NULL ;
	}

	pNode = kgetnodeSBL( wszName, NameLength );
	if ( pNode ) 
	{ 
		dprintf( "ok! | SBLBuildNode(); | 该沙箱已存在,无需加入链表 \n" );
		return pNode; 
	}

	// 2. 申请节点
	bRet = SBLAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SBLBuildNode() - SBLAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	// 3. 填充之
	pNode->wszName = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszName )
	{
		dprintf( "error! | SBLBuildNode() - kmalloc(); | 申请内存失败! \n" );
		kfree( (PVOID)pNode ); // 释放内存
		return NULL ;
	}

	RtlZeroMemory( pNode->wszName, NameLength );
	RtlCopyMemory( pNode->wszName, wszName, NameLength );

	pNode->NameLength = NameLength ;
	pNode->Index = pTotalHead->nTotalCounts + 1 ;

	// 4. 插入链表中
	kInsertTailSB( pNode );
	return (PVOID)pNode ;
}



PVOID
SBLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName,
	IN ULONG NameLength
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
	LPSANDBOX_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 )
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
	pCurrentNode = (LPSANDBOX_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( NameLength == pCurrentNode->NameLength && 0 == _wcsicmp(wszName, pCurrentNode->wszName) ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPSANDBOX_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



PVOID
SBLFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Index
	)
{
	PVOID pResult = NULL ;
	LPSANDBOX_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	// 1. 校验参数合法性
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || Index < 1 || Index > g_nSandboxMaxCouts ) { return NULL; }

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
		return NULL ;
	}

	// 2. 遍历链表,查找指定节点
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = (LPSANDBOX_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( Index == pCurrentNode->Index ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPSANDBOX_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



VOID
SBLDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPSANDBOX_NODE_INFO pCurrentNode = NULL ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SBLDistroyAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	// 1. 删除所有的子节点
	dprintf( "*** 开始卸载SandboxsList节点管理器 *** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPSANDBOX_NODE_INFO) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx, BoxName:%ws \n", i++, (PVOID)pCurrentNode, pCurrentNode->wszName );
		kfree( (PVOID)pCurrentNode->wszName );
		kfree( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 2. 删除总节点
	SBLDeleteTotalHead( &_TotalHead );

//	dprintf( "*** 结束卸载SandboxsList节点管理器 *** \n" );
	return ;
}



VOID
SBLWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////