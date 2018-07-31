#pragma once

#include <ObjIdl.h>

//////////////////////////////////////////////////////////////////////////

#define DIERR_NOTFOUND 0x80070002


typedef DWORD PST_PROVIDERCAPABILITIES;

typedef DWORD PST_REPOSITORYCAPABILITIES;

typedef DWORD PST_KEY;

typedef DWORD PST_ACCESSMODE;

typedef DWORD PST_ACCESSCLAUSETYPE;

typedef GUID UUID;

typedef ULARGE_INTEGER PST_PROVIDER_HANDLE;

typedef GUID PST_PROVIDERID;

typedef PST_PROVIDERID __RPC_FAR *PPST_PROVIDERID;

typedef struct  _PST_PROVIDERINFO
{
	DWORD cbSize;
	PST_PROVIDERID ID;
	PST_PROVIDERCAPABILITIES Capabilities;
	LPWSTR szProviderName;
}	PST_PROVIDERINFO;

typedef struct _PST_PROVIDERINFO __RPC_FAR *PPST_PROVIDERINFO;

typedef struct  _PST_TYPEINFO
{
	DWORD cbSize;
	LPWSTR szDisplayName;
}	PST_TYPEINFO;

typedef struct _PST_TYPEINFO __RPC_FAR *PPST_TYPEINFO;

typedef struct  _PST_PROMPTINFO
{
	DWORD cbSize;
	DWORD dwPromptFlags;
	HWND  hwndApp;
	LPCWSTR szPrompt;
}	PST_PROMPTINFO;

typedef struct _PST_PROMPTINFO __RPC_FAR *PPST_PROMPTINFO;

typedef struct  _PST_ACCESSCLAUSE
{
	DWORD cbSize;
	PST_ACCESSCLAUSETYPE ClauseType;
	DWORD cbClauseData;
	/* [size_is] */ VOID __RPC_FAR *pbClauseData;
}	PST_ACCESSCLAUSE;

typedef struct _PST_ACCESSCLAUSE __RPC_FAR *PPST_ACCESSCLAUSE;

typedef struct  _PST_ACCESSRULE
{
	DWORD cbSize;
	PST_ACCESSMODE AccessModeFlags;
	DWORD cClauses;
	/* [size_is] */ PST_ACCESSCLAUSE __RPC_FAR *rgClauses;
}	PST_ACCESSRULE;

typedef struct _PST_ACCESSRULE __RPC_FAR *PPST_ACCESSRULE;

typedef struct  _PST_ACCESSRULESET
{
	DWORD cbSize;
	DWORD cRules;
	/* [size_is] */ PST_ACCESSRULE __RPC_FAR *rgRules;
}	PST_ACCESSRULESET;

typedef struct _PST_ACCESSRULESET __RPC_FAR *PPST_ACCESSRULESET;


typedef struct _SbiePst_dat_FileInfo_ 
{
/*0x000 */	HANDLE hFile ;
/*0x004 */  ULONG  DataLength ;
/*0x008 */  PBYTE  pDataPointer ;
/*0x00C */  WCHAR  pBuffer[1] ;

} SbiePst_dat_FileInfo, *PSbiePst_dat_FileInfo ;


// 微软定义的结构
typedef enum tagINSTALLSTATE
{
	INSTALLSTATE_NOTUSED = -7,
	INSTALLSTATE_BADCONFIG = -6,
	INSTALLSTATE_INCOMPLETE = -5,
	INSTALLSTATE_SOURCEABSENT = -4,
	INSTALLSTATE_MOREDATA = -3,
	INSTALLSTATE_INVALIDARG = -2,
	INSTALLSTATE_UNKNOWN = -1,
	INSTALLSTATE_BROKEN = 0,
	INSTALLSTATE_ADVERTISED = 1,
	INSTALLSTATE_ABSENT = 2,
	INSTALLSTATE_LOCAL = 3,
	INSTALLSTATE_SOURCE = 4,
	INSTALLSTATE_DEFAULT = 5
} INSTALLSTATE;

typedef unsigned long MSIHANDLE;


//////////////////////////////////////////////////////////////////////////


//
// 对应于Com相关的函数如 IPStoreVtbl_CreateType()
//


typedef struct _IPStoreVtbl_Type_Info_grandsun_		// size - 0x18
{ 
/*0x000 */ struct _IPStoreVtbl_Type_Info_grandsun_ * pFlink ;		// 上个节点
/*0x004 */ struct _IPStoreVtbl_Type_Info_grandsun_ * pBlink ;		// 下个节点   
/*0x008 */ LPWSTR  szItemName ;
/*0x00C */ DWORD	Flag ;
/*0x010 */ PBYTE	pbData ;
/*0x014 */ DWORD	cbData ;

} IPStoreVtbl_Type_Info_grandsun, *PIPStoreVtbl_Type_Info_grandsun ;


struct IPStoreVtbl_Type_Info_GrandsunStruct 
{
/*0x000 */  struct _IPStoreVtbl_Type_Info_grandsun_ * pFlink ;		// 上个节点
/*0x004 */  struct _IPStoreVtbl_Type_Info_grandsun_ * pBlink ;		// 下个节点   
/*0x008 */  DWORD nGrandsunCounts ;
} ;


typedef struct _IPStoreVtbl_Type_Info_sun_	// size - 0x2C
{ 
/*0x000 */ struct _IPStoreVtbl_Type_Info_sun_ * pFlink ;		// 上个节点
/*0x004 */ struct _IPStoreVtbl_Type_Info_sun_ * pBlink ;		// 下个节点   
/*0x008 */ IID		pType ; 
/*0x018 */ LPWSTR	szDisplayName ;
/*0x01C */ DWORD	Flag ;
/*0x020 */ struct IPStoreVtbl_Type_Info_GrandsunStruct pGrandsunNode ;

} IPStoreVtbl_Type_Info_sun, *PIPStoreVtbl_Type_Info_sun ;


typedef struct _IPStoreVtbl_Type_Info_SunStruct_
{
/*0x000 */  struct _IPStoreVtbl_Type_Info_sun_ * pFlink ;		// 上个节点
/*0x004 */  struct _IPStoreVtbl_Type_Info_sun_ * pBlink ;		// 下个节点   
/*0x008 */  DWORD nSunCounts ;

} IPStoreVtbl_Type_Info_SunStruct, *LPIPStoreVtbl_Type_Info_SunStruct ;


typedef struct _IPStoreVtbl_Type_Info_	// size - 0x2C
{ 
/*0x000 */ struct _IPStoreVtbl_Type_Info_ * pFlink ;		// 上个节点
/*0x004 */ struct _IPStoreVtbl_Type_Info_ * pBlink ;		// 下个节点   
/*0x008 */ IID		pType ; 
/*0x018 */ LPWSTR	szDisplayName ;
/*0x01C */ DWORD	Flag ;
/*0x020 */ IPStoreVtbl_Type_Info_SunStruct pSunNode ;

} IPStoreVtbl_Type_Info, *PIPStoreVtbl_Type_Info ;


typedef struct _PStoreCreateInstance_Info_Litlle_  // size - 0x20
{
/*0x000*/ LIST_ENTRY_EX ListEntry ; // 指向 PIPStoreVtbl_Type_Info
/*0x00C*/ ULONG Reserved ;
/*0x010*/ LARGE_INTEGER lpMappedAddrOld ;
/*0x018*/ PLARGE_INTEGER lpMappedAddr ;
/*0x01C*/ 
		union 
		{
			BYTE bFlag1;
			BOOL bFlag2 ;
		} u;

} PStoreCreateInstance_Info_Litlle, *LPPStoreCreateInstance_Info_Litlle ;


typedef struct _PStoreCreateInstance_Info_  // size - 0x38
{
/*0x000*/ /*CIPStoreIDataObject**/ PVOID lpVtbl ;
/*0x004*/ ULONG Ref ;
/*0x008*/ ULONG CoTaskMemAlloc_addr ;
/*0x00C*/ HANDLE hMutex ;
/*0x010*/ HANDLE hMapped ;
/*0x014*/ HANDLE hHeap ;
/*0x018*/ PStoreCreateInstance_Info_Litlle LittleInfo ;

} PStoreCreateInstance_Info, *LPPStoreCreateInstance_Info ;


// PBIPStoreGetNode()
typedef struct _RPC_IN_PBIPStoreGetNode_	// size - 0x18
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  IID	iid_data ;

} RPC_IN_PBIPStoreGetNode, *LPRPC_IN_PBIPStoreGetNode ;


typedef struct _RPC_OUT_PBIPStoreGetNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG NameLength ;
/*0x010 */  WCHAR szDisplayName[1] ;

} RPC_OUT_PBIPStoreGetNode, *LPRPC_OUT_PBIPStoreGetNode ;


// PBIPStoreGetChildNode()
typedef struct _RPC_IN_PBIPStoreGetChildNode_	// size - 0x28
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  IID	iid_data ;
/*0x018 */  IID	iid_data_sun ;

} RPC_IN_PBIPStoreGetChildNode, *LPRPC_IN_PBIPStoreGetChildNode ;


typedef struct _RPC_OUT_PBIPStoreGetChildNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG NameLength ;
/*0x010 */  WCHAR szDisplayName[1] ;

} RPC_OUT_PBIPStoreGetChildNode, *LPRPC_OUT_PBIPStoreGetChildNode ;


// PBIPStoreGetGrandchildNode()
typedef struct _RPC_IN_PBIPStoreGetGrandchildNode_	// size - 0x02C
{ 
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */ IID		iid_data ; 
/*0x018 */ IID		iid_data_sun ; 
/*0x028 */ ULONG	Length ;
/*0x02C */ WCHAR    szItemName[1] ;

} RPC_IN_PBIPStoreGetGrandchildNode, *LPRPC_IN_PBIPStoreGetGrandchildNode ;


typedef struct _RPC_OUT_PBIPStoreGetGrandchildNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  DWORD	cbData ;
/*0x00C */  WCHAR Data[1] ;

} RPC_OUT_PBIPStoreGetGrandchildNode, *LPRPC_OUT_PBIPStoreGetGrandchildNode ;



// PBIPStoreEnumSubtypes()
typedef struct _RPC_IN_PBIPStoreEnumSubtypes_	// size - 0x020
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ int Key ; 
/*0x00C */ GUID ClsidType ; 
/*0x01C */ BOOL bEnumSubtypes;

} RPC_IN_PBIPStoreEnumSubtypes, *LPRPC_IN_PBIPStoreEnumSubtypes ;


typedef struct _RPC_OUT_PBIPStoreEnumSubtypes_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  int	nCounts ;
/*0x00C */  LPCLSID Data[1] ;

} RPC_OUT_PBIPStoreEnumSubtypes, *LPRPC_OUT_PBIPStoreEnumSubtypes ;



// PBIPStoreEnumTypes()
typedef struct _PBIPStoreEnumTypes_CLSID_Info_	// size - 0x014
{ 
/*0x000 */ LIST_ENTRY ListEntry ;
/*0x008 */ CLSID clsid ; 

} PBIPStoreEnumTypes_CLSID_Info, *LPPBIPStoreEnumTypes_CLSID_Info ;


typedef struct _PBIPStoreEnumTypes_Little_Info_	// size - 0x014
{ 
/*0x000 */ LIST_ENTRY_EX ListEntryEx ; // 指向 LPPBIPStoreEnumTypes_CLSID_Info
/*0x00C */ ULONG Reserved1 ; 
/*0x010 */ ULONG Reserved2 ; 

} PBIPStoreEnumTypes_Little_Info, *LPPBIPStoreEnumTypes_Little_Info ;


typedef struct _PBIPStoreEnumTypes_Info_	// size - 0x010
{ 
/*0x000 */ PVOID lpVtbl ;
/*0x004 */ LPPBIPStoreEnumTypes_Little_Info pNodeLittle ; 
/*0x008 */ ULONG Reserved1 ; 
/*0x00C */ ULONG Reserved2 ;

} PBIPStoreEnumTypes_Info, *LPPBIPStoreEnumTypes_Info ;


typedef struct _RPC_IN_PBIPStoreEnumTypes_	// size - 0x020
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ int Key ; 
/*0x00C */ GUID ClsidType ; 
/*0x01C */ BOOL bEnumSubtypes ; 

} RPC_IN_PBIPStoreEnumTypes, *LPRPC_IN_PBIPStoreEnumTypes ;


typedef struct _RPC_OUT_PBIPStoreEnumTypes_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  int	nCounts ;
/*0x00C */  LPCLSID Data[1] ;

} RPC_OUT_PBIPStoreEnumTypes, *LPRPC_OUT_PBIPStoreEnumTypes ;


// PBIPStoreEnumItems
typedef struct _PBIPStoreEnumItems_Datas_	// size - 0x014
{ 
/*0x000 */ LIST_ENTRY ListEntry ; 
/*0x008 */ WCHAR szItemName[ 1 ];

} PBIPStoreEnumItems_Datas, *LPPBIPStoreEnumItems_Datas ;

typedef struct _PBIPStoreEnumItems_Little_Info_	// size - 0x014
{ 
/*0x000 */ LIST_ENTRY_EX ListEntryEx ; // 指向 LPPBIPStoreEnumItems_Datas
/*0x00C */ ULONG Reserved1 ; 
/*0x010 */ ULONG Reserved2 ; 

} PBIPStoreEnumItems_Little_Info, *LPPBIPStoreEnumItems_Little_Info ;


typedef struct _PBIPStoreEnumItems_Info_	// size - 0x014
{ 
/*0x000 */ PVOID lpVtbl ;
/*0x004 */ LPPBIPStoreEnumItems_Little_Info pNodeLittle ; 
/*0x008 */ ULONG Reserved1 ; 
/*0x00C */ ULONG Reserved2 ;
/*0x010 */ ULONG CoTaskMemAlloc_addr ;

} PBIPStoreEnumItems_Info, *LPPBIPStoreEnumItems_Info ;


// PBIPStoreEnumItems
typedef struct _RPC_IN_PBIPStoreEnumItems_	// size - 0x02C
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ int Key ; 
/*0x00C */ GUID ClsidType ; 
/*0x01C */ GUID ClsidStubType ; 

} RPC_IN_PBIPStoreEnumItems, *LPRPC_IN_PBIPStoreEnumItems ;


typedef struct _RPC_OUT_PBIPStoreEnumItems_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  int	nCounts ;
/*0x00C */  WCHAR Data[1] ;

} RPC_OUT_PBIPStoreEnumItems, *LPRPC_OUT_PBIPStoreEnumItems ;




//////////////////////////////////////////////////////////////////////////

struct __declspec(uuid("A35409CC-FE07-4398-8C41-5CD98DCF2CC9"))
IIPStoreIDataObjectInterface : public IUnknown
{
	// 	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject ) = 0;
	// 
	// 	virtual ULONG WINAPI AddRef ( IDataObject* iface ) = 0;
	// 
	// 	virtual ULONG WINAPI Release ( IDataObject* iface ) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetInfo( PPST_PROVIDERINFO __RPC_FAR *ppProperties ) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetProvParam( DWORD dwParam,DWORD __RPC_FAR *pcbData,BYTE __RPC_FAR *__RPC_FAR *ppbData,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetProvParam( DWORD dwParam,DWORD cbData,BYTE __RPC_FAR *pbData,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateType( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO pInfo,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteType( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateSubtype( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_TYPEINFO pInfo,PPST_ACCESSRULESET pRules,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteSubtype( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_ACCESSRULESET __RPC_FAR *ppRules,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_ACCESSRULESET pRules,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE EnumTypes( PST_KEY Key,DWORD dwFlags,/*IEnumPStoreTypes*/ int __RPC_FAR *__RPC_FAR *ppenum) = 0;

	virtual HRESULT STDMETHODCALLTYPE EnumSubtypes( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags,/*IEnumPStoreTypes*/int __RPC_FAR *__RPC_FAR *ppenum) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE ReadItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		DWORD __RPC_FAR *pcbData,
		BYTE __RPC_FAR *__RPC_FAR *ppbData,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE WriteItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		DWORD cbData,
		BYTE __RPC_FAR *pbData,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwDefaultConfirmationStyle,
		DWORD dwFlags ) = 0;

	virtual HRESULT STDMETHODCALLTYPE OpenItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		PST_ACCESSMODE ModeFlags,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags ) = 0;

	virtual HRESULT STDMETHODCALLTYPE CloseItem( PST_KEY Key,const GUID __RPC_FAR *pItemType,const GUID __RPC_FAR *pItemSubtype,LPCWSTR szItemName,DWORD dwFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE EnumItems( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		DWORD dwFlags,
		/*IEnumPStoreItems*/int __RPC_FAR *__RPC_FAR *ppenum) = 0;
};


class ATL_NO_VTABLE CIPStoreIDataObject : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IIPStoreIDataObjectInterface
{
public:
	CIPStoreIDataObject()
	{
	}

	DECLARE_NOT_AGGREGATABLE(CIPStoreIDataObject)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CIPStoreIDataObject)
		COM_INTERFACE_ENTRY(IIPStoreIDataObjectInterface)
	END_COM_MAP()

public:
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject );

	virtual ULONG WINAPI AddRef ( IDataObject* iface );

	virtual ULONG WINAPI Release ( IDataObject* iface );

	virtual HRESULT STDMETHODCALLTYPE GetInfo( PPST_PROVIDERINFO __RPC_FAR *ppProperties );

	virtual HRESULT STDMETHODCALLTYPE GetProvParam( DWORD dwParam,DWORD __RPC_FAR *pcbData,BYTE __RPC_FAR *__RPC_FAR *ppbData,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE SetProvParam( DWORD dwParam,DWORD cbData,BYTE __RPC_FAR *pbData,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE CreateType( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO pInfo,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE DeleteType( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE CreateSubtype( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_TYPEINFO pInfo,PPST_ACCESSRULESET pRules,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_TYPEINFO __RPC_FAR *ppInfo,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE DeleteSubtype( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_ACCESSRULESET __RPC_FAR *ppRules,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset( PST_KEY Key,const GUID __RPC_FAR *pType,const GUID __RPC_FAR *pSubtype,PPST_ACCESSRULESET pRules,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE EnumTypes( PST_KEY Key,DWORD dwFlags,/*IEnumPStoreTypes*/int __RPC_FAR *__RPC_FAR *ppenum);

	virtual HRESULT STDMETHODCALLTYPE EnumSubtypes( PST_KEY Key,const GUID __RPC_FAR *pType,DWORD dwFlags,/*IEnumPStoreTypes*/int __RPC_FAR *__RPC_FAR *ppenum);

	virtual HRESULT STDMETHODCALLTYPE DeleteItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags
		);

	virtual HRESULT STDMETHODCALLTYPE ReadItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		DWORD __RPC_FAR *pcbData,
		BYTE __RPC_FAR *__RPC_FAR *ppbData,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags
		);

	virtual HRESULT STDMETHODCALLTYPE WriteItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		DWORD cbData,
		BYTE __RPC_FAR *pbData,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwDefaultConfirmationStyle,
		DWORD dwFlags 
		);

	virtual HRESULT STDMETHODCALLTYPE OpenItem( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		LPCWSTR szItemName,
		PST_ACCESSMODE ModeFlags,
		PPST_PROMPTINFO pPromptInfo,
		DWORD dwFlags
		);

	virtual HRESULT STDMETHODCALLTYPE CloseItem( PST_KEY Key,const GUID __RPC_FAR *pItemType,const GUID __RPC_FAR *pItemSubtype,LPCWSTR szItemName,DWORD dwFlags);

	virtual HRESULT STDMETHODCALLTYPE EnumItems( 
		PST_KEY Key,
		const GUID __RPC_FAR *pItemType,
		const GUID __RPC_FAR *pItemSubtype,
		DWORD dwFlags,
		/*IEnumPStoreItems*/int __RPC_FAR *__RPC_FAR *ppenum
		);

public:

	BOOL PBIPStoreHandler();

	PVOID PBIPStoreAllocateNode( IN BOOL bOverWrite );

	NTSTATUS 
		PBIPStoreAllocateNodeEx ( 
		OUT PVOID *pBuffer,
		IN HANDLE hFile
		);

	NTSTATUS 
		PBIPStoreParse (
		IN PSbiePst_dat_FileInfo pNode, 
		OUT LPBYTE pData
		);

	NTSTATUS 
		PBIPStoreParseEx (
		IN OUT PSbiePst_dat_FileInfo pNode
		);

	NTSTATUS 
		PBIPStoreParseExp (
		IN OUT PSbiePst_dat_FileInfo pFileNode,
		IN ULONG nCounts,
		OUT LPBYTE pData
		);

	BOOL
		PBIPStoreDecode (
		IN PSbiePst_dat_FileInfo pNode,
		OUT PVOID* DecodedData,
		OUT int *pDataLength
		);

	BOOL PBIPStoreRelease( IN PSbiePst_dat_FileInfo pNode );

	BOOL 
		PBIPStoreBuild (
		IN PSbiePst_dat_FileInfo pNode
		);

	BOOL
		PBIPStoreBuildChildren (
		IN PSbiePst_dat_FileInfo pNode,
		IN PIPStoreVtbl_Type_Info pNode_father
		);

	BOOL
		PBIPStoreBuildGrandChildren (
		IN PSbiePst_dat_FileInfo pNode,
		IN PIPStoreVtbl_Type_Info_sun pNode_father
		);

	PVOID
		PBIPStoreGetNode (
		IN const GUID * pType
		);

	PVOID
		PBIPStoreGetChildNode (
		IN PIPStoreVtbl_Type_Info pNode,
		IN const GUID * pSubtype
		);

	PVOID
		PBIPStoreGetGrandchildNode (
		IN PIPStoreVtbl_Type_Info pNode,
		IN PIPStoreVtbl_Type_Info_sun pChildNode,
		IN LPWSTR szItemName
		);

	PVOID
		PBIPStoreSetGrandchildNode (
		IN PIPStoreVtbl_Type_Info_sun pChildNode,
		IN PIPStoreVtbl_Type_Info_grandsun pGrandSunNode, 
		IN LPWSTR szItemName,
		IN DWORD cbData,
		IN PBYTE pbData
		);

	LPVOID 
		PBIPStoreInsertNode (
		IN const GUID * pType,
		IN PPST_TYPEINFO pInfo
		);

	LPVOID
		PBIPStoreInsertChildNode (
		IN PIPStoreVtbl_Type_Info pNode,
		IN const GUID * pSubtype,
		IN PPST_TYPEINFO pInfo
		);

	VOID PBIPStoreHandlerFile();

	NTSTATUS 
		PBIPStoreWriteFilePhrase1 (
		IN PSbiePst_dat_FileInfo pNode,
		IN int nCounts
		);

	NTSTATUS 
		PBIPStoreWriteFilePhrase2 (
		IN PSbiePst_dat_FileInfo pFileNode,
		IN UINT nCounts,
		IN PBYTE pData
		);

	BOOL
		PBIPStoreWriteFileNodes (
		IN PSbiePst_dat_FileInfo pFileNode,
		IN PIPStoreVtbl_Type_Info pNode
		);

	BOOL
		PBIPStoreWriteFileChiledNodes (
		IN PSbiePst_dat_FileInfo pFileNode, 
		IN PIPStoreVtbl_Type_Info_sun pNode_sun
		);

	BOOL
		PBIPStoreWriteFileGrandchildNodes (
		IN PSbiePst_dat_FileInfo pFileNode, 
		IN PIPStoreVtbl_Type_Info_grandsun pNode
		);

	BOOL
		PBIPStoreEncodeFile (
		IN PSbiePst_dat_FileInfo pFileNode,
		IN PBYTE pBuffer,
		IN int nCounts
		);

	NTSTATUS 
		PBIPStoreWriteFile ( 
		IN PSbiePst_dat_FileInfo pFileNode
		);

	PVOID PBIPStoreEnumTypes( IN int Key );

	PVOID PBIPStoreEnumSubtypes( int Key, const GUID *pType );

	VOID PBIPStoreAddClsid( LPPBIPStoreEnumTypes_Info pNode, LPCLSID clsid );

	PVOID PBIPStoreEnumItems( ULONG key, const GUID * pItemtype, const GUID * pItemSubtype );

	VOID PBIPStoreEnumItemsEx( LPPBIPStoreEnumItems_Info pNode, LPWSTR szItemName );

protected:
	PStoreCreateInstance_Info Info ;

};


extern CComObjectGlobal<CIPStoreIDataObject> g_IPStore_IDataObject_VTable ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef struct _EnumPStoreTypes_Info_List_  // size - 0x10
{
/*0x000*/ LIST_ENTRY_EX ListEntryEx ; 
/*0x00C*/ ULONG Ref ;
/*0x010*/ BOOL bFlag ;

} EnumPStoreTypes_Info_List, *LPEnumPStoreTypes_Info_List ;

typedef struct _EnumPStoreTypes_Little_Info_  // size - 0x
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ WCHAR szName[1] ;

} EnumPStoreTypes_Little_Info, *LPEnumPStoreTypes_Little_Info ;


typedef struct _EnumPStoreTypes_Info_  // size - 0x14
{
/*0x000*/ PVOID lpVtbl ;
/*0x004*/ LPEnumPStoreTypes_Info_List ListHead ;
/*0x008*/ LPEnumPStoreTypes_Little_Info LittleInfo ;
/*0x00C*/ ULONG Ref ;
/*0x010*/ ULONG Func ;

} EnumPStoreTypes_Info, *LPEnumPStoreTypes_Info ;


typedef struct _EnumPStoreTypesVtbl_Info_  // size - 0x10
{
/*0x000*/ PVOID lpVtbl ;
/*0x004*/ LPEnumPStoreTypes_Info_List ListHead ;
/*0x004*/ LPEnumPStoreTypes_Little_Info LittleInfo ;
/*0x00C*/ ULONG Ref ;

} EnumPStoreTypesVtbl_Info, *LPEnumPStoreTypesVtbl_Info ;


//////////////////////////////////////////////////////////////////////////

struct __declspec(uuid("417C9136-DAD8-4428-A496-83F83758F4B8"))
IEnumPStoreTypesInterface : public IUnknown
{
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject ) = 0;

	virtual ULONG WINAPI AddRef ( IDataObject* iface ) = 0;

	virtual ULONG WINAPI Release ( IDataObject* iface ) = 0;

	virtual HRESULT WINAPI Next ( ULONG celt,LPWSTR *rgelt,ULONG *pceltFethed ) = 0;

	virtual HRESULT WINAPI Skip ( ULONG celt ) = 0;

	virtual HRESULT WINAPI Reset ( ) = 0;

	virtual HRESULT WINAPI Clone (  IEnumPStoreTypesInterface** ppenum ) = 0;
};


class ATL_NO_VTABLE CENUMPSTORETPYES : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IEnumPStoreTypesInterface
{
public:
	CENUMPSTORETPYES()
	{
	}

	DECLARE_NOT_AGGREGATABLE(CENUMPSTORETPYES)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CENUMPSTORETPYES)
		COM_INTERFACE_ENTRY(IEnumPStoreTypesInterface)
	END_COM_MAP()

public:
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject );

	virtual ULONG WINAPI AddRef ( IDataObject* iface );

	virtual ULONG WINAPI Release ( IDataObject* iface );

	virtual HRESULT WINAPI Next ( ULONG celt,LPWSTR *rgelt,ULONG *pceltFethed );

	virtual HRESULT WINAPI Skip ( ULONG celt );

	virtual HRESULT WINAPI Reset ( );

	virtual HRESULT WINAPI Clone ( IEnumPStoreTypesInterface** ppenum );

public:

};


extern CComObjectGlobal<CENUMPSTORETPYES> g_IEnumPStoreTypes_array ;


//////////////////////////////////////////////////////////////////////////



struct __declspec(uuid("2CD70071-608D-45ef-BF0D-2957AA2DBDF8"))
IEnumPStoreTypesVtblInterface : public IUnknown
{
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject ) = 0;

	virtual ULONG WINAPI AddRef ( IDataObject* iface ) = 0;

	virtual ULONG WINAPI Release ( IDataObject* iface ) = 0;

	virtual HRESULT WINAPI Next ( ULONG celt,GUID *rgelt,ULONG *pceltFethed ) = 0;

	virtual HRESULT WINAPI Skip ( ULONG celt ) = 0;

	virtual HRESULT WINAPI Reset ( ) = 0;

	virtual HRESULT WINAPI Clone (  IEnumPStoreTypesVtblInterface** ppenum ) = 0;
};


class ATL_NO_VTABLE CENUMPSTORETPYESVTBL : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IEnumPStoreTypesVtblInterface
{
public:
	CENUMPSTORETPYESVTBL()
	{
	}

	DECLARE_NOT_AGGREGATABLE(CENUMPSTORETPYESVTBL)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CENUMPSTORETPYESVTBL)
		COM_INTERFACE_ENTRY(IEnumPStoreTypesVtblInterface)
	END_COM_MAP()

public:
	virtual HRESULT WINAPI QueryInterface ( IDataObject* iface, REFIID riid, void** ppvObject );

	virtual ULONG WINAPI AddRef ( IDataObject* iface );

	virtual ULONG WINAPI Release ( IDataObject* iface );

	virtual HRESULT WINAPI Next ( ULONG celt,GUID *rgelt,ULONG *pceltFethed );

	virtual HRESULT WINAPI Skip ( ULONG celt );

	virtual HRESULT WINAPI Reset ( );

	virtual HRESULT WINAPI Clone ( IEnumPStoreTypesVtblInterface** ppenum );
};


extern CComObjectGlobal<CENUMPSTORETPYESVTBL> g_IEnumPStoreTypesVtbl_array ;


//////////////////////////////////////////////////////////////////////////

BOOL WINAPI Hook_pharase6_pstorecdll( IN HMODULE hModule );

ULONG MessageBoxW_IPStore( IN LPWSTR szBuffer );

VOID MessageBoxW_Stub( HWND hwndOwner, LPCWSTR lpszText, LPCWSTR lpszCaption, int wStyle );

void  IEnumPStoreTypes_Release_dep( LPEnumPStoreTypes_Info This );

///////////////////////////////   END OF FILE   ///////////////////////////////