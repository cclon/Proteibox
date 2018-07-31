#pragma once

//////////////////////////////////////////////////////////////////////////

#define WM_KEYFIRST			0x0100
#define WM_KEYLAST			0x0109
#define WM_MOUSEFIRST		0x0200
#define WM_MOUSELAST		0x0209
#define WM_TABLET_FIRST		0x02c0
#define WM_TABLET_LAST		0x02df

// used for NtUserQueryWindow
#define QUERY_WINDOW_UNIQUE_PROCESS_ID	0x00
#define QUERY_WINDOW_UNIQUE_THREAD_ID	0x01
#define QUERY_WINDOW_ISHUNG	0x04

// used for NtUserSetWinEventHook
#define WINEVENT_OUTOFCONTEXT   0x0000  // Events are ASYNC
#define WINEVENT_SKIPOWNTHREAD  0x0001  // Don't call back for events on installer's thread
#define WINEVENT_SKIPOWNPROCESS 0x0002  // Don't call back for events on installer's process
#define WINEVENT_INCONTEXT      0x0004  // Events are SYNC, this causes your dll to be injected into every process
#define WINEVENT_32BITCALLER    0x8000  // ;Internal
#define WINEVENT_VALID          0x8007  // ;Internal

// used for NtUserSetWindowsHookEx
#define WH_JOURNALPLAYBACK	1
#define WH_SYSMSGFILTER		6
#define WH_KEYBOARD_LL		13
#define WH_MOUSE_LL			14

// used for NtUserSystemParametersInfo
#define SPI_ICONHORIZONTALSPACING 0x000D
#define SPI_ICONVERTICALSPACING 0x0018


//
// 定义一个特殊的标志位,用来避免fake_NtUserSendInput / fake_NtUserBlockInput的重入问题
// 但可能被坏蛋利用,因为只要有此标志,就会调用原始函数; 利用者只要在R3故意加上此标志就可突破我们
// 故该Flag在每次驱动加载时应为随机值,由R3产生传入R0保存
//

#define _SimulationButton_Flag_ 0xF5000000


// 每个fake处理函数的前置处理宏定义

#define PreHelper( Tag )	\
	Prolog();	\
	PreHelperEx( Tag );


#define PreHelperEx( Tag )	\
	FuncInfo = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag );			\
	if ( NULL == FuncInfo ) { goto _Pass_ ; }	\
	AddRef( FuncInfo->FakeFuncInfo.RefCounts );						\
	ret = Prepare_for_NtUser ( (PVOID)&pNode );						\
	if ( _PREPARE_RETURN_TYPE_PASS_  == ret ) { PostHelper( Tag ); goto _Pass_ ; }	\
	if ( _PREPARE_RETURN_TYPE_DENNY_ == ret ) { PostHelper( Tag ); goto _Denny_ ; }		


#define PreRpcHelper( Tag )	\
	Prolog();	\
	PreRpcHelperEx( Tag )

#define PreRpcHelperEx( Tag )	\
	FuncInfo = (LPSSDT_SSSDT_FUNC) Get_sdt_Array_spec( Tag );		\
	if ( NULL == FuncInfo ) { goto _Pass_ ; }	\
	AddRef( FuncInfo->FakeFuncInfo.RefCounts );						\
	ret = Prepare_for_RPC( (PVOID)&pNode );							\
	if ( _PREPARE_RETURN_TYPE_PASS_  == ret ) { PostRpcHelper( Tag ); goto _Pass_ ; }	\
	if ( _PREPARE_RETURN_TYPE_DENNY_ == ret ) { PostRpcHelper( Tag ); goto _Denny_ ; }		


// 每个fake处理函数的后置处理宏定义

#define PostHelper( Tag )	\
	FuncInfo = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag );			\
	if ( FuncInfo ) { DecRef( FuncInfo->FakeFuncInfo.RefCounts ); }	


#define PostRpcHelper( Tag )	\
	FuncInfo = (LPSSDT_SSSDT_FUNC) Get_sdt_Array_spec( Tag );		\
	if ( FuncInfo ) { DecRef( FuncInfo->FakeFuncInfo.RefCounts ); }	


extern ULONG g_LsaAuthPkg ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


// apfnSimpleCall[] 数组中包含一堆函数指针,通过Index寻址,以下是受关注的序列号

typedef struct _Win32k_apfnSimpleCall_Index_ 
{
	ULONG SFI_XXXENABLEWINDOW ; // EnableWindow

} Win32k_apfnSimpleCall_Index, *LPWin32k_apfnSimpleCall_Index ;

extern Win32k_apfnSimpleCall_Index g_Win32k_apfnSimpleCall_Index_Info ;

//
// RPC相关
//

typedef ULONG CSR_API_NUMBER;
#define CSR_MAKE_API_NUMBER( DllIndex, ApiIndex ) (CSR_API_NUMBER)(((DllIndex) << 16) | (ApiIndex))

#define BASESRV_SERVERDLL_INDEX         1
#define BASESRV_FIRST_API_NUMBER        0

#define CONSRV_SERVERDLL_INDEX          2
#define CONSRV_FIRST_API_NUMBER         512

#define USERSRV_SERVERDLL_INDEX         3
#define USERSRV_FIRST_API_NUMBER        1024

#define GDISRV_SERVERDLL_INDEX          4
#define GDISRV_FIRST_API_NUMBER         1536

#define MMSNDSRV_SERVERDLL_INDEX        5
#define MMSNDSRV_FIRST_API_NUMBER       2048

#define USERK_SERVERDLL_INDEX           6
#define USERK_FIRST_API_NUMBER          2560


typedef enum _USER_API_NUMBER {
	UserpExitWindowsEx = USERSRV_FIRST_API_NUMBER,
	UserpEndTask,
	UserpLogon,
	UserpRegisterServicesProcess,
	UserpActivateDebugger,
	UserpGetThreadConsoleDesktop,
	UserpDeviceEvent,
	UserpRegisterLogonProcess,
	UserpWin32HeapFail,
	UserpWin32HeapStat,
	UserpMaxApiNumber
} USER_API_NUMBER, *PUSER_API_NUMBER;

typedef struct _EXITWINDOWSEXMSG {
	DWORD dwLastError;
	UINT uFlags;
	DWORD dwReserved;
	BOOL fSuccess;
} EXITWINDOWSEXMSG, *PEXITWINDOWSEXMSG;

typedef struct _ENDTASKMSG {
	DWORD dwLastError;
	HWND hwnd;
	BOOL fShutdown;
	BOOL fForce;
	BOOL fSuccess;
} ENDTASKMSG, *PENDTASKMSG;


typedef struct _USER_API_MSG 
{
	PORT_MESSAGE h;
	PVOID CaptureBuffer;
	CSR_API_NUMBER ApiNumber;
	ULONG ReturnValue;
	ULONG Reserved;

	//
	// 下面的结构体我们无需关心 ...
	//
} USER_API_MSG, *PUSER_API_MSG;



enum _INFO_TYPE_ 
{
	_SendInput_Type_ = 1 ,
	_BlockInput_Type_ ,
	_DestroyWindow_Type_ ,
	_SetSysColors_Type_ ,
	_SetWinEventHook_Type_ ,
	_SetWindowsHookEx_Type_ ,
	_SystemParametersInfoW_Type_ ,
	_NtRequestPort_Type_ ,
	_NtRequestWaitReplyPort_Type_ ,
	_NtAlpcSendWaitReceivePort_Type_ ,
};


enum _PREPARE_RETURN_TYPE_
{
	_PREPARE_RETURN_TYPE_PASS_ = 1 ,
	_PREPARE_RETURN_TYPE_DENNY_ ,
	_PREPARE_RETURN_TYPE_GOON_
};


enum _MSG_TYPE_
{
	_MSG_TYPE_SEND_ = 1 ,
	_MSG_TYPE_POST_ 
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID fake_NtUserMessageCall(VOID);

VOID fake_NtUserSendMessageCallback(VOID);

VOID fake_NtUserSendNotifyMessage(VOID);

VOID fake_NtUserPostMessage(VOID);

VOID fake_NtUserPostThreadMessage(VOID);

VOID fake_NtUserCallHwndParamLock(VOID);

VOID fake_NtUserDestroyWindow(VOID);

VOID fake_NtUserSendInput(VOID);

VOID fake_NtUserBlockInput(VOID);

VOID fake_NtUserSetWindowsHookEx(VOID);

VOID fake_NtUserSetWinEventHook(VOID);

VOID fake_NtUserSetSysColors(VOID);

VOID fake_NtUserSystemParametersInfo(VOID);

VOID fake_NtRequestPort(VOID);

VOID fake_NtRequestWaitReplyPort(VOID);

VOID fake_NtAlpcSendWaitReceivePort(VOID);

int Prepare_for_RPC ( OUT PVOID* _pNode );
int Prepare_for_NtUser ( OUT PVOID* _pNode );

BOOL Prepare_for_NtUserEx ();

VOID 
ShowWarningInfo (
	IN int InfoType,
	IN HANDLE VictimHandle
	);

BOOL 
MsgFilter (
	IN PVOID _pNode,
	IN int Flag,
	IN HWND hWnd,
	IN UINT msg
	);

NTSTATUS
RpcFilter (
	IN HANDLE PortHandle,
	IN PPORT_MESSAGE RequestMessage
	);

NTSTATUS
RpcFilterEx (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	);

NTSTATUS
RpcFilterExp (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	);

BOOL
IsWhiteProcess (
	IN PVOID _pNode,
	IN HANDLE PID
	);


///////////////////////////////   END OF FILE   ///////////////////////////////