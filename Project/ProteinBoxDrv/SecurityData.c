/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/02 [2:6:2010 - 17:33]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\SecurityData.c
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
#include "SecurityData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

LPREGISTRY_USERSID_INFO_HEAND g_ListHead__RegistryUserSID = NULL ;
BOOL g_SecurityDataManager_Inited_ok = FALSE ;


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
InitSecurityData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_SecurityDataManager_Inited_ok )
	{
		bRet = SDCreateTotalHead( (PVOID) &g_ListHead__RegistryUserSID );
		g_SecurityDataManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitSecurityData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
SDCreateTotalHead(
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
	LPSDHEAD *pTotalHead = (LPSDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPSDHEAD) kmalloc( sizeof( SDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | SDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( SDHEAD ) );

	// 初始化资源锁
	bRet = InitResource( &((LPSDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SDCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPSDHEAD)*pTotalHead )->ListHead );

	((LPSDHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
SDDeleteTotalHead(
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
SDAllocateNode(
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
	LPSDNODE* pCurrenList = (LPSDNODE*) _pCurrenList_ ;
	*pCurrenList = (LPSDNODE) kmalloc( sizeof( SDNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | SDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( SDNODE ) );

	return TRUE ;
}



PVOID
SDBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  新建&填充SD结构体
    
Arguments:
  _TotalHead - 链表头
  RegisterUserID - SID

Return Value:
  新建的SD结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPSDNODE pNode = NULL ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == _TotalHead || NULL == RegisterUserID )
	{
		dprintf( "error! | SDBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 查找是否是已存在的节点 
	//

	pNode = (LPSDNODE) kgetnodeSD( RegisterUserID );
	if ( pNode )
	{ 
	//	dprintf( "ok! | SDBuildNode() - SDFindNode(); | 当期SD节点已存在于链表中 SID=%ws \n", RegisterUserID );
		return pNode ;
	}

	//
	// 3. 填充新节点
	//

	pNode = SDBuildNodeEx( _TotalHead, RegisterUserID );
	if ( NULL == pNode )
	{
		dprintf( "error! | SDBuildNode() - SDBuildNodeEx() | \n" );
		return NULL ;
	}

	//
	// 4. 和R3交互的,暂时保留不用
	//

	if ( pNode->Length_RegitryUserSID == 0xFFFFFFFF )
	{
		OBJECT_ATTRIBUTES ObjectAttributes ; 
		UNICODE_STRING uniBuffer ;
		HANDLE Handle = NULL ;

		RtlInitUnicodeString( &uniBuffer, L"\\BaseNamedObjects\\Global\\Proteinbox_WorkEvent" );
		InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

		status = ZwOpenEvent( &Handle, EVENT_MODIFY_STATE, &ObjectAttributes );
		if( NT_SUCCESS( status ) )
		{
			ZwSetEvent( Handle, 0 );                      // 设置为受信状态
			ZwClose( Handle );
		}
	}

	return (PVOID) pNode ;
}



PVOID
SDBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  填充SD结构体   

--*/
{
	BOOL bRet = FALSE ;
	LPSDNODE pNode = NULL ;
	LPSDHEAD pTotalHead	 = (LPSDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == RegisterUserID )
	{
		dprintf( "error! | SDBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. 分配内存,填充节点
	//

	bRet = SDAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SDBuildNodeEx() - SDAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	pNode->Length_RegitryUserSID = 0xFFFFFFFF ;
	pNode->Length_CurrentUserName = 0x12340015 ;
	pNode->Length_CurrentUserName = 0 ;
	pNode->Length_RegitryUserSID = wcslen( RegisterUserID );

	wcscpy( pNode->RegitryUserSID, RegisterUserID );

	//
	// 3. 若需要,则插入新节点到链表尾
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	if ( NULL == pTotalHead->ListHead.RegitryUserSID && 0 == pTotalHead->ListHead.Length_RegitryUserSID ) 
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
SDFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  在链表中查找指定节点,返回之
    
Arguments:
  _TotalHead - 总的链表头
  RegisterUserID - 待匹配的值

Return Value:
  待查找的节点单元指针
    
--*/
{
	PVOID  pResult = NULL ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead	= (LPSDHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == RegisterUserID ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. 遍历链表,查找指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( 0 == _wcsicmp( RegisterUserID, pNode->RegitryUserSID ) ) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



VOID
SDDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead = (LPSDHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. 删除所有的子节点
	//

	SDWalkNodes( pTotalHead );

	dprintf( "*** 开始卸载SID节点管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; RegitryUserSID:\"%ws\" \n", i++, (PVOID)pNode, pNode->RegitryUserSID );

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = pHead->pFlink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. 删除总节点
	//

	SDDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载SID节点管理器 *** \n" );
	return ;
}



VOID
SDWalkNodes (
	IN PVOID _TotalHead
	)
{
	int i = 1 ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead	= (LPSDHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SDWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | SDWalkNodes() | 链表为空,不可遍历 \n" );
		return ;
	}

	dprintf( "\n**** Starting walking SecurityData Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] RegitryUserSID=\"%ws\", CurrentUserName=\"%ws\" \n",
			i++,
			pNode->RegitryUserSID,
			pNode->CurrentUserName
			);

		pNode = pNode->pFlink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	dprintf( "**** End of walking SecurityData Lists **** \n\n" );

#endif
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////