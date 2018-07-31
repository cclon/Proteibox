#pragma once

//////////////////////////////////////////////////////////////////////////

extern ULONG_PTR g_CM_Get_Device_Interface_Property_ExW_addr ;
extern int g_pMappedAddr_DeviceSetupClasses_array ;
extern int g_pMappedAddr_DeviceSetupClasses_ClassIndex ;
extern int g_pMappedAddr_DeviceIdList_array ;
extern int g_pMappedAddr_DeviceIdList_ClassIndex ;

typedef struct _SP_DEVINFO_DATA 
{
	DWORD     cbSize;
	GUID      ClassGuid;
	DWORD     DevInst;
	ULONG_PTR Reserved;
} SP_DEVINFO_DATA, *PSP_DEVINFO_DATA;


typedef GUID  DEVPROPGUID, *PDEVPROPGUID ;
typedef ULONG DEVPROPID, *PDEVPROPID ;

struct DEVPROPKEY
{
	DEVPROPGUID fmtid;
	DEVPROPID   pid;
};

typedef ULONG DEVPROPTYPE, *PDEVPROPTYPE;


typedef struct _RPC_IN_SetupDiGetDevicePropertyW_	// size - 0x34
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
typedef struct _RPC_IN_CM_Get_DevNode_Status_	// size - 0xXX
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



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

ULONG
CM_Get_Class_Name_filter (
	ULONG ulFlags,
	LPGUID ClassGuid,
	PVOID Buffer, 
	PULONG pulLength, 
	BOOL bIsUnicode
	);

DWORD 
CM_Get_Device_ID_List_filter (
	BOOL bIsUnicode,
	ULONG ulFlags, 
	PCSTR pszFilter, 
	PVOID Buffer, 
	ULONG BufferLen
	);

ULONG
CM_Get_Device_Interface_List_filter (
	LPGUID ClassGuid,
	PULONG pulOutSize, 
	BOOL bIsUnicode,
	PVOID DeviceInterface, 
	ULONG ulSize,
	int ulFlags
	);

int 
CM_Get_DevNode_Registry_Property_filter (
	ULONG ulProperty,
	DWORD dnDevInst,
	ULONG ulFlags,
	PULONG pulRegDataType, 
	PVOID Buffer, 
	PULONG pulLength, 
	BOOL bIsUnicode
	);

VOID Get_CM_Get_Device_Interface_Property_ExW_address( HMODULE hModule );

///////////////////////////////   END OF FILE   ///////////////////////////////
