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


typedef struct _RPC_OUT_COMMONINFO_ // size - 0x20
{
/*0x000 */  ULONG ReturnLength ;
/*0x004 */  NTSTATUS status ;
/*0x008 */  HANDLE RedirectedFileHandle ;
/*0x00C */	ULONG Reserved1 ;
/*0x010 */	int err ;
/*0x014 */	IO_STATUS_BLOCK IoStatusBlock;
/*0x01C */  ULONG Reserved2 ; 

} RPC_OUT_COMMONINFO, *LPRPC_OUT_COMMONINFO ;


typedef struct _RPCNTREADFILE_LITTLE_INFO_ 
{
/*0x000*/ struct _RPCNTREADFILE_LITTLE_INFO_ *Flink ;
/*0x000*/ struct _RPCNTREADFILE_LITTLE_INFO_ *Blink ;
/*0x008*/ int Flag ;
/*0x00C*/ HANDLE hFileOrignal ;
/*0x010*/ HANDLE hFileRedirected ;
/*0x014*/ HANDLE hEvent ;

} RPCNTREADFILE_LITTLE_INFO, *LPRPCNTREADFILE_LITTLE_INFO ;


typedef struct _RPCFILE_INFO_ // size - 0x24
{
/*0x000*/ CRITICAL_SECTION cs ;
/*0x018*/ LIST_ENTRY_EX ListEntryEx ; // struct _RPCNTREADFILE_LITTLE_INFO_ 

} RPCFILE_INFO, *LPRPCFILE_INFO ;


// call RpcNtReadFile()
typedef struct _RPC_IN_NtReadFile_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;
/*0x00C*/	ULONG BufferLength ;

} RPC_IN_NtReadFile, *LPRPC_IN_NtReadFile ;


typedef struct _RPC_OUT_NtReadFile_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ PVOID IoStatusBlock_Pointer ;
/*0x00C*/ ULONG Reserved1 ;
/*0x010*/ ULONG IoStatusBlock_Information ;
/*0x014*/ ULONG Reserved2 ;
/*0x018*/ ULONG BufferLength ;
/*0x01C*/ WCHAR Buffer[1] ;

} RPC_OUT_NtReadFile, *LPRPC_OUT_NtReadFile ;


// call RpcPipeCommunication();
typedef struct _RPC_IN_Imitate_Pipe_Communication_  // size - 0xEC
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG CreateOptions ;
/*0x00C */  WCHAR szName[0x40] ;
/*0x08C */	WCHAR PipeName[0x30];

} RPC_IN_Imitate_Pipe_Communication, *LPRPC_IN_Imitate_Pipe_Communication ;


// call RpcCloseLittleHandle();
typedef struct _RPC_IN_HANDLE_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;

} RPC_IN_HANDLE, *LPRPC_IN_HANDLE ;


// call RpcNtSetInformationFile();
typedef struct _RPC_IN_FilePipeInformation_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ HANDLE hFile ;
/*0x00C*/ ULONG FileInfoLength ;
/*0x010*/ PVOID FileInfo[1] ;

} RPC_IN_FilePipeInformation, *LPRPC_IN_FilePipeInformation ;


// call RpcNtWriteFile()
typedef struct _RPC_IN_NtWriteFile_ // size - 0x14
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;
/*0x00C*/	ULONG Length ;
/*0x010*/	WCHAR Buffer[1] ;

} RPC_IN_NtWriteFile, *LPRPC_IN_NtWriteFile ;


typedef struct _RPC_OUT_NtWriteFile_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ PVOID IoStatusBlock_Pointer ;
/*0x00C*/ ULONG Reserved1 ;
/*0x010*/ ULONG IoStatusBlock_Information ;
/*0x014*/ ULONG Reserved2 ;

} RPC_OUT_NtWriteFile, *LPRPC_OUT_NtWriteFile ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcFile : public CRPCHandler
{
public:
	CRpcFile(void);
	~CRpcFile(void);

public:
	VOID HandlerFile( PVOID pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
		ProcFile(
		IN PVOID _pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:
	PVOID RpcPipeCommunication( PVOID _pNode, PVOID _pRpcInBuffer, int Flag );
	PVOID RpcCloseLittleHandle( PVOID _pNode, PVOID _pRpcInBuffer, int Flag );
	PVOID RpcNtSetInformationFile( PVOID _pNode, PVOID _pRpcInBuffer, int Flag );
	PVOID RpcNtReadFile( PVOID _pNode, PVOID _pRpcInBuffer, int Flag );
	PVOID RpcNtWriteFile( PVOID _pNode, PVOID _pRpcInBuffer, int Flag );
	VOID  RpcClearUpFile( PVOID _pNode, int Flag );

private:
	HANDLE 
	GetRedirectedFile( 
		IN LPRPCFILE_INFO pNode,
		IN HANDLE hFile,
		IN int Flag,
		OUT HANDLE *phEvent,
		IN BOOL bRemoveNode
		);

	VOID RtlImpersonateSelf();
	
};


extern CRpcFile g_CRpcFile ;

///////////////////////////////   END OF FILE   ///////////////////////////////
