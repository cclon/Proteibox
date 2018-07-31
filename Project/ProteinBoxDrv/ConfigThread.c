/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/06 [6:7:2010 - 16:04]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ConfigThread.c
* 
* Description:
*      
*   创建一个线程,其会一直等待R3; 应用层做好工作后会设置一个事件激活它,然后将配置信息保存于驱动中                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ConfigThread.h"
#include "Common.h"
#include "Config.h"
#include "ProcessData.h"

//////////////////////////////////////////////////////////////////////////

#define g_ConfEvent_InitConfigData_wakeup_R0	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R0"
#define g_ConfEvent_InitConfigData_wakeup_R3	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R3"


THREAD_INFO g_tmpThread_info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+						开始系统线程                          +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

VOID 
CreateConfigThread(
	IN BOOL bTerminate
	) 
{
	HANDLE ThreadHandle ;
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	
	if ( FALSE == bTerminate ) { // 创建进程
		
		if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
			THREAD_ALL_ACCESS,
			NULL,
			0L,
			NULL,
			ConfigThread,
			NULL))
		   ) 
		{
			dprintf("CreateConfigThread() -->PsCreateSystemThread. Failed\n");
			return;
		}
		
		ObReferenceObjectByHandle (ThreadHandle,
			THREAD_ALL_ACCESS,
			NULL,
			KernelMode,
			(PVOID *)&g_tmpThread_info.pThreadObj,
			NULL);
		
		//
		// 初始化该进程的结构体
		//

		g_tmpThread_info.bIamRunning = TRUE ;
		ZwClose( ThreadHandle );
	} 
	else // 销毁进程
	{  	
		if ( FALSE == g_tmpThread_info.bIamRunning ) { return ; } 

		// 若线程没有结束,需等待其结束,随后释放内存
		g_tmpThread_info.bIamRunning = FALSE ;

		ZwSetEvent( g_tmpThread_info.hEvent, 0 );  // 设置为"受信"状态,唤醒等待的线程
		
		if ( g_tmpThread_info.pThreadObj != NULL ) { // 等待线程的结束
			KeWaitForSingleObject( g_tmpThread_info.pThreadObj, Executive, KernelMode, FALSE, NULL );	
		}
		
		if ( g_tmpThread_info.pThreadObj != NULL ) { // 若 thread object存在,释放掉,要不会BSOD
			ObDereferenceObject( g_tmpThread_info.pThreadObj );
			g_tmpThread_info.pThreadObj = NULL;
		}
	}
	
	return ;
}



VOID
ConfigThread (
	IN PVOID StartContext
	)
{	
	BOOL bRet = FALSE ;

	// 打开等待事件
	bRet = OpenEvent( g_ConfEvent_InitConfigData_wakeup_R0, EVENT_ALL_ACCESS, &g_tmpThread_info.hEvent );
	if( FALSE == bRet )
	{
		dprintf( "error! | ConfigThread() - OpenEvent(); | \"%s\" \n", g_ConfEvent_InitConfigData_wakeup_R0 );
		goto _OVER_ ;
	}

	// 等待事件，一直到"受信"状态
	dprintf( "[PB] I'm Waiting for Config Data ... \n" );
	ZwWaitForSingleObject( g_tmpThread_info.hEvent, FALSE, NULL );

	if ( FALSE == g_tmpThread_info.bIamRunning ) { goto _CLEAR_ ; }
	
	// 若驱动未完成初始化操作,当前状态为不响应任何操作
	if ( FALSE == g_Driver_Inited_phrase1 )
	{
		dprintf( "error! | ConfigThread(); | 已收到R3传来激活命令,但驱动未完成初始化操作,当前状态为不响应任何操作. \n" );
		goto _CLEAR_ ; 
	}

	//
	// 驱动需要获取完整的配置文件信息,于是设置事件激活应用层的等待线程,
	// R3将全部的数据抛给R0后会置另一等待事件,让驱动结束等待,完成初始化.
	//

	bRet = LoadConfig();
	if( FALSE == bRet )
	{
		dprintf( "error! | ConfigThread() - LoadConfig(); | \n" );
		goto _CLEAR_ ;
	}

	if ( NULL == g_ProteinBox_Conf )
	{
		// 表明是首次加载配置文件
		g_ProteinBox_Conf = g_ProteinBox_Conf_TranshipmentStation ;
		dprintf( "ok! | ConfigThread(); | 配置文件初始化成功! \n" );
	}

	if ( g_ProteinBox_Conf != g_ProteinBox_Conf_TranshipmentStation )
	{
		// 表明是Reload配置文件的操作, 释放掉旧的数据链表
		g_ProteinBox_Conf = g_ProteinBox_Conf_TranshipmentStation ;

		CDDistroyAllEx( (PVOID)g_ProteinBox_Conf_Old );
		dprintf( "ok! | ConfigThread(); | 配置文件Reload成功! \n" );
	}

	kWalkCD();
//	GetPBDllPathFromIni( NULL, TRUE ); // Proteinboxdll.dll的全路径通过注册表获取,就不用在INI中设置了,太麻烦

_CLEAR_ :
	ZwClose ( g_tmpThread_info.hEvent );

_OVER_ :
	if ( g_ProteinBox_Conf != g_ProteinBox_Conf_TranshipmentStation )
	{
		// 如打开事件没有成功,那么临时申请的g_ProteinBox_Conf_TranshipmentStation这块内存必须释放
		CDDistroyAllEx( (PVOID)g_ProteinBox_Conf_TranshipmentStation );
	}

	g_tmpThread_info.bIamRunning = FALSE ;
	SetEvent( g_ConfEvent_InitConfigData_wakeup_R3, EVENT_MODIFY_STATE ); // 无论如何,都要激活等待的R3

	// 为了让所有进程结点中保存的配置信息更新，必须遍历所有沙箱进程，清空已有的配置数据。
	kConfResetPD();

	dprintf( "ConfigThread(); | 系统线程工作完成,退出之. \n" );
	PsTerminateSystemThread( STATUS_SUCCESS );
}


/////////////////////////////////////// END OF FILE ///////////////////////////////////////////