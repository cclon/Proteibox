/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/29 [29:1:2012 - 14:51]
* MODULE : \Code\Project\ProteinBoxDLL\PBOle32dll\PBOle32dllData.cpp
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

#include "../StdAfx.h"
#include "../common.h"
#include "../PBDynamicData.h"
#include "../PBComData.h"
#include "../PBFilesData.h"
#include "../PBRegs.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../MemoryManager.h"
#include "../PBUser32dll/PBUser32dll.h"
#include "PBOle32dllData.h"


//////////////////////////////////////////////////////////////////////////

_CoInitializeEx_ g_CoInitializeEx_addr = NULL ;
_RegisterDragDrop_ g_RegisterDragDrop_addr = NULL ;
_RevokeDragDrop_ g_RevokeDragDrop_addr = NULL ;
_ReleaseStgMedium_ g_ReleaseStgMedium_addr = NULL ;
_OleSetClipboard_ g_OleSetClipboard_addr = NULL ;
_NdrAsyncClientCall_ g_NdrAsyncClientCall_addr = NULL ;
_RpcAsyncCompleteCall_ g_RpcAsyncCompleteCall_addr = NULL ;

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
FixDropFile (
	PVOID p, 
	IDataObject *pIDataObjectSrc,
	int hdrop
	)
{
	HMODULE hModule = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx) p ;

	This->lpVtbl1 = (COLEClipbrdIDataObject*) &g_OLEClipbrd_IDataObject_VTable ;
	This->lpVtbl2 = (COLEClipbrdIEnumFORMATETC*) &g_OLEClipbrd_IEnumFORMATETC_VTable ;

	hModule = LoadLibraryW(L"shell32.dll");
	This->hModule_shell32_dll = hModule;
	if ( hModule )
		This->DragQueryFileW_Addr = (ULONG) GetProcAddress( hModule, "DragQueryFileW" );
	else
		This->DragQueryFileW_Addr = 0 ;

	This->pIDataObjectSrc = pIDataObjectSrc ;
	if ( pIDataObjectSrc ) { pIDataObjectSrc->AddRef(); }

	This->hdrop = hdrop ;
	This->RefCounts = 1 ;

	return (PVOID)This ;
}


BOOL
IStorageWriteFile (
	LPWSTR lpFileName,
	int CStorage
	)
{
	BOOL Ret = TRUE ;
	HANDLE hFile = NULL ;
	OLECHAR *Buffer = NULL ;
	DWORD n = 0 , NumberOfBytesWritten = 0, nNumberOfBytesToWrite = 0 ;

	typedef VOID (WINAPI * _xxFunc_) (int, OLECHAR *, unsigned int, DWORD *);
	_xxFunc_ Func = NULL ;

	hFile = CreateFileW( lpFileName, GENERIC_WRITE, 7, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( hFile == (HANDLE)INVALID_HANDLE_VALUE ) { return FALSE; }
	
	Func = *(_xxFunc_*)( *(DWORD *)CStorage + 0xC );

	Buffer = (OLECHAR *) kmalloc( 0x4000 );
	Func( CStorage, Buffer, 0x4000, &nNumberOfBytesToWrite );

	if ( 0 == nNumberOfBytesToWrite ) { goto _over_; }
	
	n = nNumberOfBytesToWrite;
	while ( TRUE )
	{
		NumberOfBytesWritten = 0;
		WriteFile( hFile, Buffer, n, &NumberOfBytesWritten, NULL );
		if ( NumberOfBytesWritten != nNumberOfBytesToWrite ) { break; }

		nNumberOfBytesToWrite = 0 ;
		Func( CStorage, Buffer, 0x4000, &nNumberOfBytesToWrite );

		n = nNumberOfBytesToWrite ;
		if ( 0 == nNumberOfBytesToWrite ) { goto _over_; }
	}

	Ret = FALSE ;
	
_over_:
	kfree( Buffer );
	CloseHandle( hFile );
	return Ret ;
}


int
Win2kSrc__AVIGetFromClipboard (
	IDataObject *pDataObj
	)
{
	BOOL bFlag = FALSE ;
	HRESULT hr = NOERROR ;
	HGLOBAL hGlobal = NULL ;
	PVOID pGlobal = NULL ;
	ULONG celtFetched = 0, FileGroupDescriptorW_format = 0, FileContents_format = 0 ;
	ULONG Ret = 0 , lp = 0, pBuffer = 0, Flag = 0, CurLength = 0, size = 0, nCounts = 0 ;
	LPWSTR szPath = NULL, ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL, ptr5 = NULL , ptr6 = NULL, ptr6Dummy = NULL ;
	LPENUMFORMATETC lpEnum = NULL ;
	FORMATETC fetc ;
	STGMEDIUM stg, stg2 ;
	WCHAR achTemp[ MAX_PATH ] ;
	FILETIME SystemTimeAsFileTime ;


	if ( NULL == g_ReleaseStgMedium_addr || pDataObj->EnumFormatEtc(DATADIR_GET, &lpEnum) < 0 )
		return NULL;

	lpEnum->Reset();

	while( lpEnum->Next( 1,(LPFORMATETC)&fetc, &celtFetched) >= 0 ) 
	{
		if ( 0 == celtFetched ) { break; }
		
		if ( (fetc.cfFormat < 0xC000) || (fetc.cfFormat > 0xFFFF) ) { continue; }

		achTemp[0] = TEXT('\0');
		g_GetClipboardFormatNameW_addr( fetc.cfFormat, achTemp, sizeof(achTemp) / sizeof(achTemp[0]) );

		if ( 0 == wcsicmp(achTemp, L"FileGroupDescriptorW") )
		{
			FileGroupDescriptorW_format = fetc.cfFormat ;
		}
		else if ( 0 == wcsicmp(achTemp, L"FileContents") )
		{
			FileContents_format = fetc.cfFormat ;
		}
	}

	lpEnum->Release();

	if ( 0 == FileGroupDescriptorW_format || 0 == FileContents_format ) { return NULL; }

	szPath = (LPWSTR) kmalloc( 0x308 );
	GetTempPathW( MAX_PATH, szPath );
	wcscat( szPath, L"SBIE_Temp\\" );
	CreateDirectoryW( szPath, NULL );

	ptr1 = &szPath[ wcslen(szPath) ];
	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );
	_itow( SystemTimeAsFileTime.dwHighDateTime, ptr1, 16 );
	
	ptr2 = &ptr1[ wcslen(ptr1) ];
	_itow( SystemTimeAsFileTime.dwLowDateTime, ptr2, 16 );

	ptr3 = &ptr2[ wcslen(ptr2) ];
	CreateDirectoryW( szPath, 0 );
	*ptr3 = '\\' ;
	ptr4 = ptr3 + 1;

	stg.tymed = 0;
	stg.hGlobal = 0;
	stg.pUnkForRelease = 0;

	fetc.cfFormat = (CLIPFORMAT) FileGroupDescriptorW_format ;
	fetc.ptd = 0;
	fetc.dwAspect = DVASPECT_CONTENT ;
	fetc.lindex = -1 ;
	fetc.tymed = TYMED_HGLOBAL ;

	hr = pDataObj->GetData( &fetc, &stg );
	if ( hr < 0 || stg.tymed != 1 )
	{
		kfree( szPath );
		return NULL;
	}

	lp = (ULONG) GlobalLock( stg.hGlobal );
	if ( 0 == *(DWORD *)lp ) { goto _over_ ; }


	pBuffer = (ULONG) kmalloc( 520 * (*(DWORD *)lp) );
	
	ptr5 = (LPWSTR)(pBuffer + 0x14);
	*(DWORD *) pBuffer = 0x14 ;
	*(DWORD *)(pBuffer + 0x10) = 1 ;

	ptr6 = (LPWSTR) (lp + 0x4C) ;

	while ( TRUE )
	{
		Flag = *(DWORD *)((DWORD)ptr6 - 0x48);

		if ( Flag & 4 && *(BYTE *)((DWORD)ptr6 - 0x24) & 0x10 )
		{
			wcscpy( ptr4, ptr6 );
			CreateDirectoryW( szPath, NULL );

			if (   0 == CurLength 
				|| ptr6[ CurLength ] != '\\'
				|| wcsncmp( ptr6, ptr6Dummy, CurLength )
				)
			{
				bFlag = TRUE ;

				wcscpy( ptr5, szPath );
				ptr5 += wcslen(ptr5) + 1;
				
				ptr6Dummy = ptr6 ;
				CurLength = wcslen( ptr6 );
			}

			goto _While_Next2_ ;
		}

		if ( !(Flag & 0x40)
			|| *(DWORD *)((DWORD)ptr6 - 8)
			|| (fetc.cfFormat = (CLIPFORMAT)FileContents_format,
			stg2.tymed = 0,
			stg2.hGlobal = 0,
			stg2.pUnkForRelease = 0,
			fetc.lindex = Ret,
			fetc.ptd = 0,
			fetc.dwAspect = DVASPECT_CONTENT,
			fetc.tymed = TYMED_ISTREAM,
			pDataObj->GetData( &fetc, &stg2 ) < 0)
			) 
		{
			goto _While_Next2_ ;
		}

		if ( TYMED_ISTREAM == stg2.tymed )
		{
			wcscpy( ptr4, ptr6 );

			if ( IStorageWriteFile(szPath, (int)stg2.hGlobal) )
			{
				if ( CurLength
					&& ptr6[ CurLength ] == '\\'
					&& 0 == wcsncmp(ptr6, ptr6Dummy, CurLength) )
				{
					goto _While_Next1_ ;
				}

				wcscpy( ptr5, szPath );
				ptr5 += wcslen(ptr5) + 1;
				bFlag = TRUE ;
			}
		}

		CurLength = 0 ;
		ptr6Dummy = NULL ;

_While_Next1_:
		g_ReleaseStgMedium_addr( &stg2 );

_While_Next2_:
		Ret = nCounts + 1 ;
		ptr6 = (LPWSTR) ( (ULONG)ptr6 + 0x250 );

		if ( nCounts++ >= *(DWORD *)lp )
		{
			Ret = 0 ;
			break ;
		}
	} // end-of-while

	if ( bFlag )
	{
		*(PULONG) ptr5 = Ret ;
		size = (DWORD)((char *)ptr5 - pBuffer + 2);
		hGlobal = GlobalAlloc( GMEM_MOVEABLE, size );

		if ( hGlobal )
		{
			pGlobal = GlobalLock( hGlobal );
			memcpy( pGlobal, (PVOID)pBuffer, size );
			GlobalUnlock( hGlobal );
		}
	}

_over_ :
	GlobalUnlock( stg.hGlobal );
	g_ReleaseStgMedium_addr( &stg );
	kfree( szPath );
	return Ret ;
}


BOOL IsClsidCOM( IN LPCLSID clsid )
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/30 [30:1:2012 - 13:30]

Routine Description:
  判断@clsid是否匹配以下2个CLSID,是则返回TRUE:
  {4E14FBA2-2E22-11D1-9964-00C04FBBB345} -- g_clsid_CEventSystem
  {1BE1F766-5536-11D1-B726-00C04FB926AF} -- g_clsid_DCOM
    
--*/
{
	const CLSID CLSID_CEventSystem = { 0x4E14FBA2, 0x2E22, 0x11D1, {0x99, 0x64, 0x00, 0xC0, 0x4F, 0xBB, 0xB3, 0x45} } ;
	if ( IsEqualCLSID((const GUID &)clsid, (const GUID &)CLSID_CEventSystem) )
		return TRUE;

	const CLSID CLSID_DCOM = { 0x1BE1F766, 0x5536, 0x11D1, {0xB7, 0x26, 0x00, 0xC0, 0x4F, 0xB9, 0x26, 0xAF} } ;
	if ( IsEqualCLSID((const GUID &)clsid, (const GUID &)CLSID_DCOM) )
		return TRUE;

	return FALSE;
}


HRESULT HandlerWhiteCLSID( IN LPCLSID rclsid, OUT LPVOID *ppv )
{
	RPC_IN_HandlerWhiteCLSID RpcInBuffer = {};

	*ppv = 0;
	RpcInBuffer.RpcHeader.DataLength = sizeof(RpcInBuffer);
	RpcInBuffer.RpcHeader.Flag		 = _PBSRV_APINUM_HandlerWhiteCLSID_ ;
	memcpy( &RpcInBuffer.rclsidCur, rclsid, 0x10 );
	memcpy( &RpcInBuffer.rclsid_IClassFactory, &IID_IClassFactory, 0x10 );

	LPRPC_OUT_HEADER pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &RpcInBuffer );
	if ( NULL == pOutBuffer )
	{
		RaiseException(RPC_S_SERVER_UNAVAILABLE, 1, 0, 0);
		return E_ABORT;
	}

	DWORD ErrorCode = pOutBuffer->u.ErrorCode;
	if ( ErrorCode )
	{
		PB_FreeReply( pOutBuffer );
		RaiseException(ErrorCode, 1, 0, 0);
		return E_ABORT;
	}

	LPRPC_OUT_HandlerWhiteCLSID pRpcOutBuffer = (LPRPC_OUT_HandlerWhiteCLSID) pOutBuffer;
	HRESULT hr = pRpcOutBuffer->MappedAddress_PID;
	if ( hr >= 0 )
	{
		int pv = 0;
		hr = FixupCOMVTable( 2, pRpcOutBuffer->MappedAddress_unknown1, 1, (LPVOID *)&pv );
		if ( hr >= 0 )
		{
			*(DWORD *)(pv + 0x20) = (DWORD)sub_7D227B50;
			*(DWORD *)(pv + 0x2C) = (DWORD)sub_7D2296B0;
			*(DWORD *)(pv + 0x30) = (DWORD)fake_NtSaveKey;

			memcpy( (LPVOID)(pv + 0x14), rclsid, 0x10 );
			*ppv = (LPVOID)pv;
		}
	}

	PB_FreeReply( pOutBuffer );
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// OpenClsid 选项暂时不用到，后期完善后对以下函数进行完善
// 2012-01-30 19:08:00 by sudami
//

HRESULT FixupCOMVTable(int nCounts, int a2, int a3, LPVOID *ppv)
{
	int pv = (int) kmalloc( 4 * nCounts + 0x2C );
	*ppv = (LPVOID)pv;
	if ( 0 == pv ) { return E_OUTOFMEMORY; }

	*(DWORD *)pv = pv + 0x20;
	*(DWORD *)(pv + 4) = 1;
	*(DWORD *)(pv + 8) = a2;
	*(DWORD *)(pv + 0xC) = a3;
	*(DWORD *)(pv + 0x24) = (DWORD)sub_7D228800;
	*(DWORD *)(pv + 0x28) = (DWORD)sub_7D228830;

	return S_OK;
}


void sub_7D228770(int a1, char a2)
{
	int pRpcInBuffer = (int)kmalloc(0x10);
	if ( 0 == pRpcInBuffer ) { return; }
	
	*(DWORD *) pRpcInBuffer = 0x10;
	*(DWORD *)(pRpcInBuffer + 4) = 0x1B04;
	*(DWORD *)(pRpcInBuffer + 8) = *(DWORD *)(a1 + 8);
	*(BYTE *) (pRpcInBuffer + 0xC) = a2;

	LPRPC_OUT_HEADER pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer((PRPC_IN_HEADER)pRpcInBuffer);
	kfree((PVOID)pRpcInBuffer);

	if ( pRpcOutBuffer )
	{
		DWORD ExceptionCode = pRpcOutBuffer->u.Status;
		PB_FreeReply(pRpcOutBuffer);
		if ( ExceptionCode )
			RaiseException(ExceptionCode, 1u, 0, 0);
	}
	else
	{
		RaiseException(RPC_S_SERVER_UNAVAILABLE, 1, 0, 0);
	}
	
	return;
}


LONG sub_7D228800(int a1)
{
	if ( *(BYTE *)(a1 + 12) & 1 )
		sub_7D228770(a1, 0x61);

	return InterlockedIncrement((LPLONG)(a1 + 4));
}


LONG sub_7D228830(int a1)
{
	LONG v1; // ebx@3
	int v2; // edi@5

	if ( *(BYTE *)(a1 + 12) & 1 )
		sub_7D228770(a1, 0x72);

	v1 = InterlockedDecrement((LPLONG)(a1 + 4));
	if ( !v1 )
	{
		if ( *(BYTE *)(a1 + 12) & 2 )
		{
			v2 = *(DWORD *)(a1 + 0x2C);
			if ( v2 )
			{
				(*(void (__stdcall **)(DWORD))(*(DWORD *)v2 + 16))(*(DWORD *)(a1 + 0x2C));
				(*(void (__stdcall **)(int))(*(DWORD *)v2 + 8))(v2);
			}
		}

		kfree((PVOID)a1);
	}

	return v1;
}


HRESULT sub_7D227B50(int a1, LPCLSID clsid, int a3)
{
	BOOL bOK = FALSE;
	do 
	{
		if ( IsEqualCLSID((const GUID &)clsid, (const GUID &)IID_IUnknown) )
		{
			bOK = TRUE;
			break;
		}

		if ( IsEqualCLSID((const GUID &)clsid, (const GUID &)IID_IClassFactory) )
		{
			bOK = TRUE;
			break;
		}

	} while (FALSE);

	HRESULT hr = S_OK;
	if ( bOK )
	{
		(*(void (__stdcall **)(int))(*(DWORD *)a1 + 4))(a1);
		*(DWORD *)a3 = a1;
		hr = 0;
	}
	else
	{
		*(DWORD *)a3 = 0;
		hr = E_NOINTERFACE;
	}

	return hr;	
}


HRESULT sub_7D2296B0(int a1, int a2, int a3, int a4)
{
	*(DWORD *)a4 = 0;
	int pRpcInBuffer = (int)kmalloc(0x1C);
	if ( 0 == pRpcInBuffer ) { return E_OUTOFMEMORY; }

	*(DWORD *)pRpcInBuffer = 0x1C;
	*(DWORD *)(pRpcInBuffer + 4)    = 0x1B02;
	*(DWORD *)(pRpcInBuffer + 8)    = *(DWORD *)(a1 + 8);
	*(DWORD *)(pRpcInBuffer + 0xC)  = *(DWORD *)a3;
	*(DWORD *)(pRpcInBuffer + 0x10) = *(DWORD *)(a3 + 4);
	*(DWORD *)(pRpcInBuffer + 0x14) = *(DWORD *)(a3 + 8);
	*(DWORD *)(pRpcInBuffer + 0x18) = *(DWORD *)(a3 + 12);

	int pRpcOutBuffer = (int)PB_CallServer((PRPC_IN_HEADER)pRpcInBuffer);
	kfree((PVOID)pRpcInBuffer);
	if ( !pRpcOutBuffer )
	{
		RaiseException(RPC_S_SERVER_UNAVAILABLE, EXCEPTION_NONCONTINUABLE, 0, 0);
		return E_ABORT;
	}

	DWORD ExceptionCode = *(DWORD *)(pRpcOutBuffer + 4);
	if ( ExceptionCode )
	{
		PB_FreeReply((PVOID)pRpcOutBuffer);
		RaiseException(ExceptionCode, EXCEPTION_NONCONTINUABLE, 0, 0);
		return E_ABORT;
	}

	HRESULT hr = *(DWORD *)(pRpcOutBuffer + 8);
	if ( hr >= 0 )
	{
		DWORD v10 = *(DWORD *)(pRpcOutBuffer + 0xC);
		if ( v10 )
			hr = sub_7D229620(a3, v10, a4);
		else
			hr = E_FAIL;
	}

	PB_FreeReply((PVOID)pRpcOutBuffer);
	return hr;
}


ULONG sub_7D229620(int a1, int a2, int a3)
{
	//
	// 东西太多了，后期补充
	//

	return 0;
}


///////////////////////////////   END OF FILE   ///////////////////////////////