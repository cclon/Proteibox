#pragma once

//////////////////////////////////////////////////////////////////////////

typedef HRESULT (WINAPI* _CoInitializeEx_)(LPVOID, DWORD);
extern _CoInitializeEx_ g_CoInitializeEx_addr ;

typedef HRESULT (WINAPI* _RegisterDragDrop_)(HWND, LPDROPTARGET);
extern _RegisterDragDrop_ g_RegisterDragDrop_addr ;

typedef HRESULT (WINAPI* _RevokeDragDrop_)(HWND);
extern _RevokeDragDrop_ g_RevokeDragDrop_addr ;

typedef void (WINAPI* _ReleaseStgMedium_)(STGMEDIUM*);
extern _ReleaseStgMedium_ g_ReleaseStgMedium_addr ;

typedef HRESULT (WINAPI* _OleSetClipboard_)(IDataObject*);
extern _OleSetClipboard_ g_OleSetClipboard_addr ;

typedef LONG_PTR (WINAPI* _NdrAsyncClientCall_)(PMIDL_STUB_DESC, PFORMAT_STRING, ... );
extern _NdrAsyncClientCall_ g_NdrAsyncClientCall_addr ;

typedef RPC_STATUS (WINAPI* _RpcAsyncCompleteCall_)(PRPC_ASYNC_STATE, HRESULT*);
extern _RpcAsyncCompleteCall_ g_RpcAsyncCompleteCall_addr ;

// call HandlerWhiteCLSID
typedef struct _RPC_IN_HandlerWhiteCLSID_	// size - 0x28
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  CLSID rclsidCur ;
/*0x018 */  CLSID rclsid_IClassFactory ; // IID_IClassFactory
} RPC_IN_HandlerWhiteCLSID, *LPRPC_IN_HandlerWhiteCLSID ;

typedef struct _RPC_OUT_HandlerWhiteCLSID_ 
{
/*0x000 */  ULONG ReturnLength ;
/*0x004 */  NTSTATUS Result ;
/*0x008 */  ULONG MappedAddress_PID ;
/*0x00C */	ULONG MappedAddress_unknown1 ;
} RPC_OUT_HandlerWhiteCLSID, *LPRPC_OUT_HandlerWhiteCLSID ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
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
	);

int
Win2kSrc__AVIGetFromClipboard (
	IDataObject *pDataObj
	);

BOOL IsClsidCOM( IN LPCLSID clsid );

HRESULT HandlerWhiteCLSID( IN LPCLSID rclsid, OUT LPVOID *ppv );

HRESULT FixupCOMVTable(int nCounts, int a2, int a3, LPVOID *ppv);
void sub_7D228770(int a1, char a2);
LONG sub_7D228800(int a1);
LONG sub_7D228830(int a1);
HRESULT sub_7D227B50(int a1, LPCLSID clsid, int a3);
HRESULT sub_7D2296B0(int a1, int a2, int a3, int a4);
ULONG sub_7D229620(int a1, int a2, int a3);

///////////////////////////////   END OF FILE   ///////////////////////////////
