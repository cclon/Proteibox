/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/29 [29:1:2012 - 14:16]
* MODULE : \Code\Project\ProteinBoxDLL\PBOle32dll\PBOle32dll.cpp
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
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBComData.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "../PBUser32dll/PBUser32dll.h"
#include "PBOle32dll.h"

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI fake_CoGetClassObject (LPCLSID,DWORD,PVOID,PVOID,LPVOID *);
ULONG WINAPI fake_CoCreateInstance (LPCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID *);
ULONG WINAPI fake_CoCreateInstanceEx (LPCLSID,LPUNKNOWN,DWORD,COSERVERINFO*,ULONG,MULTI_QI*);
ULONG WINAPI fake_CoUnmarshalInterface (IStream *,REFIID,LPVOID *);
ULONG WINAPI fake_CoMarshalInterface (IStream *,REFIID,IUnknown *,DWORD,void *,DWORD);

typedef enum _EPBOle32Type_
{
	CoGetClassObject_TAG = 0,
	CoCreateInstance_TAG,
	CoCreateInstanceEx_TAG,
	CoUnmarshalInterface_TAG,
	CoMarshalInterface_TAG,
} EPBOle32Type ;

static HOOKINFOLittle g_HookInfoOle_Array [] = 
{
	{ Nothing_TAG, CoGetClassObject_TAG, "CoGetClassObject", 0, NULL, fake_CoGetClassObject },
	{ Nothing_TAG, CoCreateInstance_TAG, "CoCreateInstance", 0, NULL, fake_CoCreateInstance },
	{ Nothing_TAG, CoCreateInstanceEx_TAG, "CoCreateInstanceEx", 0, NULL, fake_CoCreateInstanceEx },
	{ Nothing_TAG, CoUnmarshalInterface_TAG, "CoUnmarshalInterface", 0, NULL, fake_CoUnmarshalInterface },
	{ Nothing_TAG, CoMarshalInterface_TAG, "CoMarshalInterface", 0, NULL, fake_CoMarshalInterface },
};

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_CoGetClassObject (
	LPCLSID rclsid, 
	DWORD dwClsContext,
	PVOID pServerInfo, 
	PVOID iid,
	LPVOID *ppv
	)
{
	if ( IsClsidCOM(rclsid) )
	{
		*ppv = NULL;
		return E_ACCESSDENIED;
	}

	if ( pServerInfo || !PB_IsOpenClsid(rclsid, dwClsContext, FALSE) )
	{
		typedef HANDLE (WINAPI* _CoGetClassObject_) (LPCLSID, DWORD, PVOID, PVOID, LPVOID *);
		_CoGetClassObject_ addr = (_CoGetClassObject_) g_HookInfoOle_Array[ CoGetClassObject_TAG ].OrignalAddress ;

		return (ULONG)addr( rclsid, dwClsContext, pServerInfo, iid, ppv ) ;
	}

	// 白名单CLSID
	ULONG ret = HandlerWhiteCLSID( rclsid, ppv );
	return ret;
}


ULONG WINAPI
fake_CoCreateInstance (
	LPCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID iid,
	LPVOID *ppv
	)
{
	if ( IsClsidCOM(rclsid) )
	{
		*ppv = NULL;
		return E_ACCESSDENIED;
	}

	if ( !PB_IsOpenClsid(rclsid, dwClsContext, FALSE) )
	{
		typedef HRESULT (WINAPI* _CoCreateInstance_) (LPCLSID, LPUNKNOWN,DWORD,REFIID,PVOID *);
		_CoCreateInstance_ addr = (_CoCreateInstance_) g_HookInfoOle_Array[ CoCreateInstance_TAG ].OrignalAddress ;

		return (ULONG)addr( rclsid, pUnkOuter, dwClsContext, iid, ppv ) ;
	}

	// 白名单CLSID
	ULONG ret = HandlerWhiteCLSID( rclsid, ppv );
	if ( ret >= 0 )
	{
		IClassFactory* pClassFactory = (IClassFactory*)*ppv;

		ret = pClassFactory->CreateInstance( pUnkOuter, iid, ppv );
		pClassFactory->Release();
// 		ret = (*(int (__stdcall **)(LPVOID *, LPUNKNOWN, REFIID, LPVOID *))(*(DWORD *)ppv + 0xC))(ppv,pUnkOuter,iid,ppv);
// 		(*(void (__stdcall **)(DWORD))(*(DWORD *)ppv + 8))(ppv);
	}
	
	return ret;
}


ULONG WINAPI
fake_CoCreateInstanceEx (
	LPCLSID      rclsid,
	LPUNKNOWN     pUnkOuter,
	DWORD         dwClsContext,
	COSERVERINFO* pServerInfo,
	ULONG         cmq,
	MULTI_QI*     pResults
	)
{
	if ( IsClsidCOM(rclsid) )
		return E_ACCESSDENIED;

	if ( 0 == wcsicmp(g_BoxInfo.ProcessName, L"wlmail.exe") )
	{
		const CLSID CLSID_DCOM_WindowsManagement = { 0x4991D34B, 0x80A1, 0x4291, {0x83, 0xB6, 0x33, 0x28, 0x36, 0x6B, 0x90, 0x97} } ;
		if ( IsEqualCLSID((const GUID &)rclsid, (const GUID &)CLSID_DCOM_WindowsManagement) )
			return E_ACCESSDENIED;
	}

	if ( !PB_IsOpenClsid(rclsid, dwClsContext, FALSE) )
	{
		typedef HRESULT (WINAPI* _CoCreateInstanceEx_) ( LPCLSID,LPUNKNOWN,DWORD,COSERVERINFO*,ULONG,MULTI_QI* );
		_CoCreateInstanceEx_ addr = (_CoCreateInstanceEx_) g_HookInfoOle_Array[ CoCreateInstanceEx_TAG ].OrignalAddress ;

		return (ULONG)addr( rclsid, pUnkOuter, dwClsContext, pServerInfo, cmq, pResults ) ;
	}

	// 白名单CLSID
	ULONG successCount = 0;
	LPVOID pv = NULL;
	ULONG ret = HandlerWhiteCLSID( rclsid, &pv );
	if ( ret >= 0 )
	{
		IClassFactory* pClassFactory = (IClassFactory*)pv;

		if ( cmq > 0 )
		{
			// Then, Create for all the interfaces requested.
			for (ULONG index = 0; index < cmq; index++)
			{
				pResults[index].hr = pClassFactory->CreateInstance( pUnkOuter, (const IID &)pResults[index].pIID,  (VOID**)&(pResults[index].pItf) );
				if (pResults[index].hr == S_OK)
					successCount++;
			}

			ret = S_OK;
		}

		pClassFactory->Release();
		if (successCount == 0)
			ret = E_NOINTERFACE;

		if (successCount!=cmq)
			ret = CO_S_NOTALLINTERFACES;
	}

	return ret;
}


ULONG WINAPI
fake_CoUnmarshalInterface (
	IStream *pStream,
	REFIID riid,
	LPVOID *ppv
	)
{
	ULARGE_INTEGER libNewPosition = {};
	ULARGE_INTEGER libNewPositionDummy = {};
	LARGE_INTEGER dlibMove = { 0, 0 };
	HRESULT hr = pStream->Seek( dlibMove, 1, &libNewPosition );
	if ( !SUCCEEDED(hr) ) { return hr; }

	typedef HRESULT (WINAPI* _CoUnmarshalInterface_) (IStream *, REFIID , LPVOID *);
	_CoUnmarshalInterface_ addr = (_CoUnmarshalInterface_) g_HookInfoOle_Array[ CoUnmarshalInterface_TAG ].OrignalAddress ;
	hr = addr( pStream, riid, ppv ) ;
	if ( hr != 0x80070776 ) { return hr; }

	libNewPositionDummy = libNewPosition;
	dlibMove.u.HighPart = libNewPosition.u.HighPart;
	dlibMove.u.LowPart  = libNewPosition.u.LowPart;
	hr = pStream->Seek( dlibMove, 0, &libNewPosition );
	if ( !SUCCEEDED(hr) ) { return hr; }

	ULONG cbRead = 0;
	IID iid_dummy = {} ;
	OBJREF objref = {} ;
	hr = pStream->Read( &objref, 0x18, &cbRead );
	if ( !SUCCEEDED(hr) ){ return hr; }
	if ( cbRead != 0x18 ) { return RPC_E_INVALID_OBJREF; }

	ULONG StructLength = 0;
	switch (objref.flags)
	{
	case 1:
		StructLength = 0x2C;
		break;

	case 2:
		StructLength = 0x3C;
		break;

	case 4:
		StructLength = 0x18;
		break;
	case 8:
		StructLength = 0x28;
		break;

	default:
		return RPC_E_INVALID_OBJREF;
	}

	memcpy( &iid_dummy, &objref.iid, 0x10 );

	LPRPC_IN_CoUnmarshalInterface pRpcInBuffer = (LPRPC_IN_CoUnmarshalInterface) kmalloc(StructLength + 0x38);
	if ( NULL == pRpcInBuffer ) { return E_OUTOFMEMORY; }

	pRpcInBuffer->RpcHeader.DataLength	= StructLength + 0x38;
	pRpcInBuffer->RpcHeader.Flag		= _PBSRV_APINUM_CoUnmarshalInterface_;
	memcpy( &pRpcInBuffer->riid, &riid, 0x10 );
	pRpcInBuffer->objrefDummy_TotalLength = StructLength + 0x18;
	memcpy( &pRpcInBuffer->objref, &objref, sizeof(objref) );

	hr = pStream->Read( &pRpcInBuffer->objrefDummy, StructLength, &cbRead );
	if ( !SUCCEEDED(hr) )
	{
		kfree(pRpcInBuffer);
		return hr;
	}

	if ( cbRead + 0x18 != pRpcInBuffer->objrefDummy_TotalLength )
	{
		kfree(pRpcInBuffer);
		return RPC_E_INVALID_OBJREF;
	}

	LPRPC_OUT_HEADER pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( pRpcInBuffer );
	if ( NULL == pOutBuffer )
	{
		dlibMove.u.HighPart = libNewPositionDummy.u.HighPart;
		dlibMove.u.LowPart  = libNewPositionDummy.u.LowPart;
		pStream->Seek( dlibMove, 0, &libNewPositionDummy );
		return 0x80070776;
	}

	DWORD ErrorCode = pOutBuffer->u.ErrorCode;
	if ( ErrorCode )
	{
		PB_FreeReply( pOutBuffer );

		if ( ErrorCode == RPC_S_SERVER_UNAVAILABLE )
		{
			dlibMove.u.HighPart = libNewPositionDummy.u.HighPart;
			dlibMove.u.LowPart  = libNewPositionDummy.u.LowPart;
			pStream->Seek( dlibMove, 0, &libNewPositionDummy );
			return 0x80070776;
		}

		RaiseException(ErrorCode, 1, 0, 0);
		return E_ABORT;
	}

	hr = *(HRESULT *)((int)pOutBuffer + 8);
	do 
	{
		if ( hr < 0 )
			break;
		if ( !*(DWORD *)((int)pOutBuffer + 0xC) )	
		{
			hr = E_FAIL;
			break;
		}

		// 如果riid存在，就用riid
		CLSID CLSID_Temp = {} ;
		RtlZeroMemory( &CLSID_Temp, 0x10 );
		if ( FALSE == IsEqualCLSID((const GUID &)riid, (const GUID &)CLSID_Temp) )
			memcpy( &iid_dummy, &riid, 0x10 );
		
		hr = sub_7D229620((int)&iid_dummy, *(DWORD *)((int)pOutBuffer + 0xC), (int)ppv);
	} while (FALSE);

	PB_FreeReply( pOutBuffer );
	return hr;
}


ULONG WINAPI
fake_CoMarshalInterface (
	IStream *pStream, 
	REFIID riid,
	IUnknown *pUnk,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlFlags
	)
{
	const CLSID CLSID_ISearchNotifyInlineSite = { 0xB5702E61, 0xE75C, 0x4B64, {0x82, 0xA1, 0x6C, 0xB4, 0xF8, 0x32, 0xFC, 0xCF} } ;
	if ( FALSE == IsEqualCLSID((const GUID &)riid, (const GUID &)CLSID_ISearchNotifyInlineSite) )
	{
		typedef HRESULT (WINAPI* _CoMarshalInterface_) ( IStream *,REFIID,IUnknown *,DWORD,void *,DWORD );
		_CoMarshalInterface_ addr = (_CoMarshalInterface_) g_HookInfoOle_Array[ CoMarshalInterface_TAG ].OrignalAddress ;

		return  (ULONG)addr( pStream, riid, pUnk, dwDestContext, pvDestContext, mshlFlags ) ;
	}

	RPC_IN_CoMarshalInterface RpcInBuffer = {};
	RpcInBuffer.RpcHeader.DataLength = sizeof(RpcInBuffer);
	RpcInBuffer.RpcHeader.Flag		 = _PBSRV_APINUM_CoMarshalInterface_ ;
	RpcInBuffer.Flag				 = 0xFFFFFFFF;
	RpcInBuffer.dwDestContext		 = dwDestContext;
	RpcInBuffer.mshlFlags			 = mshlFlags;

	memcpy( &RpcInBuffer.riid, &riid, 0x10 );

	LPRPC_OUT_CoMarshalInterface pOutBuffer = (LPRPC_OUT_CoMarshalInterface) PB_CallServer( &RpcInBuffer );
	if ( NULL == pOutBuffer )
	{
		RaiseException(RPC_S_SERVER_UNAVAILABLE, 1, 0, 0);
		return E_ABORT;
	}

	DWORD ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ErrorCode )
	{
		PB_FreeReply( pOutBuffer );
		RaiseException(ErrorCode, 1, 0, 0);
		return E_ABORT;
	}

	HRESULT hr = (HRESULT)pOutBuffer->MappedAddress_PID ;
	if ( hr >= 0 )
	{
		ULONG cbWritten = 0;
		hr = pStream->Write( (const void*)&pOutBuffer->riid, pOutBuffer->MappedAddress_Length, &cbWritten );
	}

	PB_FreeReply( pOutBuffer );
	return hr;
}


ULONG WINAPI fake_CoInitializeEx( LPVOID pvReserved, DWORD dwCoInit )
{
	if ( g_bDoWork_OK ) { PB_StartCOM(); }

	return (ULONG)g_CoInitializeEx_addr(pvReserved, dwCoInit);
}


ULONG WINAPI fake_RegisterDragDrop(HWND hWnd, LPDROPTARGET pDropTarget)
{
	ULONG ret, Style ; 
	HMODULE hModule ;

	ret = g_RegisterDragDrop_addr( hWnd, pDropTarget );
	if ( ret < 0 ) { return ret; }

	Style = g_GetWindowLongW_addr( hWnd, GWL_EXSTYLE );
	if ( Style & WS_EX_ACCEPTFILES ) { return ret; }

	g_SetWindowLongW_addr( hWnd, GWL_EXSTYLE, Style | WS_EX_ACCEPTFILES );
	g_SetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget, pDropTarget );

	if ( NULL == g_DragFinish_addr )
	{
		hModule = LoadLibraryW( L"shell32.dll" );
		g_DragFinish_addr = (_DragFinish_) GetProcAddress( hModule, "DragFinish" );
	}

	HandlerWindowLong( hWnd, TRUE );
	return ret;
}


ULONG WINAPI fake_RevokeDragDrop(HWND hwnd)
{
	if ( g_Atom_SBIE_DropTarget )
		g_RemovePropW_addr( hwnd, (LPCWSTR)g_Atom_SBIE_DropTarget );

	return g_RevokeDragDrop_addr(hwnd);
}


ULONG WINAPI fake_OleSetClipboard( IDataObject *pDataObj )
{
	int hdrop ;
	IDataObject *pdataObj ;
	IDataObject *FixedBuffer ;
	LPOLEClipbrdEx Buffer  ; 

	if ( pDataObj )
	{
		Buffer = (LPOLEClipbrdEx) kmalloc( 0x20 );
		hdrop = Win2kSrc__AVIGetFromClipboard( pDataObj );
		if ( hdrop ) { pdataObj = NULL ; }

		if ( Buffer )
		{
			FixedBuffer = (IDataObject *) FixDropFile( Buffer, pdataObj, hdrop );
			return g_OleSetClipboard_addr(FixedBuffer);
		}

		pdataObj = NULL ;
	}

	return g_OleSetClipboard_addr( pdataObj );
}


ULONG WINAPI fake_NdrAsyncClientCall( PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ... )
{
	//
	// _nake 函数啊,后续补充
	//

	return 0;
}


ULONG WINAPI fake_RpcAsyncCompleteCall(PRPC_ASYNC_STATE pRpcAsyncState, HRESULT *phr)
{
	//
	// 较为繁琐,后续补充
	//

	return 0;
}


BOOL WINAPI Hook_pharase21_rpcrt4()
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_bHookRpc ) { return TRUE; }

	HMODULE hModule = GetModuleHandleW( L"rpcrt4.dll" );
	if ( NULL == hModule ) { return TRUE; }

	g_NdrAsyncClientCall_addr   = (_NdrAsyncClientCall_)   GetProcAddress( hModule, "NdrAsyncClientCall"   );
	g_RpcAsyncCompleteCall_addr = (_RpcAsyncCompleteCall_) GetProcAddress( hModule, "RpcAsyncCompleteCall" );

	bRet = Mhook_SetHook( (PVOID*)&g_NdrAsyncClientCall_addr, fake_NdrAsyncClientCall );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase21_rpcrt4() - Mhook_SetHook(); | \"NdrAsyncClientCall\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_RpcAsyncCompleteCall_addr, fake_RpcAsyncCompleteCall );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase21_rpcrt4() - Mhook_SetHook(); | \"RpcAsyncCompleteCall\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase5_ole32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	int i = 0, TotalCounts = 0 ;

	// 获取函数原始地址
	LPHOOKINFOLittle pArray = g_HookInfoOle_Array;
	TotalCounts = ARRAYSIZEOF( g_HookInfoOle_Array );
	for( i=0; i<TotalCounts; i++ )
	{
		pArray[i].OrignalAddress = (PVOID) GetProcAddress( hModule, pArray[i].FunctionName );
	}

	g_CoTaskMemAlloc_addr = (_CoTaskMemAlloc_) GetProcAddress( hModule, "CoTaskMemAlloc" );
	if ( NULL == g_CoTaskMemAlloc_addr ) { return FALSE; }

	if ( FALSE == PB_IsOpenCOM() )
	{
		// 进行Hook
		for( i=0; i<TotalCounts; i++ )
		{
			bRet = HookOne( &pArray[i] );
			if ( FALSE == bRet )
			{
				MYTRACE( L"error! | Hook_pharase5_ole32dll() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
				return FALSE ;
			}
		}

		PB_IsOpenClsid( (LPCLSID)&IID_IUnknown, CLSCTX_LOCAL_SERVER, FALSE );

		//
		g_CoInitializeEx_addr = (_CoInitializeEx_) GetProcAddress( hModule, "CoInitializeEx" );
		if ( NULL == g_CoInitializeEx_addr ) { return TRUE; }

		bRet = Mhook_SetHook( (PVOID*)&g_CoInitializeEx_addr, fake_CoInitializeEx );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase5_ole32dll() - Mhook_SetHook(); | \"CoInitializeEx\" \n" );
			return FALSE ;
		}

		if ( g_bFlag_Hook_OpenWinClass_etc )
		{
			//
			g_RegisterDragDrop_addr = (_RegisterDragDrop_) GetProcAddress( hModule, "RegisterDragDrop" );
			if ( NULL == g_RegisterDragDrop_addr ) { return TRUE; }

			bRet = Mhook_SetHook( (PVOID*)&g_RegisterDragDrop_addr, fake_RegisterDragDrop );
			if ( FALSE == bRet )
			{
				MYTRACE( L"error! | Hook_pharase5_ole32dll() - Mhook_SetHook(); | \"RegisterDragDrop\" \n" );
				return FALSE ;
			}

			//
			g_RevokeDragDrop_addr = (_RevokeDragDrop_) GetProcAddress( hModule, "RevokeDragDrop" );
			if ( NULL == g_RevokeDragDrop_addr ) { return TRUE; }

			bRet = Mhook_SetHook( (PVOID*)&g_RevokeDragDrop_addr, fake_RevokeDragDrop );
			if ( FALSE == bRet )
			{
				MYTRACE( L"error! | Hook_pharase5_ole32dll() - Mhook_SetHook(); | \"RevokeDragDrop\" \n" );
				return FALSE ;
			}
		}	
	}

	if ( g_bIs_Inside_Explorer_exe )
	{
		g_ReleaseStgMedium_addr = (_ReleaseStgMedium_) GetProcAddress( hModule, "ReleaseStgMedium" );

		g_OleSetClipboard_addr = (_OleSetClipboard_) GetProcAddress( hModule, "OleSetClipboard" );
		bRet = Mhook_SetHook( (PVOID*)&g_OleSetClipboard_addr, fake_OleSetClipboard );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase5_ole32dll() - Mhook_SetHook(); | \"OleSetClipboard\" \n" );
			return FALSE ;
		}
	}

	Hook_pharase21_rpcrt4();
	return TRUE;
}





///////////////////////////////   END OF FILE   ///////////////////////////////