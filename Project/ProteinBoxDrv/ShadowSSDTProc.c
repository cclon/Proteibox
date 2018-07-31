/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/08/05 [5:8:2010 - 11:30]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ShadowSSDTProc.c
* 
* Description:
*      
*   fake Shadow SSDT 处理函数,负责窗口/消息隔离等操作                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "Common.h"
#include "HookEngine.h"
#include "ProcessData.h"
#include "GrayList.h"
#include "SdtData.h"
#include "ObjectProc.h"
#include "ShadowSSDTProc.h"

//////////////////////////////////////////////////////////////////////////

_NtUserSendInput_			g_NtUserSendInput_addr				= NULL ;
_NtUserQueryWindow_			g_NtUserQueryWindow_addr			= NULL ;
_NtUserGetClassName_		g_NtUserGetClassName_addr			= NULL ;
_NtUserGetForegroundWindow_ g_NtUserGetForegroundWindow_addr	= NULL ;


ULONG g_LsaAuthPkg = -1 ;
ULONG g_SendInput_RetValue = 0 ;

Win32k_apfnSimpleCall_Index g_Win32k_apfnSimpleCall_Index_Info = { 0 } ;


// 禁止向沙箱外的白名单进程发送以下消息
static const UINT g_forbidden_Message_Array[] = 
{
	WM_DESTROY ,
	WM_SETREDRAW ,
	WM_CLOSE ,
	WM_QUERYENDSESSION ,
	WM_QUIT ,
	WM_ENDSESSION ,
	WM_NOTIFY ,
	WM_NCDESTROY ,
	WM_COMMAND ,
	WM_SYSCOMMAND ,
	0x319 ,
};


// 可以通过NtUserSystemParametersInfo函数发送以下命令
static const UINT g_SystemParametersInfo_WhiteList_Array [] = 
{
	1 ,		// SPI_GETBEEP
	3 ,		// SPI_GETMOUSE
	5 ,		// SPI_GETBORDER
	0x0A ,	// SPI_GETKEYBOARDSPEED
	0x0D ,	// SPI_ICONHORIZONTALSPACING
	0x0E ,	// SPI_GETSCREENSAVETIMEOUT
	0x10 ,	// SPI_GETSCREENSAVEACTIVE
	0x12 ,	// SPI_GETGRIDGRANULARITY
	0x16 ,	// SPI_GETKEYBOARDDELAY
	0x18 ,	// SPI_ICONVERTICALSPACING
	0x19 ,	// SPI_GETICONTITLEWRAP
	0x1B ,	// SPI_GETMENUDROPALIGNMENT
	0x1F ,	// SPI_GETICONTITLELOGFONT
	0x23 ,	// SPI_GETFASTTASKSWITCH
	0x26 ,	// SPI_GETDRAGFULLWINDOWS
	0x29 ,	// SPI_GETNONCLIENTMETRICS
	0x2B ,	// SPI_GETMINIMIZEDMETRICS
	0x2D ,	// SPI_GETICONMETRICS
	0x30 ,	// SPI_GETWORKAREA
	0x42 ,	// SPI_GETHIGHCONTRAST
	0x44 ,	// SPI_GETKEYBOARDPREF
	0x46 ,	// SPI_GETSCREENREADER
	0x48 ,	// SPI_GETANIMATION
	0x4A ,	// SPI_GETFONTSMOOTHING
	0x4F ,	// SPI_GETLOWPOWERTIMEOUT
	0x50 ,	// SPI_GETPOWEROFFTIMEOUT
	0x53 ,	// SPI_GETLOWPOWERACTIVE
	0x54 ,  // SPI_GETPOWEROFFACTIVE
	0x59 ,	// SPI_GETDEFAULTINPUTLANG
	0x5C ,	// SPI_GETWINDOWSEXTENSION
	0x5E ,	// SPI_GETMOUSETRAILS
	0x32 ,	// SPI_GETFILTERKEYS
	0x34 ,	// SPI_GETTOGGLEKEYS
	0x36 ,	// SPI_GETMOUSEKEYS
	0x38 ,	// SPI_GETSHOWSOUNDS
	0x3A ,	// SPI_GETSTICKYKEYS
	0x3C ,	// SPI_GETACCESSTIMEOUT
	0x3E ,	// SPI_GETSERIALKEYS
	0x40 ,	// SPI_GETSOUNDSENTRY
	0x5F ,	// SPI_GETSNAPTODEFBUTTON
	0x62 ,	// SPI_GETMOUSEHOVERWIDTH
	0x64 ,	// SPI_GETMOUSEHOVERHEIGHT
	0x66 ,	// SPI_GETMOUSEHOVERTIME
	0x68 ,	// SPI_GETWHEELSCROLLLINES
	0x6A ,	// SPI_GETMENUSHOWDELAY
	0x6C ,	// SPI_GETWHEELSCROLLCHARS
	0x6E ,	// SPI_GETSHOWIMEUI
	0x70 ,	// SPI_GETMOUSESPEED
	0x72 ,	// SPI_GETSCREENSAVERRUNNING
	0x73 ,	// SPI_GETDESKWALLPAPER
	0x74 ,	// SPI_GETAUDIODESCRIPTION
	0x76 ,	// SPI_GETSCREENSAVESECURE
	0x1000 ,	// SPI_GETACTIVEWINDOWTRACKING
	0x1002 ,	// SPI_GETMENUANIMATION
	0x1004 ,	// SPI_GETCOMBOBOXANIMATION
	0x1006 ,	// SPI_GETLISTBOXSMOOTHSCROLLING
	0x1008 ,	// SPI_GETGRADIENTCAPTIONS
	0x100A ,	// SPI_GETKEYBOARDCUES
	0x100C ,	// SPI_GETACTIVEWNDTRKZORDER
	0x100E ,	// SPI_GETHOTTRACKING
	0x1012 ,	// SPI_GETMENUFADE
	0x1014 ,	// SPI_GETSELECTIONFADE
	0x1016 ,	// SPI_GETTOOLTIPANIMATION
	0x1018 ,	// SPI_GETTOOLTIPFADE
	0x101A ,	// SPI_GETCURSORSHADOW
	0x101C ,	// SPI_GETMOUSESONAR
	0x101E ,	// SPI_GETMOUSECLICKLOCK
	0x1020 ,	// SPI_GETMOUSEVANISH
	0x1022 ,	// SPI_GETFLATMENU
	0x1024 ,	// SPI_GETDROPSHADOW
	0x1026 ,	// SPI_GETBLOCKSENDINPUTRESETS
	0x103E ,	// SPI_GETUIEFFECTS
	0x1040 ,	// SPI_GETDISABLEOVERLAPPEDCONTENT
	0x1042 ,	// SPI_GETCLIENTAREAANIMATION
	0x1048 ,	// SPI_GETCLEARTYPE
	0x104A ,	// SPI_GETSPEECHRECOGNITION
	0x2000 ,	// SPI_GETFOREGROUNDLOCKTIMEOUT
	0x2002 ,	// SPI_GETACTIVEWNDTRKTIMEOUT
	0x2004 ,	// SPI_GETFOREGROUNDFLASHCOUNT
	0x2006 ,	// SPI_GETCARETWIDTH
	0x2008 ,	// SPI_GETMOUSECLICKLOCKTIME
	0x200A ,	// SPI_GETFONTSMOOTHINGTYPE
	0x200C ,	// SPI_GETFONTSMOOTHINGCONTRAST
	0x200E ,	// SPI_GETFOCUSBORDERWIDTH
	0x2010 ,	// SPI_GETFOCUSBORDERHEIGHT
	0x2012 ,	// SPI_GETFONTSMOOTHINGORIENTATION
	0x2014 ,	// 
	0x2016 ,	// 
};

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


/*
VOID
fake_NtUserMessageCall(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN ULONG_PTR xParam,
    IN DWORD xpfnProc,
    IN BOOL bAnsi
	)
*/
VOID _declspec(naked) fake_NtUserMessageCall()
{
	int ret, msg, hwnd ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SendMessageTimeoutW );
	
	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, (HWND)hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 1Ch
	}

_Pass_ :
	FinisOrig();
}



// VOID
// fake_NtUserSendMessageCallback(
//     IN HWND hwnd,
//     IN UINT wMsg,
//     IN WPARAM wParam,
//     IN LPARAM lParam,
//     IN /*SENDASYNCPROC*/PVOID lpResultCallBack,
//     IN ULONG_PTR dwData
// 	)
VOID _declspec(naked) fake_NtUserSendMessageCallback()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SendMessageCallbackW );

	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 18h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSendNotifyMessage(
    IN HWND hwnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam OPTIONAL
	)
*/
VOID _declspec(naked) fake_NtUserSendNotifyMessage()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SendNotifyMessageW_win2k );

	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserPostMessage(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
	)
*/
VOID _declspec(naked) fake_NtUserPostMessage()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_PostMessageW );

	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_POST_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserPostThreadMessage(
    IN DWORD id,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
	)
*/
VOID _declspec(naked) fake_NtUserPostThreadMessage()
{
	UINT ret, msg ;
	HANDLE TID, PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_PostThreadMessageW );

	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		TID, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( NULL == TID ) { TID = PsGetCurrentThreadId(); }

	if ( pNode->XWnd.bFlag_SendPostMessage_allowed ) { goto _Pass_ ; } // 允许NtUser**发送消息,放行

	if ( ! (msg & 0xFFFF0000) && (msg > 0xC000) ) { goto _Pass_ ; }

	if ( IsApprovedTID( (PVOID)pNode, TID, &PID ) ) { goto _Pass_ ; } // 操作的是沙箱中的进程,放行

	if ( IsWhiteProcess( (PVOID)pNode, PID ) ) { goto _Pass_ ; } // 操作的是沙箱外的白进程,放行

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserCallHwndParamLock(
    IN HWND hwnd,
    IN ULONG_PTR dwParam,
    IN DWORD xpfnProc
	)
*/
VOID _declspec(naked) fake_NtUserCallHwndParamLock()
{
	UINT ret, xpfnProc ;
	HANDLE PID ;
	HWND hWnd ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_EnableWindow );
	PID = hWnd = NULL ;
	
	// 2. 过滤之
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // 取参数
		mov		hWnd, eax

		mov	    eax, dword ptr [ebp+10h]
		mov		xpfnProc, eax
	}

	// 只关注EnableWindow对应的Index
	if ( xpfnProc != g_Win32k_apfnSimpleCall_Index_Info.SFI_XXXENABLEWINDOW ) { goto _Pass_ ; } 

	// hWnd --> PID ; 转换失败则拒绝掉
	PID = g_NtUserQueryWindow_addr( hWnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );
	if ( NULL == PID ) { goto _Denny_ ; }

	// 自己给自己发消息 或 给沙箱中的进程发消息,放行
	if ( PsGetCurrentProcessId() == PID || IsApprovedPIDEx( (PVOID)pNode, (ULONG)PID) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserDestroyWindow(
    IN HWND hwnd
	)
*/
VOID _declspec(naked) fake_NtUserDestroyWindow()
{
	int ret ;
	HWND hwnd;
	HANDLE PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_DestroyWindow );
	hwnd = PID = NULL ;

	__asm // 取参数
	{
		mov	    eax, dword ptr [ebp+8h]
		mov		hwnd, eax
	}

	// 2. 过滤之
	PID = g_NtUserQueryWindow_addr( hwnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );
	if ( PID )
	{
		// 窗口的自我销毁,不管不顾
		if ( PsGetCurrentProcessId() == PID ) { goto _Pass_ ; } 
		
		// 操作的是同一沙箱中的程序, 放行之
		if ( IsApprovedPIDEx( (PVOID)pNode, (ULONG)PID ) )  { goto _Pass_ ; } 

		// 其他情况禁止掉,弹消息提示用户
		ShowWarningInfo( _DestroyWindow_Type_, (HANDLE)PID );
	}

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 4h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSendInput(
	IN UINT    cInputs,
	IN int pInputs,
	IN int     cbSize
	);
*/

VOID _declspec(naked) fake_NtUserSendInput()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/17 [17:8:2010 - 18:26]

Routine Description:
  禁止向沙箱外的程序发送模拟按键(鼠标 & 键盘消息). 调用NtUserGetForegroundWindow,NtUserQueryWindow
  得到受害者的PID,通过Find_Node_from_PID()查看其是否为沙箱内部程序.不是则禁止掉,是内部程序则放行之.

  测试程序如下:
  HWND hwnd = FindWindow(NULL,"驱动加载工具 V1.3版");
  SetForegroundWindow(hwnd);
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_ESCAPE;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = VK_ESCAPE;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

  SendInput(2,inputs,sizeof(INPUT)); // 其内部会调用NtUserSendInput,该函数第一个参数貌似永远是1(不确定)
  // 一般内部调用:NtUserSendInput(1, &kbd, sizeof(INPUT)); kbd这个数组保存了以上2个小数组单元的内容
    
--*/
{
	int cInputs, pInputs, cbSize, n, ret ;
	BOOL bJumpOut ;
	HWND hwndActive;
	HANDLE PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SendInput );	
	n = ret = cInputs = 0 ;
	hwndActive = PID = NULL ;

	/*
	kd> u win32k!NtUserSendInput
	win32k!NtUserSendInput:
	bf849b03 6a18            push    18h
	bf849b05 6858b398bf      push    offset win32k!`string'+0x5b8 (bf98b358)
	bf849b0a e8d7d448c2      call    81cd6fe6 // 我们的Hook,故返回地址应在NtUserSendInput内部

	堆栈情况如下:
	esp     <-- 因为当前函数没有ebp,故ebp的内容属于KiFastCallEntry空间
	retAddr <-- 返回到  call    81cd6fe6的下条地址
	arg1    <-- 18h
	arg2    <-- offset win32k!`string'+0x5b8 (bf98b358)
	retAddr <-- nt!KiFastCallEntry函数内部
	arg1    <-- cInputs  (NtUserSendInput的参数1)
	arg2    <-- pInputs  (NtUserSendInput的参数2)
	arg3    <-- cbSize   (NtUserSendInput的参数3)

	// 1.1 在fake函数中有时会call NtUserSendInput,要保证重入时fake函数直接放行.即判断返回地址是否在当前函数内部
	*/
	__asm
	{
		// 取参数
		mov		eax, dword ptr [ebp+14h]
		mov		cInputs, eax

		mov		eax, dword ptr [ebp+18h]
		mov		pInputs, eax

		mov		eax, dword ptr [ebp+1Ch]
		mov		cbSize, eax
	}

	// 2. 若参数1带有特殊标记,表明是想要调用原始函数,走原始流程
	if ( cInputs & _SimulationButton_Flag_ ) 
	{
		ClearFlag( cInputs, _SimulationButton_Flag_ );
		__asm
		{ 
			mov   eax, cInputs
			mov   dword ptr [ebp+14h], eax
		}

		goto _Pass_ ;
	}
	
	// 3.1 判断是否禁止虚拟按键,若放行则走原流程
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput ) { goto _Pass_ ; }

	// 3.2 需要禁止,则走过滤流程
	while ( cInputs && cInputs < 0x10 )
	{
		// 得到当前窗口 --> PID --> 验证是否在沙箱中
		bJumpOut = FALSE ;
		hwndActive = g_NtUserGetForegroundWindow_addr();
		if ( hwndActive )
		{
			PID = g_NtUserQueryWindow_addr( hwndActive, QUERY_WINDOW_UNIQUE_PROCESS_ID );
			if ( NULL == kgetnodePD( PID ) )
			{
				bJumpOut = TRUE ; // 当前操作的窗口不在沙箱中,要禁止掉
			}
		}
		else
		{
			bJumpOut = TRUE ;
		}

		if ( bJumpOut )
		{
			if ( pNode->XWnd.CurrentHwnd != hwndActive ) { pNode->XWnd.CurrentHwnd = hwndActive ; }
			break ;
		}

		// 是正常操作,调用原始函数
		if ( 1 != g_NtUserSendInput_addr( 1 | _SimulationButton_Flag_ , pInputs, cbSize ) ) { break ; }

		// cInputs 指明了当前指令数组的个数; pInputs 是指令数组的头指针; cbSize是每个指令单元的大小
		-- cInputs ;		// 数组个数递减
		pInputs += cbSize ; // 将指针移动到下个数组单元
		++ n ;
	}

_Denny_ :
	if ( bJumpOut ) { ShowWarningInfo( _SendInput_Type_, (HANDLE)PID ); }
	g_SendInput_RetValue = n ;

	FinisDenny() ;
	__asm
	{
		add esp, 0Ch // 清掉返回地址 (返回到call 81cd6fe6的下条地址 ) + 局部变量 2个,属于SHE函数
		mov eax, g_SendInput_RetValue	// 修改返回值
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserBlockInput(
    IN BOOL fBlockIt
	)
*/
VOID _declspec(naked) fake_NtUserBlockInput()
{
	int ret ;
	BOOL fBlockIt ;
	HWND hwndActive;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_BlockInput );

	fBlockIt = FALSE ;

	// 2. 判断是否禁止虚拟按键,若放行则走原流程
	__asm
	{
		mov	eax, dword ptr [ebp+8]
		mov	fBlockIt, eax
	}
	
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput || FALSE == fBlockIt ) { goto _Pass_ ; }

	// 3. 显示拦截到的消息
	hwndActive = g_NtUserGetForegroundWindow_addr();
	if ( pNode->XWnd.CurrentHwnd != hwndActive ) { pNode->XWnd.CurrentHwnd = hwndActive ; }

	ShowWarningInfo( _BlockInput_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 4h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetSysColors(
    IN int cElements,
    IN CONST INT * lpaElements,
    IN CONST COLORREF * lpaRgbValues,
    IN UINT  uOptions
	)
*/
VOID _declspec(naked) fake_NtUserSetSysColors()
{
	int ret ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SetSysColors );

	if ( 0 == pNode->XWnd.bFlag_BlockSysParam ) { goto _Pass_ ; }

	ShowWarningInfo( _SetSysColors_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
BOOL
APIENTRY
NtUserSystemParametersInfo (
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
	);
*/
VOID _declspec(naked) fake_NtUserSystemParametersInfo(VOID)
{
	int ret, Index ;
	UINT uiAction, pvParam ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SystemParametersInfoW );
	uiAction = pvParam = 0 ;

	if ( 0 == pNode->XWnd.bFlag_BlockSysParam ) { goto _Pass_ ; }

	/*
	kd> u win32k!NtUserSystemParametersInfo
    win32k!NtUserSystemParametersInfo:
	bf843888 6850020000      push    250h
	bf84388d 6880ae98bf      push    offset win32k!`string'+0xe0 (bf98ae80)
	bf843892 e84fd7bfc2      call    82440fe6 // 我们的Hook点

	堆栈情况如下:
	esp     <-- 因为当前函数没有ebp,故ebp的内容属于KiFastCallEntry空间
	retAddr <-- 返回到  call    81cd6fe6的下条地址
	arg1    <-- 250h
	arg2    <-- offset win32k!`string'+0xe0 (bf98ae80)
	retAddr <-- nt!KiFastCallEntry函数内部
	arg1    <-- cInputs  (NtUserSystemParametersInfo的参数1)
	...

	*/
	__asm 
	{
		mov	    eax, dword ptr [ebp+14h] // 取参数
		mov		uiAction, eax

		mov	    eax, dword ptr [ebp+1Ch]
		mov		pvParam, eax
	}

	// 允许发送以下命令
	for ( Index=0; Index<ARRAYSIZEOF(g_SystemParametersInfo_WhiteList_Array); Index++ )
	{
		if ( g_SystemParametersInfo_WhiteList_Array[ Index ] == uiAction ) 
		{
			// 在白名单中; 即使是白的,也要禁止以下情况
			if ( (SPI_ICONHORIZONTALSPACING == uiAction || SPI_ICONVERTICALSPACING == uiAction) && (0 == pvParam) )
			{
				break ;
			}
			else
			{
				goto _Pass_ ;
			}
		}
	}

	// 不在白名单中,禁止掉啦~
	ShowWarningInfo( _SystemParametersInfoW_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch // 清掉返回地址 (返回到call 82440fe6 // 我们的Hook点的下条地址 ) + 局部变量 2个,属于SHE函数
		mov eax, 0	 // 修改返回值
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetWindowsHookEx(
    IN HANDLE hmod,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN DWORD idThread,
    IN int nFilterType,
    IN PROC pfnFilterProc,
    IN DWORD dwFlags
	)
*/
VOID _declspec(naked) fake_NtUserSetWindowsHookEx()
{
	int ret, idThread, nFilterType ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SetWindowsHookExW );
	idThread = nFilterType = 0 ;

	if ( 0 == pNode->XWnd.bFlag_BlockWinHooks ) { goto _Pass_ ; }

	__asm
	{
		mov	eax, dword ptr [ebp+10h]
		mov	idThread, eax

		mov	eax, dword ptr [ebp+14h]
		mov	nFilterType, eax
	}

	// 放行3个类型的消息钩子:1,13,14
	if ( 0 == nFilterType || WH_JOURNALPLAYBACK == nFilterType || WH_KEYBOARD_LL == nFilterType || WH_MOUSE_LL == nFilterType ) { goto _Pass_ ; }

	// 禁止 WH_SYSMSGFILTER 消息
	if ( WH_SYSMSGFILTER == nFilterType )  { goto _Denny_ ; }
	
	// 操作的是沙箱中的进程,放行
	if ( idThread && IsApprovedTID((PVOID)pNode, (HANDLE)idThread, NULL) )  { goto _Pass_ ; } 
	
	ShowWarningInfo( _SetWindowsHookEx_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 18h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetWinEventHook(
    IN DWORD           eventMin,
    IN DWORD           eventMax,
    IN HMODULE         hmodWinEventProc,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN PVOID    pfnWinEventProc,
    IN DWORD           idEventProcess,
    IN DWORD           idEventThread,
    IN DWORD           dwFlags
	)
*/
VOID _declspec(naked) fake_NtUserSetWinEventHook()
{
	int ret, idEventProcess, idEventThread, dwFlags ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreHelper( Tag_SetWinEventHook );

	if ( 0 == pNode->XWnd.bFlag_BlockWinHooks ) { goto _Pass_ ; }

	__asm
	{
		mov	eax, dword ptr [ebp+1Ch]
		mov	idEventProcess, eax

		mov	eax, dword ptr [ebp+20h]
		mov	idEventThread, eax

		mov	eax, dword ptr [ebp+24h]
		mov	dwFlags, eax
	}

	if ( !(dwFlags & WINEVENT_INCONTEXT) ) { goto _Pass_ ; }

	if ( idEventThread && IsApprovedTID((PVOID)pNode, (HANDLE)idEventThread, NULL) )  { goto _Pass_ ; } // 操作的是沙箱中的进程,放行
	
	if ( idEventProcess && IsApprovedPIDEx((PVOID)pNode, (ULONG)idEventProcess) ) { goto _Pass_ ; } // 操作的是沙箱中的进程,放行
	
	ShowWarningInfo( _SetWinEventHook_Type_, (HANDLE)idEventProcess );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 20h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtRequestPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage
    )
*/
VOID _declspec(naked) fake_NtRequestPort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreRpcHelper( TagSpec_NtRequestPort );

	/*
	kd> u nt!NtRequestPort
	nt!NtRequestPort:
	805978b4 6a3c            push    3Ch
	805978b6 68a0a04d80      push    offset nt!_real+0xb0 (804da0a0)
	805978bb e82657d801      call    8231cfe6 // 我们的Hook
	805978c0 33db            xor     ebx,ebx

	堆栈情况如下:
	esp     <-- 因为当前函数没有ebp,故ebp的内容属于KiFastCallEntry空间
	retAddr <-- 返回到  call    81cd6fe6的下条地址
	arg1    <-- 3Ch
	arg2    <-- offset nt!_real+0xb0 (804da0a0)
	retAddr <-- nt!KiFastCallEntry函数内部
	arg1    <-- PortHandle		(NtRequestPort的参数1)
	arg2    <-- RequestMessage  (NtRequestPort的参数2)

	*/
	__asm
	{
		mov		eax, dword ptr [ebp+14h] // 取参数1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+18h] // 取参数2
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PVOID)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtRequestPort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // 清掉返回地址 (返回到call 8231cfe6 // 我们的Hook点的下条地址 ) + 局部变量 2个,属于SHE函数
		mov eax, 0C000010Ah		// 修改返回值 STATUS_PROCESS_IS_TERMINATING;
		ret 8h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtRequestWaitReplyPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    )
*/
VOID _declspec(naked) fake_NtRequestWaitReplyPort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreRpcHelper( TagSpec_NtRequestWaitReplyPort );

	/*
	kd> u nt!NtRequestWaitReplyPort
	nt!NtRequestWaitReplyPort:
	80597be0 6a7c            push    7Ch
	80597be2 68b8a04d80      push    offset nt!_real+0xc8 (804da0b8)
	80597be7 e8fa733102      call    828aefe6 // 我们的Hook
	80597bec 33f6            xor     esi,esi

	堆栈情况如下:
	esp     <-- 因为当前函数没有ebp,故ebp的内容属于KiFastCallEntry空间
	retAddr <-- 返回到  call    828aefe6的下条地址
	arg1    <-- 7Ch
	arg2    <-- offset nt!_real+0xc8 (804da0b8)
	retAddr <-- nt!KiFastCallEntry函数内部
	arg1    <-- PortHandle		(NtRequestWaitReplyPort的参数1)
	arg2    <-- RequestMessage  (NtRequestWaitReplyPort的参数2)

	*/
	__asm
	{
		mov		eax, dword ptr [ebp+14h] // 取参数1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+18h] // 取参数2
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PVOID)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtRequestWaitReplyPort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // 清掉返回地址 (返回到call 828aefe6 // 我们的Hook点的下条地址 ) + 局部变量 2个,属于SHE函数
		mov eax, 0C000010Ah		// 修改返回值 STATUS_PROCESS_IS_TERMINATING;
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtAlpcSendWaitReceivePort(
	IN HANDLE PortHandle,
	IN int a2,
	IN void *RequestMessage,
	int a4, int a5, int a6, int a7, int a8
	)
*/
VOID _declspec(naked) fake_NtAlpcSendWaitReceivePort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. 前置处理
	PreRpcHelper( TagSpec_NtAlpcSendWaitReceivePort );

	__asm
	{
		mov		eax, dword ptr [ebp+14h] // 取参数1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+1Ch] // 取参数3
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PPORT_MESSAGE)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtAlpcSendWaitReceivePort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // 清掉返回地址 (返回到call 828aefe6 // 我们的Hook点的下条地址 ) + 局部变量 2个,属于SHE函数
		mov eax, 0C000010Ah		// 修改返回值 STATUS_PROCESS_IS_TERMINATING;
		ret 20h
	}

_Pass_ :
	FinisOrig();
}






/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         Others                            +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


int
Prepare_for_RPC (
	OUT PVOID* _pNode
	)
{
	BOOL bRet = TRUE ;
	LPPDNODE pNode = NULL ;
	
	// 1. 查找PID对应的进程总节点,若未找到,表明非沙箱中的程序,放行
	pNode = (LPPDNODE) kgetnodePD( 0 );
	if ( NULL == pNode ) { return _PREPARE_RETURN_TYPE_PASS_ ; }

	// 2. 若是遭遗弃的节点,返回拒绝标志位
	if ( pNode->bDiscard ) { return _PREPARE_RETURN_TYPE_DENNY_ ; }
	
	// 3. 一切正常,返回继续标志位
	*_pNode = (PVOID) pNode ;
	return _PREPARE_RETURN_TYPE_GOON_ ;
}



int
Prepare_for_NtUser (
	OUT PVOID* _pNode
	)
{
	BOOL bRet = TRUE ;
	LPPDNODE pNode = NULL ;

	// 1. 获取部分shaodw ssdt的原始地址,保存至全局变量中; 获取地址异常默认放行
	bRet = Prepare_for_NtUserEx ();
	if ( FALSE == bRet ) { return _PREPARE_RETURN_TYPE_PASS_ ; }
	
	// 2. 查找PID对应的进程总节点,若未找到,表明非沙箱中的程序,放行
	pNode = (LPPDNODE) kgetnodePD( 0 );
	if ( NULL == pNode ) { return _PREPARE_RETURN_TYPE_PASS_ ; }

	// 3. 若是遭遗弃的节点,返回拒绝标志位
	if ( pNode->bDiscard ) { return _PREPARE_RETURN_TYPE_DENNY_ ; }
	
	// 4. 建立相应的黑白名单,建立失败则将该节点标记为"已遗弃",返回拒绝标志位
	if ( FALSE == pNode->XWnd.bFlagInited ) 
	{
		bRet = BuildGrayList_for_Wnd( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			dprintf( "error! | fake_NtUser*() - BuildGrayList_for_Wnd(); | 建立灰名单失败. \n" );
			pNode->bDiscard = 1 ;
			return _PREPARE_RETURN_TYPE_DENNY_ ;
		}
	}

	// 5. 一切正常,返回继续标志位
	*_pNode = (PVOID) pNode ;
	return _PREPARE_RETURN_TYPE_GOON_ ;
}



BOOL Prepare_for_NtUserEx ()
{
	BOOL bRet = FALSE ;

	if ( NULL == g_NtUserSendInput_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserSendInput_addr, Tag_SendInput );
		if ( FALSE == bRet || 0 == g_NtUserSendInput_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserGetForegroundWindow_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserGetForegroundWindow_addr, Tag_GetForegroundWindow );
		if ( FALSE == bRet || 0 == g_NtUserGetForegroundWindow_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserQueryWindow_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserQueryWindow_addr, Tag_IsHungAppWindow );
		if ( FALSE == bRet || 0 == g_NtUserQueryWindow_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserGetClassName_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserGetClassName_addr, Tag_GetClassNameW );
		if ( FALSE == bRet || 0 == g_NtUserGetClassName_addr )
		{
			return FALSE ;
		}
	}

	return TRUE ;
}



BOOL
MsgFilter (
	IN PVOID _pNode,
	IN int Flag,
	IN HWND hWnd,
	IN UINT msg
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/22 [22:8:2010 - 0:58]

Routine Description:
  NtUserMessageCall,NtUserMessageCallback,NtUserMessageNotify,NtUserPostMessage
  主要的消息过滤函数MsgFilter()的过滤规则如下
  1) 0x15F BYTE bFlag_SendPostMessage_allowed ; // 是否允许NtUser**发送消息.允许则直接放行
  2) 0x15D BYTE bFlag_BlockFakeInput ;// 是否禁止虚拟按键,TRUE为禁止; 如果禁止,则继续3
     如果配置文件未对此处做限制,要放行一部分Msg,剩余的消息继续走3的过滤规则
  3) 如果是自己给自己发消息 或者给另外一个沙箱中的程序发消息,放行之
  4) 如果PID在沙箱的进程链中,直接走7的过滤规则
  5) 如果PID不在沙箱的进程链中,且Msg == 0x3E4,放行之
  6) 如果PID不在沙箱的进程链中,且Msg != 0x3E4,调用IsWhiteProcess(),若返回TRUE,则放行之
  7) 调用Is_Name_in_NodeWnd_normal()判断接收消息的进程窗体类名是否存在于白名单中. 不存在则直接禁止掉
     (只允许沙箱中的程序发消息到外界指定的程序,即白名单程序)
  8) 若消息是发送到白名单程序,也同样不允许向白名单中的这些窗体发送g_forbidden_Message_Array数组中的消息
  9) 允许沙箱中的程序向explorer.exe中的类窗口CicMarshalWndClass发送0x400以上的消息
    
Arguments:
  _pNode - 当前进程对应的结点
  Flag  - 标记是SendMessage 或者 PostMessage
  hWnd - eg: NtUserMessageCall的参数
  msg - eg: NtUserMessageCall的参数

Return Value:
  TRUE - 允许发送消息; FALSE - 禁止当前消息的发送
    
--*/
{
	BOOL bIsWhite = FALSE ;
	ULONG ClassNameLength = 0, Index = 0 ;
	SIZE_T RegionSize = 0x1000 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE PID = NULL ;
	PERESOURCE QueueLockList = NULL ;
	LPName_Info lpNameInfo = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. 校验参数合法性
	if ( NULL == pNode ){ return TRUE ; } // 参数不合法,默认放行掉
	if ( pNode->XWnd.bFlag_SendPostMessage_allowed ) { return TRUE ; } // 允许NtUser**发送消息,放行
	
	// 2. 若无需禁止虚拟按键,要放行一部分Msg,剩余的消息继续走过滤规则
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput )
	{
		if (   0xFF == msg 
			|| (msg >= WM_KEYFIRST   && msg <= WM_KEYLAST)
			|| (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
			|| (msg >= 0x2A0 && msg <= 0x2A3)
			|| (msg >= WM_TABLET_FIRST && msg <= WM_TABLET_LAST)
			)
		{
			return TRUE ;
		}
	}

	// 3. 若是自己给自己发消息 或者给沙箱中的其他程序发消息,放行
	PID = g_NtUserQueryWindow_addr( hWnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );

	if ( NULL == PID ) { return FALSE ; }
	if ( PsGetCurrentProcessId() == PID || IsApprovedPIDEx(_pNode, (ULONG)PID) ) { return TRUE ; }

	// 4. 处理以下情况: 向沙箱外的进程发消息
	if ( NULL == kgetnodePD( PID ) )
	{
		if ( 0x3E4 == msg ) { return TRUE ; }
		if ( IsWhiteProcess( _pNode, PID ) ) { return TRUE ; }
	}

	// 5. 在当前进程中申请一块足够大的临时内存
	if ( NULL == pNode->XWnd.TranshipmentStation )
	{
		status = ZwAllocateVirtualMemory (
			NtCurrentProcess() ,
			 (PVOID *)&pNode->XWnd.TranshipmentStation ,
			0,
			&RegionSize,
			MEM_COMMIT,
			PAGE_READWRITE
			);

		if ( ! NT_SUCCESS(status) )
		{
			dprintf( "error! | MsgFilter() - ZwAllocateVirtualMemory(); | status=0x8lx \n", status );
			pNode->bDiscard = 1 ;
			return FALSE ;
		}
	}

	if ( NULL == pNode->XWnd.VictimClassName )
	{
		pNode->XWnd.VictimClassName = kmalloc( 0x100 );
		if ( NULL == pNode->XWnd.VictimClassName )
		{
			dprintf( "error! | MsgFilter() - kmalloc(); | (Length=0x100) \n" );
			pNode->bDiscard = 1 ;
			return FALSE ;
		}
	}

	// 6. hWnd-->Name,将字符串保存至pNode->XWnd.VictimClassName
	__try
	{
		lpNameInfo = (LPName_Info) pNode->XWnd.TranshipmentStation ;
		RtlZeroMemory( lpNameInfo, 0x1000 );

		lpNameInfo->NameInfo.Name.Length			= 0 ;
		lpNameInfo->NameInfo.Name.MaximumLength	= 0xFE ;
		lpNameInfo->NameInfo.Name.Buffer			= (LPWSTR)((int)lpNameInfo + 8) ;

		ProbeForRead( lpNameInfo->pBuffer, 0x100, 2 );

		// 查询得到hwnd对应的窗口类名
		ClassNameLength = g_NtUserGetClassName_addr( hWnd, FALSE, (PUNICODE_STRING)lpNameInfo );
		if ( 0 == ClassNameLength || ClassNameLength >= 0x80 )
		{
			dprintf( "error! | MsgFilter() - g_NtUserGetClassName_addr(); | ClassNameLength=%d \n", ClassNameLength );
			return FALSE ;
		}

		// 保存 & 加上结束符
		RtlCopyMemory( pNode->XWnd.VictimClassName, lpNameInfo->pBuffer, ClassNameLength * sizeof(WCHAR) );
		*(WORD*)( (ULONG)pNode->XWnd.VictimClassName + ClassNameLength * sizeof(WCHAR) ) = UNICODE_NULL ;
		RtlZeroMemory( lpNameInfo, 0x1000 );

	}
	 __except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | MsgFilter() - __try __except (); | \n" );
	    return FALSE ;
	}

	//
	// 以下是处理向沙箱外的窗口发消息
	//

	// 7. VictimClassName过白名单
	QueueLockList = pNode->XWnd.pResource ;
	bIsWhite = GLFindNodeEx( &pNode->XWnd.WndListHead, QueueLockList, pNode->XWnd.VictimClassName, NULL );

	// 7.1 向沙箱外的 "非" 白名单窗口发消息,拒绝掉
	if ( FALSE == bIsWhite ) { return FALSE ; }
	
	if ( msg < 0x400 )
	{
		// 7.2 不允许向沙箱外的白名单进程发送以下消息
		for ( Index=0; Index<ARRAYSIZEOF(g_forbidden_Message_Array); Index++ )
		{
			// 当前进程的消息操作非法,拒绝掉!
			if ( g_forbidden_Message_Array[ Index ] == msg ) { return FALSE ; }
		}

		return TRUE ;
	}
	else
	{
		// 7.3 对沙箱外的白名单窗口发送0x400以上的消息,都拒绝掉; 只放行explorer.exe进程所拥有的类名为"CicMarshalWndClass"的窗口
		BOOL bRet = TRUE, bPass = FALSE ;
		LPWSTR lpImageFileShortName = NULL ;
		PUNICODE_STRING lpImageFileName = NULL ;

		// 得到PID对应的进程路径
		bRet = GetProcessImageFileName( PID, &lpImageFileName, &lpImageFileShortName ); // 调用者负责释放内存
		if ( FALSE == bRet ) { return TRUE ; } // 未得到进程名就默认放行

		if (   (0 == _wcsicmp( lpImageFileShortName, L"explorer.exe" ))
			&& (0x12 == ClassNameLength)
			&& (0 == _wcsicmp( pNode->XWnd.VictimClassName, L"CicMarshalWndClass" ))
			)
		{
			bPass = TRUE ; // Allowed SendMessage to explorer's CicMarshalWndClass
		}

		kfree( (PVOID)lpImageFileName ); // 释放内存
		return bPass ;
	}

	return FALSE ;
}



NTSTATUS
RpcFilter (
	IN HANDLE PortHandle,
	IN PPORT_MESSAGE RequestMessage
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/26 [26:8:2010 - 18:48]

Routine Description:
  RPC过滤函数,负责过滤 NtRequestProt / NtRequestWaitReplyPort / NtAlpcSendWaitReceivePort
    
--*/
{
	BOOL bHasNoName = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	PVOID Object = NULL, NewObject = NULL ;
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;

	// 1. 校验参数合法性
	if ( NULL == PortHandle || NULL == RequestMessage )
	{
	//	dprintf( "error! | RpcFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	// 2. 获得对象名; Handle --> Object --> Object Name
	status = ObReferenceObjectByHandle( PortHandle, 0, NULL, KernelMode, &Object, NULL );
	if ( ! NT_SUCCESS(status) )
	{
	//	dprintf( "error! | RpcFilter() - ObReferenceObjectByHandle(); | status=0x8lx \n", status );
		return status ;
	}

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject )
	{
	//	dprintf( "error! | RpcFilter() - GetFileObject(); | NULL == NewObject \n" );
		status = STATUS_ACCESS_DENIED ;
		goto _OVER_ ;
	}

	ObjectNameInfo = SepQueryNameString( NewObject, &bHasNoName ); // 由调用者释放内存
	if ( NULL == ObjectNameInfo )
	{
		if ( bHasNoName ) {
			status = STATUS_SUCCESS ;
		} else {
		//	dprintf( "error! | RpcFilter() - SepQueryNameString(); | NULL == ObjectNameInfo \n" );
			status = STATUS_UNSUCCESSFUL ;
		}

		status = STATUS_SUCCESS ;
		goto _OVER_ ;
	}

	// 3. 过滤对象名
	status = RpcFilterEx( RequestMessage, (PUNICODE_STRING)ObjectNameInfo );
	if ( STATUS_BAD_INITIAL_PC == status )
	{
		status = RpcFilterExp( RequestMessage, (PUNICODE_STRING)ObjectNameInfo );
		if ( STATUS_BAD_INITIAL_PC == status )
			status = STATUS_SUCCESS ;
	}

	// 4. Clear Up
	kfree( (PVOID)ObjectNameInfo );
_OVER_ :
	if ( Object ) { ObfDereferenceObject(Object); }
	return status ;
}



NTSTATUS
RpcFilterEx (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/31 [31:8:2010 - 15:17]

Routine Description:
  这一层的主要目的是拦截向对象 L"\\Sessions\\*\\Windows\\ApiPort" 发送以下RPC消息:
  UserpExitWindowsEx / UserpEndTask
    
--*/
{
	ULONG ApiNumber = 0 ;
	PUSER_API_MSG Msg = (PUSER_API_MSG) RequestMessage ;

	// 1. 校验参数合法性
	if ( NULL == RequestMessage || NULL == ObjectNameInfo ) { return STATUS_UNSUCCESSFUL ; }

	// 2. 过滤对象名
	if (   ObjectNameInfo->Length < 0x20
		|| _wcsicmp( &ObjectNameInfo->Buffer[ ObjectNameInfo->Length / sizeof(WCHAR) - 0x10 ], L"\\Windows\\ApiPort" )
		|| ( ObjectNameInfo->Length >= 0x20 && _wcsnicmp( ObjectNameInfo->Buffer, L"\\Sessions\\", 0xA ) )
		)
	{
		return STATUS_BAD_INITIAL_PC ;
	}

	// 3. 到这里,对象名的长度肯定>=20 且内容符合L"\\Sessions\\*\\Windows\\ApiPort"
	__try
	{
		ProbeForRead( RequestMessage, sizeof(PORT_MESSAGE), sizeof(ULONG) );
		if ( RequestMessage->u1.s1.TotalLength < 0x20 ) { return STATUS_SUCCESS ; } // 消息块大小低于0x20,直接放行

		ProbeForRead( Msg, 0x28, sizeof(ULONG) );
		ApiNumber = Msg->ApiNumber ;
		
		// 禁止关机消息
		if (   ApiNumber == CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,UserpExitWindowsEx)
			|| ApiNumber == CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,UserpEndTask)
			)
		{
			return STATUS_ACCESS_DENIED ;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	    return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
RpcFilterExp (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/31 [31:8:2010 - 15:17]

Routine Description:
  进一步特殊处理部分RPC消息
    
--*/
{
	ULONG ApiNumber = 0 ;
	PUSER_API_MSG Msg = (PUSER_API_MSG) RequestMessage ;

	// 1. 校验参数合法性
	if ( NULL == RequestMessage || NULL == ObjectNameInfo ) { return STATUS_UNSUCCESSFUL ; }

	// 2. 只关注对象名为 "\\LsaAuthenticationPort" 或者 "\\RPC Control\\lsasspirpc" 的情况
	if ( 0x2C == ObjectNameInfo->Length )
	{
		if ( _wcsicmp(ObjectNameInfo->Buffer, L"\\LsaAuthenticationPort") ) { return STATUS_BAD_INITIAL_PC ; }		
	}
	else if ( 0x2E == ObjectNameInfo->Length )
	{
		if ( _wcsicmp(ObjectNameInfo->Buffer, L"\\RPC Control\\lsasspirpc") ) { return STATUS_BAD_INITIAL_PC ; }
	}
	else 
	{
		return STATUS_BAD_INITIAL_PC ;
	}

	// 3. 到这里,对象名仅为以上2种情况
	__try
	{
		ProbeForRead( RequestMessage, sizeof(PORT_MESSAGE), sizeof(ULONG) );
		
		if ( g_Version_Info.IS_before_vista )
		{
			// 3.1 Vista以前的系统
			if ( RequestMessage->u1.s1.TotalLength < 0x30 ) { return STATUS_SUCCESS ; } // 消息块大小低于0x20,直接放行

			ProbeForRead( Msg, 0x30, sizeof(ULONG) );
			
			if (   2 == (ULONG) Msg->CaptureBuffer 
				&& g_LsaAuthPkg == (ULONG) Msg->ReturnValue
				&& *(DWORD *)((ULONG)Msg + 0x28) >= 4
			   )
			{
				ULONG Reserved = Msg->Reserved ;
				ProbeForRead( (PVOID)Reserved, 4, sizeof(ULONG) );

				if ( 5 == *(PULONG) Reserved )
				{
					return STATUS_ACCESS_DENIED ;
				}
			}
		}
		else
		{
			// 3.2 Vista / Win7下的处理
			PVOID Buffer = (PVOID) ((ULONG_PTR)Msg + sizeof(PORT_MESSAGE)) ;
			ULONG DataLength = Msg->h.u1.s1.DataLength ;

			ProbeForRead( Buffer, DataLength, sizeof(SHORT) );

			if ( DataLength >= 0x12 && wcsstr( (WCHAR*)Buffer, L"Negotiate" ) )
			{
				return STATUS_ACCESS_DENIED ;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



BOOL
IsWhiteProcess (
	IN PVOID _ProcessNode,
	IN HANDLE PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 14:45]

Routine Description:
  通过@PID得到接收消息的进程名(受害者),从入侵者@pNode得到WndListHead,遍历该白名单链表. 
  对特征为"*:"的节点单元,取特征值后面的内容,与受害者进程名名相匹配. 若相同,表明入侵者
  操作的对象我们不用管,它想给受害者发什么消息就发什么消息!也就是说,这是个特例: 
  "沙箱中的进程可以向沙箱外的特定进程发送任何消息"
  eg: 配置文件中指明OpenWinClass=*:nmSvc.exe; 则沙箱中的任何进程可向沙箱外的nmSvc.exe发任何消息

Arguments:
  _ProcessNode - 当前进程对应的结点(即发送消息的入侵者)
  PID - 接收消息的进程ID(可理解为受害者); 通过2种途径获取,一种是NtUserPostThreadMessage的参数; 一种由调用NtUserQueryWindow将hWnd转换而来
    
--*/
{
	BOOL bRet = FALSE ;
	LPWSTR wszName = NULL, lpImageFileShortName = NULL ;
	PUNICODE_STRING lpImageFileName = NULL ;
	PLIST_ENTRY ListHead = NULL, Next = NULL ;	
	PERESOURCE QueueLockList = NULL ;
	LPGRAYLIST_INFO pNode = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;

	// 1. 校验参数合法性
	if ( NULL == ProcessNode || NULL == PID ) { return FALSE ; }
	
	QueueLockList	= ProcessNode->XWnd.pResource ;
	ListHead		= &ProcessNode->XWnd.WndListHead ;
	if ( NULL == QueueLockList || NULL == ListHead ) { return FALSE ; }

	// 2. 得到PID对应的进程路径
	bRet = GetProcessImageFileName( PID, &lpImageFileName, &lpImageFileShortName ); // 调用者负责释放内存
	if ( FALSE == bRet )
	{
		dprintf( "error! | IsWhiteProcess() - GetProcessImageFileName(); | (PID=0x%08lx)\n", PID );
		return FALSE ;
	}
	
	// 3. 遍历链表
	bRet = FALSE ;
	EnterCrit( QueueLockList );	// 加锁访问
	Next = ListHead->Flink;

	while ( Next != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( Next, GRAYLIST_INFO, ListEntry ));

		wszName = pNode->wszName ; 
		if (   ( pNode->NameLength > 3 )
			&& ( L'*' == *wszName )
			&& ( L':' == wszName[1] )
			)
		{
			if ( 0 == _wcsicmp( lpImageFileShortName, (LPWSTR)((int)wszName+4) ) )
			{
				bRet = TRUE ;
				break ;
			}
		}

		Next = Next->Flink;
	}

	LeaveCrit( QueueLockList );	// 释放锁
	kfree( (PVOID)lpImageFileName ); // 释放内存
	return bRet ;
}



VOID 
ShowWarningInfo (
	IN int InfoType,
	IN HANDLE VictimHandle
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/21 [21:8:2010 - 15:54]

Routine Description:
  显示从fake shadow ssdt 函数拦截到的各种待阻止的信息    
    
Arguments:
  InfoType - 打印消息的类型
  VictimHandle - 当@InfoType == _SendInput_Type_,该参数有值,为入侵者PID
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PEPROCESS VictimProcess = NULL;
	ULONG InvaderProcess = (ULONG)PsGetCurrentProcess() ;

	switch ( InfoType )
	{
	case _SendInput_Type_ :
		dprintf( "[Warining] 检测到模拟按键 (SendInput),已成功阻止 \n" );
		break ;

	case _BlockInput_Type_ :
		dprintf( "[Warining] 检测到模拟按键 (BlockInput),已成功阻止 \n" );
		break;

	case _DestroyWindow_Type_ :
		dprintf( "[Warining] 检测到销毁窗口 (DestroyWindow),已成功阻止 \n" );
		break;

	case _SetSysColors_Type_ :
		dprintf( "[Warining] 检测到设置窗口颜色 (SetSysColors),已成功阻止 \n" );
		break;

	case _SystemParametersInfoW_Type_ :
		dprintf( "[Warining] 检测到操作桌面相关设置 (SystemParametersInfoW),已成功阻止 \n" );
		break;

	case _SetWinEventHook_Type_ :
		dprintf( "[Warining] 检测到设置全局钩子 (SetWinEventHook),已成功阻止 \n" );
		break;

	case _SetWindowsHookEx_Type_ :
		dprintf( "[Warining] 检测到设置全局钩子 (SetWindowsHookEx),已成功阻止 \n" );
		break;

	case _NtRequestPort_Type_ :
	case _NtRequestWaitReplyPort_Type_ :
	case _NtAlpcSendWaitReceivePort_Type_ :
		dprintf( "[Warining] 检测到不合法的RPC通信 (Nt(Alpc)Request*),已成功阻止 \n" );
		break;
	
	default :
		return ;
	}	

	if ( VictimHandle )
	{
		// 取得受害进程对象
		status = PsLookupProcessByProcessId( (HANDLE)VictimHandle, &VictimProcess);
		if (!NT_SUCCESS( status )) { return ; }

		ObDereferenceObject( (PVOID)VictimProcess ) ;

		dprintf( 
			"来源进程(入侵者): %s \n"
			"目标进程(受害者): %s \n\n",
			(PCHAR)(InvaderProcess + g_ImageFileName_Offset),
			(PCHAR)((ULONG)VictimProcess + g_ImageFileName_Offset)
			);
	}
	else
	{
		dprintf( 
			"来源进程(入侵者): %s \n",
			(PCHAR)(InvaderProcess + g_ImageFileName_Offset)
			);
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////