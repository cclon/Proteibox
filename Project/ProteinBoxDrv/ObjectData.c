/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/11 [11:6:2010 - 16:02]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ObjectData.c
* 
* Description:
*      
*   管理OjectHook链表                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ObjectData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

LPOBJECT_DATA_INFO_HEAND g_ListHead__ObjectData = NULL ;
BOOL g_ObjectData_Inited_ok = FALSE ;


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
InitObjectData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ObjectData_Inited_ok )
	{
		bRet = ODCreateTotalHead( (PVOID) &g_ListHead__ObjectData );
		g_ObjectData_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitObjectData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
ODCreateTotalHead(
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
	LPODHEAD *pTotalHead = (LPODHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPODHEAD) kmalloc( sizeof( ODHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | ODCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( ODHEAD ) );

	// 初始化资源锁
	bRet = InitResource( &((LPODHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ODCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPODHEAD)*pTotalHead )->ListHead );

	((LPODHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
ODDeleteTotalHead(
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
ODAllocateNode(
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
	LPODNODE* pCurrenList = (LPODNODE*) _pCurrenList_ ;
	*pCurrenList = (LPODNODE) kmalloc( sizeof( ODNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | ODAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( ODNODE ) );

	return TRUE ;
}



PVOID
ODBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  新建&填充OD结构体
    
Arguments:
  _TotalHead - 链表头
  pInBuffer - 包含待初始化的所有节点信息

Return Value:
  新建的OD结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	PVOID pNode = NULL ;
	LPOBJECT_INIT Buffer = (LPOBJECT_INIT) pInBuffer ;

	//
	// 1. 校验参数合法性
	//

	if ( (NULL == _TotalHead) || (NULL == Buffer) || (FALSE == IS_OBJECT_INDEX( Buffer->ObjectIndex )) )
	{
		dprintf( "error! | ODBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 查找是否是已存在的节点 
	//

	pNode = kgetnodeOD( Buffer->ObjectIndex );
	if ( pNode )
	{ 
		dprintf( "ok! | ODBuildNode() - ODFindNode(); | 当期OD节点已存在于链表中 ObjectIndex=%d \n", Buffer->ObjectIndex );
		return pNode ;
	}

	//
	// 3. 填充新节点
	//

	pNode = ODBuildNodeEx( _TotalHead, pInBuffer );

	return (PVOID) pNode ;
}



PVOID
ODBuildNodeEx (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Arguments:
  _TotalHead - 链表头
  pInBuffer - 包含待初始化的所有节点信息 

--*/
{
	BOOL bRet = FALSE ;
	LPODNODE pNode = NULL ;
	LPODHEAD pTotalHead	 = (LPODHEAD) _TotalHead ;
	LPOBJECT_INIT Buffer = (LPOBJECT_INIT) pInBuffer ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == Buffer )
	{
		dprintf( "error! | ODBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. 分配内存,填充节点
	//

	bRet = ODAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ODBuildNodeEx() - ODAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	pNode->ObjectIndex		= Buffer->ObjectIndex ;
	pNode->TypeInfoOffset	= Buffer->TypeInfoOffset ;
	wcscpy( pNode->szObjecType, Buffer->szObjecType );

	if ( Buffer->FakeAddrOpen )
	{
		pNode->Open.bCare			= TRUE ;
		pNode->Open.bHooked			= FALSE ;
		pNode->Open.OrignalAddr		= 0 ;
		pNode->Open.UnhookPoint		= 0 ;
		pNode->Open.ProcedureOffset = Buffer->ProcedureOffsetOpen ;
		pNode->Open.FakeAddr		= Buffer->FakeAddrOpen ;
	}

	if ( Buffer->FakeAddrParse )
	{
		pNode->Parse.bCare				= TRUE ;
		pNode->Parse.bHooked			= FALSE ;
		pNode->Parse.OrignalAddr		= 0 ;
		pNode->Parse.UnhookPoint		= 0 ;
		pNode->Parse.ProcedureOffset	= Buffer->ProcedureOffsetParse ;
		pNode->Parse.FakeAddr			= Buffer->FakeAddrParse ;
	}

	//
	// 4. 若需要,则插入新节点到链表尾
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	if ( 0 == pTotalHead->ListHead.ObjectIndex && 0 == pTotalHead->ListHead.TypeInfoOffset && NULL == pTotalHead->ListHead.szObjecType ) 
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
		InsertTailList( (PLIST_ENTRY) &pTotalHead->ListHead, (PLIST_ENTRY) pNode );
		pTotalHead->nTotalCounts ++ ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// 释放锁
	return (PVOID)pNode ;
}



PVOID
ODFindNode (
	IN PVOID _TotalHead ,
	IN ULONG ObjectIndex
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  在链表中查找指定节点,返回之
    
Arguments:
  _TotalHead - 总的链表头
  ObjectIndex - 待匹配的值

Return Value:
  待查找的节点单元指针
    
--*/
{
	PVOID  pResult = NULL ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead	   = (LPODHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( (NULL == pTotalHead) || (NULL == pTotalHead->QueueLockList) || (FALSE == IS_OBJECT_INDEX( ObjectIndex )) ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. 遍历链表,查找指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( ObjectIndex == pNode->ObjectIndex ) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁
	return pResult ;
}



VOID
ODDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead = (LPODHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | ODDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. 删除所有的子节点
	//

	ODWalkNodes( pTotalHead );

	dprintf( "*** 开始卸载OD节点管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; szObjecType:\"%ws\" \n", i++, (PVOID)pNode, pNode->szObjecType );

		// 卸载Hook
		if ( pNode->Open.bHooked )
		{
			*(PULONG) pNode->Open.UnhookPoint = pNode->Open.OrignalAddr ;
			pNode->Open.bHooked = FALSE ;
		}

		if ( pNode->Parse.bHooked )
		{
			*(PULONG) pNode->Parse.UnhookPoint = pNode->Parse.OrignalAddr ;
			pNode->Parse.bHooked = FALSE ;
		}

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = pHead->pFlink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. 删除总节点
	//

	ODDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载OD节点管理器 *** \n" );
	return ;
}



VOID
ODWalkNodes (
	IN PVOID _TotalHead
	)
{
	INT i = 1 ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead = (LPODHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | ODWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | ODWalkNodes() | 链表为空,不可遍历 \n" );
		return ;
	}

	dprintf( "\n**** Starting walking ObjectData Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[ObjectIndex=%d] \n"
			"  szObjecType=\"%ws\" \n"
			"  Open.OrignalAddr: 0x%08lx\n"
			"  Parse.OrignalAddr: 0x%08lx\n",
			pNode->ObjectIndex,
			pNode->szObjecType,
			pNode->Open.OrignalAddr,
			pNode->Parse.OrignalAddr
			);

		pNode = pNode->pFlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁
	dprintf( "**** End of walking ObjectData Lists **** \n\n" );

#endif
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////