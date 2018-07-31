/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/03/03 [3:3:2011 - 14:50]
* MODULE : e:\Data\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDLL\PBIPStore.cpp
* 
* Description:
*
*   1.简介：什么是PStore？
*     PStore的全称为：Protected Storage。在系统服务中我们可以看到它(9x没有)。它的作用就是为应用程序的安全保存做一个接口。在它里面记录了一些隐密的信息，比方说：
*     1. Outlook 密码
*     2. 删除的Outlook帐号密码
*     3. IE 密码保存站点密码
*     4. MSN登陆密码
*     5. IE 自动保存密码
*                        
*   在上面函数中的IPStore是一个基于COM的接口，它提供了一引起方法以供调用：
*   方法            描述
*   CloseItem       Closes a specified data item from protected storage.
*   CreateType      Creates the specified type with the specified name.
*   CreateSubtype   Creates the specified subtype within the specified type.
*   DeleteItem      Deletes the specified item from the protected storage.
*   DeleteSubtype   Deletes the specified item subtype from the protected storage.
*   DeleteType      Deletes the specified type from the protected storage.
*   EnumItems       Returns the interface pointer of a subtype for enumerating items in the protected storage database.
*   EnumSubtypes    Returns an interface for enumerating subtypes of the types currently registered in the protected database.
*   EnumTypes       Returns an interface for enumerating the types currently registered in the protected database.
*   GetInfo         Retrieves information about the storage provider.
*   GetProvParam    Not implemented.
*   GetSubtypeInfo  Retrieves information associated with a subtype.
*   GetTypeInfo     Retrieves information associated with a type.
*   OpenItem        Opens an item for multiple accesses.
*   ReadAccessRuleSet     Not implemented.
*   ReadItem        Reads the specified data item from protected storage.
*   SetProvParam    Sets the specified parameter information.
*   WriteAccessRuleset     Not implemented.
*   WriteItem       Writes a data item to protected storage.
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBFilesData.h"
#include "PBLoadData.h"
#include "PBComData.h"
#include "PBUser32dll/PBUser32dllData.h"
#include "PBDynamicData.h"
#include "PBIPStore.h"

#pragma warning(disable : 4995) 


//////////////////////////////////////////////////////////////////////////

typedef ULONG (WINAPI* _PStoreCreateInstance_) ( int,int,int,int );
_PStoreCreateInstance_ g_PStoreCreateInstance_addr = NULL ;

BOOL g_OpenProtectedStorage_state = TRUE ;
/*
[DefaultBox]
OpenProtectedStorage=yes

Indicates that programs running in the DefaultBox sandbox will update the global system Protected Storage, rather than a sandboxed instance of it.
*/

CComObjectGlobal<CIPStoreIDataObject>	g_IPStore_IDataObject_VTable ;
CComObjectGlobal<CENUMPSTORETPYES>		g_IEnumPStoreTypes_array ;
CComObjectGlobal<CENUMPSTORETPYESVTBL>	g_IEnumPStoreTypesVtbl_array ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


PVOID
PStoreCreateInstance_filter (
	IN int p,
	IN int CoTaskMemAlloc_addr
	)
{
	BOOL bFlagMapped = FALSE ;
	HANDLE hMutex = NULL, hMapped = NULL, lpMapping = NULL, hOpenFileMapped = NULL ;
	PLARGE_INTEGER lpMappedAddr = NULL ;
	LPPStoreCreateInstance_Info This = (LPPStoreCreateInstance_Info) p ;

	This->lpVtbl = &g_IPStore_IDataObject_VTable ;
	This->Ref = 1 ;
	This->CoTaskMemAlloc_addr = (ULONG) CoTaskMemAlloc_addr ;
	This->hMutex = This->hMapped = This->hHeap = This->LittleInfo.lpMappedAddr = 0;
	This->LittleInfo.lpMappedAddrOld.LowPart = This->LittleInfo.lpMappedAddrOld.HighPart = 0;

	ClearStruct( &This->LittleInfo.ListEntry );

	This->hMutex = hMutex = CreateMutexW( NULL, FALSE, L"Proteinbox_ProtectedStorage_Mutex" );
	if ( NULL == hMutex ) { return (PVOID) This; }
	
	WaitForSingleObject( hMutex, INFINITE );

	This->hMapped = hOpenFileMapped = OpenFileMappingW( SECTION_ALL_ACCESS, FALSE, L"Proteinbox_ProtectedStorage_Section" );
	if ( NULL == hOpenFileMapped )
	{
		This->hMapped = hMapped = CreateFileMappingW( (HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 8, L"Proteinbox_ProtectedStorage_Section" );
		if ( hMapped ) { bFlagMapped = TRUE; }
	}

	lpMapping = This->hMapped ;
	if ( lpMapping )
	{
		lpMappedAddr = (PLARGE_INTEGER) MapViewOfFile( lpMapping, FILE_MAP_ALL_ACCESS, 0, 0, 8 );
		This->LittleInfo.lpMappedAddr = lpMappedAddr ;

		if ( bFlagMapped )
			*(QWORD *)&lpMappedAddr->LowPart = *(QWORD *)&This->LittleInfo.lpMappedAddrOld.LowPart + 1i64 ;
	}

	ReleaseMutex( This->hMutex );
	return (PVOID) This;
}


ULONG WINAPI fake_PStoreCreateInstance( int ppProvider, int a2, int a3, int a4 )
{
	int pBuffer = 0 ;

	if ( NULL == g_CoTaskMemAlloc_addr )
	{
		g_CoTaskMemAlloc_addr = (_CoTaskMemAlloc_) GetProcAddress( GetModuleHandleW(L"ole32.dll"), "CoTaskMemAlloc" );
	}

	if ( NULL == g_CoTaskMemAlloc_addr ) { return E_FAIL; }

	*(int *)ppProvider = 0 ;
	pBuffer = (int) HeapAllocEx( 0x38 );
	if ( pBuffer )
	{
		*(int *)ppProvider = (int) PStoreCreateInstance_filter( pBuffer, (int)g_CoTaskMemAlloc_addr );
	}

	return 0 ;
}


BOOL WINAPI Hook_pharase6_pstorecdll( IN HMODULE hModule )
{
	BOOL bRet = TRUE;

	//
	// 1. 先查询配置文件,是否需要做 OpenProtectedStorage 的防护
	//

	// 这里需要查询配置文件,调用PB_QueryConf函数; 我们先简化一下儿,后期调试时再修补  - -|
	if ( g_OpenProtectedStorage_state ) { return TRUE; } // 至TRUE表明沙箱中可以直接访问系统的全局设置

	//
	// 2. 表明需要重定向到沙箱中去
	//

	g_PStoreCreateInstance_addr = (_PStoreCreateInstance_) GetProcAddress( hModule, "PStoreCreateInstance" );
	bRet = Mhook_SetHook( (PVOID*)&g_PStoreCreateInstance_addr, fake_PStoreCreateInstance );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase6_pstorecdll() - Mhook_SetHook(); | \"PStoreCreateInstance\" \n" );
		return FALSE ;
	}

	return TRUE;
}






HRESULT WINAPI CIPStoreIDataObject::QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject )
{
// 	this->lpVtbl->AddRef( (IDataObject *)this );
// 	*(DWORD *)ppvObject = (DWORD)this ;
// 	return 0;
	
	LPOLEClipbrdEx This = (LPOLEClipbrdEx) iface ;

	This->lpVtbl1->AddRef( (IDataObject *)This );
	*(DWORD *)ppvObject = (DWORD)This ;
	return 0;
}



ULONG WINAPI CIPStoreIDataObject::AddRef( IDataObject* iface )
{
	ULONG Ref = 0 ;
	LPPStoreCreateInstance_Info This = (LPPStoreCreateInstance_Info) iface ; 

	Ref = This->Ref + 1 ;
	This->Ref = Ref ;
	return Ref;
//	return InterlockedIncrement( &this->m_dwRef );
}



ULONG WINAPI CIPStoreIDataObject::Release( IDataObject* iface )
{
	ULONG Ref = 0 ;
	LPPStoreCreateInstance_Info This = (LPPStoreCreateInstance_Info) iface ; 

	Ref = This->Ref - 1 ;
	This->Ref = Ref ;
	if ( Ref ) { return Ref; }
	
	This->lpVtbl = &g_IPStore_IDataObject_VTable ;
	if ( This->hHeap ) { HeapDestroy( This->hHeap ); }

	if ( This->hMapped ) { CloseHandle( This->hMapped ); }

	if ( This->hMutex ) { CloseHandle( This->hMutex ); }

	HeapFree( GetProcessHeap(), 4, This );
	return 0;
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::GetInfo( PPST_PROVIDERINFO __RPC_FAR *ppProperties )
{
	return MessageBoxW_IPStore( L"GetInfo" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::GetProvParam( DWORD dwParam,DWORD __RPC_FAR *pcbData,BYTE __RPC_FAR *__RPC_FAR *ppbData,DWORD dwFlags )
{
	return MessageBoxW_IPStore( L"GetProvParam" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::SetProvParam( DWORD dwParam,DWORD cbData,BYTE __RPC_FAR *pbData,DWORD dwFlags )
{
	return MessageBoxW_IPStore( L"SetProvParam" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::CreateType( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO pInfo,DWORD dwFlags )
{
	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	if ( PBIPStoreGetNode( pType ) ) { return INET_E_CANNOT_CONNECT; }
	
	PBIPStoreInsertNode( pType, pInfo );

	PBIPStoreHandlerFile();
	
	return NO_ERROR ;
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::GetTypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags )
{
	return MessageBoxW_IPStore( L"GetTypeInfo" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::DeleteType( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags )
{
	PIPStoreVtbl_Type_Info pNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pType );
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	pNode->Flag |= 1 ;
	PBIPStoreHandlerFile();

	return NO_ERROR ;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::CreateSubtype( 
	PST_KEY Key,
	const GUID __RPC_FAR *pType,
	const GUID __RPC_FAR *pSubtype,
	PPST_TYPEINFO pInfo,
	PPST_ACCESSRULESET pRules,
	DWORD dwFlags
	)
{
	PIPStoreVtbl_Type_Info pNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pType );
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	if ( PBIPStoreGetChildNode( pNode, pSubtype ) ) { return INET_E_CANNOT_CONNECT; }
	
	PBIPStoreInsertChildNode( pNode, pSubtype, pInfo );

	PBIPStoreHandlerFile();

	return NO_ERROR ;
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::GetSubtypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags)
{
	return MessageBoxW_IPStore( L"GetSubtypeInfo" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::DeleteSubtype( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,DWORD dwFlags)
{
	PIPStoreVtbl_Type_Info pNode = NULL ;
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pType );
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }

	pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreGetChildNode( pNode, pSubtype );
	if ( NULL == pChildNode ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	pChildNode->Flag |= 1 ;
	PBIPStoreHandlerFile();
	
	return NO_ERROR;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::ReadAccessRuleset( 
	PST_KEY Key,
	const GUID __RPC_FAR *pType,
	const GUID __RPC_FAR *pSubtype,
	PPST_ACCESSRULESET __RPC_FAR *ppRules,
	DWORD dwFlags
	)
{
	return MessageBoxW_IPStore( L"ReadAccessRuleset" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::WriteAccessRuleset( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_ACCESSRULESET pRules,DWORD dwFlags)
{
	return MessageBoxW_IPStore( L"WriteAccessRuleset" );
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::EnumTypes( PST_KEY Key,DWORD dwFlags,/*IEnumPStoreTypes*/int __RPC_FAR *__RPC_FAR *ppenum)
{
	int dat = 0 ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }

	dat = (int) PBIPStoreEnumTypes( Key );
	*(DWORD *)ppenum = dat ;

	return ((dat != 0) - 1) & 0x8007000E ;
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::EnumSubtypes( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags,/*IEnumPStoreTypes*/int __RPC_FAR *__RPC_FAR *ppenum)
{
	int dat = 0 ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }

	dat = (int) PBIPStoreEnumSubtypes( Key, pType );
	*(DWORD *)ppenum = dat ;

	return ((dat != 0) - 1) & 0x8007000E ;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::DeleteItem ( 
	PST_KEY Key,
	const GUID __RPC_FAR *pItemType,
	const GUID __RPC_FAR *pItemSubtype,
	LPCWSTR szItemName,
	PPST_PROMPTINFO pPromptInfo,
	DWORD dwFlags
	)
{
	PIPStoreVtbl_Type_Info pNode = NULL ;
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;
	PIPStoreVtbl_Type_Info_grandsun pGrandSunNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pItemType);
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }

	pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreGetChildNode( pNode, pItemSubtype );
	if ( NULL == pChildNode ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	pGrandSunNode = (PIPStoreVtbl_Type_Info_grandsun) PBIPStoreGetGrandchildNode( pNode, pChildNode, (LPWSTR)szItemName );
	if ( NULL == pGrandSunNode ) { return INET_E_CANNOT_INSTANTIATE_OBJECT; }
	
	pGrandSunNode->Flag |= 1 ;
	PBIPStoreHandlerFile();
	
	return NO_ERROR;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::ReadItem ( 
	PST_KEY Key,
	const GUID __RPC_FAR *pItemType,
	const GUID __RPC_FAR *pItemSubtype,
	LPCWSTR szItemName,
	DWORD __RPC_FAR *pcbData,
	BYTE __RPC_FAR *__RPC_FAR *ppbData,
	PPST_PROMPTINFO pPromptInfo,
	DWORD dwFlags
	)
{
	PVOID ppbDataDummy = NULL ;
	PIPStoreVtbl_Type_Info pNode = NULL ;
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;
	PIPStoreVtbl_Type_Info_grandsun pGrandSunNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }

	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pItemType );
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }

	pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreGetChildNode( pNode, pItemSubtype );
	if ( NULL == pChildNode ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	pGrandSunNode = (PIPStoreVtbl_Type_Info_grandsun) PBIPStoreGetGrandchildNode( pNode, pChildNode, (LPWSTR)szItemName );
	if ( NULL == pChildNode ) { return INET_E_CANNOT_INSTANTIATE_OBJECT; }
	
	*(DWORD *)ppbData = 0;
	*pcbData = pGrandSunNode->cbData ;

	if ( pGrandSunNode->cbData )
	{
		ppbDataDummy = (PVOID)((int (__thiscall *)(DWORD)) this->Info.CoTaskMemAlloc_addr)( pGrandSunNode->cbData );

		*(DWORD *)ppbData = (DWORD)ppbDataDummy ;
		memcpy( ppbDataDummy, pGrandSunNode->pbData, pGrandSunNode->cbData );
	}
	
	return NO_ERROR ;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::WriteItem( 
	PST_KEY Key,
	const GUID __RPC_FAR *pItemType,
	const GUID __RPC_FAR *pItemSubtype,
	LPCWSTR szItemName,
	DWORD cbData,
	BYTE __RPC_FAR *pbData,
	PPST_PROMPTINFO pPromptInfo,
	DWORD dwDefaultConfirmationStyle,
	DWORD dwFlags 
	)
{
	PIPStoreVtbl_Type_Info pNode = NULL ;
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;
	PIPStoreVtbl_Type_Info_grandsun pGrandSunNode = NULL ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }

	pNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pItemType );
	if ( NULL == pNode ) { return INET_E_RESOURCE_NOT_FOUND; }

	pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreGetChildNode( pNode, pItemSubtype );
	if ( NULL == pChildNode ) { return INET_E_RESOURCE_NOT_FOUND; }

	pGrandSunNode = (PIPStoreVtbl_Type_Info_grandsun) PBIPStoreGetGrandchildNode( pNode, pChildNode, (LPWSTR)szItemName );
	if ( pGrandSunNode )
	{
		if ( dwFlags & 2 ) { return INET_E_CANNOT_LOAD_DATA; }

		if ( pGrandSunNode->pbData )
		{
			HeapFree( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, pGrandSunNode->pbData );
			pGrandSunNode->pbData = 0 ;
			pGrandSunNode->cbData = 0 ;
		}
	}

	PBIPStoreSetGrandchildNode( pChildNode, pGrandSunNode, (LPWSTR)szItemName, cbData, pbData );
	PBIPStoreHandlerFile();

	return NO_ERROR;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::OpenItem ( 
	PST_KEY Key,
	const GUID __RPC_FAR *pItemType,
	const GUID __RPC_FAR *pItemSubtype,
	LPCWSTR szItemName,
	PST_ACCESSMODE ModeFlags,
	PPST_PROMPTINFO pPromptInfo,
	DWORD dwFlags
	)
{
	return NO_ERROR;
}



HRESULT STDMETHODCALLTYPE CIPStoreIDataObject::CloseItem( PST_KEY Key,const GUID __RPC_FAR *pItemType,const GUID __RPC_FAR *pItemSubtype,LPCWSTR szItemName,DWORD dwFlags)
{
	return NO_ERROR;
}



HRESULT STDMETHODCALLTYPE 
CIPStoreIDataObject::EnumItems ( 
	PST_KEY Key,
	const GUID __RPC_FAR *pItemType,
	const GUID __RPC_FAR *pItemSubtype,
	DWORD dwFlags,
	/*IEnumPStoreItems*/int __RPC_FAR *__RPC_FAR *ppenum
	)
{
	int dat = 0 ;

	if ( FALSE == PBIPStoreHandler() ) { return E_FAIL; }
	
	dat = (int) PBIPStoreEnumItems( Key, pItemType, pItemSubtype );
	*(DWORD *)ppenum = dat ;
	
	return ((dat != 0) - 1) & E_OUTOFMEMORY ;
}



//////////////////////////////////////////////////////////////////////////


typedef int ( WINAPI * _MessageBoxW_ ) ( HWND,LPCWSTR,LPCWSTR,UINT );

ULONG MessageBoxW_IPStore( IN LPWSTR szBuffer )
{
	HMODULE hModule = NULL ;
	_MessageBoxW_ Func = NULL ;
	WCHAR szInfo[ MAX_PATH ] = L"" ;

	swprintf( szInfo, L"IPStore::%s is not implemented\n", szBuffer );

	hModule = Get_hModule_user32DLL();
	if ( NULL == hModule ) { return E_FAIL ; }

	Func = (_MessageBoxW_) GetProcAddress( hModule, "MessageBoxW" );
	if ( Func ) { Func( NULL, szInfo, L"Proteinbox Protected Storage", MB_ICONWARNING ); }

	FreeLibrary( hModule );
	return E_FAIL ;
}



VOID MessageBoxW_Stub( HWND hwndOwner, LPCWSTR lpszText, LPCWSTR lpszCaption, int wStyle )
{
	HMODULE hModule = NULL ;
	_MessageBoxW_ Func = NULL ;

	hModule = Get_hModule_user32DLL();
	if ( NULL == hModule ) { return; }
	
	Func = (_MessageBoxW_) GetProcAddress( hModule, "MessageBoxW" );
	if ( Func ) { Func( hwndOwner, lpszText, lpszCaption, wStyle ); }

	FreeLibrary( hModule );
	return ;
}



BOOL CIPStoreIDataObject::PBIPStoreHandler()
{
	int Data = 0 ; 
	NTSTATUS status = STATUS_SUCCESS ;
	PLARGE_INTEGER lpMappedAddr = NULL ;
	PSbiePst_dat_FileInfo pNode = NULL ;

	if ( NULL == this->Info.LittleInfo.lpMappedAddr ) { return FALSE; }
	
	// 1. 无限等待事件变为受信
	WaitForSingleObject( this->Info.hMutex, INFINITE );
	lpMappedAddr = this->Info.LittleInfo.lpMappedAddr;

	// 2.1 等待成功,若地址相同,则无需更新结构体信息
	if (   this->Info.LittleInfo.lpMappedAddrOld.LowPart  == lpMappedAddr->LowPart 
		&& this->Info.LittleInfo.lpMappedAddrOld.HighPart == lpMappedAddr->HighPart 
		)
	{
		goto _over_ ;
	}
	
	// 2.2 地址不同,需要更新结构体中的数据
	this->Info.LittleInfo.lpMappedAddrOld.LowPart  = lpMappedAddr->LowPart ;
	this->Info.LittleInfo.lpMappedAddrOld.HighPart = lpMappedAddr->HighPart ;
	if ( this->Info.hHeap ) { HeapDestroy( this->Info.hHeap ); }

	this->Info.hHeap = HeapCreate( HEAP_GENERATE_EXCEPTIONS, 0x1000, 0 );
	ClearStruct( &this->Info.LittleInfo.ListEntry );

	pNode = (PSbiePst_dat_FileInfo) PBIPStoreAllocateNode( FALSE );
	if ( NULL == pNode ) { goto _over_ ; }
	
	status = PBIPStoreParse( pNode, (LPBYTE)&Data );
	if ( ! NT_SUCCESS(status) ) { goto _free_heap_ ; }

	this->Info.LittleInfo.u.bFlag1 = 0 ;
	if ( 0xA1B2C3D4 == Data )
	{
		if ( PBIPStoreParse( pNode, (LPBYTE)&Data ) ) { Data = 0 ; }

		this->Info.LittleInfo.u.bFlag1 = 1 ;
	}

	if ( Data )
	{
		while ( PBIPStoreBuild( pNode ) )
		{
			-- Data ;
			if ( 0 == Data ) { goto _free_heap_ ; }
		}

		MessageBoxW_Stub (
			NULL,
			L"There was a problem reading the file which contains the data for protected storage.  Protected storage data (such as that used to auto-complete Web forms) will not be available.",
			L"Proteinbox Protected Storage",
			MB_ICONWARNING
			);
	}

_free_heap_:
	PBIPStoreRelease( pNode );

_over_ :
	ReleaseMutex( this->Info.hMutex );
	return TRUE;
}



PVOID CIPStoreIDataObject::PBIPStoreAllocateNode( IN BOOL bOverWrite )
{
	HANDLE hFile = NULL ;
	BOOL bFlag = FALSE ;
	WCHAR szFileName[ 0x288 ] = L"" ;
	PSbiePst_dat_FileInfo pNew = NULL ;
	DWORD dwShareMode, dwDesiredAccess, dwCreationDisposition, dwFlagsAndAttributes ; 

	memset( szFileName, 0, 0x288 );
	GetWindowsDirectoryW( szFileName, 0x104 );
	wcscat( szFileName, L"\\PBPst.dat" );

	if ( bOverWrite )
	{
		dwShareMode = 0;
		dwDesiredAccess = GENERIC_WRITE ;
		dwCreationDisposition = CREATE_ALWAYS ;
		dwFlagsAndAttributes = FILE_ATTRIBUTE_HIDDEN ;       
	}
	else
	{
		dwShareMode = 1;
		dwDesiredAccess = GENERIC_READ ;
		dwCreationDisposition = OPEN_EXISTING ;
		dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL ;
	}

	hFile = CreateFileW( szFileName, dwDesiredAccess, dwShareMode, 0, dwCreationDisposition, dwFlagsAndAttributes, 0 );
	
	if ( hFile != (HANDLE)INVALID_HANDLE_VALUE )
	{
		bFlag = TRUE;
	}
	else if ( ERROR_ACCESS_DENIED == GetLastError() && bOverWrite )
	{
		hFile = CreateFileW( szFileName, dwDesiredAccess, dwShareMode, 0, TRUNCATE_EXISTING, dwFlagsAndAttributes, 0 );
		if ( hFile != (HANDLE)INVALID_HANDLE_VALUE ) { bFlag = TRUE; }
	}
	else
	{
		bFlag = FALSE;
	}

	if ( bFlag )
	{
		PBIPStoreAllocateNodeEx( (PVOID *)&pNew, hFile );
	}
	
	return (PVOID)pNew ;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreAllocateNodeEx ( 
	OUT PVOID *pBuffer,
	IN HANDLE hFile
	)
{
	PSbiePst_dat_FileInfo pNew = NULL ;

	*pBuffer = NULL ;
	pNew = (PSbiePst_dat_FileInfo) HeapAlloc( GetProcessHeap(), 0, 0x1000 );
	if ( NULL == pNew ) { return STATUS_INSUFFICIENT_RESOURCES; }

	pNew->hFile = hFile;
	pNew->DataLength = 0;
	pNew->pDataPointer = (PBYTE)pNew->pBuffer ;

	*pBuffer = pNew;
	return STATUS_SUCCESS;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreParse (
	IN PSbiePst_dat_FileInfo pNode, 
	OUT LPBYTE pData
	)
{
	NTSTATUS status = STATUS_SUCCESS ;

	if ( 0 == pNode->DataLength )
	{
		status = PBIPStoreParseEx( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}

	// 1. Step1...
	*pData = *pNode->pDataPointer ;
	++ pNode->pDataPointer ;
	-- pNode->DataLength ;
	if ( 0 == pNode->DataLength )
	{
		status = PBIPStoreParseEx( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}
	
	// 2. Step2...
	pData[1] = *pNode->pDataPointer ;
	++ pNode->pDataPointer ;
	-- pNode->DataLength ;
	if ( 0 == pNode->DataLength )
	{
		status = PBIPStoreParseEx( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}

	// 3. Step3...
	pData[2] = *pNode->pDataPointer ;
	++ pNode->pDataPointer ;
	-- pNode->DataLength ;
	if ( 0 == pNode->DataLength )
	{
		status = PBIPStoreParseEx( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}
	
	// 4. Step4...
	pData[3] = *pNode->pDataPointer ;
	++ pNode->pDataPointer ;
	-- pNode->DataLength ;
	
	return STATUS_SUCCESS ;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreParseEx (
	IN OUT PSbiePst_dat_FileInfo pNode
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	IO_STATUS_BLOCK IoStatusBlock ;

	status = ZwReadFile( pNode->hFile, 0, 0, 0, &IoStatusBlock, pNode->pBuffer, 0xFF0, 0, 0 );

	pNode->pDataPointer = (PBYTE)pNode->pBuffer ;
	pNode->DataLength = IoStatusBlock.Information ;

	if ( NT_SUCCESS(status) && 0 == IoStatusBlock.Information ) { status = STATUS_END_OF_FILE; }
	return status;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreParseExp (
	IN OUT PSbiePst_dat_FileInfo pFileNode,
	IN ULONG nCounts,
	OUT LPBYTE pData
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	IO_STATUS_BLOCK IoStatusBlock ;

	if ( 0 == nCounts ) { return STATUS_SUCCESS; }

	while ( TRUE )
	{
		if ( 0 == pFileNode->DataLength )
		{
			status = ZwReadFile( pFileNode->hFile, 0, 0, 0, &IoStatusBlock, pFileNode->pBuffer, 0xFF0, 0, 0 );

			pFileNode->DataLength   = IoStatusBlock.Information ;
			pFileNode->pDataPointer = (PBYTE)pFileNode->pBuffer ;

			if ( ! NT_SUCCESS(status) ) { return status; }

			if ( ! IoStatusBlock.Information ) { break; }
		}

		*pData = *pFileNode->pDataPointer ;

		++ pData ;
		++ pFileNode->pDataPointer ;
		-- nCounts ;
		-- pFileNode->DataLength ;

		if ( 0 == nCounts )  { return STATUS_SUCCESS; }
	}

	return STATUS_END_OF_FILE ;
}



BOOL
CIPStoreIDataObject::PBIPStoreDecode (
	IN PSbiePst_dat_FileInfo pNode,
	OUT PVOID* DecodedData,
	OUT int *pDataLength
	)
{
	ULONG nIndex = 0, Data = 0, Data2 = 0 ;
	LPVOID Buffer = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	// 1. 取大小,分配足够的内存
	status = PBIPStoreParse( pNode, (LPBYTE)&Data );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	Buffer = HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, Data );
	*DecodedData = Buffer ;

	// 2. 读取指定长度(Data)的数据到Buffer中
	status = PBIPStoreParseExp( pNode, Data, (LPBYTE)Buffer );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	status = PBIPStoreParse( pNode, (LPBYTE)&Data2 );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	if ( ~Data2 != Data ) { return FALSE; }

	// 3. 解密
	*pDataLength = Data ;

	if ( Data > 0 )
	{
		do
		{
			*( (LPBYTE)Buffer + nIndex ) ^= nIndex ;
			++ nIndex ;
		}
		while ( nIndex < Data );
	}

	return TRUE;
}



BOOL CIPStoreIDataObject::PBIPStoreRelease( IN PSbiePst_dat_FileInfo pNode )
{
	ZwClose( pNode->hFile );
	return HeapFree( GetProcessHeap(), 0, pNode );
}



BOOL 
CIPStoreIDataObject::PBIPStoreBuild (
	IN PSbiePst_dat_FileInfo pNode
	)
{
	BOOL bInsert = FALSE ; 
	NTSTATUS status = STATUS_SUCCESS ;
	int nCounts = 0, ReturnLength = 0 ;
	PIPStoreVtbl_Type_Info pNode_father = NULL ;

	// 1. 分配新节点内存
	pNode_father = (PIPStoreVtbl_Type_Info) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info) );

	// 2.1 读取文件,将获得的 pType 内容填充至新节点
	status = PBIPStoreParseExp( pNode, 0x10, (LPBYTE)&pNode_father->pType );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	// 2.2 读取文件,将获得的 szDisplayName 内容填充至新节点
	if ( FALSE == PBIPStoreDecode( pNode, (PVOID*)&pNode_father->szDisplayName, &ReturnLength ) )  { return FALSE; }
	
	// 2.3 读取文件,将获得的Flag内容填充至新节点
	pNode_father->Flag = 0;
	if ( this->Info.LittleInfo.u.bFlag1 && PBIPStoreParse( pNode, (LPBYTE)&pNode_father->Flag ) ) { return FALSE; }
	
	//  
	// 3.1 继续读取PBPst.dat文件内容,失败直接返回;读取成功则判断当前一个字节的值,分两种情况:
	//     A) 为0,表明当前新节点无子节点,则插入到总链表中
	//     B) 不为0，表明当前新节点包含N个子节点,需将子节点插入到当前新节点的链表中; 而后执行A)流程
	//  

	status = PBIPStoreParse( pNode, (LPBYTE)&nCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }
	
	ClearStruct( (LPLIST_ENTRY_EX) &pNode_father->pSunNode );

	if ( 0 == nCounts )
	{
		bInsert = TRUE ;
	}
	else
	{
		while ( PBIPStoreBuildChildren( pNode, pNode_father ) )
		{
			-- nCounts ;
			if ( 0 == nCounts ) 
			{ 
				bInsert = TRUE ;
				break;
			}
		}
	}
	
	if ( bInsert )
	{
		InsertListA( &this->Info.LittleInfo.ListEntry, NULL, (PLIST_ENTRY)pNode_father );
		return TRUE;
	}	

	return FALSE ;
}



BOOL
CIPStoreIDataObject::PBIPStoreBuildChildren (
	IN PSbiePst_dat_FileInfo pNode,
	IN PIPStoreVtbl_Type_Info pNode_father
	)
{
	int nCounts = 0, ReturnLength = 0 ;
	BOOL bResult = FALSE, bInsert = FALSE ; 
	NTSTATUS status = STATUS_SUCCESS ;
	PIPStoreVtbl_Type_Info_sun pNode_child = NULL ;

	// 1. 分配新节点内存
	pNode_child = (PIPStoreVtbl_Type_Info_sun) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info_sun) );
	pNode_child->Flag = 0 ;

	// 2.1 读取文件,将获得的pType内容填充至新节点
	status = PBIPStoreParseExp( pNode, 0x10, (LPBYTE)&pNode_child->pType );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }
  
	// 2.2 读取文件,将获得的szDisplayName内容填充至新节点
	bResult = PBIPStoreDecode( pNode, (PVOID*)&pNode_child->szDisplayName, &ReturnLength );
	if ( FALSE == bResult ) { return FALSE; }
	
	// 2.3 读取文件,将获得的Flag内容填充至新节点
	if ( this->Info.LittleInfo.u.bFlag1 && PBIPStoreParse( pNode, (LPBYTE)&pNode_child->Flag ) ) { return FALSE; }
	
	//  
	// 3.1 继续读取PBPst.dat文件内容,失败直接返回;读取成功则判断当前一个字节的值,分两种情况:
	//     A) 为0,表明当前新节点无子节点,则插入到总链表中
	//     B) 不为0，表明当前新节点包含N个子节点,需将子节点插入到当前新节点的链表中; 而后执行A)流程
	//  

	status = PBIPStoreParse( pNode, (LPBYTE)&nCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	ClearStruct( (LPLIST_ENTRY_EX)&pNode_child->pGrandsunNode );

	if ( 0 == nCounts )
	{
		bInsert = TRUE ;
	}
	else
	{
		while ( PBIPStoreBuildGrandChildren(pNode, pNode_child) )
		{
			-- nCounts ;
			if ( 0 == nCounts ) 
			{ 
				bInsert = TRUE ;
				break;
			}
		}
	}

	if ( bInsert )
	{
		InsertListA( (LPLIST_ENTRY_EX)&pNode_father->pSunNode, 0, (PLIST_ENTRY)pNode_child );
		return TRUE;
	}	
	
	return FALSE;
}



BOOL
CIPStoreIDataObject::PBIPStoreBuildGrandChildren (
	IN PSbiePst_dat_FileInfo pNode,
	IN PIPStoreVtbl_Type_Info_sun pNode_father
	)
{
	int ReturnLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	PIPStoreVtbl_Type_Info_grandsun pNode_grand = NULL ;
	
	pNode_grand = (PIPStoreVtbl_Type_Info_grandsun) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info_grandsun) );

	if ( FALSE == PBIPStoreDecode( pNode, (PVOID *)&pNode_grand->szItemName, &ReturnLength ) )  { return FALSE; }

	pNode_grand->Flag = 0 ;

	if ( this->Info.LittleInfo.u.bFlag1 )
	{
		status = PBIPStoreParse( pNode, (LPBYTE)&pNode_grand->Flag );
		if ( ! NT_SUCCESS(status) ) { return FALSE; }
	}

	if ( FALSE == PBIPStoreDecode(pNode, (PVOID *)&pNode_grand->pbData, (int *)&pNode_grand->cbData) ) { return FALSE; }

	InsertListA( (LPLIST_ENTRY_EX)&pNode_father->pGrandsunNode, NULL, (PLIST_ENTRY)pNode_grand );

	return TRUE;
}



PVOID
CIPStoreIDataObject::PBIPStoreGetNode (
	IN const GUID * pType
	)
{
	PIPStoreVtbl_Type_Info pNode = NULL ;
	LPRPC_OUT_PBIPStoreGetNode pRpcOutBuffer = NULL ;
	RPC_IN_PBIPStoreGetNode pRpcInBuffer = { 0 } ;
	PST_TYPEINFO pInfo = { 0 } ;

	pNode = (PIPStoreVtbl_Type_Info) this->Info.LittleInfo.ListEntry.Flink ;

	// 1. 处理pNode存在的情况: 遍历链表,找到与参数二@pType对应的结点,返回之
	if ( pNode )
	{
		while ( TRUE )
		{
			if ( IsEqualIID( (REFIID)pNode->pType, (REFIID)pType ) ) 
			{ 
				// 找到了匹配的IID,合法则返回节点,不合法则返回空值
				if ( !(pNode->Flag & 1) ) { return pNode; }
				return NULL;
			}

			// 若未找到,需要RPC通信
			pNode = pNode->pFlink ;
			if ( NULL == pNode ) { break; }
		}
	}

	// 2. 处理pNode不存在的情况,通过RPC通信得到对应的szDisplayName,而后分配内存,填充返回之
	pRpcInBuffer.RpcHeader.Flag = 0x1101 ;
	pRpcInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_PBIPStoreGetNode ) ;
	memcpy( &pRpcInBuffer.iid_data, pType, sizeof(GUID) );

	// rpc通信
	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetNode) PB_CallServer( &pRpcInBuffer );
	if ( pRpcOutBuffer )
	{
		if ( STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
		{
			pInfo.cbSize = sizeof( PST_TYPEINFO ) ;
			pInfo.szDisplayName = pRpcOutBuffer->szDisplayName ;

			pNode = (PIPStoreVtbl_Type_Info) PBIPStoreInsertNode( pType, &pInfo );
		}

		PB_FreeReply( (PVOID)pRpcOutBuffer );
	}

	return (PVOID)pNode ;
}



PVOID
CIPStoreIDataObject::PBIPStoreGetChildNode (
	IN PIPStoreVtbl_Type_Info pNode,
	IN const GUID * pSubtype
	)
{
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;
	LPRPC_OUT_PBIPStoreGetChildNode pRpcOutBuffer = NULL ;
	RPC_IN_PBIPStoreGetChildNode pRpcInBuffer = { 0 };
	PST_TYPEINFO pInfo = { 0 } ;

	pChildNode = pNode->pSunNode.pFlink ;

	// 1. 处理pNode存在的情况: 遍历链表,找到与参数二@pType对应的结点,返回之
	if ( pChildNode )
	{
		while ( TRUE )
		{
			if ( IsEqualIID( (REFIID)pChildNode->pType, (REFIID)pSubtype ) ) 
			{ 
				// 找到了匹配的IID,合法则返回节点,不合法则返回空值
				if ( !(pChildNode->Flag & 1) ) { return pChildNode; }
				return NULL;
			}

			// 若未找到,需要RPC通信
			pChildNode = pChildNode->pFlink ;
			if ( NULL == pChildNode ) { break; }
		}
	}

	// 2. 处理pNode不存在的情况,通过RPC通信得到对应的szDisplayName,而后分配内存,填充返回之
	pRpcInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_PBIPStoreGetChildNode ) ;
	pRpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_PStoreGetChildNode_ ;

	memcpy( &pRpcInBuffer.iid_data, &pNode->pType, sizeof(GUID) );
	memcpy( &pRpcInBuffer.iid_data_sun , pSubtype, sizeof(GUID) );

	// rpc通信
	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetChildNode) PB_CallServer( &pRpcInBuffer );
	if ( pRpcOutBuffer )
	{
		if ( STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
		{
			pInfo.cbSize = sizeof( PST_TYPEINFO ) ;
			pInfo.szDisplayName = pRpcOutBuffer->szDisplayName ;

			pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreInsertChildNode( pNode, pSubtype, &pInfo );
		}

		PB_FreeReply( (PVOID)pRpcOutBuffer );
	}

	return (PVOID)pChildNode;
}



PVOID
CIPStoreIDataObject::PBIPStoreGetGrandchildNode (
	IN PIPStoreVtbl_Type_Info pNode,
	IN PIPStoreVtbl_Type_Info_sun pChildNode,
	IN LPWSTR szItemName
	)
{
	ULONG_PTR DataLength = 0 ;
	PIPStoreVtbl_Type_Info_grandsun pGrandsunNode = NULL ;
	LPRPC_IN_PBIPStoreGetGrandchildNode pRpcInBuffer = NULL ;
	LPRPC_OUT_PBIPStoreGetGrandchildNode pRpcOutBuffer = NULL ;

	pGrandsunNode = pChildNode->pGrandsunNode.pFlink ;

	// 1. 处理pNode存在的情况: 遍历链表,找到与参数二@pType对应的结点,返回之
	if ( pGrandsunNode )
	{
		while ( TRUE )
		{
			if ( 0 == wcscmp(pGrandsunNode->szItemName, szItemName) ) 
			{ 
				// 找到了匹配的IID,合法则返回节点,不合法则返回空值
				if ( !(pGrandsunNode->Flag & 1) ) { return pGrandsunNode; }
				return NULL;
			}

			// 若未找到,需要RPC通信
			pGrandsunNode = pGrandsunNode->pFlink ;
			if ( NULL == pGrandsunNode ) { break; }
		}
	}

	// 2. 处理pNode不存在的情况,通过RPC通信得到对应的szDisplayName,而后分配内存,填充返回之
	DataLength = 2 * wcslen(szItemName) + 0x32 ;
	pRpcInBuffer = (LPRPC_IN_PBIPStoreGetGrandchildNode) kmalloc( DataLength );

	pRpcInBuffer->RpcHeader.Flag = _PBSRV_APINUM_PStoreGetGrandchildNode_ ;
	pRpcInBuffer->RpcHeader.DataLength = DataLength ;
	
	memcpy( &pRpcInBuffer->iid_data, &pNode->pType, sizeof(GUID) );
	memcpy( &pRpcInBuffer->iid_data_sun , &pChildNode->pType, sizeof(GUID) );

	pRpcInBuffer->Length = wcslen(szItemName);
	wcscpy( pRpcInBuffer->szItemName, szItemName );

	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetGrandchildNode) PB_CallServer( pRpcInBuffer );
	if ( pRpcOutBuffer && STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
	{
		pGrandsunNode = (PIPStoreVtbl_Type_Info_grandsun) PBIPStoreSetGrandchildNode( pChildNode, NULL, szItemName, pRpcOutBuffer->cbData, (PBYTE)pRpcOutBuffer->Data );
	}

	kfree( pRpcInBuffer );
	if ( pRpcOutBuffer ) { PB_FreeReply( pRpcOutBuffer ); }

	return (PVOID)pGrandsunNode;
}



PVOID
CIPStoreIDataObject::PBIPStoreSetGrandchildNode (
	IN PIPStoreVtbl_Type_Info_sun pChildNode,
	IN PIPStoreVtbl_Type_Info_grandsun pGrandSunNode, 
	IN LPWSTR szItemName,
	IN DWORD cbData,
	IN PBYTE pbData
	)
{
	ULONG Length = 0 ;
	PIPStoreVtbl_Type_Info_grandsun pNew = pGrandSunNode ;

	if ( NULL == pGrandSunNode )
	{
		pNew = (PIPStoreVtbl_Type_Info_grandsun) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info_grandsun) );
		pNew->Flag = 0 ;

		InsertListB( (LPLIST_ENTRY_EX)&pChildNode->pGrandsunNode, NULL, (PLIST_ENTRY)pNew );
	}

	Length = 2 * wcslen(szItemName) + 2 ;
	pNew->szItemName = (LPWSTR) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, Length );
	memcpy( pNew->szItemName, szItemName, Length );

	pNew->pbData = 0 ;
	pNew->cbData = cbData ;

	if ( cbData )
	{
		pNew->pbData = (PBYTE) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, cbData );
		memcpy( pNew->pbData, pbData, cbData );
	}

	return (PVOID)pNew ;
}



LPVOID 
CIPStoreIDataObject::PBIPStoreInsertNode (
	IN const GUID * pType,
	IN PPST_TYPEINFO pInfo
	)
{
	ULONG length = 0 ;
	PIPStoreVtbl_Type_Info pNodeNew = NULL ;

	pNodeNew = (PIPStoreVtbl_Type_Info) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info) );

	pNodeNew->Flag = 0 ;
	ClearStruct( (LPLIST_ENTRY_EX)&pNodeNew->pSunNode );

	memcpy( &pNodeNew->pType , pType, sizeof(GUID) );

	length = 2 * wcslen( pInfo->szDisplayName ) + 2;
	pNodeNew->szDisplayName = (LPWSTR) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, length );
	memcpy( pNodeNew->szDisplayName, pInfo->szDisplayName, length );
	
	InsertListB( &this->Info.LittleInfo.ListEntry, 0, (PLIST_ENTRY)pNodeNew );
	return (LPVOID)pNodeNew ;
}



LPVOID
CIPStoreIDataObject::PBIPStoreInsertChildNode (
	IN PIPStoreVtbl_Type_Info pNode,
	IN const GUID * pSubtype,
	IN PPST_TYPEINFO pInfo
	)
{
	ULONG length = 0 ;
	PIPStoreVtbl_Type_Info_sun pNodeNew = NULL ;

	pNodeNew = (PIPStoreVtbl_Type_Info_sun) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(IPStoreVtbl_Type_Info_sun) );

	pNodeNew->Flag = 0 ;
	ClearStruct( (LPLIST_ENTRY_EX)&pNodeNew->pGrandsunNode );

	memcpy( &pNodeNew->pType , pSubtype, sizeof(GUID) );

	length = 2 * wcslen( pInfo->szDisplayName ) + 2;
	pNodeNew->szDisplayName = (LPWSTR) HeapAlloc( this->Info.hHeap, HEAP_GENERATE_EXCEPTIONS, length );
	memcpy( pNodeNew->szDisplayName, pInfo->szDisplayName, length );

	InsertListB( (LPLIST_ENTRY_EX)&pNode->pSunNode, NULL, (PLIST_ENTRY)pNodeNew );
	return (LPVOID)pNodeNew;
}



VOID CIPStoreIDataObject::PBIPStoreHandlerFile(  )
{
	PSbiePst_dat_FileInfo pNode = NULL ;
	PIPStoreVtbl_Type_Info i    = NULL ;

	if ( NULL == this->Info.LittleInfo.lpMappedAddr ) { return; }

	WaitForSingleObject( this->Info.hMutex, INFINITE );

	pNode = (PSbiePst_dat_FileInfo) PBIPStoreAllocateNode( TRUE );
	if ( pNode )
	{
		PBIPStoreWriteFilePhrase1( pNode, 0xA1B2C3D4 );
		PBIPStoreWriteFilePhrase1( pNode, this->Info.LittleInfo.ListEntry.TotalCounts );

		for ( i = (PIPStoreVtbl_Type_Info) this->Info.LittleInfo.ListEntry.Flink; i; i = i->pFlink )
		{
			if ( FALSE == PBIPStoreWriteFileNodes( pNode, i ) ) { break; }
		}

		PBIPStoreWriteFile( pNode );
		PBIPStoreRelease( pNode );
	}

	ReleaseMutex( this->Info.hMutex );
	return;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreWriteFilePhrase1 (
	IN PSbiePst_dat_FileInfo pNode,
	IN int nCounts
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 

	if ( 0xFF0 == pNode->DataLength )
	{
		status = PBIPStoreWriteFile( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}

	*pNode->pDataPointer = nCounts & 0x000f ;
	++ pNode->DataLength ;
	++ pNode->pDataPointer ;

	if ( 0xFF0 == pNode->DataLength )
	{
		status = PBIPStoreWriteFile( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}

	*pNode->pDataPointer = (nCounts & 0x00f0) >> 8 /*BYTE1(nCounts)*/ ;
	++ pNode->DataLength ;
	++ pNode->pDataPointer ;

	if ( 0xFF0 == pNode->DataLength )
	{
		status = PBIPStoreWriteFile( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}
	
	*pNode->pDataPointer = (nCounts & 0x0f00) >> 16 /*BYTE2(nCounts)*/ ;
	++ pNode->DataLength ;
	++ pNode->pDataPointer ;

	if ( 0xFF0 == pNode->DataLength )
	{
		status = PBIPStoreWriteFile( pNode );
		if ( ! NT_SUCCESS(status) ) { return status; }
	}
	
	*pNode->pDataPointer = (nCounts & 0xf000) >> 24 /*BYTE3(nCounts)*/ ;
	++ pNode->DataLength ;
	++ pNode->pDataPointer ;
	
	return STATUS_SUCCESS ;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreWriteFilePhrase2 (
	IN PSbiePst_dat_FileInfo pFileNode,
	IN UINT nCounts,
	IN PBYTE pData
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	IO_STATUS_BLOCK IoStatusBlock ;

	if ( 0 == nCounts ) { return STATUS_SUCCESS; }
	
	while ( TRUE )
	{
		if ( pFileNode->DataLength == 0xFF0 )
		{
			status = ZwWriteFile( pFileNode->hFile, 0, 0, 0, &IoStatusBlock, pFileNode->pBuffer, 0xFF0, 0, 0 );
			if ( NT_SUCCESS(status) && IoStatusBlock.Information != pFileNode->DataLength )
			{
				status = STATUS_DISK_FULL ;
			}

			pFileNode->DataLength = 0;
			pFileNode->pDataPointer = (PBYTE)pFileNode->pBuffer ;
			if ( status < 0 ) { break; }
		}

		*pFileNode->pDataPointer = *pData ;
		
		++ pData ;
		++ pFileNode->pDataPointer ;
		++ pFileNode->DataLength ;

		-- nCounts ;
		if ( 0 == nCounts ) { return STATUS_SUCCESS; }
	}

	return status;
}



BOOL
CIPStoreIDataObject::PBIPStoreWriteFileNodes (
	IN PSbiePst_dat_FileInfo pFileNode,
	IN PIPStoreVtbl_Type_Info pNode
	)
{
	ULONG Length = 0 ;
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;
	PIPStoreVtbl_Type_Info_sun pNode_sun = NULL ;

	status = PBIPStoreWriteFilePhrase2( pFileNode, 0x10, (PBYTE)&pNode->pType );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }
	
	Length = 2 * wcslen(pNode->szDisplayName) + 2 ;
	bRet = PBIPStoreEncodeFile( pFileNode, (PBYTE)pNode->szDisplayName, Length );
	if ( FALSE == bRet ) { return FALSE; }
	
	status = PBIPStoreWriteFilePhrase1( pFileNode, pNode->Flag );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	status = PBIPStoreWriteFilePhrase1( pFileNode, pNode->pSunNode.nSunCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }
	
	pNode_sun = pNode->pSunNode.pFlink ;
	if ( NULL == pNode_sun ) { return TRUE; }

	while ( PBIPStoreWriteFileChiledNodes( pFileNode, pNode_sun ) )
	{
		pNode_sun = pNode_sun->pFlink;
		if ( NULL == pNode_sun ) { return TRUE; }
	}

	return FALSE ;
}



BOOL
CIPStoreIDataObject::PBIPStoreWriteFileChiledNodes (
	IN PSbiePst_dat_FileInfo pFileNode, 
	IN PIPStoreVtbl_Type_Info_sun pNode_sun
	)
{
	ULONG Length = 0 ;
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;
	PIPStoreVtbl_Type_Info_grandsun pNode_grandsun = NULL;

	status = PBIPStoreWriteFilePhrase2( pFileNode, 0x10, (PBYTE)&pNode_sun->pType );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	Length = 2 * wcslen(pNode_sun->szDisplayName) + 2 ;
	bRet = PBIPStoreEncodeFile( pFileNode, (PBYTE)pNode_sun->szDisplayName, Length );
	if ( FALSE == bRet ) { return FALSE; }

	status = PBIPStoreWriteFilePhrase1( pFileNode, pNode_sun->Flag );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	status = PBIPStoreWriteFilePhrase1( pFileNode, pNode_sun->pGrandsunNode.nGrandsunCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	pNode_grandsun = pNode_sun->pGrandsunNode.pFlink;
	if ( NULL == pNode_grandsun ) { return TRUE; }

	while ( PBIPStoreWriteFileGrandchildNodes( pFileNode, pNode_grandsun ) )
	{
		pNode_grandsun = pNode_grandsun->pFlink;
		if ( NULL == pNode_grandsun ) { return TRUE; }
	}

	return FALSE;
}



BOOL
CIPStoreIDataObject::PBIPStoreWriteFileGrandchildNodes (
	IN PSbiePst_dat_FileInfo pFileNode, 
	IN PIPStoreVtbl_Type_Info_grandsun pNode
	)
{
	ULONG Length = 0 ;
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;

	Length = 2 * wcslen(pNode->szItemName) + 2 ;
	bRet = PBIPStoreEncodeFile( pFileNode, (PBYTE)pNode->szItemName, Length );
	if ( FALSE == bRet ) { return FALSE; }

	status = PBIPStoreWriteFilePhrase1( pFileNode, pNode->Flag );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	bRet = PBIPStoreEncodeFile( pFileNode, pNode->pbData, pNode->cbData );
	return bRet ;
}



BOOL
CIPStoreIDataObject::PBIPStoreEncodeFile (
	IN PSbiePst_dat_FileInfo pFileNode,
	IN PBYTE pBuffer,
	IN int nCounts
	)
{
	int i = 0 ; 
	NTSTATUS status = STATUS_SUCCESS ;
	
	status = PBIPStoreWriteFilePhrase1( pFileNode, nCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	for ( i = 0; i < nCounts; ++i )
	{
		pBuffer[ i ] ^= i ;
	}

	status = PBIPStoreWriteFilePhrase2( pFileNode, nCounts, pBuffer );

	for ( i = 0; i < nCounts; ++i )
	{
		pBuffer[ i ] ^= i ;
	}

	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	status = PBIPStoreWriteFilePhrase1( pFileNode, ~nCounts );
	if ( ! NT_SUCCESS(status) ) { return FALSE; }
	
	return TRUE;
}



NTSTATUS 
CIPStoreIDataObject::PBIPStoreWriteFile ( 
	IN PSbiePst_dat_FileInfo pFileNode
	)
{
	ULONG DataLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	IO_STATUS_BLOCK IoStatusBlock ;

	DataLength = pFileNode->DataLength ;
	if ( 0 == DataLength ) { return STATUS_SUCCESS; }
	
	status = ZwWriteFile( pFileNode->hFile, 0, 0, 0, &IoStatusBlock, pFileNode->pBuffer, DataLength, 0, 0 );

	if ( NT_SUCCESS(status) && IoStatusBlock.Information != pFileNode->DataLength )
	{
		status = STATUS_DISK_FULL ;
	}

	pFileNode->pDataPointer = (PBYTE)pFileNode->pBuffer ;
	pFileNode->DataLength = 0 ;

	return status ;
}



PVOID CIPStoreIDataObject::PBIPStoreEnumTypes( IN int Key )
{
	int n = 0 ;
	CLSID *pCurrentClsid = NULL ;
	PIPStoreVtbl_Type_Info i = NULL ;
	LPPBIPStoreEnumTypes_Info pNode = NULL ;
	LPPBIPStoreEnumTypes_Little_Info pNodeLittle = NULL ;
	LPRPC_OUT_PBIPStoreEnumTypes pRpcOutBuffer = NULL ;
	RPC_IN_PBIPStoreEnumTypes RpcInBuffer ;

	pNode = (LPPBIPStoreEnumTypes_Info) kmalloc( sizeof(PBIPStoreEnumTypes_Info) );
	if ( pNode )
	{
		pNodeLittle = (LPPBIPStoreEnumTypes_Little_Info) kmalloc( sizeof(PBIPStoreEnumTypes_Little_Info) );

		pNode->pNodeLittle = pNodeLittle;
		ClearStruct( &pNodeLittle->ListEntryEx );

		pNode->pNodeLittle->Reserved2 = 0 ;
		pNode->pNodeLittle->Reserved1 = 1;
		pNode->Reserved1 = 0 ;
		pNode->Reserved2 = 1 ;
		pNode->lpVtbl = &g_IEnumPStoreTypesVtbl_array ;
	}

	RtlZeroMemory( &RpcInBuffer, sizeof(RPC_IN_PBIPStoreEnumTypes) );

	RpcInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_PBIPStoreEnumTypes ) ;
	RpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_PStoreEnumSubtypes_ ;
	RpcInBuffer.Key = Key ;

	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreEnumTypes) PB_CallServer( &RpcInBuffer );
	if ( pRpcOutBuffer && STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
	{
		if ( pRpcOutBuffer->nCounts )
		{
			pCurrentClsid = (CLSID *)pRpcOutBuffer->Data ;
			do
			{
				PBIPStoreAddClsid( pNode, pCurrentClsid );

				++ n ;
				++ pCurrentClsid ;
			}
			while ( n < pRpcOutBuffer->nCounts );
		}
	}

	if ( pRpcOutBuffer ) { PB_FreeReply( pRpcOutBuffer ); }

	if ( 0 == Key )
	{
		for ( i = (PIPStoreVtbl_Type_Info)this->Info.LittleInfo.ListEntry.Flink; i; i = i->pFlink )
		{
			PBIPStoreAddClsid( pNode, (LPCLSID)&i->pType );
		}
	}

	if ( !LOBYTE(pNode->pNodeLittle->Reserved2) )
		pNode->Reserved1 = (ULONG) pNode->pNodeLittle->ListEntryEx.Flink ;

	return pNode ;
}



PVOID CIPStoreIDataObject::PBIPStoreEnumSubtypes( int Key, const GUID *pType )
{
	int n = 0 ;
	CLSID *pCurrentClsid = NULL ;
	PIPStoreVtbl_Type_Info_sun i = NULL ;
	PIPStoreVtbl_Type_Info TypeNode = NULL ;
	LPPBIPStoreEnumTypes_Info pNode = NULL ;
	LPPBIPStoreEnumTypes_Little_Info pNodeLittle = NULL ;
	LPRPC_OUT_PBIPStoreEnumSubtypes pRpcOutBuffer = NULL ;
	RPC_IN_PBIPStoreEnumSubtypes RpcInBuffer ;

	pNode = (LPPBIPStoreEnumTypes_Info) kmalloc( sizeof(PBIPStoreEnumTypes_Info) );
	if ( pNode )
	{
		pNodeLittle = (LPPBIPStoreEnumTypes_Little_Info) kmalloc( sizeof(PBIPStoreEnumTypes_Little_Info) );
		
		pNode->pNodeLittle = pNodeLittle;
		ClearStruct( &pNodeLittle->ListEntryEx );
		
		pNode->pNodeLittle->Reserved2 = 0 ;
		pNode->pNodeLittle->Reserved1 = 1;
		pNode->Reserved1 = 0 ;
		pNode->Reserved2 = 1 ;
		pNode->lpVtbl = &g_IEnumPStoreTypesVtbl_array ;
	}

	TypeNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pType );
	if ( NULL == TypeNode )
	{
		pNode->pNodeLittle->Reserved2 = 1 ;
		return pNode;
	}

	RpcInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_PBIPStoreEnumSubtypes ) ;
	RpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_PStoreEnumSubtypes_ ;
	RpcInBuffer.Key = 0;
	RpcInBuffer.bEnumSubtypes = TRUE ;

	memcpy( &RpcInBuffer.ClsidType, pType, sizeof(GUID) );

	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreEnumSubtypes) PB_CallServer( &RpcInBuffer );

	if ( pRpcOutBuffer && STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
	{
		if ( pRpcOutBuffer->nCounts )
		{
			pCurrentClsid = (CLSID *)pRpcOutBuffer->Data ;
			do
			{
				PBIPStoreAddClsid( pNode, pCurrentClsid );

				++ n ;
				++ pCurrentClsid ;
			}
			while ( n < pRpcOutBuffer->nCounts );
		}
	}

	if ( pRpcOutBuffer ) { PB_FreeReply( pRpcOutBuffer ); }

	if ( 0 == Key )
	{
		for ( i = TypeNode->pSunNode.pFlink; i; i = i->pFlink )
		{
			PBIPStoreAddClsid( pNode, (LPCLSID)&i->pType );
		}
	}

	if ( !LOBYTE(pNode->pNodeLittle->Reserved2) )
		pNode->Reserved1 = (ULONG) pNode->pNodeLittle->ListEntryEx.Flink;

	return pNode;
}



VOID CIPStoreIDataObject::PBIPStoreAddClsid( LPPBIPStoreEnumTypes_Info pNode, LPCLSID clsid )
{
	int ret = 0 ;
	WCHAR guid1[ MAX_PATH ] = L"" ;
	WCHAR guid2[ MAX_PATH ] = L"" ;
	LPPBIPStoreEnumTypes_CLSID_Info pNew = NULL ;
	LPPBIPStoreEnumTypes_CLSID_Info pNodeCLSID = NULL ;

	swprintf ( 
		guid1,
		L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		clsid->Data1,
		clsid->Data2,
		clsid->Data3,
		clsid->Data4[0],
		clsid->Data4[1],
		clsid->Data4[2],
		clsid->Data4[3],
		clsid->Data4[4],
		clsid->Data4[5],
		clsid->Data4[6],
		clsid->Data4[7]
	);

	pNodeCLSID = (LPPBIPStoreEnumTypes_CLSID_Info) pNode->pNodeLittle->ListEntryEx.Flink ;

	if ( pNodeCLSID )
	{
		while ( TRUE )
		{
			swprintf (
				guid2,
				L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				pNodeCLSID->clsid.Data1,
				pNodeCLSID->clsid.Data2,
				pNodeCLSID->clsid.Data3,
				pNodeCLSID->clsid.Data4[0],
				pNodeCLSID->clsid.Data4[1],
				pNodeCLSID->clsid.Data4[2],
				pNodeCLSID->clsid.Data4[3],
				pNodeCLSID->clsid.Data4[4],
				pNodeCLSID->clsid.Data4[5],
				pNodeCLSID->clsid.Data4[6],
				pNodeCLSID->clsid.Data4[7]
			);

			ret = wcscmp( guid2, guid1 );
			if ( 0 == ret ) { break; }

			if ( ret <= 0 )
			{
				pNodeCLSID = (LPPBIPStoreEnumTypes_CLSID_Info) pNodeCLSID->ListEntry.Flink ;
				if ( pNodeCLSID ) { continue; }
			}

			goto _insert_ ;
		}
	}
	else
	{
_insert_:
		pNew = (LPPBIPStoreEnumTypes_CLSID_Info) kmalloc( sizeof(PBIPStoreEnumTypes_CLSID_Info) );

		memcpy( &pNew->clsid, clsid, sizeof(GUID) );

		if ( pNodeCLSID )
		{
			InsertListB( &pNode->pNodeLittle->ListEntryEx, &pNodeCLSID->ListEntry, &pNew->ListEntry );
		}
		else
		{
			InsertListA( &pNode->pNodeLittle->ListEntryEx, NULL, &pNew->ListEntry );
		}
	}

	return;
}



PVOID CIPStoreIDataObject::PBIPStoreEnumItems( ULONG key, const GUID * pItemtype, const GUID * pItemSubtype )
{
	int n = 0 ;
	LPWSTR szItemName = NULL ;
	LPPBIPStoreEnumItems_Info pNode = NULL ;
	PIPStoreVtbl_Type_Info TypeNode = NULL ;
	PIPStoreVtbl_Type_Info_sun pChildNode = NULL ;
	PIPStoreVtbl_Type_Info_grandsun i = NULL ;
	LPRPC_OUT_PBIPStoreEnumItems pRpcOutBuffer = NULL ;
	RPC_IN_PBIPStoreEnumItems RpcInBuffer ;

	pNode = (LPPBIPStoreEnumItems_Info) kmalloc( sizeof(PBIPStoreEnumItems_Info) );
	if ( pNode )
	{
		pNode->pNodeLittle = (LPPBIPStoreEnumItems_Little_Info) kmalloc( sizeof(PBIPStoreEnumItems_Little_Info) );

		ClearStruct( &pNode->pNodeLittle->ListEntryEx );
		pNode->pNodeLittle->Reserved1 = 1 ;
		pNode->pNodeLittle->Reserved2 = 0 ;
		pNode->Reserved1 = 0 ;
		pNode->Reserved2 = 1 ;
		pNode->CoTaskMemAlloc_addr = this->Info.CoTaskMemAlloc_addr ;
		pNode->lpVtbl = &g_IEnumPStoreTypes_array ;
	}

	TypeNode = (PIPStoreVtbl_Type_Info) PBIPStoreGetNode( pItemtype );
	if ( NULL == TypeNode ) 
	{ 
		pNode->pNodeLittle->Reserved2 = 0 ;
		return pNode; 
	}

	pChildNode = (PIPStoreVtbl_Type_Info_sun) PBIPStoreGetChildNode( TypeNode, pItemSubtype );
	if ( NULL == pChildNode ) 
	{ 
		pNode->pNodeLittle->Reserved2 = 0 ;
		return pNode; 
	}

	memset( &RpcInBuffer, 0, sizeof(RPC_IN_PBIPStoreEnumItems) );

	RpcInBuffer.Key = key ;
	memcpy( &RpcInBuffer.ClsidType, pItemtype, sizeof(GUID) );
	memcpy( &RpcInBuffer.ClsidStubType, pItemSubtype, sizeof(GUID) );

	RpcInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_PBIPStoreEnumItems ) ;
	RpcInBuffer.RpcHeader.Flag = 0x1105 ;

	pRpcOutBuffer = (LPRPC_OUT_PBIPStoreEnumItems) PB_CallServer( &RpcInBuffer );
	if ( pRpcOutBuffer && STATUS_SUCCESS == pRpcOutBuffer->RpcHeader.u.Status )
	{
		if ( pRpcOutBuffer->nCounts )
		{
			szItemName = pRpcOutBuffer->Data ;
			do
			{
				PBIPStoreEnumItemsEx( pNode, szItemName );

				szItemName += wcslen(szItemName) + 1 ;
				++ n ;
			}
			while ( n < pRpcOutBuffer->nCounts );
		}
	}
	
	if ( pRpcOutBuffer ) { PB_FreeReply( pRpcOutBuffer ); }

	if ( 0 == key )
	{
		for ( i = pChildNode->pGrandsunNode.pFlink; i; i = i->pFlink )
		{
			PBIPStoreEnumItemsEx( pNode, i->szItemName );
		}
	}

	if ( !LOBYTE(pNode->pNodeLittle->Reserved2) )
	{
		pNode->Reserved1 = (ULONG) pNode->pNodeLittle->ListEntryEx.Flink ;
	}

	return pNode;
}



VOID CIPStoreIDataObject::PBIPStoreEnumItemsEx( LPPBIPStoreEnumItems_Info pNode, LPWSTR szItemName )
{
	int ret = 0 ;
	BOOL bInsert = FALSE ;
	LPPBIPStoreEnumItems_Datas pBuffer  = NULL ;
	LPPBIPStoreEnumItems_Datas pNew		= NULL ;

	pBuffer = (LPPBIPStoreEnumItems_Datas) pNode->pNodeLittle->ListEntryEx.Flink ;
	if ( NULL == pBuffer ) 
	{ 
		bInsert = TRUE ; 
	}
	else
	{
		while ( TRUE )
		{
			ret = wcsicmp( pBuffer->szItemName, szItemName );
			if ( 0 == ret ) { break; }

			if ( ret <= 0 )
			{
				pBuffer = (LPPBIPStoreEnumItems_Datas) pBuffer->ListEntry.Flink ;
				if ( pBuffer ) { continue; }
			}

			bInsert = TRUE ; 
			break ;
		}

	}

	if ( FALSE == bInsert ) { return; }

	pNew = (LPPBIPStoreEnumItems_Datas) kmalloc( 2 * wcslen(szItemName) + 0x1A );

	wcscpy( pNew->szItemName, szItemName );
	
	if ( pBuffer ) 
	{
		InsertListB( &pNode->pNodeLittle->ListEntryEx, &pBuffer->ListEntry, &pNew->ListEntry );
	} 
	else 
	{
		InsertListA( &pNode->pNodeLittle->ListEntryEx, NULL, &pNew->ListEntry );
	}

	return;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


HRESULT WINAPI CENUMPSTORETPYES::QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject )
{
// 	this->lpVtbl->AddRef( (IDataObject *)this );
// 	*(DWORD *)ppvObject = (DWORD)this ;
// 	return 0;

	LPOLEClipbrdEx This = (LPOLEClipbrdEx) iface ;

	This->lpVtbl1->AddRef( (IDataObject *)This );
	*(DWORD *)ppvObject = (DWORD)This ;
	return 0;
}



ULONG WINAPI CENUMPSTORETPYES::AddRef( IDataObject* iface )
{
	return InterlockedIncrement( &this->m_dwRef );
}



ULONG WINAPI CENUMPSTORETPYES::Release( IDataObject* iface )
{
	ULONG RefCounts= 0 ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) iface ;

	RefCounts = This->Ref - 1 ;
	This->Ref = RefCounts ;

	if ( 0 == RefCounts )
	{
		This->lpVtbl = &g_IEnumPStoreTypes_array ;

		IEnumPStoreTypes_Release_dep( This );
		kfree( This );
		RefCounts = 0;
	}

	return RefCounts;
}



HRESULT WINAPI CENUMPSTORETPYES::Next( ULONG celt,LPWSTR *rgelt,ULONG *pceltFethed )
{
	ULONG Length = 0 ;
	LPWSTR Buffer = NULL ;
	LPEnumPStoreTypes_Little_Info LittleInfo = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( pceltFethed ) { *pceltFethed = 0; }

	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }
	
	if ( 0 == celt ) { return 0; }

	while ( TRUE )
	{
		LittleInfo = This->LittleInfo ;
		if ( NULL == LittleInfo ) { break; }

		Length = wcslen( LittleInfo->szName );
		Buffer = (LPWSTR)((int (__stdcall *)(DWORD))This->Func)(2 * Length + 2);
		if ( NULL == Buffer ) { return E_OUTOFMEMORY; }

		wcscpy( Buffer, This->LittleInfo->szName );
		*rgelt = Buffer ;
		++ rgelt ;

		if ( pceltFethed ) { ++*pceltFethed ; }

		This->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->LittleInfo->ListEntry.Flink ;
		-- celt ;
		if ( 0 == celt ) { return 0; }
	}

	return 0x80070103;
}



HRESULT WINAPI CENUMPSTORETPYES::Skip ( ULONG celt )
{
	ULONG Length = 0 ;
	LPWSTR Buffer = NULL ;
	LPEnumPStoreTypes_Little_Info LittleInfo = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }

	if ( 0 == celt ) { return 0; }
	if ( 0 == This->Ref ) { return DIERR_NOTFOUND; }

	while ( TRUE )
	{
		LittleInfo = This->LittleInfo ;
		if ( NULL == LittleInfo ) { break; }

		This->LittleInfo = (LPEnumPStoreTypes_Little_Info) LittleInfo->ListEntry.Flink ;
		-- celt ;
		if ( 0 == celt ) { return 0; }
	}

	return 0x80070103;
}



HRESULT WINAPI CENUMPSTORETPYES::Reset ( )
{
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }

	This->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->ListHead->ListEntryEx.Flink ;
	return 0;
}



HRESULT WINAPI CENUMPSTORETPYES::Clone ( IEnumPStoreTypesInterface** ppenum )
{
	LPEnumPStoreTypes_Info pNew = NULL ;
	LPEnumPStoreTypes_Info_List pNode = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	pNew = (LPEnumPStoreTypes_Info) kmalloc( sizeof(EnumPStoreTypes_Info) );
	if ( NULL == pNew )
	{
		*ppenum = NULL;
		return 0 ;
	}
	
	pNode = (LPEnumPStoreTypes_Info_List) This->ListHead->ListEntryEx.Flink;
	pNew->ListHead = pNode ;
	++ pNode->Ref ;

	pNew->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->ListHead->ListEntryEx.Blink ;
	pNew->Ref = 1 ;
	pNew->lpVtbl = &g_IEnumPStoreTypes_array;
	pNew->Func = This->Func;

	*ppenum = (IEnumPStoreTypesInterface*)pNew ;
	return 0 ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


HRESULT WINAPI CENUMPSTORETPYESVTBL::QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject )
{
	// 	this->lpVtbl->AddRef( (IDataObject *)this );
	// 	*(DWORD *)ppvObject = (DWORD)this ;
	// 	return 0;

	LPOLEClipbrdEx This = (LPOLEClipbrdEx) iface ;

	This->lpVtbl1->AddRef( (IDataObject *)This );
	*(DWORD *)ppvObject = (DWORD)This ;
	return 0;
}



ULONG WINAPI CENUMPSTORETPYESVTBL::AddRef( IDataObject* iface )
{
	return InterlockedIncrement( &this->m_dwRef );
}



ULONG WINAPI CENUMPSTORETPYESVTBL::Release ( IDataObject* iface )
{
	ULONG RefCounts= 0 ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) iface ;

	RefCounts = This->Ref - 1 ;
	This->Ref = RefCounts ;

	if ( 0 == RefCounts )
	{
		This->lpVtbl = &g_IEnumPStoreTypesVtbl_array ;

		IEnumPStoreTypes_Release_dep( This );
		kfree( This );
		RefCounts = 0;
	}

	return RefCounts;
}



HRESULT WINAPI CENUMPSTORETPYESVTBL::Next ( ULONG celt,GUID *rgelt,ULONG *pceltFethed )
{
	ULONG Length = 0 ;
	LPWSTR Buffer = NULL ;
	LPEnumPStoreTypes_Little_Info LittleInfo = NULL ;
	LPEnumPStoreTypes_Info_List pListHead = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( pceltFethed ) { *pceltFethed = 0; }

	pListHead = This->ListHead ;
	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }

	if ( 0 == celt ) { return 0; }

	if ( 0 == pListHead->ListEntryEx.TotalCounts ) { return DIERR_NOTFOUND; }

	while ( TRUE )
	{
		LittleInfo = This->LittleInfo ;
		if ( NULL == LittleInfo ) { break; }

		memcpy( rgelt, (GUID *)LittleInfo->szName, sizeof(GUID) );

		if ( pceltFethed ) { ++ *pceltFethed ; }

		This->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->LittleInfo->ListEntry.Flink ;
		-- celt ;
		if ( 0 == celt ) { return 0; }
	}

	return 0x80070103;
}



HRESULT WINAPI CENUMPSTORETPYESVTBL::Skip ( ULONG celt )
{
	ULONG Length = 0 ;
	LPWSTR Buffer = NULL ;
	LPEnumPStoreTypes_Little_Info LittleInfo = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }

	if ( 0 == celt ) { return 0; }
	if ( 0 == This->Ref ) { return DIERR_NOTFOUND; }

	while ( TRUE )
	{
		LittleInfo = This->LittleInfo ;
		if ( NULL == LittleInfo ) { break; }

		This->LittleInfo = (LPEnumPStoreTypes_Little_Info) LittleInfo->ListEntry.Flink ;
		-- celt ;
		if ( 0 == celt ) { return 0; }
	}

	return 0x80070103;
}



HRESULT WINAPI CENUMPSTORETPYESVTBL::Reset ( )
{
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	if ( LOBYTE(This->ListHead->bFlag) ) { return INET_E_RESOURCE_NOT_FOUND; }

	This->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->ListHead->ListEntryEx.Flink ;
	return 0;
}



HRESULT WINAPI CENUMPSTORETPYESVTBL::Clone ( IEnumPStoreTypesVtblInterface** ppenum )
{
	LPEnumPStoreTypesVtbl_Info pNew = NULL ;
	LPEnumPStoreTypes_Info_List pNode = NULL ;
	LPEnumPStoreTypes_Info This = (LPEnumPStoreTypes_Info) this ;

	pNew = (LPEnumPStoreTypesVtbl_Info) kmalloc( sizeof(EnumPStoreTypesVtbl_Info) );
	if ( NULL == pNew )
	{
		*ppenum = NULL;
		return 0 ;
	}

	pNode = (LPEnumPStoreTypes_Info_List) This->ListHead->ListEntryEx.Flink;
	pNew->ListHead = pNode ;
	++ pNode->Ref ;

	pNew->LittleInfo = (LPEnumPStoreTypes_Little_Info) This->ListHead->ListEntryEx.Blink ;
	pNew->Ref = 1 ;
	pNew->lpVtbl = &g_IEnumPStoreTypesVtbl_array ;
	
	*ppenum = (IEnumPStoreTypesVtblInterface*)pNew ;
	return 0 ;
}


//////////////////////////////////////////////////////////////////////////


void  IEnumPStoreTypes_Release_dep( LPEnumPStoreTypes_Info This )
{
	LPEnumPStoreTypes_Info_List pNodeHead = NULL ;
	LPEnumPStoreTypes_Info_List pCurrentNode = NULL ;

	pNodeHead = (LPEnumPStoreTypes_Info_List) This->ListHead->ListEntryEx.Blink ;
	-- pNodeHead->Ref ;

	if ( pNodeHead->Ref ) { return; }

	pCurrentNode = (LPEnumPStoreTypes_Info_List) pNodeHead->ListEntryEx.Flink ;
	if ( pCurrentNode )
	{
		do
		{
			RemoveEntryListEx( &pNodeHead->ListEntryEx, (PLIST_ENTRY)&pCurrentNode->ListEntryEx.Flink );
			kfree( pCurrentNode );

			pNodeHead = (LPEnumPStoreTypes_Info_List) This->ListHead->ListEntryEx.Blink ;
			pCurrentNode = (LPEnumPStoreTypes_Info_List) pNodeHead->ListEntryEx.Blink ;
		}
		while ( pCurrentNode );
	}

	kfree( pNodeHead );
	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////