/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/10 [10:5:2010 - 14:01]
* MODULE : D:\Work\Program\Coding\沙箱\SandBox\Code\!TestCode\ImageNotifyDll\DoWork.c
* 
* Description:
*   
*   干活的模块                   
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "DoWork.h"
#include "Version.h"
#include "Memory.h"
#include "Security.h"
#include "SecurityData.h"
#include "SdtData.h"
#include "SSDT.h"
#include "ShadowSSDT.h"
#include "Notify.h"
#include "ProcessData.h"
#include "RegHiveData.h"
#include "ObjectHook.h"
#include "ObjectData.h"
#include "Config.h"
#include "ShadowSSDT.h"
#include "HookEngine.h"
#include "SandboxsList.h"
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

BOOL g_Driver_Inited_phrase1 = FALSE ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID
DoWork (
	BOOL bWork
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  干活的函数

Arguments:
  bWork - [IN] 是否干活?

--*/
{
	if ( TRUE == bWork )
	{	
		Starting();
	}
	else
	{
		Stoping();
	}

	return ;
}



VOID
Starting (
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL	 bRet	= FALSE ;

	// 1. 初始化系统信息
	bRet = GetVersion () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - GetVersion(); \n" );
		return ;
	}

	GetProcessNameOffset();

	// 2.1 初始化内存管理器
	if ( FALSE == InitMemoryManager() )
	{
		dprintf( "error! | DoWork() - InitMemoryManager(); \n" );
		return ;
	}

	// 2.2 初始化进程节点管理器
	if ( FALSE == InitProcessData() )
	{
		dprintf( "error! | DoWork() - InitProcessData(); \n" );
		return ;
	}

	// 2.3 初始化SID节点管理器
	if ( FALSE == InitSecurityData() )
	{
		dprintf( "error! | DoWork() - InitSecurityData(); \n" );
		return ;
	}

	// 2.4 初始化RegHive节点管理器
	if ( FALSE == InitRegHive() )
	{
		dprintf( "error! | DoWork() - InitRegHive(); \n" );
		return ;
	}

	// 2.5 初始化ObjectHook节点管理器
	if ( FALSE == InitObjectData() )
	{
		dprintf( "error! | DoWork() - InitObjectData(); \n" );
		return ;
	}

	// 2.6 初始化配置文件相关信息
	if ( FALSE == InitConfig() )
	{
		dprintf( "error! | DoWork() - InitConfig(); \n" );
		return ;
	}

	// 2.7 初始化Inline Hook引擎
	if ( FALSE == LoadInlineHookEngine() )
	{
		dprintf( "error! | DoWork() - LoadInlineHookEngine(); \n" );
		return ;
	}

	// 2.8 初始化沙箱's 链表
	if ( FALSE == InitSandboxsData() )
	{
		dprintf( "error! | DoWork() - InitSandboxsData(); \n" );
		return ;
	}

	// 2.9 初始化强制运行功能's 链表
	if ( FALSE == InitForceProcData() )
	{
		dprintf( "error! | DoWork() - InitForceProcData(); \n" );
		return ;
	}

	//
	// 3. 创建一个安全描述符
	//

	bRet = CreateAcl() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - CreateAcl(); \n" );
		return ;
	}

	//
	// 4. 复制系统进程的句柄,赋予一定的权限
	//

	bRet = AdjustPrivilege() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - AdjustPrivilege(); \n" );
		return ;
	}

	//
	// 5. 准备SSDT相关数据
	//

	bRet = HandlerSSDT() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - HandlerSSDT(); \n" );
		return ;
	}

	//
	// 6. 创建各种回调; (模块/进程)
	//

	bRet = SetNotify( TRUE ) ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - SetNotify(); \n" );
		return ;
	}

	//
	// 7. 准备Object相关数据
	//

	bRet = HandlerObject () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - HandlerObject(); \n" );
		return ;
	}

	g_Driver_Inited_phrase1 = TRUE ;
	return ;
}



VOID
Stoping (
	)
{
	dprintf( " --------------- Unload Driver  --------------- \n" );

	SetNotify( FALSE ) ; // 卸载各种回调; (模块/进程)
	
	UnhookSSDT();
	UnhookShadowSSDT() ;
	UnloadInlineHookEngine();

	kfreeCD() ; // 释放配置文件相关信息
	kfreeOD() ; // 释放ObjectHookData节点链表
	kfreeRH() ; // 释放RegHive节点链表
	kfreeSD() ; // 释放SID节点链表
	kfreePD() ; // 释放进程节点管理器
	kfreeMP() ; // 释放MappedPE 相关内存数据
	kfreeSBL() ;
	kfreeFPL() ;
	kfreeMM() ; // 释放内存管理器 [这个总是最后一步]

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
