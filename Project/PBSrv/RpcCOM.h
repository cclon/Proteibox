#pragma once

#include "RPCHandler.h"


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



// call RpcHandlerWhiteCLSID
typedef struct _RPC_IN_HandlerWhiteCLSID_	// size - 0x28
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  CLSID rclsidCur ;
/*0x018 */  CLSID rclsid_IClassFactory ; // IID_IClassFactory
} RPC_IN_HandlerWhiteCLSID, *LPRPC_IN_HandlerWhiteCLSID ;


// HandlerCOM()
typedef struct _COMCONTEXT_ 
{
/*0x000*/ CRITICAL_SECTION cs ; 
/*0x018*/ LIST_ENTRY_EX ListEntryEx; // struct _PROCCOM_DATA_ 
/*0x024*/ HANDLE hEvent; 
} COMCONTEXT, *LPCOMCONTEXT ;

typedef struct _MappedAddressInfo_ 
{
/*0x000*/ ULONG Flag ;
/*0x004*/ ULONG PID; // Msg+0x8偏移处的内容
/*0x008*/ ULONG CoMarshalInterface_Flag;
/*0x00C*/ DWORD CoMarshalInterface_dwDestContext;
/*0x010*/ DWORD CoMarshalInterface_mshlFlags;
/*0x014*/ ULONG Length;
/*0x018*/ CLSID rclsid1 ;
/*0x028*/ CLSID rclsid2 ;
} MappedAddressInfo, *LPMappedAddressInfo ;

typedef struct _PROCCOM_DATA_ 
{
/*0x000*/ LIST_ENTRY ListEntry ;
/*0x008*/ WCHAR sid[0x60];
/*0x0C8*/ WCHAR szBoxName[0x30];
/*0x128*/ ULONG SessionId;
/*0x12C*/ CRITICAL_SECTION cs ; 
/*0x144*/ int Flag;
/*0x148*/ HANDLE hEvent_ComProxy1;	// "Proteinbox_ComProxy_%s_%s_%d_Event1"
/*0x14C*/ HANDLE hEvent_ComProxy2;	// "Proteinbox_ComProxy_%s_%s_%d_Event2" 
/*0x150*/ HANDLE hFileMapping;		// "Proteinbox_ComProxy_%s_%s_%d_Map" 
/*0x154*/ LPMappedAddressInfo MappedAddress; 
/*0x158*/ HANDLE hProcess;
} PROCCOM_DATA, *LPPROCCOM_DATA ;


typedef struct tagOBJREF // size - 0x18
{
	unsigned long signature;
	unsigned long flags;
	GUID iid;
/*
	[switch_is(flags)] union {
		[case(OBJREF_STANDARD)] struct OR_STANDARD {
			STDOBJREF std;
			DUALSTRINGARRAY saResAddr;
		} u_standard;
		[case(OBJREF_HANDLER)] struct OR_HANDLER {
			STDOBJREF std;
			CLSID clsid;
			DUALSTRINGARRAY saResAddr;
		} u_handler;
		[case(OBJREF_CUSTOM)] struct OR_CUSTOM {
			CLSID clsid;
			unsigned long cbExtension;
			ULONG size;
			[size_is(size), ref] byte *pData;
		} u_custom;
	} u_objref;
*/
} OBJREF;


// fake_CoUnmarshalInterface
typedef struct _RPC_IN_CoUnmarshalInterface_	// size >= 0x5C
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ IID riid ;
/*0x018*/ ULONG objrefDummy_TotalLength ;// sizeof(OBJREF) + data[]的长度
/*0x01C*/ OBJREF objref;
/*0x034*/ OBJREF objrefDummy;
/*0x05C*/ BYTE Data[];
} RPC_IN_CoUnmarshalInterface, *LPRPC_IN_CoUnmarshalInterface ;


// RpcCoMmarshalInterface
typedef struct _RPC_IN_CoMarshalInterface_	// size - 0x28
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ ULONG Flag;
/*0x00C*/ IID riid ;
/*0x01C*/ DWORD dwDestContext;
/*0x020*/ DWORD mshlFlags;
} RPC_IN_CoMarshalInterface, *LPRPC_IN_CoMarshalInterface ;

typedef struct _RPC_OUT_CoMarshalInterface_	// size - MappedAddress_Length + 0x14
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ ULONG MappedAddress_PID;
/*0x00C*/ ULONG MappedAddress_Length ;
/*0x010*/ IID riid ;
} RPC_OUT_CoMarshalInterface, *LPRPC_OUT_CoMarshalInterface ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcCOM : public CRPCHandler
{
public:

	CRpcCOM(void);
	~CRpcCOM(void);

	PVOID HandlerCOM( PVOID _pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
	ProcCOM(
		IN PVOID _pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

	BOOL RemoveNode_COM(LPCOMCONTEXT pInfo, LPPROCCOM_DATA pNode);

protected:
	PVOID RpcHandlerWhiteCLSID( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag );
	PVOID RpcCoUnmarshalInterface( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag );
	PVOID RpcCoMarshalInterface( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag );

	int WaitForCOMProc(LPCOMCONTEXT pInfo, LPPROCCOM_DATA pNode, int Flag, int *PID);
	LPPROCCOM_DATA GetCOMData(LPCOMCONTEXT pInfo, int dwProcessId, int Flag);

private:
	int m_PID_COM;
};


extern CRpcCOM g_CRpcCOM ;
extern PVOID g_Buffer_COM;

///////////////////////////////   END OF FILE   ///////////////////////////////
