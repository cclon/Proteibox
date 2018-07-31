/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/17 [17:5:2010 - 15:16]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\Memory.c
* 
* Description:
*      
*   内存管理模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "Memory.h"

//////////////////////////////////////////////////////////////////////////

LPMMHEAD g_ListHead__MemoryManager = NULL  ;
BOOL	 g_MemoryManager_Inited_ok = FALSE ;


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
InitMemoryManager (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_MemoryManager_Inited_ok )
	{
		bRet = MMCreateTotalHead( (PVOID) &g_ListHead__MemoryManager );
		g_MemoryManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitMemoryManager(); \n" );
		}
	}

	return bRet ;
}



BOOL 
MMCreateTotalHead(
	OUT PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  创建总结构体,初始化之    
    
Arguments:
  _TotalHead - 要初始化的总结构体指针的地址

Return Value:
  BOOL
    
--*/
{
	BOOL bRet = FALSE ;
	LPMMHEAD *pTotalHead = (LPMMHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构分配内存
	*pTotalHead = (LPMMHEAD) kmalloc( sizeof( MMHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | MMCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( MMHEAD ) );

	// 初始化资源锁
	bRet = InitResource( &((LPMMHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | MMCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( &( (LPMMHEAD)*pTotalHead )->ListHead.MemoryListEntry );

	((LPMMHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
MMDeleteTotalHead(
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
MMAllocateNode(
	OUT PVOID* _pCurrenList_
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
	LPMMNODE* pCurrenList = (LPMMNODE*) _pCurrenList_ ;
	*pCurrenList = (LPMMNODE) kmalloc( sizeof( MMNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | MMAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( MMNODE ) );

	return TRUE ;
}



ULONG
MMBuildNode (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:09]

Routine Description:
  根据@nSize的大小分配内存,将内存节点插入链表中,返回内存地址
    
Arguments:
  _TotalHead - 总的链表头
  nSize - 新申请的内存大小

Return Value:
  申请的内存地址
    
--*/
{
	ULONG pAddress	 = 0	;
	LPMMNODE pNewMember = NULL, pCurrent = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == nSize )
	{
		dprintf( "error! | MMBuildNode() | Invalid Parameters \n" );
		return 0 ;
	}

	//
	// 2. 查找链表中是否已存在该节点; 若存在直接返回之
	//

	pCurrent = (LPMMNODE) MMFindNode( _TotalHead, Tag, nSize );
	if ( NULL != pCurrent ) { return pCurrent->pAddress ; }

	//
	// 3. 没有该节点,新建之; 申请内存
	//

	pAddress = (ULONG) kmalloc( nSize );
	if ( 0 == pAddress )
	{
		dprintf( "error! | MMBuildNode() | Allocate memory failed! \n" );
		return 0 ;
	}

	RtlZeroMemory( (PVOID)pAddress, nSize );

	//
	// 3. 插入到链表中
	//

	pNewMember = (LPMMNODE) MMBuildNodeEx( pTotalHead, nSize, pAddress, Tag );
	if ( NULL == pNewMember )
	{
		dprintf( "error! | MMBuildNode() - MMBuildNodeEx() | NULL == pNode; Memory Tag = %d \n", Tag );
		kfree( (PVOID)pAddress );
		return 0 ;
	}

	dprintf( "[Memory Tag: %d] 新增节点已插入链表; \n", Tag );
	return pAddress ;
}



PVOID
MMBuildNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG pAddress,
	IN ULONG Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  往链表中新增一个节点 
    
Arguments:
  _TotalHead - 总的链表头
  nSize - 新申请的内存大小
  pAddress - 新申请的内存地址

--*/
{
	LPMMNODE pCurrent = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == nSize || 0 == pAddress )
	{
		dprintf( "error! | MMBuildNodeEx() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 分配内存,填充节点
	//

	if ( FALSE == MMAllocateNode( &pNode ) || NULL == pNode )
	{
		dprintf( "error! | MMBuildNodeEx() | MMAllocateNode() failed \n" );
		return NULL ;
	}

	pNode->nSize	= nSize		;
	pNode->pAddress	= pAddress	;
	pNode->Tag		= Tag		;

	//
	// 3. 若需要,则插入新节点到链表尾
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	if ( 0 == pTotalHead->ListHead.nSize && 0 == pTotalHead->ListHead.pAddress ) 
	{
		// 链表头未被使用.use it
		pTotalHead->nTotalCounts = 1 ;
		pTotalHead->ListHead = *pNode ;
		InitializeListHead( (PLIST_ENTRY)&pTotalHead->ListHead );

		kfree( (PVOID)pNode ); // 释放内存
		pNode = &pTotalHead->ListHead ;
	}
	else
	{
		InsertTailList( (PLIST_ENTRY)&pTotalHead->ListHead, (PLIST_ENTRY)pNode );
		pTotalHead->nTotalCounts ++ ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 释放锁
	return (PVOID)pNode ;
}



ULONG
MMFindNode (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  在链表中查找指定内存地址,返回之
    
Arguments:
  _TotalHead - 总的链表头
  Tag - 待匹配的内存地址标记
  nSize - 当各种内存块具有相同的TAG时候,需要区分内存块大小才能找到指定的内存块

Return Value:
  待查找的内存地址
    
--*/
{
	ULONG  pAddress	 = 0 ;
	LPMMNODE pResult	 = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || -1 == Tag ) { return 0 ; }

	// 2. 查找Tag对应的内存节点
	pResult = MMFindNodeEx( pTotalHead, Tag, nSize );
	if ( NULL == pResult ) { return 0 ; }

	// 3. 返回对应的内存地址
	pAddress = pResult->pAddress ;
	return pAddress ;
}



PVOID
MMFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  在链表中查找指定节点,返回之
    
Arguments:
  _TotalHead - 总的链表头
  Tag - 待匹配的内存地址标记
  nSize - 当各种内存块具有相同的TAG时候,需要区分内存块大小才能找到指定的内存块

--*/
{
	PVOID pResult = NULL ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || -1 == Tag ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts /* || TRUE == IsListEmpty( &pTotalHead->ListHead.MemoryListEntry )*/ ) { return NULL ; }

	//
	// 2. 遍历链表,查找指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( Tag == pNode->Tag ) 
		{
			if ( nSize ) // 在TAG相同的前提下,若nSize存在,说明要进一步比较内存大小!找到内存大小相同的节点,返回之
			{
				if ( nSize == pNode->nSize )
				{
					pResult = pNode ;
					break ;
				}
			} 
			else // 如果
			{
				pResult = pNode ;
				break ;
			}
		}

		pNode = (LPMMNODE) pNode->MemoryListEntry.Flink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



BOOL
MMDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  删除链表中指定节点,用内存地址进行匹配
    
Arguments:
  _TotalHead - 总的链表头
  pAddress - 待匹配的内存地址

Return Value:
  BOOL

--*/
{
	BOOL bRet = FALSE ;
	PLIST_ENTRY ListHead = NULL, NextEntry = NULL ;
	LPMMNODE		pCurrent = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == pAddress )
	{
		dprintf( "error! | MMDeleteNode() | Invalid Parameters \n" );
		return FALSE ;
	}

	//
	// 2. 遍历链表,删除指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	NextEntry = ListHead = &pTotalHead->ListHead.MemoryListEntry ;

	do 
	{
		pCurrent = CONTAINING_RECORD( NextEntry, MMNODE, MemoryListEntry );

		if ( pAddress == (ULONG)pCurrent->pAddress ) 
		{
			RemoveEntryList( NextEntry );

			kfree( (PVOID)pAddress );
			kfree( (PVOID)pCurrent );
			pTotalHead->nTotalCounts -- ;

			bRet = TRUE ;
			break ;
		}

		NextEntry = ListHead->Blink;
	} while ( NextEntry != ListHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return bRet ;
}



VOID
MMDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MMDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. 删除所有的子节点
	//

	MMWalkNodes( pTotalHead );

	dprintf( "*** 开始卸载内存管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		kfree( (PVOID) pNode->pAddress );

		if ( pHead != pNode )
		{
			dprintf( "  [%d] MemoryNode:0x%08lx \n", i++, (PVOID)pNode );

			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = (LPMMNODE) pHead->MemoryListEntry.Flink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. 删除总节点
	//

	MMDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载内存管理器 *** \n" );
	return ;
}



VOID
MMWalkNodes (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 17:13]

Routine Description:
  打印所有节点信息
    
--*/
{
	INT i = 1 ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MMWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | MMWalkNodes() | 链表为空,不可遍历 \n" );
		return ;
	}

	dprintf( "\n**** Starting walking Memory Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] nSize = 0x%x, pAddress = 0x%08lx \n",
			i++,
			pNode->nSize,
			pNode->pAddress
			);

		pNode = (LPMMNODE) pNode->MemoryListEntry.Flink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	dprintf( "**** End of walking Memory Lists **** \n\n" );

#endif
	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////