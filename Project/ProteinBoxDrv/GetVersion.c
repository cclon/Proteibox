/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 15:30]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\GetVersion.c
* 
* Description:
*      
*   取得驱动版本信息的主模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "GetVersion.h"
#include "DispatchIoctl.h"

//////////////////////////////////////////////////////////////////////////

#define g_Sys_VersionA  "1.0.1"		// 当前驱动版本号
#define g_Sys_VersionW  L"1.0.1"	// 


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
Ioctl_GetSysVersion (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 16:11]

Routine Description:
  取得驱动版本信息  
    
--*/
{
	ULONG Length = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PIOCTL_GETSYSVERSION_BUFFER lpData = (PIOCTL_GETSYSVERSION_BUFFER) getIoctlBufferBody(pInBuffer) ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pInBuffer || NULL == lpData )
	{
		dprintf( "error! | Ioctl_GetSysVersion(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	//
	// 2. 取驱动版本信息
	//

	Length = (wcslen (g_Sys_VersionW) + 1) * sizeof(WCHAR) ;

	__try
	{
		if ( IsWrittenKernelAddr(lpData->wszVersion, 0x10) ) 
		{
			dprintf( "error! | Ioctl_GetSysVersion() - IsWrittenKernelAddr(); | 参数不合法,待写入的地址:0x%08lx \n", lpData->wszVersion );
			return STATUS_INVALID_PARAMETER_4 ;
		}

		ProbeForWrite( lpData->wszVersion, Length, 2 );
		memcpy( lpData->wszVersion, &g_Sys_VersionW, Length );
		status = STATUS_SUCCESS ;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	    dprintf( "error! | Ioctl_GetSysVersion(); | __try __except \n" );
	}

	return status ;
}






///////////////////////////////   END OF FILE   ///////////////////////////////