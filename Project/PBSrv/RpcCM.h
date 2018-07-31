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

typedef struct _HandlerCM_INFO_ 
{
/*0x000*/ ULONG SetupDiGetDevicePropertyW_addr ;
/*0x004*/ ULONG SetupDiGetDeviceInterfacePropertyW_addr;
/*0x008*/ ULONG CM_Get_Class_Property_ExW_addr;
/*0x00C*/ ULONG WinStationQueryInformationW_addr;

} HandlerCM_INFO, *LPHandlerCM_INFO ;


typedef GUID  DEVPROPGUID, *PDEVPROPGUID ;
typedef ULONG DEVPROPID, *PDEVPROPID ;

struct DEVPROPKEY
{
	DEVPROPGUID fmtid;
	DEVPROPID   pid;
};

typedef struct _RPC_IN_SetupDiGetDevicePropertyW_	// size - 0x230
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  WCHAR DevInst[ 0x100 ] ;
/*0x208 */  GUID ClassGuid ;
/*0x218 */  DEVPROPKEY PropertyKey ;
/*0x22C */  DWORD Flags ;

} RPC_IN_SetupDiGetDevicePropertyW, *LPRPC_IN_SetupDiGetDevicePropertyW ;


typedef struct _RPC_OUT_SetupDiGetDevicePropertyW_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG PropertyType ;
/*0x00C */  ULONG BufferSize ;
/*0x010 */  WCHAR PropertyBuffer[1] ;

} RPC_OUT_SetupDiGetDevicePropertyW, *LPRPC_OUT_SetupDiGetDevicePropertyW ;


// fake_CM_Get_Device_Interface_ListA
typedef struct _RPC_IN_CM_Get_Device_Interface_List_	// size - 0x20
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  GUID ClassGuid ;
/*0x018 */  ULONG ulFlags ;
/*0x01C */  BOOL bIsUnicode ;

} RPC_IN_CM_Get_Device_Interface_List, *LPRPC_IN_CM_Get_Device_Interface_List ;


typedef struct _RPC_OUT_CM_Get_Device_Interface_List_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Length ;
/*0x00C */  BYTE DeviceInterface[1] ;

} RPC_OUT_CM_Get_Device_Interface_List, *LPRPC_OUT_CM_Get_Device_Interface_List ;


// fake_CM_Get_Device_Interface_Alias_ExW
typedef struct _RPC_IN_CM_Get_Device_Interface_Alias_ExW_	// size - 0x24
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  GUID AliasInterfaceGuid ;
/*0x018 */  ULONG ulFlags ;
/*0x01C */  ULONG DeviceInterfaceLength ;
/*0x020 */  WCHAR Buffer[1] ;

} RPC_IN_CM_Get_Device_Interface_Alias_ExW, *LPRPC_IN_CM_Get_Device_Interface_Alias_ExW ;


typedef struct _RPC_OUT_CM_Get_Device_Interface_Alias_ExW_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG ReturnLength ;
/*0x00C */  WCHAR ReturnBuffer[1] ;

} RPC_OUT_CM_Get_Device_Interface_Alias_ExW, *LPRPC_OUT_CM_Get_Device_Interface_Alias_ExW ;


// fake_CM_Get_Device_Interface_Property_ExW
typedef struct _CM_Get_Device_Interface_Property_ExW_ArgInfo_   // size - 0x14
{
/*0x000*/ GUID ClassGuid ;
/*0x010*/ ULONG Reserved ;

} CM_Get_Device_Interface_Property_ExW_ArgInfo, *LPCM_Get_Device_Interface_Property_ExW_ArgInfo ;


typedef struct _RPC_IN_CM_Get_Device_Interface_Property_ExW_	// size - 0x21C
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  WCHAR DevInst[0x100] ;
/*0x208 */  DEVPROPKEY PropertyKey ;

} RPC_IN_CM_Get_Device_Interface_Property_ExW, *LPRPC_IN_CM_Get_Device_Interface_Property_ExW ;


typedef struct _RPC_OUT_CM_Get_Device_Interface_Property_ExW_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG ReturnLength ;
/*0x010 */  WCHAR ReturnBuffer[1] ;

} RPC_OUT_CM_Get_Device_Interface_Property_ExW, *LPRPC_OUT_CM_Get_Device_Interface_Property_ExW ;


// fake_CM_Get_Class_Property_ExW_Win7
typedef struct _RPC_IN_CM_Get_Class_Property_ExW_Win7_	// size - 0x30
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  GUID ClassGuid1 ;
/*0x018 */  DEVPROPKEY PropertyKey ;
/*0x02C */  ULONG Reserved ;

} RPC_IN_CM_Get_Class_Property_ExW_Win7, *LPRPC_IN_CM_Get_Class_Property_ExW_Win7 ;


typedef struct _RPC_OUT_CM_Get_Class_Property_ExW_Win7_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG ReturnLength ;
/*0x010 */  WCHAR ReturnBuffer[1] ;

} RPC_OUT_CM_Get_Class_Property_ExW_Win7, *LPRPC_OUT_CM_Get_Class_Property_ExW_Win7 ;


// fake_CM_Get_DevNode_Status
typedef struct _RPC_IN_CM_Get_DevNode_Status_	// size - 0x208
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  WCHAR DevInst[0x100] ;

} RPC_IN_CM_Get_DevNode_Status, *LPRPC_IN_CM_Get_DevNode_Status ;


typedef struct _RPC_OUT_CM_Get_DevNode_Status_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG ulStatus ;
/*0x00C */  ULONG ulProblemNumber;

} RPC_OUT_CM_Get_DevNode_Status, *LPRPC_OUT_CM_Get_DevNode_Status ;


// fake_WinStaQueryInformationW
typedef struct _RPC_IN_WINSTAQUERYINFORMATIONW_  // size - 0x14
{
/*0x000 */  RPC_IN_HEADER RpcHeader ; 
/*0x008*/	ULONG ulLogonId ;
/*0x00C*/	int WinStationInformationClass ;
/*0x010*/	ULONG ulWinStationInformationLength ;

} RPC_IN_WINSTAQUERYINFORMATIONW, *LPRPC_IN_WINSTAQUERYINFORMATIONW ;


typedef struct _RPC_OUT_WINSTAQUERYINFORMATIONW_	// size - 0x10
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG ulReturnLength ;
/*0x00C */  WCHAR WinStationInformation[1] ;

} RPC_OUT_WINSTAQUERYINFORMATIONW, *LPRPC_OUT_WINSTAQUERYINFORMATIONW ;




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcCM : public CRPCHandler
{
public:

	CRpcCM(void);
	~CRpcCM(void);

	VOID HandlerCM( PVOID _pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
	ProcCM(
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:
	PVOID RpcSetupDiGetDevicePropertyW( PVOID _pInfo, PVOID _pRpcInBuffer );
	PVOID RpcCM_Get_Device_Interface_List_filter( PVOID _pRpcInBuffer );
	PVOID RpcCM_Get_Device_Interface_Alias_ExW( PVOID _pRpcInBuffer );
	PVOID RpcCM_Get_Device_Interface_Property_ExW( PVOID _pInfo, PVOID _pRpcInBuffer );
	PVOID RpcCM_Get_Class_Property_ExW_Win7( PVOID _pInfo, PVOID _pRpcInBuffer );
	PVOID RpcCM_Get_DevNode_Status( PVOID _pInfo, PVOID _pRpcInBuffer );
	PVOID RpcWinStaQueryInformationW( PVOID _pInfo, PVOID _pRpcInBuffer );

private:

};


extern CRpcCM g_CRpcCM ;


///////////////////////////////   END OF FILE   ///////////////////////////////
