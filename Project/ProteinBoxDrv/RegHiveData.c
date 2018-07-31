/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/07 [7:6:2010 - 15:55]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\RegHiveData.c
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
#include "RegHiveData.h"
#include "ProcessData.h"
#include "SdtData.h"
#include "Security.h"
#include "Memory.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////


LPREGHIVE_NODE_INFO_HEAND g_ListHead__RegHive = NULL ;
BOOL g_RegHive_Inited_ok = FALSE ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


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
InitRegHive (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_RegHive_Inited_ok )
	{
		bRet = RHCreateTotalHead( (PVOID) &g_ListHead__RegHive );
		g_RegHive_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitRegHive(); \n" );
		}
	}

	return bRet ;
}



BOOL 
RHCreateTotalHead(
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
	LPRHHEAD *pTotalHead = (LPRHHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPRHHEAD) kmalloc( sizeof( RHHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | RHCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( RHHEAD ) );

	// 初始化资源锁
	bRet = InitResource( &((LPRHHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | RHCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPRHHEAD)*pTotalHead )->ListHead );

	((LPRHHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
RHDeleteTotalHead(
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
RHAllocateNode(
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
	LPRHNODE* pCurrenList = (LPRHNODE*) _pCurrenList_ ;
	*pCurrenList = (LPRHNODE) kmalloc( sizeof( RHNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | RHAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( RHNODE ) );

	return TRUE ;
}



PVOID
RHBuildNode (
	IN PVOID _TotalHead,
	IN PVOID _ProcessNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  新建&填充RH结构体
    
Arguments:
  _TotalHead - 链表头
  _ProcessNode - 当前进程的总节点

Return Value:
  新建的RH结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPRHNODE pNode = NULL ;
	LPRHHEAD pTotalHead	 = (LPRHHEAD) _TotalHead ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;
	LPWSTR HiveRegPath = NULL ;
	LPWSTR HiveFilePath = NULL ;
	WCHAR __HiveFilePath [ MAX_PATH ] = L"" ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == ProcessNode || NULL == ProcessNode->pNode_C )
	{
		dprintf( "error! | RHBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	HiveRegPath	 = ProcessNode->pNode_C->KeyRootPath  ;
	HiveFilePath = ProcessNode->pNode_C->FileRootPath ;
	if (  NULL == HiveRegPath || NULL == HiveFilePath )
	{
		dprintf( "error! | RHBuildNode() | Invalid Parameters,(ProcessNode) \n" );
		return NULL ;
	}

	wcscpy( __HiveFilePath, HiveFilePath );
	wcscat( __HiveFilePath, L"\\RegHive" );

	//
	// 该循环负责当前沙箱中进程间的同步
	//

	while ( TRUE )
	{
		//
		// 2. 查找是否是已存在的节点 
		//

		pNode = (LPRHNODE) kgetnodeRH( HiveRegPath, __HiveFilePath, &bRet );
		if ( FALSE == bRet )
		{
			dprintf( "error! | RHBuildNode() - RHFindNode(); | 引用计数原因,不合法 \n" );
			return NULL ;
		}

		if ( pNode )
		{ 
			// 2.1 已存在节点
		//	dprintf( "ok! | RHBuildNode() - RHFindNode(); | 当前RH节点已存在于链表中 \n" );
		}
		else
		{
			// 2.2 填充新节点
			pNode = (LPRHNODE) RHBuildNodeEx( _TotalHead, HiveRegPath, __HiveFilePath );
			if ( NULL == pNode )
			{
				dprintf( "error! | RHBuildNode() - RHBuildNodeEx() | \n" );
				return NULL ;
			}
		}

		//
		// 3. 检查Hive注册表相关,不存在则创建对应的Hive文件 [这段代码巨难看 (╯3╰) ]
		//

		EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

		if (  pNode->ProcessesLock ) { goto _WHILE_NEXT_ ; }
		
		if ( pNode->PorcessRefCounts ) 
		{
_ok_ :
			pNode->PorcessRefCounts ++ ;
			pNode->bNeedToDistroy = FALSE ;
			ProcessNode->pNode_RegHive = (PVOID) pNode;

			LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
			break ; 
		}

		pNode->ProcessesLock = 1 ;
		
		LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
		bRet = CheckRegHive( (PVOID)pNode, (PVOID)ProcessNode );
		EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

		pNode->ProcessesLock = 0 ;

		if ( TRUE == bRet ) 
		{ 
			goto _ok_ ;
		}
		else
		{
			LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
			return NULL ;
		}

_WHILE_NEXT_:
		LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
		ZwYieldExecution();
	} // end of while
	
	return (PVOID) pNode ;
}



PVOID
RHBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  填充RH结构体   

--*/
{
	BOOL bRet = FALSE ;
	LPRHNODE pNode = NULL ;
	LPRHHEAD pTotalHead	 = (LPRHHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == HiveRegPath || NULL == HiveFilePath )
	{
		dprintf( "error! | RHBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. 分配内存,填充节点
	//

	bRet = RHAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | RHBuildNodeEx() - RHAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	wcscpy( pNode->HiveRegPath, HiveRegPath );
	wcscpy( pNode->HiveFilePath, HiveFilePath );

	pNode->PorcessRefCounts = 0 ;
	pNode->ProcessesLock	= 0 ;	
	pNode->bNeedToDistroy	= FALSE ;

	//
	// 3. 若需要,则插入新节点到链表尾
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	if ( NULL == pTotalHead->ListHead.HiveFilePath && NULL == pTotalHead->ListHead.HiveRegPath ) 
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
RHFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath,
	OUT BOOL* bResult
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
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead	= (LPRHHEAD) _TotalHead ;

	//
	// 1. 校验参数合法性
	//

	*bResult = TRUE ;
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == HiveRegPath || NULL == HiveFilePath ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. 遍历链表,查找指定节点
	//

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if (	(0 == _wcsicmp( HiveRegPath, pNode->HiveRegPath ))
			&&	(0 == _wcsicmp( HiveFilePath, pNode->HiveFilePath ))
			) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		if ( pNode->PorcessRefCounts )
		{
			dprintf( "error! | RHFindNode(); | 当前沙箱的Hive引用计数不为空 \n" );
			*bResult = FALSE ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 加锁访问
	return pResult ;
}



VOID
RHDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead = (LPRHHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | RHDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. 删除所有的子节点
	//

	RHWalkNodes( pTotalHead );

	dprintf( "*** 开始卸载RegHive节点管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; \nHiveRegPath:\"%ws\" \nHiveFilePath:\"%ws\" \n", i++, (PVOID)pNode, pNode->HiveRegPath, pNode->HiveFilePath );

		if ( pHead != pNode )
		{	
			RemoveEntryList( (PLIST_ENTRY)pNode );
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

	RHDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载RegHive节点管理器 *** \n" );
	return ;
}



VOID
RHWalkNodes (
	IN PVOID _TotalHead
	)
{
	INT i = 1 ;
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead	= (LPRHHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | RHWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | RHWalkNodes() | 链表为空,不可遍历 \n" );
		return ;
	}

	dprintf( "\n**** Starting walking RegHive Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] PorcessRefCounts=%d,HiveFilePath=\"%ws\", HiveRegPath=\"%ws\" \n",
			i++,
			pNode->PorcessRefCounts,
			pNode->HiveFilePath,
			pNode->HiveRegPath
			);

		pNode = pNode->pFlink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁
	dprintf( "**** End of walking RegHive Lists **** \n\n" );

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


BOOL
CheckRegHive (
	IN PVOID _pNode,
	IN PVOID _ProcessNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 15:27]

Routine Description:
  检查当前沙箱的Hive相关信息    

--*/
{
	HANDLE KeyHandle;
	BOOL bRet = FALSE ;
	ULONG Length = 0, ResLength = 0;
	PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo = NULL ;
	UNICODE_STRING FileName, KeyName, uniBuffer ;
	OBJECT_ATTRIBUTES FileObjectAttributes, KeyObjectAttributes, objattri ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPRHNODE pNode = (LPRHNODE) _pNode ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == _pNode || NULL == _ProcessNode )
	{
		dprintf( "error! | CheckRegHive(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 打开HiveRegPath,查看对应的注册表键值是否存在
	//

	RtlInitUnicodeString( &FileName, pNode->HiveFilePath );
	
	InitializeObjectAttributes (
		&FileObjectAttributes,
		&FileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	RtlInitUnicodeString( &KeyName, pNode->HiveRegPath );

	InitializeObjectAttributes (
		&KeyObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ | KEY_WRITE, &KeyObjectAttributes );
	if ( ! NT_SUCCESS(status) ) 
	{
		// 不存在,则创建Hive文件
		dprintf( "ok! | CheckRegHive() - ZwOpenKey(); | 当前的键值\"%ws\"不存在,调用CmLoadKey()创建Hive文件 \n", pNode->HiveFilePath );

		bRet = CmLoadKey( _ProcessNode, &KeyObjectAttributes, &FileObjectAttributes );
		return bRet ;
	}

	//
	// 3. 存在,校验合法性
	//
	
	ZwClose( KeyHandle );
	
	RtlInitUnicodeString( &uniBuffer, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\hivelist" );

	InitializeObjectAttributes (
		&objattri,
		&uniBuffer,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ | KEY_WRITE, &objattri );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | CheckRegHive() - ZwOpenKey(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, 0, 0, &ResLength );
	if( STATUS_BUFFER_TOO_SMALL != status )
	{
		dprintf( "error! | CheckRegHive() - ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

	ResLength += sizeof( KEY_VALUE_PARTIAL_INFORMATION );
	KeyValuePartialInfo = kmalloc( ResLength );
	Length = ResLength ;

	if( NULL == KeyValuePartialInfo ) 
	{
		dprintf( "error! | CheckRegHive() - kmalloc(); | 分配内存失败,大小:%d \n", ResLength );
		goto _END_ ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, (PVOID)KeyValuePartialInfo, Length, &ResLength );
	if( ! NT_SUCCESS(status) || (REG_SZ != KeyValuePartialInfo->Type) )
	{
		dprintf( "error! | CheckRegHive() - kmalloc(); | ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _FREEMEMORY_ ;
	}

	if ( 0 == _wcsicmp( (LPWSTR)KeyValuePartialInfo->Data, FileName.Buffer ) )
	{
		bRet = TRUE ;
	}

_FREEMEMORY_ :
	kfree( KeyValuePartialInfo );
_END_ :
	ZwClose( KeyHandle );
	return bRet ;
}



BOOL
CmLoadKey(
	IN PVOID _ProcessNode,
	IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	IN POBJECT_ATTRIBUTES FileObjectAttributes
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 15:30]

Routine Description:
  对ZwLoadKey函数的包装,调用前需要提权
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;
	HANDLE TokenHandle = NULL ;
	BOOL bRet = FALSE ;
	ULONG DefaultDacl_Length = 0 ;
	PTOKEN_DEFAULT_DACL	LocalDefaultDacl = NULL	;


	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == _ProcessNode || NULL == KeyObjectAttributes || NULL == FileObjectAttributes )
	{
		dprintf( "error! | CmLoadKey(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2.1 保存旧的权限信息
	//

	status = ZwOpenProcessToken( (HANDLE)0xFFFFFFFF, TOKEN_ADJUST_DEFAULT | TOKEN_QUERY, &TokenHandle );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | CmLoadKey() - ZwOpenProcessToken(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	bRet = QueryInformationToken( TokenHandle, TokenDefaultDacl, &LocalDefaultDacl, &DefaultDacl_Length );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CmLoadKey() - QueryInformationToken(); |  \n" );
		ZwClose( TokenHandle );
		return FALSE ;
	}

	// 2.2 获取 ZwSetInformationToken 的原始地址
	bRet = kgetaddrSDT( ZwSetInformationToken );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CmLoadKey() - kgetaddrSDT(); | 无法获取ZwSetInformationToken的地址 \n" );
		goto _CLEAR_UP_ ;
	}

	// 2.3 提权
	status = g_ZwSetInformationToken_addr( TokenHandle, TokenDefaultDacl, &g_DefaultDacl_new, sizeof(DWORD) );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwSetInformationToken() | 设置新Token失败; status == 0x%08lx \n", status );
		bRet = FALSE ;
		goto _CLEAR_UP_ ;
	}

	// 2.4 映射注册表和Hive文件
	bRet =  TRUE ;

	status = ZwLoadKey( KeyObjectAttributes, FileObjectAttributes );
	if ( ! NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwLoadKey() | status == 0x%08lx \n", status );
		bRet =  FALSE ;
	}

	// 2.5 恢复至旧的权限
	status = g_ZwSetInformationToken_addr( TokenHandle, TokenDefaultDacl, LocalDefaultDacl, DefaultDacl_Length );
	if ( ! NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwSetInformationToken() | 恢复旧Token失败; status == 0x%08lx \n", status );
		bRet =  FALSE ;
	}

_CLEAR_UP_ :
	kfree( (PVOID)LocalDefaultDacl );
	ZwClose( TokenHandle );
	return bRet ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////