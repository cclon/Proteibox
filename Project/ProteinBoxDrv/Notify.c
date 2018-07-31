/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 15:43]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\Notify.c
* 
* Description:
*      
*   各种回调的主模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "ProcessNofity.h"
#include "ImageNotify.h"
#include "InjectDll.h"


//////////////////////////////////////////////////////////////////////////

BOOL g_bMonitor_Notify = TRUE ; // 在R3告之R0,驱动中调用 Ioctl_HookShadow() 过后,会被置TRUE,表明开启Notify监控


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
SetNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  创建各种回调; (模块/进程)
    
Arguments:
  bInstall - TRUE - 安装; FALSE - 卸载 

--*/
{
	BOOL bRet = FALSE ;

	if ( TRUE == bInstall )
	{
		// 1. 创建模块回调
		bRet = SetImageNotify( TRUE );
		if ( FALSE == bRet )
		{
			dprintf( "error! | SetNotify() - SetImageNotify(); | \n" );
			return FALSE ;
		}

		// 2. 创建进程回调
		bRet = SetProcessNotify( TRUE );
		if ( FALSE == bRet )
		{
			dprintf( "error! | SetNotify() - SetProcessNotify(); | \n" );
			return FALSE ;
		}

		dprintf( "ok! | 创建各种回调; (模块/进程) \n" );
	}
	else
	{
		// 卸载各种回调
		bRet = SetImageNotify( FALSE );
		bRet = SetProcessNotify( FALSE );

		dprintf( "ok! | 卸载各种回调; (模块/进程) \n" );
	}

	return bRet ;
}



BOOL
SetImageNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  创建模块回调    
    
Arguments:
  bInstall - TRUE - 安装; FALSE - 卸载 

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	//
	// 注册模块加载回调
	//

	if ( TRUE == bInstall )
	{	
		if ( TRUE == CheckImageNotifyState() ) { return TRUE ; }

		// 初始化获取SSDT函数地址
		if ( FALSE == Get_ZwProtectVirtualMemory_addr() ) { return FALSE ; }	

		// 注册模块回调
		status = PsSetLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "error! | SetImageNotify() - PsSetLoadImageNotifyRoutine(); | (status=0x%08lx) \n", status );
			g_ImageNotify_Info.bNotifyState = FALSE ;
			return FALSE ;
		}

		g_ImageNotify_Info.bNotifyState = TRUE ;
		dprintf( "ok! | Set ImageNotify \n" );
	}

	//
	// 卸载模块加载回调
	//

	else
	{
		if ( FALSE == CheckImageNotifyState() ) { return TRUE ; }

		PsRemoveLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );

		g_ImageNotify_Info.bNotifyState  = FALSE	;
		g_ImageNotify_Info.NotifyRoutine = NULL		;

		dprintf( "ok! | Remove ImageNotify \n" );
	}

	return TRUE ;
}



BOOL
SetProcessNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  创建进程回调    
    
Arguments:
  bInstall - TRUE - 安装; FALSE - 卸载 

--*/
{

	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	//
	// 注册进程加载回调
	//

	if ( TRUE == bInstall )
	{	
		if ( TRUE == CheckProcessNotifyState() ) { return TRUE ; }

		// 注册模块回调
		status = PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine, FALSE );

		if ( STATUS_INVALID_PARAMETER == status )
		{
			status = __PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine );
		}

		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "error! | SetProcessNotify() - PsSetCreateProcessNotifyRoutine(); | (status=0x%08lx) \n", status );
			g_ProcessNotify_Info.bNotifyState = FALSE ;
			return FALSE ;
		}

		g_ProcessNotify_Info.bNotifyState = TRUE ;
		dprintf( "ok! | Set ProcessNotify \n" );
	}

	//
	// 卸载进程加载回调
	//

	else
	{
		if ( FALSE == CheckProcessNotifyState() ) { return TRUE ; }

		PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine, TRUE );

		g_ProcessNotify_Info.bNotifyState  = FALSE	;
		g_ProcessNotify_Info.NotifyRoutine = NULL	;

		dprintf( "ok! | Remove ProcessNotify \n" );
	}

	return TRUE ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////