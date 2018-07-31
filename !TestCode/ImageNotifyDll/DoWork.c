/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/10 [10:5:2010 - 14:01]
* MODULE : D:\Work\Program\Coding\沙箱\SandBox\Code\!TestCode\ImageNotifyDll\DoWork.c
* 
* Description:
*   
*   驱动在ImageNotify里注入DLL到指定的进程.方式为修改内存中的PE结构                     
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <ntddk.h>

#include "struct.h"
#include "DoWork.h"
#include "InjectDll.h"
#include "Common.h"
#include "DllData.h"

//////////////////////////////////////////////////////////////////////////

IMAGENOTIY_INFO g_ImageNotify_Info = { FALSE, NULL } ;


//////////////////////////////////////////////////////////////////////////

void
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
	NTSTATUS status ;

	//
	// 注册模块加载回调
	//

	if ( TRUE == bWork )
	{	
		if ( TRUE == CheckNotifyState() ) { return ; }

		// 初始化获取SSDT函数地址
		if ( FALSE == InitSSDT() ) { return ; }

		// 释放测试驱动到C盘根目录
		status = PutFile( g_szDllPathW, g_dll_data, sizeof(g_dll_data) ) ;
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "PutFile(); Failed: 0x%08lx \n", status );
			return ;
		}

		// 注册模块回调
		status = PsSetLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "PsSetLoadImageNotifyRoutine(); Failed: 0x%08lx \n", status );
			g_ImageNotify_Info.bNotifyState = FALSE ;
			return ;
		}

		g_ImageNotify_Info.bNotifyState = TRUE ;
		dprintf( "*** ImageNotify Set OK! \n" );
	}

	//
	// 卸载模块加载回调
	//

	else
	{
		if ( FALSE == CheckNotifyState() ) { return ; }

		PsRemoveLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );

		g_ImageNotify_Info.bNotifyState  = FALSE	;
		g_ImageNotify_Info.NotifyRoutine = NULL		;

		dprintf( "*** ImageNotify Remove OK! \n" );
	}

	return ;
}



BOOL 
CheckNotifyState (
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
	BOOL bNotifyState = g_ImageNotify_Info.bNotifyState ;

	if ( NULL == g_ImageNotify_Info.NotifyRoutine )
	{
		g_ImageNotify_Info.NotifyRoutine = (PVOID) ImageNotifyRoutine ;
	}

	return bNotifyState ;
}



//
// 关注的进程列表 [在列表中的进程会被注入sudami.dll]
//

static const LPCSTR g_special_processName_Array [] = 
{
	"C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE" ,
	"C:\\WINDOWS\\NOTEPAD.EXE"
};


BOOL
Is_special_process (
	IN PUNICODE_STRING  FullImageName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  匹配名称,是否为我们关注的进程
    
Arguments:
  FullImageName - [IN] 待匹配的字符串; 格式形如:
  "\Device\HGFS\vmware-host\Shared Folders\Transhipment station\1.exe"
  "\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume2\1.exe"
  "\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume1\1.exe"
  "\Device\HarddiskVolume1\Documents and Settings\KHacker\1.exe"
  ...

Return Value:
  是否匹配成功
    
--*/
{
	int		i			= 0		;
	BOOL	bRet		= FALSE ;
	PCHAR	pImagePath	= NULL	;
	CHAR szDosPath[ MAX_PATH ] = "" ;
	ANSI_STRING  ansiNameString = { 0, 0, NULL } ;

	//
	// 1. 验证参数合法性
	//

	if ( (NULL == FullImageName) || (0 == FullImageName->Length) && (FALSE == MmIsAddressValid( (PVOID)FullImageName->Buffer )) )
	{
		dprintf( "error! | Is_special_process(); Invalid Paramaters. \n" );
		return FALSE ;
	}
	
	//
	// 2. 转换字符串UNICODE 为 ANSI 格式
	//

	RtlZeroMemory( &ansiNameString, sizeof(ansiNameString) );
	RtlUnicodeStringToAnsiString( &ansiNameString, FullImageName, TRUE );

	pImagePath = ansiNameString.Buffer ;
	if ( FALSE == MmIsAddressValid( (PVOID)pImagePath ) )
	{
		dprintf( "error! | Is_special_process(); NULL == pImagePath; \n" );
		return FALSE ;
	}

	//
	// 3. 将nt路径转换为Dos路径;即 "\Device\HarddiskVolume1\xx" --> "c:\xx"
	//

	NtPath2DosPathA( pImagePath, szDosPath, MAX_PATH );

	//
	// 4. 匹配之
	//

	for( i = 0; i < ARRAYSIZEOF( g_special_processName_Array ); ++ i )
	{
		if ( 0 == _stricmp( szDosPath, g_special_processName_Array[ i ] ) )
		{
			bRet = TRUE ;
			break ;
		}
	}

	if ( ansiNameString.Buffer ) { RtlFreeAnsiString( &ansiNameString ); } // 释放内存
	
	return bRet ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                       模块回调函数                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


void
ImageNotifyRoutine (
  IN PUNICODE_STRING  FullImageName,
  IN HANDLE  ProcessId,
  IN PIMAGE_INFO  ImageInfo
  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	PROCESS_BASIC_INFORMATION ProcessBasicInfo;
	ULONG ReturnLength		= 0x18	;
	ULONG ImageBaseAddress	= 0		;
	
	//
	// 1. 校验参数合法性
	//

	if ( (NULL == ProcessId) || (TRUE == ImageInfo->SystemModeImage ) ) { return ; }

	//
	// 2. 黑名单过滤
	//

	if ( FALSE == Is_special_process( FullImageName ) ) { return ; }

	//
	// 3. 进一步验证是否适合注入
	//

	status = ZwQueryInformationProcess(
		(HANDLE) -1 ,
		ProcessBasicInformation ,
		&ProcessBasicInfo ,
		sizeof( ProcessBasicInfo ) ,
		&ReturnLength
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "ImageNotifyRoutine() - ZwQueryInformationProcess(); Failed: 0x%08lx \n", status );
		return ;
	}

	if ( NULL == ProcessBasicInfo.PebBaseAddress )
	{
		dprintf( "ImageNotifyRoutine(); NULL == ProcessBasicInfo.PebBaseAddress; \n" );
		return ;
	}

	ProbeForRead( (LPVOID)((ULONG)ProcessBasicInfo.PebBaseAddress + 8), 4, 4 );

	ImageBaseAddress = *(PULONG) ((ULONG)ProcessBasicInfo.PebBaseAddress + 8) ;

	if ( ImageBaseAddress != (ULONG)ImageInfo->ImageBase )
	{
		dprintf ( 
			"ImageNotifyRoutine(); ImageBaseAddress != ImageInfo->ImageBase; \n"
			"ImageBaseAddress: 0x%08lx \n"
			"ImageInfo->ImageBase: 0x%08lx \n",
			ImageBaseAddress,
			ImageInfo->ImageBase
			);

		return ;
	}

	if ( 0 == ProcessBasicInfo.InheritedFromUniqueProcessId )
	{
		dprintf( "ImageNotifyRoutine(); 0 == ProcessBasicInfo.InheritedFromUniqueProcessId; \n" );
		return ;
	}

	//
	// 4. 关键一句,注入DLL
	//

	dprintf( "Starting Inject DLL with modifying PE ...\n" );

	status = InjectDll_by_reconstruct_pe( ImageBaseAddress ); 
	if( NT_SUCCESS( status ) )
	{
		dprintf( "[ImageNotify Reconstruct PE]   Inject \"%s\",Done! (╰_╯)#  \n", g_szDllPathA );
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
