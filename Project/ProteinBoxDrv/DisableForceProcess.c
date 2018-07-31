/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 17:42]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\StartProcess.c
* 
* Description:
*      
*   和强制运行相关 [代码写的很难懂啊, 0 0!]                
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "DispatchIoctl.h"
#include "ProcessData.h"
#include "SandboxsList.h"
#include "ConfigData.h"
#include "ForceRunList.h"
#include "DisableForceProcess.h"

//////////////////////////////////////////////////////////////////////////

#define MAX_FORPATH_LENGTH 0x2000

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS
Ioctl_DisableForceProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/11 [11:7:2011 - 16:20]

Routine Description:
  控制 强制程序运行进程 的开关  
    
--*/
{
	BOOL bRet = FALSE, bRunInSandbox = FALSE, bNeedForcePath = FALSE ;
	LPWSTR szFullImageName =  NULL ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPIOCTL_DISABLEFORCEPROCESS_BUFFER Buffer = NULL ;
	LPFORCEPROC_NODE_INFO lpData = NULL ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_DisableForceProcess(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_DISABLEFORCEPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_DisableForceProcess() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}

		// 判断Flag的值
		if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = TRUE ;
			g_bAllRunOutofSandbox	 = FALSE ;

			kEnableAllFPL();
		}
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = FALSE ;
			g_bAllRunOutofSandbox    = TRUE ;

			kDisableAllFPL();
		}
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_ == Buffer->Flag )
		{
			g_bAllRunOutofSandbox	 = FALSE ;
			bNeedForcePath = TRUE;
			bRunInSandbox = TRUE ;
		} 
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = FALSE ;
			bNeedForcePath = TRUE;
			bRunInSandbox = FALSE ;
		}
		else
		{
			dprintf( "error! | Ioctl_DisableForceProcess(); | 不合法的Flag:%n \n", Buffer->Flag );
			return STATUS_NOT_IMPLEMENTED;
		}

		if ( FALSE == bNeedForcePath )
		{
			return STATUS_SUCCESS;
		}

		// 需要取出进程全路径
		ProbeForRead( Buffer->szProcName, Buffer->NameLength, 2 );

		if ( Buffer->NameLength < 0 || Buffer->NameLength > MAX_FORPATH_LENGTH )
		{
			dprintf( "error! | Ioctl_DisableForceProcess((); | 字符串长度超过限制:length=%n \n", Buffer->NameLength );
			__leave ;
		}

		szFullImageName = (LPWSTR) kmalloc( MAX_FORPATH_LENGTH );
		if ( NULL == szFullImageName ) { return STATUS_UNSUCCESSFUL; }

		memcpy( szFullImageName, Buffer->szProcName, Buffer->NameLength );

		// 在配置文件中匹配路径
		bRet = kIsValueNameExist( SeactionName, L"ForceProcess", szFullImageName );
		if ( FALSE == bRet )
		{
			bRet = kIsValueNameExist( SeactionName, L"ForceFolder", szFullImageName );
		}

		if ( FALSE == bRet )
		{
			dprintf( "error! | Ioctl_DisableForceProcess() | 未匹配到强制运行的文件路径,请求无效 \n" );
			kfree( szFullImageName );
			return STATUS_UNSUCCESSFUL ;
		}

		// 新建节点
		lpData = kbuildnodeFPL( szFullImageName, Buffer->NameLength );
		if ( NULL == lpData ) 
		{
			dprintf( "error! | Ioctl_DisableForceProcess() - kbuildnodeFPL(); | 新建节点失败(%ws) \n", szFullImageName );
			kfree( szFullImageName );
			return STATUS_UNSUCCESSFUL ;
		}

		lpData->bRunInSandbox = bRunInSandbox ;	
		kfree( szFullImageName );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_DisableForceProcess() - __try __except(); | ProbeForWrite对应的地址不合法 \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////