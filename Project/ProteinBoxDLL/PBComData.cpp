/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/01/12 [12:1:2011 - 14:24]
* MODULE : \Code\Project\ProteinBoxDLL\PBComData.cpp
* 
* Description:
*
*   处理COM部分的过滤
*                        
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
#include "PBUser32dll/PBUser32dll.h"
#include "PBComData.h"

#pragma warning(disable : 4995) 

//////////////////////////////////////////////////////////////////////////


CComObjectGlobal<COLEClipbrdIDataObject> g_OLEClipbrd_IDataObject_VTable ;
CComObjectGlobal<COLEClipbrdIEnumFORMATETC> g_OLEClipbrd_IEnumFORMATETC_VTable ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


HRESULT WINAPI 
COLEClipbrdIDataObject::QueryInterface (
	IN IDataObject* iface,
	IN REFIID riid,
	OUT void** ppvObject
	)
{
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	if ( NULL == This || NULL == ppvObject ) { return E_INVALIDARG; }

	*ppvObject = NULL ;

	if( (FALSE == IsEqualIID(riid, (REFIID)IID_IUnknown)) && (FALSE == IsEqualIID(riid, (REFIID)IID_IDataObject)) ) { return E_NOINTERFACE; }

	*ppvObject = This ;
	This->lpVtbl1->AddRef( (IDataObject *)This );
	
	return S_OK;
}



ULONG WINAPI 
COLEClipbrdIDataObject::AddRef (
	IN IDataObject* iface
	)
{
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	return InterlockedIncrement( &This->RefCounts );
}



ULONG WINAPI 
COLEClipbrdIDataObject::Release (
	IN IDataObject* iface
	)
{
	ULONG RefCounts = 0 ;
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	RefCounts = InterlockedDecrement( &This->RefCounts );

	if ( 0 == RefCounts )
	{
		pIDataObjectSrc = This->pIDataObjectSrc ;
		if ( pIDataObjectSrc ) { pIDataObjectSrc->Release(); }

		if ( This->hModule_shell32_dll ) { FreeLibrary( This->hModule_shell32_dll ); }

		kfree( This );
		RefCounts = 0 ;
	}

	return RefCounts;
}



HRESULT WINAPI
COLEClipbrdIDataObject::GetData (
	IN IDataObject* iface,
	IN LPFORMATETC pformatetcIn,
	IN STGMEDIUM* pmedium
	)
{
	HRESULT hr ;
	ULONG size = 0 ;
	LPVOID src  = NULL ;
	HGLOBAL hData = NULL ;
	BOOL bCopyData = FALSE ;
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	if ( NULL == This ) { return E_INVALIDARG; }

	pIDataObjectSrc = This->pIDataObjectSrc ;
	if ( pIDataObjectSrc )
	{
		hr = pIDataObjectSrc->GetData( pformatetcIn, pmedium );
		if ( hr < 0 ) { return hr; }
	}
	else
	{
		if ( pformatetcIn->cfFormat != CF_HDROP ) { return DV_E_FORMATETC; }

		if ( (pformatetcIn->tymed & TYMED_HGLOBAL) != TYMED_HGLOBAL ) { return DV_E_TYMED; }

		if ( pformatetcIn->lindex != -1 ) { return DV_E_LINDEX; }

		if ( pformatetcIn->dwAspect != DVASPECT_CONTENT ) { return DV_E_DVASPECT; }

		size = GlobalSize( (HGLOBAL)This->hdrop );
		hData = GlobalAlloc( GMEM_MOVEABLE, size );
		if ( NULL == hData ) { return E_OUTOFMEMORY; }

		src = GlobalLock( (HGLOBAL)This->hdrop );
		memcpy( GlobalLock(hData), src, size );

		GlobalUnlock( (HGLOBAL)This->hdrop );
		GlobalUnlock( hData );

		pmedium->hGlobal = hData ;
		pmedium->tymed = TYMED_HGLOBAL ;
		pmedium->pUnkForRelease = NULL ;

		hr = S_OK;
	}

	CopyClipboardData( (int)This, pformatetcIn, pmedium );
	return hr ;
}



HRESULT WINAPI
COLEClipbrdIDataObject::GetDataHere (
	IN IDataObject* iface,
	IN LPFORMATETC pformatetcIn,
	IN STGMEDIUM* pmedium
	)
{
	HRESULT hr = S_OK ;
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	if ( NULL == This ) { return E_INVALIDARG; }

	pIDataObjectSrc = This->pIDataObjectSrc ;
	if ( NULL == pIDataObjectSrc ) { return E_FAIL; }

	hr = pIDataObjectSrc->GetDataHere( pformatetcIn, pmedium );
	if ( hr >= 0 ) { CopyClipboardData( (int)This, pformatetcIn, pmedium ); }

	return hr ;
}



HRESULT WINAPI
COLEClipbrdIDataObject::QueryGetData (
	IN IDataObject* iface,
	IN LPFORMATETC pformatetc
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->QueryGetData( pformatetc );
	}

	if ( pformatetc->cfFormat != CF_HDROP ) { return DV_E_FORMATETC; }

	if ( pformatetc->tymed != TYMED_HGLOBAL ) { return DV_E_TYMED; }

	if ( pformatetc->lindex != -1 ) { return DV_E_LINDEX; }

	return pformatetc->dwAspect != DVASPECT_CONTENT ? DV_E_DVASPECT : S_OK ;
}



HRESULT WINAPI 
COLEClipbrdIDataObject::GetCanonicalFormatEtc (
	IN IDataObject* iface,
	IN LPFORMATETC pformatectIn,
	OUT LPFORMATETC pformatetcOut
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	if ( NULL == pformatectIn || NULL == pformatetcOut ) { return E_INVALIDARG; }

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->GetCanonicalFormatEtc( pformatectIn, pformatetcOut );
	}
	
	RtlZeroMemory( pformatetcOut, sizeof(FORMATETC) );
	return DATA_S_SAMEFORMATETC;
}



HRESULT WINAPI 
COLEClipbrdIDataObject::SetData (
	IN IDataObject* iface,
	IN LPFORMATETC pformatetc,
	IN STGMEDIUM* pmedium,
	IN BOOL fRelease
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->SetData( pformatetc, pmedium, fRelease );
	}

	return E_NOTIMPL;
}



HRESULT WINAPI 
COLEClipbrdIDataObject::EnumFormatEtc (
	IN IDataObject* iface,
	IN DWORD dwDirection,
	OUT IEnumFORMATETC** ppenumFormatEtc
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->EnumFormatEtc( dwDirection, ppenumFormatEtc );
	}

	This->lpVtbl2->Reset( (LPENUMFORMATETC)&This->lpVtbl2 );
	*ppenumFormatEtc = (LPENUMFORMATETC) &This->lpVtbl2 ;

	return S_OK ;
}



HRESULT WINAPI
COLEClipbrdIDataObject::DAdvise (
	IN IDataObject* iface,
	IN FORMATETC* pformatetc,
	IN DWORD advf,
	IN IAdviseSink* pAdvSink,
	IN DWORD* pdwConnection
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->DAdvise( pformatetc, advf, pAdvSink, pdwConnection );
	}

	return OLE_E_ADVISENOTSUPPORTED ;
}



HRESULT WINAPI
COLEClipbrdIDataObject::DUnadvise (
	IN IDataObject* iface,
	IN DWORD dwConnection
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->DUnadvise( dwConnection );
	}

	return OLE_E_ADVISENOTSUPPORTED ;
}



HRESULT WINAPI 
COLEClipbrdIDataObject::EnumDAdvise (
	IN IDataObject* iface,
	IN IEnumSTATDATA** ppenumAdvise
	)
{
	IDataObject *pIDataObjectSrc = NULL ;
	LPOLEClipbrdEx This = (LPOLEClipbrdEx)iface ;

	pIDataObjectSrc = This->pIDataObjectSrc;
	if ( pIDataObjectSrc )
	{
		return pIDataObjectSrc->EnumDAdvise( ppenumAdvise );
	}

	return OLE_E_ADVISENOTSUPPORTED ;
}



void 
CopyClipboardData (
	IN int This,
	IN LPFORMATETC pformatetcIn,
	IN STGMEDIUM *pmedium
	)
{
	CLIPFORMAT cfFormat = 0 ;
	HGLOBAL hMemNew = NULL ;
	WCHAR szPath[ MAX_PATH ] = L"" ;

	if ( pmedium->tymed != TYMED_HGLOBAL ) { return; }

	cfFormat = pformatetcIn->cfFormat;
	if ( pformatetcIn->cfFormat == CF_HDROP )
	{
		hMemNew = CopyClipboardDataEx( This, pmedium->hGlobal );
	}
	else
	{
		if ( cfFormat < 0xC000 || cfFormat > 0xFFFF ) { return; }

		g_GetClipboardFormatNameW_addr( cfFormat, szPath, 0x3C );

		if ( 0 == wcsicmp(szPath, L"FileName") )
		{
			hMemNew = CopyClipboardDataA( pmedium->hGlobal );
		}
		else if ( 0 == wcsicmp(szPath, L"FileNameW") )
		{
			hMemNew = CopyClipboardDataW( pmedium->hGlobal );
		}
		else
		{
			return;
		}
	}

	if ( hMemNew )
	{
		GlobalFree( pmedium->hGlobal );
		pmedium->hGlobal = hMemNew ;
	}

	return;
}



HGLOBAL
CopyClipboardDataEx (
	IN int This,
	IN HGLOBAL hMem
	)
{
	typedef ULONG (__stdcall * _XX_) (HGLOBAL, ULONG, LPWSTR, ULONG);
	_XX_ Func = NULL ;
	LPWSTR szPath = NULL ;
	HANDLE hFile = NULL ;
	HGLOBAL hMemNew = NULL ;
	BOOL bFlag = FALSE, bIsHandlerSelfFilePath = FALSE ;
	ULONG Buffer = 0, nCounts = 0, nIndex = 0, Length = 0 ;

	Func = (_XX_) *(DWORD *)( This + 0x1C );
	if ( NULL == Func ) { return NULL; }

	nCounts = Func( hMem, 0xFFFFFFFF, 0, 0 );
	if ( nCounts == 0xFFFFFFFF )  { return NULL; }

	Buffer = (ULONG) kmalloc( 0x80000 );

	szPath = (LPWSTR)(Buffer + 0x14);
	*(DWORD *) Buffer = 0x14 ;
	*(DWORD *)(Buffer + 0x10) = 1 ;

	do
	{
		Func( hMem, nIndex, szPath, 0x1FE );
		hFile = CreateFileW( szPath, 0x80000000, 1, 0, 3, 0x2000000, 0 );
	
		if ( hFile != (HANDLE)INVALID_HANDLE_VALUE )
		{
			if ( STATUS_SUCCESS == PB_GetHandlePath(hFile, szPath, &bIsHandlerSelfFilePath) && bIsHandlerSelfFilePath )
			{
				PB_TranslateNtToDosPath( szPath );
				bFlag = TRUE ;
			}
			else
			{
				memset( szPath, 0, 0x400 );
				Func( hMem, nIndex, szPath, 0x1FE );
			}

			CloseHandle( hFile );
		}

		szPath += wcslen(szPath) + 1 ;
		++ nIndex ;
	}
	while ( nIndex < nCounts );

	*szPath = 0 ;
	if ( bFlag )
	{
		Length = (DWORD)( (PCHAR)szPath - (PCHAR)Buffer + 2 );
		hMemNew = GlobalAlloc( 2, Length );
		if ( hMemNew )
		{
			memcpy( GlobalLock(hMemNew), (const void *)Buffer, Length );
			GlobalUnlock( hMemNew );
		}
	}
	
	kfree( (PVOID)Buffer );
	return hMemNew ;
}



HGLOBAL 
CopyClipboardDataW (
	IN HGLOBAL hMem
	)
{
	HANDLE hFile = NULL ;
	HGLOBAL hMemNew = NULL ;
	ULONG Size = 0, Length = 0 ;
	BOOL bIsHandlerSelfFilePath = FALSE ;
	LPWSTR DefaultPath = NULL, szPath = NULL ;

	DefaultPath = (LPWSTR) GlobalLock( hMem );
	hFile = CreateFileW( DefaultPath, 0x80000000, 1, 0, 3, 0x2000000, 0 );
	
	if ( hFile != (HANDLE)INVALID_HANDLE_VALUE )
	{
		szPath = (LPWSTR) kmalloc( 0x2000 );
		if ( STATUS_SUCCESS == PB_GetHandlePath(hFile, szPath, &bIsHandlerSelfFilePath) && bIsHandlerSelfFilePath )
		{
			PB_TranslateNtToDosPath( szPath );

			Size = 2 * wcslen(szPath) + 2;
			hMemNew = GlobalAlloc( 2, Size );
			if ( hMemNew )
			{
				memcpy( GlobalLock(hMemNew), szPath, Size );
				GlobalUnlock( hMemNew );
			}
		}

		kfree( szPath );
		CloseHandle( hFile );
	}

	GlobalUnlock( hMem );
	return hMemNew ;
}



HGLOBAL 
CopyClipboardDataA (
	IN HGLOBAL hMem
	)
{
	HANDLE hFile = NULL ;
	HGLOBAL hMemNew = NULL ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;
	BOOL bIsHandlerSelfFilePath = FALSE ;
	LPSTR DefaultPath = NULL ;
	LPWSTR szPath = NULL ;
	
	DefaultPath = (LPSTR) GlobalLock( hMem );
	hFile = CreateFileA( DefaultPath, 0x80000000, 1, 0, 3, 0x2000000, 0 );
	
	if ( hFile != (HANDLE)INVALID_HANDLE_VALUE )
	{
		szPath = (LPWSTR) kmalloc( 0x2000 );
		if ( STATUS_SUCCESS == PB_GetHandlePath(hFile, szPath, &bIsHandlerSelfFilePath) && bIsHandlerSelfFilePath )
		{
			PB_TranslateNtToDosPath( szPath );
			RtlInitUnicodeString( &uniBuffer, szPath );

			ansiBuffer.Length = 0;
			ansiBuffer.MaximumLength = ((WORD)uniBuffer.Length / sizeof(WCHAR)) + 1 ;

			hMemNew = GlobalAlloc( 2, ansiBuffer.MaximumLength );
			if ( hMemNew )
			{
				ansiBuffer.Buffer = (PCHAR) GlobalLock( hMemNew );
				RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
				GlobalUnlock( hMemNew );
			}
		}

		kfree( szPath );
		CloseHandle( hFile );
	}

	GlobalUnlock( hMem );
	return hMemNew ;
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


HRESULT WINAPI 
COLEClipbrdIEnumFORMATETC::QueryInterface (
	IN IDataObject* iface,
	IN REFIID riid,
	OUT LPVOID* ppvObj
	)
{
	IEnumFORMATETCImpl *This = (IEnumFORMATETCImpl *)iface;

	// Since enumerators are separate objects from the parent data object we only need to support the IUnknown and IEnumFORMATETC interfaces
	*ppvObj = NULL ;

	if( IsEqualIID(riid, (REFIID)IID_IUnknown) || IsEqualIID(riid, (REFIID)IID_IEnumFORMATETC) )
	{
		*ppvObj = This;
	}

	if( *ppvObj )
	{
	//	IEnumFORMATETC_AddRef( (IEnumFORMATETC*)*ppvObj );
		((IEnumFORMATETC*)*ppvObj)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}



ULONG WINAPI 
COLEClipbrdIEnumFORMATETC::AddRef (
	IN IDataObject* iface
	)
{
	return 1 ;
}



ULONG WINAPI 
COLEClipbrdIEnumFORMATETC::Release (
	IN IDataObject* iface
	)
{
	return 1 ;
}



HRESULT WINAPI 
COLEClipbrdIEnumFORMATETC::Next (
	IN LPENUMFORMATETC iface,
	IN ULONG celt,
	IN FORMATETC *rgelt,
	OUT ULONG *pceltFethed
	)
{
	IEnumFORMATETCImpl *This = (IEnumFORMATETCImpl *)iface;

	if ( This->posFmt )
	{
		if ( pceltFethed ) { *pceltFethed = 0; }
	}
	else if ( celt >= 1 )
	{
		This->posFmt = 1 ;
		rgelt->cfFormat = CF_HDROP ;
		rgelt->ptd = 0 ;
		rgelt->dwAspect = 1;
		rgelt->lindex = -1 ;
		rgelt->tymed = 1 ;

		if ( pceltFethed ) { *pceltFethed = 1; }
		if ( celt == 1 ) { return S_OK; }
	}
	

	return S_FALSE;
}



HRESULT WINAPI
COLEClipbrdIEnumFORMATETC::Skip (
	IN LPENUMFORMATETC iface, 
	IN ULONG celt
	)
{
	IEnumFORMATETCImpl *This = (IEnumFORMATETCImpl *)iface;

	if ( 0 == This->posFmt && S_FALSE == celt )
	{
		return S_OK;
	}

	return S_FALSE;
}



HRESULT WINAPI 
COLEClipbrdIEnumFORMATETC::Reset (
	IN LPENUMFORMATETC iface
	)
{
	IEnumFORMATETCImpl *This = (IEnumFORMATETCImpl *)iface;
	
	This->posFmt = 0;
	return S_OK;
}



HRESULT WINAPI 
COLEClipbrdIEnumFORMATETC::Clone (
	IN LPENUMFORMATETC iface,
	OUT LPENUMFORMATETC* ppenum
	)
{
	*ppenum = iface ;
	return S_OK;
}


///////////////////////////////   END OF FILE   ///////////////////////////////