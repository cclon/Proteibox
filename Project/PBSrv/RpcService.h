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

// RpcStartService
typedef struct _RPC_IN_StartService_	// size - 0x10 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG  NameLength ;
/*0x00C */  LPWSTR szName ;

} RPC_IN_StartService, *LPRPC_IN_StartService ;


// RpcEnumServiceStatus
typedef struct _RPC_IN_EnumServiceStatus_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  USHORT Flag1 ;
/*0x00A*/   USHORT dwInfoLevel ; 
/*0x00C*/	ULONG NameLength ; 
/*0x010*/   WCHAR pServiceName[1] ; 

} RPC_IN_EnumServiceStatus, *LPRPC_IN_EnumServiceStatus ;


typedef struct _RPCENUMSERVICESTATUS_INFO_ // size - 0x54
{
/*0x000*/ ULONG Reserved ;
/*0x004*/ ULONG err;
/*0x008*/ SERVICE_STATUS_PROCESS ServiceStatus; 
/*0x02C*/ DWORD cbBytesNeeded;
/*0x030*/ QUERY_SERVICE_CONFIGW ServiceConfig;

} RPCENUMSERVICESTATUS_INFO, *LPRPCENUMSERVICESTATUS_INFO ;


// RpcGetSandboxedServices()
typedef struct _RPC_IN_GetSandboxedServices_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD dwServiceType;
/*0x00C*/ DWORD dwServiceState;

} RPC_IN_GetSandboxedServices, *LPRPC_IN_GetSandboxedServices ;


typedef struct _RPCGETSANDBOXEDSERVICES_INFO_
{
/*0x000*/ ULONG Reserved ;
/*0x004*/ ULONG err;
/*0x008*/ WCHAR ServiceNamesBuffer[1]; 

} RPCGETSANDBOXEDSERVICES_INFO, *LPRPCGETSANDBOXEDSERVICES_INFO ;


// RpcStartBoxedService()
typedef struct _RPC_IN_StartBoxedService_  // size - 0x10
{
/*0x000*/  RPC_IN_HEADER RpcHeader ;
/*0x008*/  ULONG NameLength ;
/*0x00C*/  LPWSTR szName ;

} RPC_IN_StartBoxedService, *LPRPC_IN_StartBoxedService ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcService : public CRPCHandler
{
public:

	CRpcService(void);
	~CRpcService(void);

	VOID HandlerService( PVOID pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
		ProcService(
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:

	LPPOSTPROCRPCINFO RpcStartService( IN LPRPC_IN_StartService pRpcBuffer );
	LPPOSTPROCRPCINFO RpcEnumServiceStatus( IN LPRPC_IN_EnumServiceStatus pRpcInBuffer );
	LPPOSTPROCRPCINFO RpcGetSandboxedServices( IN LPRPC_IN_GetSandboxedServices pRpcInBuffer );
	LPPOSTPROCRPCINFO RpcStartBoxedService( IN LPRPC_IN_StartBoxedService pRpcInBuffer, IN int PID);


private:

	VOID CopyProcessToken( IN HANDLE TokenHandle, IN DWORD dwProcessId );
	BOOL IsSepcailAccount( IN DWORD PID );

};


extern CRpcService g_CRpcService ;


///////////////////////////////   END OF FILE   ///////////////////////////////
