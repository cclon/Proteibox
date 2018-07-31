#pragma once

#include <ObjIdl.h>


//////////////////////////////////////////////////////////////////////////

struct __declspec(uuid("1E13968C-B9C3-4381-965F-C6C29B6A255E"))
IOLEClipbrdIDataObjectInterface : public IUnknown
{
// 	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject ) = 0;
// 
// 	virtual ULONG WINAPI AddRef ( IDataObject* iface ) = 0;
// 
// 	virtual ULONG WINAPI Release ( IDataObject* iface ) = 0;

	virtual HRESULT WINAPI GetData ( IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium ) = 0;

	virtual HRESULT WINAPI GetDataHere ( IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium ) = 0;

	virtual HRESULT WINAPI QueryGetData ( IDataObject* iface, LPFORMATETC pformatetc ) = 0;

	virtual HRESULT WINAPI GetCanonicalFormatEtc ( IDataObject* iface, LPFORMATETC pformatectIn, LPFORMATETC pformatetcOut ) = 0;

	virtual HRESULT WINAPI SetData ( IDataObject* iface,LPFORMATETC pformatetc,STGMEDIUM* pmedium,BOOL fRelease ) = 0;

	virtual HRESULT WINAPI EnumFormatEtc ( IDataObject* iface, DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc ) = 0;

	virtual HRESULT WINAPI DAdvise (IDataObject* iface,FORMATETC* pformatetc,DWORD advf,IAdviseSink* pAdvSink,DWORD* pdwConnection) = 0;

	virtual HRESULT WINAPI DUnadvise ( IDataObject* iface, DWORD dwConnection ) = 0;

	virtual HRESULT WINAPI EnumDAdvise ( IDataObject* iface, IEnumSTATDATA** ppenumAdvise ) = 0;
};


class ATL_NO_VTABLE COLEClipbrdIDataObject : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IOLEClipbrdIDataObjectInterface
{
public:
	COLEClipbrdIDataObject()
	{
	}

	DECLARE_NOT_AGGREGATABLE(COLEClipbrdIDataObject)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(COLEClipbrdIDataObject)
		COM_INTERFACE_ENTRY(IOLEClipbrdIDataObjectInterface)
	END_COM_MAP()

public:
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject );

	virtual ULONG WINAPI AddRef ( IDataObject* iface );

	virtual ULONG WINAPI Release ( IDataObject* iface );

	virtual HRESULT WINAPI GetData ( IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium );

	virtual HRESULT WINAPI GetDataHere ( IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium );

	virtual HRESULT WINAPI QueryGetData ( IDataObject* iface, LPFORMATETC pformatetc );

	virtual HRESULT WINAPI GetCanonicalFormatEtc ( IDataObject* iface, LPFORMATETC pformatectIn, LPFORMATETC pformatetcOut );

	virtual HRESULT WINAPI SetData ( IDataObject* iface,LPFORMATETC pformatetc,STGMEDIUM* pmedium,BOOL fRelease );

	virtual HRESULT WINAPI EnumFormatEtc ( IDataObject* iface, DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc );

	virtual HRESULT WINAPI DAdvise (IDataObject* iface,FORMATETC* pformatetc,DWORD advf,IAdviseSink* pAdvSink,DWORD* pdwConnection);

	virtual HRESULT WINAPI DUnadvise ( IDataObject* iface, DWORD dwConnection );

	virtual HRESULT WINAPI EnumDAdvise ( IDataObject* iface, IEnumSTATDATA** ppenumAdvise );
};


//////////////////////////////////////////////////////////////////////////

struct __declspec(uuid("E56B648A-A547-435b-B626-D2B6EAC6F378"))
IOLEClipbrdIEnumFORMATETCInterface : public IUnknown
{
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject ) = 0;

	virtual ULONG WINAPI AddRef ( IDataObject* iface ) = 0;

	virtual ULONG WINAPI Release ( IDataObject* iface ) = 0;

	virtual HRESULT WINAPI Next ( LPENUMFORMATETC iface,ULONG celt,FORMATETC *rgelt,ULONG *pceltFethed ) = 0;

	virtual HRESULT WINAPI Skip ( LPENUMFORMATETC iface, ULONG celt ) = 0;

	virtual HRESULT WINAPI Reset ( LPENUMFORMATETC iface ) = 0;

	virtual HRESULT WINAPI Clone ( LPENUMFORMATETC iface, LPENUMFORMATETC* ppenum ) = 0;
};


class ATL_NO_VTABLE COLEClipbrdIEnumFORMATETC : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IOLEClipbrdIEnumFORMATETCInterface
{
public:
	COLEClipbrdIEnumFORMATETC()
	{
	}

	DECLARE_NOT_AGGREGATABLE(COLEClipbrdIEnumFORMATETC)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(COLEClipbrdIEnumFORMATETC)
		COM_INTERFACE_ENTRY(IOLEClipbrdIEnumFORMATETCInterface)
	END_COM_MAP()

public:
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject );

	virtual ULONG WINAPI AddRef ( IDataObject* iface );

	virtual ULONG WINAPI Release ( IDataObject* iface );

	virtual HRESULT WINAPI Next ( LPENUMFORMATETC iface,ULONG celt,FORMATETC *rgelt,ULONG *pceltFethed );

	virtual HRESULT WINAPI Skip ( LPENUMFORMATETC iface, ULONG celt );

	virtual HRESULT WINAPI Reset ( LPENUMFORMATETC iface );

	virtual HRESULT WINAPI Clone ( LPENUMFORMATETC iface, LPENUMFORMATETC* ppenum );
};


extern CComObjectGlobal<COLEClipbrdIDataObject> g_OLEClipbrd_IDataObject_VTable ;

extern CComObjectGlobal<COLEClipbrdIEnumFORMATETC> g_OLEClipbrd_IEnumFORMATETC_VTable ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_QueryInterface_ ) (LPENUMFORMATETC iface, LPVOID riid, LPVOID* ppvObj);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_AddRef_ ) (LPENUMFORMATETC iface);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_Release_ ) (LPENUMFORMATETC iface);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_Next_ ) (LPENUMFORMATETC iface, ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_Skip_ ) (LPENUMFORMATETC iface, ULONG celt);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_Reset_ ) (LPENUMFORMATETC iface);
typedef HRESULT (__stdcall * _OLEClipbrd_IEnumFORMATETC_Clone_ ) (LPENUMFORMATETC iface, LPENUMFORMATETC* ppenum);


typedef struct _OLEClipbrd_IEnumFORMATETC_DEF_
{
	_OLEClipbrd_IEnumFORMATETC_QueryInterface_ OLEClipbrd_IEnumFORMATETC_QueryInterface ;
	_OLEClipbrd_IEnumFORMATETC_AddRef_ OLEClipbrd_IEnumFORMATETC_AddRef ;
	_OLEClipbrd_IEnumFORMATETC_Release_ OLEClipbrd_IEnumFORMATETC_Release ;
	_OLEClipbrd_IEnumFORMATETC_Next_ OLEClipbrd_IEnumFORMATETC_Next ;
	_OLEClipbrd_IEnumFORMATETC_Skip_ OLEClipbrd_IEnumFORMATETC_Skip ;
	_OLEClipbrd_IEnumFORMATETC_Reset_ OLEClipbrd_IEnumFORMATETC_Reset ;
	_OLEClipbrd_IEnumFORMATETC_Clone_ OLEClipbrd_IEnumFORMATETC_Clone ;

} OLEClipbrd_IEnumFORMATETC_DEF, *LPOLEClipbrd_IEnumFORMATETC_DEF ;



typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_QueryInterface_ ) (IDataObject* iface, LPVOID riid, LPVOID* ppvObj);
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_AddRef_ ) (IDataObject* iface);
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_Release_ ) (IDataObject* iface);
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_GetData_ ) (IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_GetDataHere_ ) (IDataObject* iface, LPFORMATETC pformatetcIn, STGMEDIUM* pmedium );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_QueryGetData_ ) (IDataObject* iface, LPFORMATETC pformatetcIn );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_GetCanonicalFormatEtc_ ) (IDataObject* iface, LPFORMATETC pformatetcIn, LPFORMATETC pformatetcOut );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_SetData_ ) (IDataObject* iface, LPFORMATETC pformatetc, STGMEDIUM* pmedium, BOOL fRelease );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_EnumFormatEtc_ ) (IDataObject* iface, DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc );

typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_DAdvise_ ) (IDataObject*     iface,
																 FORMATETC*       pformatetc,
																 DWORD            advf,
																 IAdviseSink*     pAdvSink,
																 DWORD*           pdwConnection);

typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_DUnadvise_ ) (IDataObject* iface, DWORD dwConnection );
typedef HRESULT (__stdcall * _OLEClipbrd_IDataObject_EnumDAdvise_ ) (IDataObject* iface, IEnumSTATDATA** ppenumAdvise );


typedef struct _OLEClipbrd_IDataObject_DEF_
{
	_OLEClipbrd_IDataObject_QueryInterface_ OLEClipbrd_IDataObject_QueryInterface ;
	_OLEClipbrd_IDataObject_AddRef_ OLEClipbrd_IDataObject_AddRef  ;
	_OLEClipbrd_IDataObject_Release_ OLEClipbrd_IDataObject_Release ;
	_OLEClipbrd_IDataObject_GetData_ OLEClipbrd_IDataObject_GetData ;
	_OLEClipbrd_IDataObject_GetDataHere_ OLEClipbrd_IDataObject_GetDataHere ;
	_OLEClipbrd_IDataObject_QueryGetData_ OLEClipbrd_IDataObject_QueryGetData ;
	_OLEClipbrd_IDataObject_GetCanonicalFormatEtc_ OLEClipbrd_IDataObject_GetCanonicalFormatEtc ;
	_OLEClipbrd_IDataObject_SetData_ OLEClipbrd_IDataObject_SetData ;
	_OLEClipbrd_IDataObject_EnumFormatEtc_ OLEClipbrd_IDataObject_EnumFormatEtc ;
	_OLEClipbrd_IDataObject_DAdvise_ OLEClipbrd_IDataObject_DAdvise ;
	_OLEClipbrd_IDataObject_DUnadvise_ OLEClipbrd_IDataObject_DUnadvise ;
	_OLEClipbrd_IDataObject_EnumDAdvise_ OLEClipbrd_IDataObject_EnumDAdvise ;

} OLEClipbrd_IDataObject_DEF, *LPOLEClipbrd_IDataObject_DEF ;



typedef struct _OLEClipbrdEx_ 
{
/*0x000*/ COLEClipbrdIDataObject* lpVtbl1 ; // 一堆函数的起始指针
/*0x004*/ IDataObject * pIDataObjectSrc ;
/*0x008*/ WPARAM hdrop ;
/*0x00C*/ LONG RefCounts ;
/*0x010*/ COLEClipbrdIEnumFORMATETC* lpVtbl2 ; // 一堆函数的起始指针
/*0x014*/ ULONG Reserved2 ;
/*0x018*/ HMODULE hModule_shell32_dll ;
/*0x01C*/ ULONG DragQueryFileW_Addr ; // DragQueryFileW函数地址

} OLEClipbrdEx, *LPOLEClipbrdEx ;


typedef int (__stdcall * _QueryInterface_ )(IDropTarget *, IID*, LPVOID *);
typedef int (__stdcall * _AddRef_ )(IDropTarget *);
typedef int (__stdcall * _Release_ )(IDropTarget *);
typedef int (__stdcall * _DragEnter_ )(IDropTarget *, IDataObject *, DWORD, LONG, LONG, DWORD *);
typedef int (__stdcall * _DragOver_ )(IDropTarget *, DWORD, LONG, LONG, DWORD *);
typedef int (__stdcall * _DragLeave_ )(IDropTarget *);
typedef void (__stdcall * _Drop_ )(IDropTarget *, IDataObject *, DWORD, LONG, LONG, DWORD *);


typedef struct _SIDropTargetVtbl_
{
	_QueryInterface_ QueryInterface ;
	_AddRef_ AddRef  ;
	_Release_ Release ;
	_DragEnter_ DragEnter ;
	_DragOver_ DragOver ;
	_DragLeave_ DragLeave ;
	_Drop_ Drop ;

} SIDropTargetVtbl, *LPSIDropTargetVtbl ;


typedef struct _SIDropTargetImpl_ 
{
	LPSIDropTargetVtbl lpVtbl ;

} SIDropTargetImpl, *LPSIDropTargetImpl ;


typedef struct
{
	/* IEnumFORMATETC VTable */
//	IEnumFORMATETCVtbl* lpVtbl;
	/*LPOLEClipbrd_IEnumFORMATETC_DEF*/ PVOID lpVtbl;

	/* IEnumFORMATETC fields */
	UINT                         posFmt;    /* current enumerator position */
	UINT                         countFmt;  /* number of EnumFORMATETC's in array */
	LPFORMATETC                  pFmt;      /* array of EnumFORMATETC's */

	/*
	* Reference count of this object
	*/
	LONG                         ref;

	/*
	* IUnknown implementation of the parent data object.
	*/
	IUnknown*                    pUnkDataObj;

} IEnumFORMATETCImpl;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


void 
CopyClipboardData (
	IN int This,
	IN LPFORMATETC pformatetcIn,
	IN STGMEDIUM *pmedium
	);

HGLOBAL
CopyClipboardDataEx (
	IN int This,
	IN HGLOBAL hMem
	);

HGLOBAL 
CopyClipboardDataW (
	IN HGLOBAL hMem
	);

HGLOBAL 
CopyClipboardDataA (
	IN HGLOBAL hMem
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
