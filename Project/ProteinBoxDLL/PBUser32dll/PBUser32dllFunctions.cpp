/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/20 [20:1:2012 - 11:37]
* MODULE : \Code\Project\ProteinBoxDLL\PBUser32dll\PBUser32dllFunctions.cpp
* 
* Description:
*
*  user32.dll中函数原始地址的申明   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "PBUser32dllFunctions.h"

//////////////////////////////////////////////////////////////////////////

_MsgWaitForMultipleObjects_ g_MsgWaitForMultipleObjects_addr = NULL ;
_PeekMessageW_ g_PeekMessageW_addr = NULL ;
_GetMessageW_ g_GetMessageW_addr = NULL ;
_GetParent_ g_GetParent_addr = NULL ;
_SetParent_ g_SetParent_addr = NULL ;
_IsWindowVisible_ g_IsWindowVisible_addr = NULL ;
_GetWindowLongW_ g_GetWindowLongW_addr = NULL ;
_GetWindowLongA_ g_GetWindowLongA_addr = NULL ;
_SetPropW_ g_SetPropW_addr = NULL ;
_SetPropA_ g_SetPropA_addr = NULL ;
_IsWindowUnicode_ g_IsWindowUnicode_addr = NULL ;
_GetClipboardOwner_ g_GetClipboardOwner_addr = NULL ;
_GetClipboardFormatNameA_ g_GetClipboardFormatNameA_addr = NULL ;
_GetClipboardFormatNameW_ g_GetClipboardFormatNameW_addr = NULL ;
_ExitWindowsEx_ g_ExitWindowsEx_addr = NULL ;
_EndTask_ g_EndTask_addr = NULL ;
_SetWindowLongW_ g_SetWindowLongW_addr = NULL ;
_SetWindowLongA_ g_SetWindowLongA_addr = NULL ;
_GetPropW_ g_GetPropW_addr = NULL ;
_CallWindowProcW_ g_CallWindowProcW_addr = NULL ;
_CallWindowProcA_ g_CallWindowProcA_addr = NULL ;
_CreateWindowExA_ g_CreateWindowExA_addr = NULL ;
_CreateWindowExW_ g_CreateWindowExW_addr = NULL ;
_DefWindowProcA_ g_DefWindowProcA_addr = NULL ;
_DefWindowProcW_ g_DefWindowProcW_addr = NULL ;
_InternalGetWindowText_ g_InternalGetWindowText_addr = NULL ;
_RealGetWindowClassW_ g_RealGetWindowClassW_addr = NULL ;
_RealGetWindowClassA_ g_RealGetWindowClassA_addr = NULL ;
_GetWindowTextA_ g_GetWindowTextA_addr = NULL ;
_GetWindowTextW_ g_GetWindowTextW_addr = NULL ;
_MoveWindow_ g_MoveWindow_addr = NULL ;
_SetWindowPos_ g_SetWindowPos_addr = NULL ;
_RegisterDeviceNotificationA_ g_RegisterDeviceNotificationA_addr = NULL ;
_RegisterDeviceNotificationW_ g_RegisterDeviceNotificationW_addr = NULL ;
_UnregisterDeviceNotification_ g_UnregisterDeviceNotification_addr = NULL ;
_RegisterClassA_ g_RegisterClassA_addr = NULL ;
_RegisterClassW_ g_RegisterClassW_addr = NULL ;
_DispatchMessageW_ g_DispatchMessageW_addr = NULL ;
_RegisterClassExA_ g_RegisterClassExA_addr = NULL ;
_RegisterClassExW_ g_RegisterClassExW_addr = NULL ;
_UnregisterClassA_ g_UnregisterClassA_addr = NULL ;
_UnregisterClassW_ g_UnregisterClassW_addr = NULL ;
_GetClassInfoA_ g_GetClassInfoA_addr = NULL ;
_GetClassInfoW_ g_GetClassInfoW_addr = NULL ;
_GetClassInfoExA_ g_GetClassInfoExA_addr = NULL ;
_GetClassInfoExW_ g_GetClassInfoExW_addr = NULL ;
_GetClassNameA_ g_GetClassNameA_addr = NULL ;
_EnumChildWindows_ g_EnumChildWindows_addr = NULL ;
_EnumThreadWindows_ g_EnumThreadWindows_addr = NULL ;
_EnumDesktopWindows_ g_EnumDesktopWindows_addr = NULL ;
_FindWindowA_ g_FindWindowA_addr = NULL ;
_FindWindowW_ g_FindWindowW_addr = NULL ;
_FindWindowExA_ g_FindWindowExA_addr = NULL ;
_FindWindowExW_ g_FindWindowExW_addr = NULL ;
_GetShellWindow_ g_GetShellWindow_addr = NULL ;
_GetPropA_ g_GetPropA_addr = NULL ;
_RemovePropA_ g_RemovePropA_addr = NULL ;
_RemovePropW_ g_RemovePropW_addr = NULL ;
_EnumWindows_ g_EnumWindows_addr = NULL ;
_PostMessageW_ g_PostMessageW_addr = NULL ;
_SetWindowsHookExW_ g_SetWindowsHookExW_addr = NULL ;
_SetWindowsHookExA_ g_SetWindowsHookExA_addr = NULL ;
_GetCursorPos_ g_GetCursorPos_addr = NULL ;
_DragFinish_ g_DragFinish_addr = NULL ;
_GetWindowThreadProcessId_ g_GetWindowThreadProcessId_addr = NULL ;
_SendMessageA_ g_SendMessageA_addr = NULL ;
_SendMessageW_ g_SendMessageW_addr = NULL ;
_SendMessageTimeoutA_ g_SendMessageTimeoutA_addr = NULL ;
_SendMessageTimeoutW_ g_SendMessageTimeoutW_addr = NULL ;
_SendNotifyMessageA_ g_SendNotifyMessageA_addr = NULL ;
_SendNotifyMessageW_ g_SendNotifyMessageW_addr = NULL ;
_PostMessageA_ g_PostMessageA_addr = NULL ;
_DispatchMessageA_ g_DispatchMessageA_addr = NULL ;
_UnhookWindowsHookEx_ g_UnhookWindowsHookEx_addr = NULL ;
_CreateDialogParamA_ g_CreateDialogParamA_addr = NULL ;
_CreateDialogParamW_ g_CreateDialogParamW_addr = NULL ;
_CreateDialogIndirectParamA_ g_CreateDialogIndirectParamA_addr = NULL ;
_CreateDialogIndirectParamW_ g_CreateDialogIndirectParamW_addr = NULL ;
_CreateDialogIndirectParamAorW_ g_CreateDialogIndirectParamAorW_addr = NULL ;
_DialogBoxParamA_ g_DialogBoxParamA_addr = NULL ;
_DialogBoxParamW_ g_DialogBoxParamW_addr = NULL ;
_DialogBoxIndirectParamA_ g_DialogBoxIndirectParamA_addr = NULL ;
_DialogBoxIndirectParamW_ g_DialogBoxIndirectParamW_addr = NULL ;
_DialogBoxIndirectParamAorW_ g_DialogBoxIndirectParamAorW_addr = NULL ;
_GetClassNameW_	    g_GetClassNameW_addr = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL GetUser32dllFuncAddrs( IN HMODULE hModule )
{
	g_GetWindowThreadProcessId_addr = (_GetWindowThreadProcessId_) GetProcAddress( hModule, "GetWindowThreadProcessId" );
	if ( NULL == g_GetWindowThreadProcessId_addr ) { return FALSE; }

	g_GetParent_addr = (_GetParent_) GetProcAddress( hModule, "GetParent" );
	if ( NULL == g_GetParent_addr ) { return FALSE; }

	g_SetParent_addr = (_SetParent_) GetProcAddress( hModule, "SetParent" );
	if ( NULL == g_SetParent_addr ) { return FALSE; }

	g_IsWindowVisible_addr = (_IsWindowVisible_) GetProcAddress( hModule, "IsWindowVisible" );
	if ( NULL == g_IsWindowVisible_addr ) { return FALSE; }

	g_GetCursorPos_addr = (_GetCursorPos_) GetProcAddress( hModule, "GetCursorPos" );
	if ( NULL == g_GetCursorPos_addr ) { return FALSE; }

	g_IsWindowUnicode_addr = (_IsWindowUnicode_) GetProcAddress( hModule, "IsWindowUnicode" );
	if ( NULL == g_IsWindowUnicode_addr ) { return FALSE; }

	g_GetClipboardOwner_addr = (_GetClipboardOwner_) GetProcAddress( hModule, "GetClipboardOwner" );
	if ( NULL == g_GetClipboardOwner_addr ) { return FALSE; }

	g_GetClipboardFormatNameA_addr = (_GetClipboardFormatNameA_) GetProcAddress( hModule, "GetClipboardFormatNameA" );
	if ( NULL == g_GetClipboardFormatNameA_addr ) { return FALSE; }

	g_GetClipboardFormatNameW_addr = (_GetClipboardFormatNameW_) GetProcAddress( hModule, "GetClipboardFormatNameW" );
	if ( NULL == g_GetClipboardFormatNameW_addr ) { return FALSE; }

	g_ExitWindowsEx_addr = (_ExitWindowsEx_) GetProcAddress( hModule, "ExitWindowsEx" );
	if ( NULL == g_ExitWindowsEx_addr ) { return FALSE; }

	g_EndTask_addr = (_EndTask_) GetProcAddress( hModule, "EndTask" );
	if ( NULL == g_EndTask_addr ) { return FALSE; }

	g_CallWindowProcA_addr = (_CallWindowProcA_) GetProcAddress( hModule, "CallWindowProcA" );
	if ( NULL == g_CallWindowProcA_addr ) { return FALSE; }

	g_CallWindowProcW_addr = (_CallWindowProcW_) GetProcAddress( hModule, "CallWindowProcW" );
	if ( NULL == g_CallWindowProcW_addr ) { return FALSE; }

	g_CreateWindowExA_addr = (_CreateWindowExA_) GetProcAddress( hModule, "CreateWindowExA" );
	if ( NULL == g_CreateWindowExA_addr ) { return FALSE; }

	g_CreateWindowExW_addr = (_CreateWindowExW_) GetProcAddress( hModule, "CreateWindowExW" );
	if ( NULL == g_CreateWindowExW_addr ) { return FALSE; }

	g_DefWindowProcA_addr = (_DefWindowProcA_) GetProcAddress( hModule, "DefWindowProcA" );
	if ( NULL == g_DefWindowProcA_addr ) { return FALSE; }

	g_DefWindowProcW_addr = (_DefWindowProcW_) GetProcAddress( hModule, "DefWindowProcW" );
	if ( NULL == g_DefWindowProcW_addr ) { return FALSE; }

	g_InternalGetWindowText_addr = (_InternalGetWindowText_) GetProcAddress( hModule, "InternalGetWindowText" );
	if ( NULL == g_InternalGetWindowText_addr ) { return FALSE; }

	g_RealGetWindowClassA_addr = (_RealGetWindowClassA_) GetProcAddress( hModule, "RealGetWindowClassA" );
	if ( NULL == g_RealGetWindowClassA_addr ) { return FALSE; }

	g_RealGetWindowClassW_addr = (_RealGetWindowClassW_) GetProcAddress( hModule, "RealGetWindowClassW" );
	if ( NULL == g_RealGetWindowClassW_addr ) { return FALSE; }

	g_GetWindowTextA_addr = (_GetWindowTextA_) GetProcAddress( hModule, "GetWindowTextA" );
	if ( NULL == g_GetWindowTextA_addr ) { return FALSE; }

	g_GetWindowTextW_addr = (_GetWindowTextW_) GetProcAddress( hModule, "GetWindowTextW" );
	if ( NULL == g_GetWindowTextW_addr ) { return FALSE; }

	g_MoveWindow_addr = (_MoveWindow_) GetProcAddress( hModule, "MoveWindow" );
	if ( NULL == g_MoveWindow_addr ) { return FALSE; }

	g_SetWindowPos_addr = (_SetWindowPos_) GetProcAddress( hModule, "SetWindowPos" );
	if ( NULL == g_SetWindowPos_addr ) { return FALSE; }

	g_RegisterDeviceNotificationA_addr = (_RegisterDeviceNotificationA_) GetProcAddress( hModule, "RegisterDeviceNotificationA" );
	if ( NULL == g_RegisterDeviceNotificationA_addr ) { return FALSE; }

	g_RegisterDeviceNotificationW_addr = (_RegisterDeviceNotificationW_) GetProcAddress( hModule, "RegisterDeviceNotificationW" );
	if ( NULL == g_RegisterDeviceNotificationW_addr ) { return FALSE; }

	g_UnregisterDeviceNotification_addr = (_UnregisterDeviceNotification_) GetProcAddress( hModule, "UnregisterDeviceNotification" );
	if ( NULL == g_UnregisterDeviceNotification_addr ) { return FALSE; }

	g_RegisterClassA_addr = (_RegisterClassA_) GetProcAddress( hModule, "RegisterClassA" );
	if ( NULL == g_RegisterClassA_addr ) { return FALSE; }

	g_RegisterClassW_addr = (_RegisterClassW_) GetProcAddress( hModule, "RegisterClassW" );
	if ( NULL == g_RegisterClassW_addr ) { return FALSE; }

	g_RegisterClassExA_addr = (_RegisterClassExA_) GetProcAddress( hModule, "RegisterClassExA" );
	if ( NULL == g_RegisterClassExA_addr ) { return FALSE; }

	g_RegisterClassExW_addr = (_RegisterClassExW_) GetProcAddress( hModule, "RegisterClassExW" );
	if ( NULL == g_RegisterClassExW_addr ) { return FALSE; }

	g_UnregisterClassA_addr = (_UnregisterClassA_) GetProcAddress( hModule, "UnregisterClassA" );
	if ( NULL == g_UnregisterClassA_addr ) { return FALSE; }

	g_UnregisterClassW_addr = (_UnregisterClassW_) GetProcAddress( hModule, "UnregisterClassW" );
	if ( NULL == g_UnregisterClassW_addr ) { return FALSE; }

	g_GetClassInfoA_addr = (_GetClassInfoA_) GetProcAddress( hModule, "GetClassInfoA" );
	if ( NULL == g_GetClassInfoA_addr ) { return FALSE; }

	g_GetClassInfoW_addr = (_GetClassInfoW_) GetProcAddress( hModule, "GetClassInfoW" );
	if ( NULL == g_GetClassInfoW_addr ) { return FALSE; }

	g_GetClassInfoExA_addr = (_GetClassInfoExA_) GetProcAddress( hModule, "GetClassInfoExA" );
	if ( NULL == g_GetClassInfoExA_addr ) { return FALSE; }

	g_GetClassInfoExW_addr = (_GetClassInfoExW_) GetProcAddress( hModule, "GetClassInfoExW" );
	if ( NULL == g_GetClassInfoExW_addr ) { return FALSE; }

	g_GetClassNameA_addr = (_GetClassNameA_) GetProcAddress( hModule, "GetClassNameA" );
	if ( NULL == g_GetClassNameA_addr ) { return FALSE; }

	g_GetClassNameW_addr = (_GetClassNameW_) GetProcAddress( hModule, "GetClassNameW" );
	if ( NULL == g_GetClassNameW_addr ) { return FALSE; }

	g_EnumWindows_addr = (_EnumWindows_) GetProcAddress( hModule, "EnumWindows" );
	if ( NULL == g_EnumWindows_addr ) { return FALSE; }

	g_EnumChildWindows_addr = (_EnumChildWindows_) GetProcAddress( hModule, "EnumChildWindows" );
	if ( NULL == g_EnumChildWindows_addr ) { return FALSE; }

	g_EnumThreadWindows_addr = (_EnumThreadWindows_) GetProcAddress( hModule, "EnumThreadWindows" );
	if ( NULL == g_EnumThreadWindows_addr ) { return FALSE; }

	g_EnumDesktopWindows_addr = (_EnumDesktopWindows_) GetProcAddress( hModule, "EnumDesktopWindows" );
	if ( NULL == g_EnumDesktopWindows_addr ) { return FALSE; }

	g_FindWindowA_addr = (_FindWindowA_) GetProcAddress( hModule, "FindWindowA" );
	if ( NULL == g_FindWindowA_addr ) { return FALSE; }

	g_FindWindowW_addr = (_FindWindowW_) GetProcAddress( hModule, "FindWindowW" );
	if ( NULL == g_FindWindowW_addr ) { return FALSE; }

	g_FindWindowExA_addr = (_FindWindowExA_) GetProcAddress( hModule, "FindWindowExA" );
	if ( NULL == g_FindWindowExA_addr ) { return FALSE; }

	g_FindWindowExW_addr = (_FindWindowExW_) GetProcAddress( hModule, "FindWindowExW" );
	if ( NULL == g_FindWindowExW_addr ) { return FALSE; }

	g_GetShellWindow_addr = (_GetShellWindow_) GetProcAddress( hModule, "GetShellWindow" );
	if ( NULL == g_GetShellWindow_addr ) { return FALSE; }

	g_GetPropA_addr = (_GetPropA_) GetProcAddress( hModule, "GetPropA" );
	if ( NULL == g_GetPropA_addr ) { return FALSE; }

	g_GetPropW_addr = (_GetPropW_) GetProcAddress( hModule, "GetPropW" );
	if ( NULL == g_GetPropW_addr ) { return FALSE; }

	g_SetPropA_addr = (_SetPropA_) GetProcAddress( hModule, "SetPropA" );
	if ( NULL == g_SetPropA_addr ) { return FALSE; }

	g_SetPropW_addr = (_SetPropW_) GetProcAddress( hModule, "SetPropW" );
	if ( NULL == g_SetPropW_addr ) { return FALSE; }

	g_RemovePropA_addr = (_RemovePropA_) GetProcAddress( hModule, "RemovePropA" );
	if ( NULL == g_RemovePropA_addr ) { return FALSE; }

	g_RemovePropW_addr = (_RemovePropW_) GetProcAddress( hModule, "RemovePropW" );
	if ( NULL == g_RemovePropW_addr ) { return FALSE; }

	g_GetWindowLongA_addr = (_GetWindowLongA_) GetProcAddress( hModule, "GetWindowLongA" );
	if ( NULL == g_GetWindowLongA_addr ) { return FALSE; }

	g_GetWindowLongW_addr = (_GetWindowLongW_) GetProcAddress( hModule, "GetWindowLongW" );
	if ( NULL == g_GetWindowLongW_addr ) { return FALSE; }

	g_SetWindowLongA_addr = (_SetWindowLongA_) GetProcAddress( hModule, "SetWindowLongA" );
	if ( NULL == g_SetWindowLongA_addr ) { return FALSE; }

	g_SetWindowLongW_addr = (_SetWindowLongW_) GetProcAddress( hModule, "SetWindowLongW" );
	if ( NULL == g_SetWindowLongW_addr ) { return FALSE; }

	g_SendMessageA_addr = (_SendMessageA_) GetProcAddress( hModule, "SendMessageA" );
	if ( NULL == g_SendMessageA_addr ) { return FALSE; }

	g_SendMessageW_addr = (_SendMessageW_) GetProcAddress( hModule, "SendMessageW" );
	if ( NULL == g_SendMessageW_addr ) { return FALSE; }

	g_SendMessageTimeoutA_addr = (_SendMessageTimeoutA_) GetProcAddress( hModule, "SendMessageTimeoutA" );
	if ( NULL == g_SendMessageTimeoutA_addr ) { return FALSE; }

	g_SendMessageTimeoutW_addr = (_SendMessageTimeoutW_) GetProcAddress( hModule, "SendMessageTimeoutW" );
	if ( NULL == g_SendMessageTimeoutW_addr ) { return FALSE; }

	g_SendNotifyMessageA_addr = (_SendNotifyMessageA_) GetProcAddress( hModule, "SendNotifyMessageA" );
	if ( NULL == g_SendNotifyMessageA_addr ) { return FALSE; }

	g_SendNotifyMessageW_addr = (_SendNotifyMessageW_) GetProcAddress( hModule, "SendNotifyMessageW" );
	if ( NULL == g_SendNotifyMessageW_addr ) { return FALSE; }

	g_PostMessageA_addr = (_PostMessageA_) GetProcAddress( hModule, "PostMessageA" );
	if ( NULL == g_PostMessageA_addr ) { return FALSE; }

	g_PostMessageW_addr = (_PostMessageW_) GetProcAddress( hModule, "PostMessageW" );
	if ( NULL == g_PostMessageW_addr ) { return FALSE; }

	g_DispatchMessageA_addr = (_DispatchMessageA_) GetProcAddress( hModule, "DispatchMessageA" );
	if ( NULL == g_DispatchMessageA_addr ) { return FALSE; }

	g_DispatchMessageW_addr = (_DispatchMessageW_) GetProcAddress( hModule, "DispatchMessageW" );
	if ( NULL == g_DispatchMessageW_addr ) { return FALSE; }

	g_SetWindowsHookExA_addr = (_SetWindowsHookExA_) GetProcAddress( hModule, "SetWindowsHookExA" );
	if ( NULL == g_SetWindowsHookExA_addr ) { return FALSE; }

	g_SetWindowsHookExW_addr = (_SetWindowsHookExW_) GetProcAddress( hModule, "SetWindowsHookExW" );
	if ( NULL == g_SetWindowsHookExW_addr ) { return FALSE; }

	g_UnhookWindowsHookEx_addr = (_UnhookWindowsHookEx_) GetProcAddress( hModule, "UnhookWindowsHookEx" );
	if ( NULL == g_UnhookWindowsHookEx_addr ) { return FALSE; }

	g_CreateDialogParamA_addr = (_CreateDialogParamA_) GetProcAddress( hModule, "CreateDialogParamA" );
	if ( NULL == g_CreateDialogParamA_addr ) { return FALSE; }

	g_CreateDialogParamW_addr = (_CreateDialogParamW_) GetProcAddress( hModule, "CreateDialogParamW" );
	if ( NULL == g_CreateDialogParamW_addr ) { return FALSE; }

	g_CreateDialogIndirectParamA_addr = (_CreateDialogIndirectParamA_) GetProcAddress( hModule, "CreateDialogIndirectParamA" );
	if ( NULL == g_CreateDialogIndirectParamA_addr ) { return FALSE; }

	g_CreateDialogIndirectParamW_addr = (_CreateDialogIndirectParamW_) GetProcAddress( hModule, "CreateDialogIndirectParamW" );
	if ( NULL == g_CreateDialogIndirectParamW_addr ) { return FALSE; }

	g_CreateDialogIndirectParamAorW_addr = (_CreateDialogIndirectParamAorW_) GetProcAddress( hModule, "CreateDialogIndirectParamAorW" );
	if ( NULL == g_CreateDialogIndirectParamAorW_addr ) { return FALSE; }

	g_DialogBoxParamA_addr = (_DialogBoxParamA_) GetProcAddress( hModule, "DialogBoxParamA" );
	if ( NULL == g_DialogBoxParamA_addr ) { return FALSE; }

	g_DialogBoxParamW_addr = (_DialogBoxParamW_) GetProcAddress( hModule, "DialogBoxParamW" );
	if ( NULL == g_DialogBoxParamW_addr ) { return FALSE; }

	g_DialogBoxIndirectParamA_addr = (_DialogBoxIndirectParamA_) GetProcAddress( hModule, "DialogBoxIndirectParamA" );
	if ( NULL == g_DialogBoxIndirectParamA_addr ) { return FALSE; }

	g_DialogBoxIndirectParamW_addr = (_DialogBoxIndirectParamW_) GetProcAddress( hModule, "DialogBoxIndirectParamW" );
	if ( NULL == g_DialogBoxIndirectParamW_addr ) { return FALSE; }

	g_DialogBoxIndirectParamAorW_addr = (_DialogBoxIndirectParamAorW_) GetProcAddress( hModule, "DialogBoxIndirectParamAorW" );
	if ( NULL == g_DialogBoxIndirectParamAorW_addr ) { return FALSE; }

	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////