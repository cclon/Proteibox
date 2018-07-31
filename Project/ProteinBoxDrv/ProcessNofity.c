/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 16:24]
* MODULE : \SandBox\Code\Project\ProteinBoxDrv\ProcessNofity.c
* 
* Description:
*      
*   进程回调相关                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "ProcessNofity.h"
#include "ProcessData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

NOTIY_INFO g_ProcessNotify_Info = { FALSE, NULL } ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                       进程回调函数                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID
ProcessNotifyRoutine (
    IN HANDLE   hParentId,
    IN HANDLE   hProcessId,
    IN BOOLEAN  Create
    )
{
	BOOL bRet = FALSE ;
    PEPROCESS  pParent	= NULL ;
    PEPROCESS  pProcess = NULL ;
    NTSTATUS   status	= STATUS_UNSUCCESSFUL ;
	LPPDNODE   pNodeParent = NULL, pNode = NULL ;
	PNODE_INFO_C pNode_c = NULL ;

	if ( FALSE == g_Driver_Inited_phrase1 || FALSE == g_bMonitor_Notify ) { return ; }

    __try
    {
        status = PsLookupProcessByProcessId( hProcessId, &pProcess );

        if ( NT_SUCCESS(status) )
        {
			PsLookupProcessByProcessId( hParentId, &pParent );
        }

		if ( TRUE == Create )
		{
			//
			// 进程创建
			//

			if ( NULL == hParentId || NULL == hProcessId ) { return ; }

			// 1. 查找PID对应的进程总节点
			pNodeParent = (LPPDNODE) kgetnodePD( hParentId );
			if ( NULL == pNodeParent ) { return ; }
			if ( 1 == pNodeParent->bDiscard ) { return ; }

			// 2.1 创建进程节点
			pNode = (LPPDNODE) kbuildnodePD( hProcessId, FALSE );
			if ( NULL == pNode )
			{
				dprintf( "error! | ProcessNotifyRoutine() - kbuildnodePD(); | 创建进程节点失败(PID=%d) \n", hProcessId );
				return ;
			}

			// 2.2 填充C节点
			pNode->pNode_C = pNode_c = PDCopyNodeC( (PVOID)pNodeParent->pNode_C );
			if ( NULL == pNode_c )
			{
				dprintf( "error! | ProcessNotifyRoutine() - PDCopyNodeC(); | 复制进程节点C失败(PID=%d) \n", hProcessId );
				kfree( pNode ); // 还未插入链表,初始化失败则释放内存
				return ;
			}

			// 2.3 填充其他单元
			pNode->XIpcPath.bFlag_Denny_Access_KnownDllsSession = pNodeParent->XIpcPath.bFlag_Denny_Access_KnownDllsSession ;

			kInsertTailPD( pNode );
#if DBG
			{
				LPWSTR lpImageFileShortName = NULL ;
				PUNICODE_STRING lpImageFileName = NULL ;

				// 得到PID对应的进程路径,调用者负责释放内存
				bRet = GetProcessImageFileName( hProcessId, &lpImageFileName, &lpImageFileShortName );
				if ( bRet ) 
				{ 
					dprintf( "[ProcessCreate] 在沙箱进程链中新增节点(PID=%d, Name:%ws) \n", hProcessId, lpImageFileShortName );
					kfree( (PVOID)lpImageFileName ); // 释放内存
				}
			}
#endif
		}
		else
		{
			//
			// 进程销毁
			//

			if ( NULL == hProcessId ) { return ; }
			bRet = kRemovePD( hProcessId ); // 从进程链表中移除当前节点
#if DBG
			if ( TRUE == bRet )
			{
				dprintf( "[ProcessDelete] 在沙箱进程链中移除节点(PID=%d) \n", hProcessId );
			}
#endif
			kCheckPD( hProcessId ); // 校验进程链中其他PID的合法性
		}
    }
    __finally
    {
        if ( pParent )
        {
            ObDereferenceObject( (PVOID) pParent );

            pParent = NULL;
        }

        if ( pProcess )
        {
            ObDereferenceObject( (PVOID) pProcess );

            pProcess = NULL;
        }
    }

	return ;
}



BOOL 
CheckProcessNotifyState (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  检查模块回调的状态

Return Value:
  TRUE - 已注册; FALSE - 未注册
    
--*/
{
	BOOL bNotifyState = g_ProcessNotify_Info.bNotifyState ;

	if ( NULL == g_ProcessNotify_Info.NotifyRoutine )
	{
		g_ProcessNotify_Info.NotifyRoutine = (PVOID) ProcessNotifyRoutine ;
	}

	return bNotifyState ;
}



NTSTATUS
__PsSetCreateProcessNotifyRoutine (
	IN PVOID NotifyRoutine
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	dprintf( "call __PsSetCreateProcessNotifyRoutine(); \n" );

	return status ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////