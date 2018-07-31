/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/05 [5:7:2011 - 15:42]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\HookHelperData.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "common.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBDynamicData.h"
#include "PBRegsData.h"
#include "HookHelperData.h"

#pragma warning(disable: 4995)

//////////////////////////////////////////////////////////////////////////

HANDLE g_KeyHandle_PBAutoExec = NULL ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


char Handler_RegKey_PBAutoExec( IN char data )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 15:44]

Routine Description:
  根据@data的内容对 "HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"的内容进行操作.
  其实该键相当于一个全局标记,表明SbieDLL.dll注册表部分的初始化工作是否完成!
    
Arguments:
  data - 0 表示要查询"HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"的值,不存在则创建,存在则返回查询到得值;
         1 表示要设置"HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"的值,
                设置为1,表示DLL初始化注册表完成,当其他程序再次Load SbieDLL.dll时便不用再次初始化了.

Return Value:
  "*\PBAutoExec"的值
    
--*/
{
	ULONG ResultLength = 0 ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	KEY_VALUE_PARTIAL_INFORMATION Buffer ;

	// 1. 有数据写入,调用ZwSetValueKey写入当前数据@data
	if ( data )
	{
		if ( g_KeyHandle_PBAutoExec )
		{
			RtlInitUnicodeString( &KeyName, NULL );
			status = ZwSetValueKey( g_KeyHandle_PBAutoExec, &KeyName, 0, REG_BINARY, &data, 1 );
			if ( ! NT_SUCCESS(status) )
			{
			}
		}
		
		return data;
	}
	
	// 2. 没有数据写入,打开键值
	wcscpy( szKeyName, L"\\registry\\user\\" );
	wcscat( szKeyName, g_BoxInfo.SID );
	wcscat( szKeyName, L"\\software\\PBAutoExec" );

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = Stub_NtOpenKey( &ObjAtr, &g_KeyHandle_PBAutoExec, 0xF003F );
	if ( status == STATUS_BAD_INITIAL_PC ) { return 0; }

	if ( status == STATUS_OBJECT_NAME_NOT_FOUND )
		status = ZwCreateKey( &g_KeyHandle_PBAutoExec, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );

	if ( ! NT_SUCCESS(status) ) { return 0; }

	RtlInitUnicodeString( &KeyName, NULL );

	status = ZwQueryValueKey(
		g_KeyHandle_PBAutoExec,
		&KeyName,
		KeyValuePartialInformation,
		&Buffer,
		0x10,
		&ResultLength
		);

	if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) { return 0; }
	if ( ! NT_SUCCESS(status) ) { return 0; }

	return data;
}



BOOL Handler_RegKey_RegLink()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:06]

Routine Description:
  调用 NtCreateKey 以 REG_OPTION_CREATE_LINK 的方式为下面的键值创建一个硬链接:
  (用A表示) HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\classes.
  链接到以下路径上去:
  (用B表示)HKEY_USERS\Sandbox_AV_DefaultBox\user\current_classes

  即操作 B 下子键就等于操作 A 的
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	HANDLE hKey = NULL, hKey1 = NULL ;
	BOOL bSetLink = FALSE ;

	// 1.
	wcscpy( szKeyName, g_BoxInfo.KeyRootPath );
	wcscat( szKeyName, L"\\user\\current" );
	wcscat( szKeyName, L"\\software\\classes" );

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateKey( &hKey, KEY_ALL_ACCESS, &ObjAtr, 0, 0, REG_OPTION_CREATE_LINK, 0 );
	if ( status == STATUS_OBJECT_NAME_COLLISION ) { return TRUE; }
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	// 2.
	wcscpy( szKeyName, g_BoxInfo.KeyRootPath );
	wcscat( szKeyName, L"\\user\\current" );
	wcscat( szKeyName, L"_classes" );

	RtlInitUnicodeString( &KeyName, szKeyName );

	status = ZwCreateKey( &hKey1, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );
	if ( NT_SUCCESS(status) )
	{
		ZwClose( hKey1 );
		bSetLink = TRUE;
	}

	if ( status == STATUS_OBJECT_NAME_COLLISION ) { bSetLink = TRUE; }

	if ( bSetLink )
	{
		RtlInitUnicodeString( &KeyName, L"SymbolicLinkValue" );

		status = ZwSetValueKey( hKey, &KeyName, 0, REG_LINK, szKeyName, 2 * wcslen(szKeyName) );
		ZwClose( hKey );
		return TRUE ;
	}

	ZwClose( hKey );
	return FALSE ;
}



VOID Handler_RegKey_DisableDCOM()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:28]

Routine Description:
  “远程协助”是 Windows XP 中的一项技术，它使得 Windows XP 用户可以通过 Internet 互相提供帮助。使用此工具，一个用户（称为“专家”）
  可以查看另一个用户（初级用户）的桌面。经过初级用户的允许，专家甚至可以共享对初级用户计算机的控制权以远程解决问题。
  [沙箱要做的便是禁止“远程协助”]

--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	HANDLE hKey = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\microsoft\\ole" );
	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( ! NT_SUCCESS(status) ) { return; }

	RtlInitUnicodeString( &KeyName, L"EnableDCOM" );
	status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"N", 4 );

	ZwClose( hKey );
	return;
}



VOID Handler_RegKey_NukeOnDelete_Recycle()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:35]

Routine Description:
  修改注册表中和"回收站"对应的键值"NukeOnDelete","UseGlobalSettings"; 设为1,表明删除时不经过回收站.
  这样在沙箱中删除文件时,为了决定在桌面上是否显示"回收站已满样子图标",系统会查询键值,结果被重定向至:
  HKEY_USERS\Sandbox_AV_DefaultBox\machine\software\microsoft\Windows\CurrentVersion\Explorer\BitBucket.
  发现@NukeOnDelete,@UseGlobalSettings 都被至1,则系统认为删除的文件不需要经过回收站,于是回收站图标不变
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;	
	NTSTATUS status = STATUS_SUCCESS ;
	KEY_BASIC_INFORMATION KeyInfo = { 0 } ;
	ULONG Flag = 0, Data = 1, Index = 0, ResultLength = 0 ;
	HANDLE hKey1 = NULL, hKey2 = NULL, hRootKey = NULL, hSubKey = NULL ;
	
	hKey1 = Stub_NtCreateKey_Special( HKEY_LOCAL_MACHINE, L"BitBucket", (int*)&Flag );
	if ( hKey1 && (hKey1 != (HANDLE)0xFFFFFFFF) )
	{
		RtlInitUnicodeString( &KeyName, L"NukeOnDelete" );

		status = ZwSetValueKey( hKey1, &KeyName, 0, REG_DWORD, &Data, 4 );
		if ( status >= 0 )
		{
			RtlInitUnicodeString( &KeyName, L"UseGlobalSettings" );
			status = ZwSetValueKey( hKey1, &KeyName, 0, REG_DWORD, &Data, 4 );

			if ( status < 0 ) { Flag = 0x33 ; }
		}
		else
		{
			Flag = 0x22 ;
		}

		ZwClose( hKey1 );
	}

	if ( Flag ) { return; }

	hKey2 = Stub_NtCreateKey_Special( HKEY_CURRENT_USER, L"BitBucket", (int*)&Flag );
	if ( NULL == hKey2 || ((HANDLE)0xFFFFFFFF == hKey2) ) { return; }

	RtlInitUnicodeString( &KeyName, L"Volume" );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hKey2, NULL );

	status = Stub_NtOpenKey( &ObjAtr, &hRootKey, 0xF003F );
	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( hKey2 );
		return;
	}

	ObjAtr.RootDirectory = hRootKey ;
	status = ZwEnumerateKey( hRootKey, 0, KeyBasicInformation, (PVOID)&KeyInfo, 0x100, &ResultLength );
	if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

	while (TRUE)
	{
		KeyInfo.Name[ KeyInfo.NameLength / sizeof(WCHAR) ] = 0 ;
		RtlInitUnicodeString( &KeyName, KeyInfo.Name );

		status = Stub_NtOpenKey( &ObjAtr, &hSubKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			RtlInitUnicodeString( &KeyName, L"NukeOnDelete" );
			ZwSetValueKey( hSubKey, &KeyName, 0, REG_DWORD, &Data, 4 );
			ZwClose( hSubKey );
		}

		++ Index ;

		status = ZwEnumerateKey( hRootKey, 0, KeyBasicInformation, (PVOID)&KeyInfo, 0x100, &ResultLength );
		if ( ! NT_SUCCESS(status) ) { break; }
	}

_END_ :
	ZwClose( hRootKey );
	ZwClose( hKey2 );
	return;
}



VOID Handler_RegKey_BrowseNewProcess()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 14:21]

Routine Description:
  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\BrowseNewProcess 以及
  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\BrowseNewProcess(前者对
  所有用户帐户生效,后者只对当前用户帐户生效) 修改串值BrowseNewProcess将其值设置为"yes"表示为IE
  浏览器使用单独进程;将其设置为"no"则表示将 Explorer.EXE 和 IEXPLORE.EXE 进程合并.
  [沙箱要做的便是设置为HKEY_CURRENT_USER\*\BrowseNewProcess为"yes",让IE使用单独进程]

--*/
{
	int Flag = 0 ;
	HANDLE hKey = NULL ;
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ;

	hKey = Stub_NtCreateKey_Special( HKEY_CURRENT_USER, L"BrowseNewProcess", &Flag );
	if ( NULL == hKey || ((HANDLE)0xFFFFFFFF == hKey) ) { return; }

	RtlInitUnicodeString( &KeyName, L"BrowseNewProcess" );
	status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"yes", 8 );
	if ( status < 0 ) { Flag = 0x22; }
	
	ZwClose( hKey );
	return;
}



static const LPWSTR g_xxClasses_Arrays[ ] = 
{ 
	L"Machine\\Software\\Classes",
	L"Machine\\Software\\Classes\\Wow64Node",
	L"User\\Current\\Software\\Classes",
	L"User\\Current\\Software\\Classes\\Wow64Node",
};


VOID Handler_RegKey_Elimination()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 16:04]

Routine Description:
  DLL初始化期间(递归)清除自身的键值(包括子键):
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CompressedFolder\Shell\Open\ddeexec"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\*\shell\sandbox"
  "HKEY_CURRENT_USER\Software\Classes\CompressedFolder\Shell\find\ddeexec"
  "HKEY_CURRENT_USER\Software\Classes\*\shell\sandbox"

--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	WCHAR szName[ MAX_PATH ] = L"" ;
	NTSTATUS status = STATUS_SUCCESS ;

	for (int i = 0; i < ARRAYSIZEOF(g_xxClasses_Arrays); i++ )
	{
		// 1.
		if ( wcsstr(g_xxClasses_Arrays[i], L"Wow64") ) { continue; }

		wcscpy( szName, L"\\Registry\\" );
		wcscat( szName, g_xxClasses_Arrays[i] );
		wcscat( szName, L"\\*\\shell\\sandbox" );

		RtlInitUnicodeString( &KeyName, szName );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			StubNtDeleteKey( hKey, TRUE );
			ZwClose( hKey );
		}

		// 2.
		if ( wcsstr(g_xxClasses_Arrays[i], L"Wow64") ) { continue; }

		wcscpy( szName, L"\\Registry\\" );
		wcscat( szName, g_xxClasses_Arrays[i] );
		wcscat( szName, L"\\CompressedFolder\\shell\\open\\ddeexec" );

		RtlInitUnicodeString( &KeyName, szName );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			StubNtDeleteKey( hKey, TRUE );
			ZwClose( hKey );
		}
	}

	return;
}



static const LPWSTR g_DirtyClSID_Arrays[ ] = 
{ 
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{de1f7eef-1851-11d3-939e-0004ac1abe1f}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{761497BB-D6F0-462C-B6EB-D4DAF1D92D43}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{DBC80044-A445-435B-BC74-9C25C1C588A9}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{E7E6F031-17CE-4C07-BC86-EABFE594F69C}",
};


VOID Handler_RegKey_clsid_1()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 17:14]

Routine Description:
  设置以下4个键值为"已删除"状态:
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{de1f7eef-1851-11d3-939e-0004ac1abe1f}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{761497BB-D6F0-462C-B6EB-D4DAF1D92D43}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{DBC80044-A445-435B-BC74-9C25C1C588A9}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{E7E6F031-17CE-4C07-BC86-EABFE594F69C}"
  
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	for (int i = 0; i < ARRAYSIZEOF(g_DirtyClSID_Arrays); i++ )
	{
		RtlInitUnicodeString( &KeyName, g_DirtyClSID_Arrays[i] );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			MarkDirtyKey( hKey );
		}
	}

	return ;
}



VOID Handler_RegKey_clsid_2()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 17:23]

Routine Description:
  设置"HKEY_USERS\Sandbox_AV_DefaultBox\machine\software\microsoft\windows nt\currentversion\winlogon"的键值"Shell"内容为"x",(本应该是"Explorer.exe");
  删除"\\registry\\machine\\software\\classes\\clsid\\{ceff45ee-c862-41de-aee2-a022c81eda92}"的键值"AppId"
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	// 1.
	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\microsoft\\windows nt\\currentversion\\winlogon" );

	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &KeyName, L"Shell" );
		status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"x", 4 );
	}

	// 2.
	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\classes\\clsid\\{ceff45ee-c862-41de-aee2-a022c81eda92}" );
	
	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &KeyName, L"AppId" );
		status = ZwDeleteValueKey( hKey, &KeyName );
	}

	return;
}



VOID HandlerSelfAutoExec()
{
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG pBuffer = 0, nIndex = 1, dwErrCode = ERROR_NO_TOKEN ;

	return;



	pBuffer = (ULONG) kmalloc( 0x2000 );

	status = PB_EnumProcessEx( g_BoxInfo.BoxName, (int*)pBuffer );
	if ( ! NT_SUCCESS(status) || 1 != *(DWORD *)pBuffer ) { kfree( (PVOID)pBuffer ); return ; }

	// 只考虑沙箱中的进程数量为1的情况; 

	//
	// 该功能没有什么用,废弃
	//

	return;
}



HANDLE Stub_NtCreateKey_Special( HKEY hRootKey, LPWSTR szKeyName, int *pFlag )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 14:30]

Routine Description:
  @hRootKey\Software\Microsoft\Windows\CurrentVersion\Explorer\@szKeyName,返回句柄.(不存在则创建之)
    
Arguments:
  hRootKey - HKEY_LOCAL_MACHINE 对应"\\Registry\\Machine"; HKEY_CURRENT_USER 对应"\\Registry\\User\\Current"
  szKeyName - 要打开的子键名

Return Value:
  指定键值的句柄

--*/
{
	LPWSTR pszKeyName = NULL ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;	
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hRootKeyDummy = NULL, hKey_Explorer = NULL, hKey_Target = NULL ;

	if ( hRootKey == HKEY_LOCAL_MACHINE )
	{
		RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine" );
	}
	else if ( hRootKey == HKEY_CURRENT_USER )
	{
		RtlInitUnicodeString( &KeyName, L"\\Registry\\User\\Current" );
	}
	else
	{
		*pFlag = 0xAA ;
		return NULL;
	}

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwOpenKey( &hRootKeyDummy, KEY_READ, &ObjAtr );
	if ( ! NT_SUCCESS(status) )
	{
		*pFlag = 0x99;
		return NULL;
	}

	RtlInitUnicodeString( &KeyName, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hRootKeyDummy, NULL );

	status = ZwOpenKey( &hKey_Explorer, KEY_READ, &ObjAtr );
	ZwClose( hRootKeyDummy );

	if ( status )
	{
		*pFlag = 0x88;
		return NULL;
	}

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hKey_Explorer, NULL );
	
	status = Stub_NtOpenKey( &ObjAtr, &hKey_Target, 0xF003F );
	if ( status == STATUS_BAD_INITIAL_PC )    // 是黑白名单
	{
		*pFlag = 0;
		return (HANDLE)0xFFFFFFFF ;
	}

	// 不存在当前键值则创建之
	if ( status == STATUS_OBJECT_NAME_NOT_FOUND )
		status = ZwCreateKey( &hKey_Target, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );

	ZwClose( hKey_Explorer );
	if ( status )
	{
		*pFlag = 0x77;
		return NULL;
	}

	return hKey_Target;
}



VOID HandlerIEEmbedding()
{
	if ( FALSE == g_bIs_Inside_iexplore_exe ) { return; }

	LPWSTR ptr = GetCommandLineW();
	if ( NULL == ptr ) { return; }

	if ( (wcsstr(ptr, L"/Embedding") || wcsstr(ptr, L"-Embedding")) && PB_IsOpenCOM() )
	{
		ULONG_PTR BaseAddr = (ULONG_PTR) GetModuleHandleW( NULL );
		ULONG_PTR AddressOfEntryPoint = (ULONG_PTR) SRtlImageNtHeader((PVOID)BaseAddr)->OptionalHeader.AddressOfEntryPoint + BaseAddr ;

		BOOL bRet = Mhook_SetHook( (PVOID*)&AddressOfEntryPoint, fake_entrypoint );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | HandlerIEEmbedding() - Mhook_SetHook(); | \"entrypoint\" \n" );
			ExitProcess(0);
		}
	}

	return;
}


VOID fake_entrypoint()
{
	HMODULE hModule = LoadLibraryW(L"ole32.dll");

	//
	// {TODO:}sudami COM云云. 未完待续!!!
	//

	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////