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


// fake_CryptProtectData
typedef struct _RPC_IN_CryptProtectData_ // size >= 0x1C
{
/*0x000*/ ULONG TotalLength ;
/*0x004*/ ULONG Flag;
/*0x008*/ DWORD dwFlags;
/*0x00C*/ DWORD pDataIn_cbData;
/*0x010*/ DWORD pOptionalEntropy_cbData;
/*0x014*/ DWORD DataDescrLength;
/*0x018*/ WCHAR Data[1];

} RPC_IN_CryptProtectData, *LPRPC_IN_CryptProtectData ;


typedef struct _RPC_OUT_CryptProtectData_  // size >= 0x14
{
/*0x000*/ ULONG Reserved1 ;
/*0x004*/ ULONG err;
/*0x008*/ ULONG Reserved2 ;
/*0x00C*/ DWORD pDataOut_cbData;
/*0x010*/ BYTE pDataOut_pbData[1];

} RPC_OUT_CryptProtectData, *LPRPC_OUT_CryptProtectData ;


// fake_CryptUnprotectData
typedef struct _RPC_IN_CryptUnprotectData_ // size >= 0x18
{
/*0x000*/ ULONG TotalLength ;
/*0x004*/ ULONG Flag;
/*0x008*/ DWORD dwFlags;
/*0x00C*/ DWORD pDataIn_cbData;
/*0x010*/ DWORD pOptionalEntropy_cbData;
/*0x014*/ WCHAR Data[1];

} RPC_IN_CryptUnprotectData, *LPRPC_IN_CryptUnprotectData ;


typedef struct _RPC_OUT_CryptUnprotectData_  // size >= 0x14
{
/*0x000*/ ULONG Reserved ;
/*0x004*/ ULONG err;
/*0x008*/ ULONG pDataOut_cbData ;
/*0x00C*/ DWORD DataDescrLength;
/*0x010*/ PBYTE pDataOut_pbData;

} RPC_OUT_CryptUnprotectData, *LPRPC_OUT_CryptUnprotectData ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcCrypt : public CRPCHandler
{
public:

	CRpcCrypt(void);
	~CRpcCrypt(void);

	VOID HandlerCrypt( PVOID pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
	ProcCrypt(
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:
	LPPOSTPROCRPCINFO RpcCryptUnprotectData( PVOID _pRpcInBuffer);
	LPPOSTPROCRPCINFO RpcCryptProtectData( PVOID _pRpcInBuffer);

private:

};


extern CRpcCrypt g_CRpcCrypt ;


///////////////////////////////   END OF FILE   ///////////////////////////////
