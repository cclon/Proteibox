/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/02 [2:6:2010 - 17:33]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\GrayList.c
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
#include "Config.h"
#include "ConfigThread.h"
#include "Common.h"
#include "Memory.h"
#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////////

#define g_DeepCounts	3

LPPB_CONFIG_TOTAL g_ProteinBox_Conf_TranshipmentStation = NULL	;
LPPB_CONFIG_TOTAL g_ProteinBox_Conf = NULL	;
LPPB_CONFIG_TOTAL g_ProteinBox_Conf_Old = NULL ;
BOOL g_ProteinBox_Conf_Inited_ok	= FALSE	;


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
InitConfigData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  创建链表头    

--*/
{
	BOOL bRet = FALSE ;

//	if ( FALSE == g_ProteinBox_Conf_Inited_ok )
	{
		if ( g_ProteinBox_Conf_TranshipmentStation )
		{
			g_ProteinBox_Conf_Old = g_ProteinBox_Conf_TranshipmentStation ;
			g_ProteinBox_Conf_TranshipmentStation = NULL;
		}

		bRet = CDCreateTotalHead( (PVOID) &g_ProteinBox_Conf_TranshipmentStation );
		g_ProteinBox_Conf_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitConfigData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
CDCreateTotalHead(
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
	LPCDHEAD *pTotalHead = (LPCDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// 为总结构体分配内存
	*pTotalHead = (LPCDHEAD) kmalloc( sizeof( CDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | CDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( CDHEAD ) );
	
	// 初始化资源锁
	bRet = InitResource( &((LPCDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDCreateTotalHead() - InitResource(); 申请资源锁内存失败! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// 初始化链表头
	InitializeListHead( (PLIST_ENTRY)&( (LPCDHEAD)*pTotalHead )->SectionListHead );
	InitializeListHead( (PLIST_ENTRY)&(( (LPCDHEAD)*pTotalHead )->SectionListHead).KeyListHead );
	InitializeListHead( (PLIST_ENTRY)&((( (LPCDHEAD)*pTotalHead )->SectionListHead).KeyListHead).ValueListHead );

	((LPCDHEAD)*pTotalHead)->SectionCounts = 0 ;

	return TRUE ;
}



VOID 
CDDeleteTotalHead(
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
CDAllocateNode(
	OUT PVOID* pCurrenList,
	IN ULONG StructSize
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
	if ( NULL == pCurrenList || StructSize <= 0 )
	{
		dprintf( "error! | CDAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pCurrenList = (PVOID) kmalloc( StructSize );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | CDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, StructSize );
	return TRUE ;
}



BOOL
CDBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  建立指定Section -> Key -> Value 
    
Arguments:
  _TotalHead - 配置文件的链表头
  pInBuffer - 要建立的节点信息
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG Type ;
	SEARCH_INFO OutBuffer = { 0 };
	LPCDNODES pSectionNode	= NULL ;
	LPCDNODEK pKeyNode		= NULL ;
	LPCDNODEV pValueNode	= NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataW lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataW) pInBuffer ;
	
	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == lpData )
	{
		dprintf( "error! | CDBuildNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 查找是否是已存在的节点
	bRet = CDFindNode( _TotalHead, pInBuffer, &OutBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDBuildNode() - CDFindNode(); | \n" );
		return FALSE ;
	}

	switch ( OutBuffer.NodeType )
	{
	case _NODE_IS_NOTHING_ :
		{
			// 表明Section & Key & Value节点都未建立
			pSectionNode = (LPCDNODES) CDBuildNodeEx( 
				pTotalHead->QueueLockList,
				(PVOID)&pTotalHead->SectionListHead, 
				sizeof(CDNODES), 
				lpData->SeactionName,
				lpData->SeactionNameLength
				);
			
			if ( NULL == pSectionNode )
			{
				dprintf( "error! | CDBuildNode() - GLBuildNodeS(); | (case _NODE_IS_NOTHING_) NULL == pSectionNode \n" );
				return FALSE ;
			}

			pKeyNode = (LPCDNODEK) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pSectionNode->KeyListHead, sizeof(CDNODEK), lpData->KeyName, lpData->KeyNameLength );
			if ( NULL == pKeyNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_NOTHING_) NULL == pKeyNode \n" );
				return FALSE ;
			}

			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_NOTHING_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_SECTION_ :
		{
			// 表明只有对应的Section节点,而Key & Value节点还未建立
			pSectionNode = (LPCDNODES) OutBuffer.pNode ;
			pKeyNode = (LPCDNODEK) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pSectionNode->KeyListHead, sizeof(CDNODEK), lpData->KeyName, lpData->KeyNameLength );
			if ( NULL == pKeyNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_SECTION_) NULL == pKeyNode \n" );
				return FALSE ;
			}

			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_SECTION_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_KEY_ :
		{
			// 表明只有对应的Section & Key节点,而Value节点还未建立
			pKeyNode = (LPCDNODEK) OutBuffer.pNode ;
			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_KEY_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_VALUE_ :
		{
			dprintf( 
				"ok! | CDBuildNode() - CDFindNode(); | 当前节点(Section:%ws; Key:%ws; Value:%ws)已存在于链表中,直接返回 \n",
				lpData->SeactionName, lpData->KeyName, lpData->ValueName
				);

			return TRUE ;
		}
		break ;
	
	default :
		dprintf( "error! | CDBuildNode();  | error Type! \n" );
		return FALSE ;
	}

	return TRUE ;
}



PVOID
CDBuildNodeEx (
	IN PERESOURCE QueueLockList,
	IN PVOID _ListHead,
	IN ULONG StructSize,
	IN LPWSTR wszName,
	IN int   NameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  新建Value节点.eg: c:\Test.ext    
    
Arguments:
  _ListHead - 链表头
  StructSize - 要新建的结构体大小
  SectionName - 待填充的节名
  NameLength - 待填充的节名长度

Return Value:
  新建的GL结构体指针
    
--*/
{
	BOOL bRet = FALSE ;
	LPPB_CONFIG_COMMON pNode = NULL ;
	LPPB_CONFIG_COMMON ListHead = (LPPB_CONFIG_COMMON) _ListHead ;

	//
	// 1. 校验参数合法性
	//

	if ( NULL == QueueLockList || NULL == ListHead || NULL == wszName || NameLength <= 0 || StructSize <= 0 )
	{
		dprintf( "error! | CDBuildNodeEx() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. 分配内存,填充节点
	//

	bRet = CDAllocateNode( &pNode, StructSize );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDBuildNodeEx() - CDAllocateNode() | 申请内存失败 \n" );
		return NULL ;
	}

	pNode->wszName = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszName )
	{
		dprintf( "error! | CDBuildNodeEx() - kmalloc(); | 申请内存失败! \n" );
		kfree( (PVOID)pNode ); // 释放内存
		return NULL ;
	}

	pNode->NameLength = NameLength ;
	RtlZeroMemory( pNode->wszName, NameLength );
	RtlCopyMemory( pNode->wszName, wszName, NameLength );

	//
	// 3. 若需要,则插入新节点到链表尾
	//

	EnterCrit( QueueLockList );	// 加锁访问

	if ( 0 == ListHead->NameLength && NULL == ListHead->wszName ) 
	{
		// 链表头未被使用.use it
		*ListHead = *pNode ;
		InitializeListHead( (PLIST_ENTRY)ListHead );

		kfree( (PVOID)pNode ); // 释放内存
		pNode = ListHead ;
	}
	else
	{
		InsertTailList( (PLIST_ENTRY)ListHead, (PLIST_ENTRY)pNode );
	}

	LeaveCrit( QueueLockList );	// 释放锁
	return (PVOID)pNode ;
}



BOOL
CDFindNode (
	IN PVOID _TotalHead ,
	IN PVOID pInBuffer,
	OUT PVOID pOutBuffer
	)
{
	LPCDNODES pSectionHead = NULL, pSectionNode = NULL ;
	LPCDNODEK pKeyHead = NULL, pKeyNode = NULL ;
	LPCDNODEV pValueHead = NULL, pValueNode = NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataW lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataW) pInBuffer ;
	LPSEARCH_INFO pInfo = (LPSEARCH_INFO) pOutBuffer ;

	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == lpData || NULL == pInfo )
	{
		dprintf( "error! | CDFindNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	pInfo->NodeType = _NODE_IS_NOTHING_ ;
	pInfo->pNode	= NULL ;

	// 2. 遍历链表,查找指定节点

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	pSectionNode = pSectionHead = &pTotalHead->SectionListHead ;

	// 2.1 遍历Section链表
	do 
	{
		if ( (lpData->SeactionNameLength == pSectionNode->NameLength) && (0 == _wcsicmp( lpData->SeactionName, pSectionNode->SectionName )) ) 
		{
			pInfo->NodeType = _NODE_IS_SECTION_ ;
			pInfo->pNode	= (PVOID) pSectionNode ;

			pKeyNode = pKeyHead = &pSectionNode->KeyListHead ;
			if ( NULL == pKeyHead || FALSE == MmIsAddressValid( (PVOID)pKeyHead ) ) { break ; }

			// 2.2 已匹配到Section,遍历Key链表
			do
			{
				if ( (lpData->KeyNameLength == pKeyNode->NameLength) && (0 == _wcsicmp( lpData->KeyName, pKeyNode->KeyName )) ) 
				{
					pInfo->NodeType = _NODE_IS_KEY_ ;
					pInfo->pNode	= (PVOID) pKeyNode ;
					
					pValueNode = pValueHead = &pKeyNode->ValueListHead ;
					if ( NULL == pValueHead || FALSE == MmIsAddressValid( (PVOID)pValueHead ) ) { break ; }

					// 2.3 已匹配到Key,遍历Value链表
					do
					{
						if ( (lpData->ValueNameLength == pValueNode->NameLength) && (0 == _wcsicmp( lpData->ValueName, pValueNode->ValueName )) ) 
						{
							pInfo->NodeType = _NODE_IS_VALUE_ ;
							pInfo->pNode	= (PVOID) pValueNode ;

							break ;
						}

						pValueNode = pValueNode->pBlink ;
					} while( pValueNode != pValueHead );
					break ;
				}

				pKeyNode = pKeyNode->pBlink ;
			} while( pKeyNode != pKeyHead );
			break ;
		}

		pSectionNode = pSectionNode->pBlink ;
	} while ( pSectionNode != pSectionHead );

	LeaveCrit( pTotalHead->QueueLockList );	// 释放锁

	return TRUE ;
}



BOOL
CDFindNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName,
	OUT PVOID pOutBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/09 [9:7:2010 - 14:33]

Routine Description:
  该函数主要是查找Name对应的节点.当要找SeactionName的节点时,会将KeyName & ValueName置空;
  当要找KeyName的节点时,会将ValueName置空; 当要找ValueName的节点时,则没有空的
    
--*/
{
	BOOL bRet = FALSE ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW InBuffer = { 0 } ;

	// 1. 校验参数合法性
	if ( NULL == _TotalHead || NULL == SeactionName || NULL == pOutBuffer )
	{
		dprintf( "error! | IsValueNameExist(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 匹配之
	InBuffer.SeactionName = SeactionName ;
	InBuffer.SeactionNameLength = ( wcslen(SeactionName) + 1 ) * sizeof(WCHAR) ; 

	if ( KeyName )
	{
		InBuffer.KeyName = KeyName ;
		InBuffer.KeyNameLength =  ( wcslen(KeyName) + 1 ) * sizeof(WCHAR) ; 
	}
	
	if ( ValueName )
	{
		InBuffer.ValueName = ValueName ;
		InBuffer.ValueNameLength = ( wcslen(ValueName) + 1 ) * sizeof(WCHAR) ; 
	}

	bRet = CDFindNode( _TotalHead, &InBuffer, pOutBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDFindNodeEx() - CDFindNode(); | \n" );
		return FALSE ;
	}

	return TRUE ;
}



VOID
CDDistroyList (
	IN PVOID ListHead,
	IN ULONG DeepCounts
	)
{
	LPPB_CONFIG_COMMONEX pNode = NULL, pHead = NULL ;

	// 1. 校验参数合法性
	if ( NULL == ListHead || DeepCounts > g_DeepCounts )
	{
		dprintf( "error! | CDDistroyList(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	if ( 0 == DeepCounts ) { return ; }

	// 2.释放当前链表及其子链表
	pNode = pHead = (LPPB_CONFIG_COMMONEX) ListHead ;

	do
	{
		kfree( (PVOID)pNode->wszName ); // 2.1 清除当前节点中的字符串内存

		CDDistroyList( &pNode->SunList, DeepCounts - 1 ); // 2.2 清除当前节点的子链表

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = (LPPB_CONFIG_COMMONEX) pHead->List.Flink ;

	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );
}



VOID
CDDistroyAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 18:08]

Routine Description:
  驱动卸载时调用此函数
    
--*/
{
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	// 2. 删除所有的子节点
	AbortWait_for_DriverUnload();
	CreateConfigThread( TRUE );

	CDWalkNodes( _TotalHead );

	dprintf( "*** 开始卸载ConfigData节点管理器 *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	CDDistroyList( &pTotalHead->SectionListHead, 3 );
	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 4. 删除总节点
	CDDeleteTotalHead( &_TotalHead );

	dprintf( "*** 结束卸载ConfigData节点管理器 *** \n" );
	return ;
}




VOID
CDDistroyAllEx (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 18:08]

Routine Description:
  R3通过PBStart.exe /Reload命令时,最终会走进这里,清除旧的链表
    
--*/
{
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	// 2. 删除所有的子节点
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问
	CDDistroyList( &pTotalHead->SectionListHead, 3 );
	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 4. 删除总节点
	CDDeleteTotalHead( &_TotalHead );
	return ;
}




VOID
CDWalkNodes (
	IN PVOID _TotalHead
	)
{
	LPCDNODES pSectionHead = NULL, pSectionNode = NULL ;
	LPCDNODEK pKeyHead = NULL, pKeyNode = NULL ;
	LPCDNODEV pValueHead = NULL, pValueNode = NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

#ifdef DBG

	// 1. 校验参数合法性
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	dprintf( "\n**** Starting walking ConfigDatas **** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// 加锁访问

	pSectionNode = pSectionHead =  &pTotalHead->SectionListHead ;

	// 2.1 遍历Section链表
	do 
	{
		dprintf( "\n[%ws]", pSectionNode->SectionName );

		pKeyNode = pKeyHead = &pSectionNode->KeyListHead ;
		if ( NULL == pKeyHead || FALSE == MmIsAddressValid( (PVOID)pKeyHead ) ) { break ; }

		// 2.2 遍历Key链表
		do
		{
			dprintf( "\n%ws=", pKeyNode->KeyName );

			pValueNode = pValueHead = &pKeyNode->ValueListHead ;
			if ( NULL == pValueHead || FALSE == MmIsAddressValid( (PVOID)pValueHead ) ) { break ; }

			// 2.3 遍历Value链表
			do
			{
				dprintf( "%ws,", pValueNode->ValueName );
				pValueNode = pValueNode->pBlink ;

				if ( NULL == pValueNode ) { break ; }
			} while( pValueNode != pValueHead );

			pKeyNode = pKeyNode->pBlink ;
			if ( NULL == pKeyNode ) { break ; }

		} while( pKeyNode != pKeyHead );

		dprintf( "\n" );
		pSectionNode = pSectionNode->pBlink ;
		if ( NULL == pSectionNode ) { break ; }

	} while ( pSectionNode != pSectionHead );

	LeaveCrit( pTotalHead->QueueLockList ); // 释放锁
	dprintf( "\n**** End of walking ConfigDatas **** \n\n" );

#endif
	return ;
}



BOOL
Is_ValueName_Exist (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:37]

Routine Description:
  查看配置文件中是否存在指定的ValueName,是则返回TRUE  
    
Arguments:
  ValueName - 待查找的字符串
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG Type = -1 ;
	PVOID pNode = NULL ;
	SEARCH_INFO InBuffer = { 0 } ;

	// 1. 校验参数合法性
	if ( NULL == _TotalHead || NULL == SeactionName || NULL == KeyName || NULL == ValueName )
	{
		dprintf( "error! | Is_ValueName_Exist(); | Invalid Paramaters; failed! (_TotalHead=%x) (KeyName=%ws) (ValueName=%ws) \n", _TotalHead, KeyName, ValueName );
		return FALSE ;
	}

	// 2. 匹配之
	bRet = CDFindNodeEx( _TotalHead, SeactionName, KeyName, ValueName, &InBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Is_ValueName_Exist() - CDFindNodeEx(); | \n" );
		return FALSE ;
	}

	if ( _NODE_IS_VALUE_ != InBuffer.NodeType ) { return FALSE ; }
	return TRUE ;
}


static char* g_InjectDllPath = NULL;

BOOL 
GetPBDllPath(
	IN ULONG uMethodType,
	IN ULONG MaxLength,
	OUT LPSTR* szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/02 [2:1:2012 - 10:46]

Routine Description:
  获取主要功能模块Proteinboxdll.dll的全路径.
  可以通过 @uMethodType 指定获取方式
    
Arguments:
  uMethodType - 1: 从注册表获取; 2: 从配置文件获取
  MaxLength - 数据最大长度
  szPath - 保存获取到的数据
    
--*/
{
	BOOL bRet = FALSE, bAppend = FALSE;
	CHAR szFinalPath[ MAX_PATH + 0x20 ] = "";

	// 1. 校验参数合法性
	if ( FALSE == IsPBDLLPathTypeValid(uMethodType) || NULL == szPath )
	{
		dprintf( "error! | GetPBDllPath(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( g_InjectDllPath ) { goto _END_; }
	
	// 2.
	switch (uMethodType)
	{
	case PBDLLPathType_Reg :
		bRet = GetPBDllPathFromReg( MaxLength, szFinalPath );
		break;

// 	case PBDLLPathType_Ini :
// 		bRet = GetPBDllPathFromIni( MaxLength, szFinalPath );
// 		break;

	default:
		break;
	}

	if ( FALSE == bRet || NULL == szFinalPath )
	{
		dprintf( "error! | GetPBDllPath() - GetPBDllPathFromReg; | \n" );
		return FALSE ;
	}

	if ( strncmp(szFinalPath, "\\??\\", 4) )
	{
		// 格式不对, 要把"c:\\" 换成"\\??\\c:\\"
		if ( ':' != szFinalPath[1] ) 
			return FALSE;

		bAppend = TRUE;
	}
	
	// 是第一次查询
	g_InjectDllPath = (char*) kmallocMM( MaxLength + 0x30, MTAG___InjectDllPath );
	if ( NULL == g_InjectDllPath ) 
	{ 
		dprintf( "error! | GetPBDllPath() - kmallocMM(); | \n" );
		return FALSE; 
	}

	RtlZeroMemory( g_InjectDllPath, MaxLength + 0x30 );

	if (bAppend)
	{
		memcpy( g_InjectDllPath, "\\??\\", 4 );
		strcat( g_InjectDllPath, szFinalPath );
	} 
	else
	{
		strcpy( g_InjectDllPath, szFinalPath );
	}

_END_:
	*szPath = g_InjectDllPath;
	return TRUE;
}


BOOL 
GetPBDllPathFromReg(
	IN ULONG MaxLength,
	OUT LPSTR szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/02 [2:1:2012 - 10:46]

Routine Description:
  读取注册表,获取主要功能模块Proteinboxdll.dll的全路径.
    
Arguments:
  MaxLength - 数据最大长度
  szPath - 保存获取到的数据
    
--*/
{
	BOOL bRet = FALSE;
	HANDLE KeyHandle = NULL;
	LPWSTR pData = NULL;
	char szBuffer[ MAX_PATH * 2 ] = "";
	ULONG Length = 0, ResLength = 0;
	PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo = NULL ;
	UNICODE_STRING KeyName, uniBuffer ;
	OBJECT_ATTRIBUTES objattri ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	// 1. 校验参数合法性
	if ( NULL == szPath )
	{
		dprintf( "error! | GetPBDllPathFromReg(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	RtlInitUnicodeString( &KeyName, L"RootFolder" );
	RtlInitUnicodeString( &uniBuffer, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Proteinbox\\Config" );

	InitializeObjectAttributes (
		&objattri,
		&uniBuffer,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ, &objattri );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | GetPBDllPathFromReg() - ZwOpenKey(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, 0, 0, &ResLength );
	if( STATUS_BUFFER_TOO_SMALL != status )
	{
		dprintf( "error! | GetPBDllPathFromReg() - ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

	ResLength += sizeof( KEY_VALUE_PARTIAL_INFORMATION );
	KeyValuePartialInfo = kmalloc( ResLength );
	Length = ResLength ;

	if( NULL == KeyValuePartialInfo ) 
	{
		dprintf( "error! | GetPBDllPathFromReg() - kmalloc(); | 分配内存失败,大小:%d \n", ResLength );
		goto _END_ ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, (PVOID)KeyValuePartialInfo, Length, &ResLength );
	if( ! NT_SUCCESS(status) || (REG_SZ != KeyValuePartialInfo->Type) )
	{
		dprintf( "error! | GetPBDllPathFromReg() -  ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _FREEMEMORY_ ;
	}

	pData = (LPWSTR)KeyValuePartialInfo->Data;
	if ( NULL == pData || !pData[0] || KeyValuePartialInfo->DataLength + 0x20 > MaxLength )
	{
		dprintf( "error! | GetPBDllPathFromReg();| 查询到的内容不合法 \n" );
		goto _FREEMEMORY_ ;
	}

	bRet = w2a( pData, szBuffer, MAX_PATH * 2 );
	if ( FALSE == bRet ) 
	{ 
		dprintf( "error! | GetPBDllPathFromReg() - w2a(); | wszPath:%ws \n", pData );
		goto _FREEMEMORY_ ;
	}

	strcpy( szPath, szBuffer );
	strcat( szPath, "\\ProteinBoxDLL.dll" );

_FREEMEMORY_ :
	kfree( KeyValuePartialInfo );
_END_ :
	ZwClose( KeyHandle );
	return TRUE ;
}


BOOL GetPBDllPathFromIni( OUT LPSTR* lpszPath, IN BOOL bReload )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/21 [21:12:2011 - 14:11]

Routine Description:
  查询配置文件，获取ProteinboxDll.dll的完整路径，保存至全局变量中。
    
Arguments:
  lpwszPath - 保存获取到的待注入的DLL全路径; 可以为NULL
  bReload - TRUE 表明要重新查询 / FALSE 表明如果不是第一次查询，就不用再查了，直接用以前保存的值即可

Return Value:
  TRUE / FALSE
    
--*/
{
	BOOL bRet = FALSE, bNeedQuery = FALSE ;
	WCHAR wszPath[ MAX_PATH ] = L"";

	do 
	{
		if ( NULL == g_InjectDllPath ) 
		{
			// 是第一次查询
			g_InjectDllPath = (char*) kmallocMM( MAX_PATH + 2, MTAG___InjectDllPath );
			if ( NULL == g_InjectDllPath ) 
			{ 
				dprintf( "error! | GetPBDllPathFromIni() - kmallocMM(); | \n" );
				return FALSE; 
			}

			bNeedQuery = TRUE;
		}

		if ( FALSE == bReload && FALSE == bNeedQuery )
			break;

		RtlZeroMemory( g_InjectDllPath, MAX_PATH );

		bRet = GetPBDllPathFromIniW( wszPath );
		if ( FALSE == bRet ) 
		{ 
			dprintf( "error! | GetPBDllPathFromIni() - GetPBDllPathFromIniW(); | \n" );
			return FALSE; 
		}

		bRet = w2a( wszPath, g_InjectDllPath, MAX_PATH );
		if ( FALSE == bRet ) 
		{ 
			dprintf( "error! | GetPBDllPathFromIni() - w2a(); | wszPath:%ws \n", wszPath );
			return FALSE; 
		}

	} while (FALSE);

 	if ( lpszPath ) { *lpszPath = g_InjectDllPath; }
	dprintf( "ok! | GetPBDllPathFromIni();| Inject dll path:\"%s\"  \n", g_InjectDllPath );
	return TRUE;
}



BOOL GetPBDllPathFromIniW( OUT LPWSTR lpwszPath )
{
	BOOL bRet = FALSE ;

	// 配置文件已被重新加载，查询之
	bRet = kGetConfSingle( L"GlobalSetting", L"InjectDllPath", lpwszPath );
	if ( FALSE == bRet ) 
	{ 
		dprintf( "error! | GetPBDllPathFromIniW() - kGetConfSingle(); | get InjectDllPath failed \n" );
		return FALSE; 
	}

	return TRUE;
}



///////////////////////////////   END OF FILE   ///////////////////////////////