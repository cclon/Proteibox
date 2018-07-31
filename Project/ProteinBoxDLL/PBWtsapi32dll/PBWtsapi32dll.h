#pragma once

//////////////////////////////////////////////////////////////////////////

typedef struct _WTS_SESSION_INFOW
{
	DWORD SessionId;
	LPWSTR pWinStationName;
	DWORD State;
} WTS_SESSION_INFOW, *PWTS_SESSION_INFOW;


typedef struct _WTSEnumerateSessionsW_INFO_ 
{
/*0x000 */	ULONG Tag ;
/*0x004 */  ULONG Reserved2 ;
/*0x008 */  WTS_SESSION_INFOW ppSessionInfo ;  
/*0x014 */  WCHAR pWinStationName[1] ; // 文件全路径

} WTSEnumerateSessionsW_INFO, *PWTSEnumerateSessionsW_INFO ;


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

BOOL WINAPI Hook_pharase14_wtsapi32dll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase15_winstadll( IN HMODULE hModule );

///////////////////////////////   END OF FILE   ///////////////////////////////
