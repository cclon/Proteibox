/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 15:50]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ImageNotify.c
* 
* Description:
*      
*   模块回调相关                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "Common.h"
#include "InjectDll.h"
#include "ProcessData.h"
#include "Security.h"
#include "ConfigData.h"
#include "ImageNotify.h"

//////////////////////////////////////////////////////////////////////////

// [仅供调试用] 是否注入DLL到沙箱中的进程
#define _INJECT_DLL_		1

NOTIY_INFO g_ImageNotify_Info = { FALSE, NULL } ;


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
	LPPDNODE pNode = NULL ;
	LPWSTR szName = NULL ;
	WCHAR wszDosPath[ MAX_PATH ] = L""  ;

	//
	// 1. 校验参数合法性
	//

	if ( FALSE == g_Driver_Inited_phrase1 || FALSE == g_bMonitor_Notify ) { return ; }

	if ( (NULL == ProcessId) || (TRUE == ImageInfo->SystemModeImage ) ) { return ; }

	//
	// 2. 进一步验证是否适合注入
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
#if 0
		dprintf ( 
			"ImageNotifyRoutine(); ImageBaseAddress != ImageInfo->ImageBase; \n"
			"ImageBaseAddress: 0x%08lx \n"
			"ImageInfo->ImageBase: 0x%08lx \n",
			ImageBaseAddress,
			ImageInfo->ImageBase
			);
#endif
		return ;
	}

	if ( 0 == ProcessBasicInfo.InheritedFromUniqueProcessId )
	{
		dprintf( "ImageNotifyRoutine(); 0 == ProcessBasicInfo.InheritedFromUniqueProcessId; \n" );
		return ;
	}

	// 2.1 获取Dos格式的模块全路径
	if ( FullImageName && FullImageName->Length && FullImageName->Buffer )
	{
		szName = FullImageName->Buffer;
	}
	else
	{
		szName = L"unknown executable image" ;
	}

	NtPath2DosPathW( szName, wszDosPath, MAX_PATH );

	// 2.2 查找PID对应的进程总节点	
	pNode = (LPPDNODE) kgetnodePD( ProcessId );
	if ( NULL == pNode ) 
	{ 
		// 为空,表明此时启动的不是沙箱中进程创建的子进程,而是沙箱外部进程. 关注"强行被运行在沙箱中的"进程
		pNode = kbuildnodePDF( ProcessId, wszDosPath );
	}
	
	if ( NULL == pNode ) { return ; }
	
	InterlockedIncrement( &pNode->XImageNotifyDLL.IncrementCounts );

	if ( (1 == pNode->bDiscard) || (1 != pNode->XImageNotifyDLL.IncrementCounts) )
	{
		dprintf( "error! | ImageNotifyRoutine(); | 当前节点是被遗弃的,或者IncrementCounts已经不为1(pNode->IncrementCounts=%d) \n", pNode->XImageNotifyDLL.IncrementCounts );
		return ;
	}

	// 2.3 填充节点
	PDFixupNode( pNode, wszDosPath );

	// 3. 关键一句,注入DLL
#if _INJECT_DLL_

	dprintf( "[ImageNotify Inject DLL] starting modifying PE:\"%ws\" \n", wszDosPath );
	do 
	{
		BOOL bRet = FALSE;
		LPSTR szDllPath = NULL;
		
		// 3.1 查询配置文件，得到待注入的DLL全路径
		bRet = GetPBDllPath( PBDLLPathType_Reg, MAX_PATH, &szDllPath );
		if ( FALSE == bRet || NULL == szDllPath )
			break;

		// 3.2 校验文件是否存在？

		// 3.3 Inject ！
		status = InjectDll_by_reconstruct_pe( (PVOID)pNode, ImageBaseAddress, szDllPath ); 
		if( NT_SUCCESS( status ) )
		{
			dprintf( "[ImageNotify Inject DLL]  Done! (╰_╯)#  \n\n" );
		}

	} while (FALSE);
#endif

	pNode->bProcessNodeInitOK = TRUE ;
	return ;
}



BOOL 
CheckImageNotifyState (
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
	"C:\\WINDOWS\\NOTEPAD.EXE",
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


///////////////////////////////   END OF FILE   ///////////////////////////////