#pragma once

#include "RPCHandler.h"
#include "CLoadDriver_2.h"

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcProcess : public CRPCHandler
{
public:

	CRpcProcess(void);
	~CRpcProcess(void);

	VOID HandlerProcess( PVOID pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
		ProcProcess(
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

	static DWORD WINAPI UnloadDrvThread(LPVOID lpParameter);

protected:
	VOID RpcLoadDrv();
	VOID RpcUnLoadDrv();
	LPPOSTPROCRPCINFO RpcKillProcess( IN LPRPC_PROCKILLPROCESS_INFO pRpcInBuffer, IN int dwProcessId );
	LPPOSTPROCRPCINFO RpcTerminateBox( IN LPRPC_PROCTERMINATEBOX_INFO pRpcInBuffer, IN int dwProcessId );
	BOOL KillProcessByID( int dwProcessId );
	void UnloadDrvThreadProc();

private:
	PVOID m_Context_LoadDrv ;
	CDriver* m_drv;
};


extern CRpcProcess g_CRpcProcess ;


///////////////////////////////   END OF FILE   ///////////////////////////////
