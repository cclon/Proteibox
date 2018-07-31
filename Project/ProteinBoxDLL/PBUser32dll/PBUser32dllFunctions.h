#pragma once

//////////////////////////////////////////////////////////////////////////

typedef DWORD (WINAPI* _MsgWaitForMultipleObjects_) (DWORD,CONST HANDLE *,BOOL,DWORD,DWORD);
extern _MsgWaitForMultipleObjects_ g_MsgWaitForMultipleObjects_addr ;

typedef BOOL (WINAPI* _PeekMessageW_) (LPMSG,HWND,UINT,UINT,UINT);
extern _PeekMessageW_ g_PeekMessageW_addr ;

typedef BOOL (WINAPI* _GetMessageW_) (LPMSG,HWND,UINT,UINT );
extern _GetMessageW_ g_GetMessageW_addr ;

typedef HWND (WINAPI* _GetParent_) (HWND);
extern _GetParent_ g_GetParent_addr ;

typedef HWND (WINAPI* _SetParent_)( HWND, HWND );
extern _SetParent_ g_SetParent_addr ;

typedef BOOL (WINAPI* _IsWindowVisible_)( HWND );
extern _IsWindowVisible_ g_IsWindowVisible_addr ;

typedef LONG (WINAPI* _GetWindowLongW_) (HWND, int);
extern _GetWindowLongW_ g_GetWindowLongW_addr ;

typedef LONG (WINAPI* _GetWindowLongA_) (HWND, int);
extern _GetWindowLongA_ g_GetWindowLongA_addr ;

typedef BOOL (WINAPI* _SetPropW_) (HWND, LPCWSTR, HANDLE);
extern _SetPropW_ g_SetPropW_addr ;

typedef BOOL (WINAPI* _SetPropA_) (HWND, LPCSTR, HANDLE);
extern _SetPropA_ g_SetPropA_addr ;

typedef HANDLE (WINAPI* _GetPropW_) (HWND, LPCWSTR);
extern _GetPropW_ g_GetPropW_addr ;

typedef BOOL (WINAPI* _IsWindowUnicode_) (HWND);
extern _IsWindowUnicode_ g_IsWindowUnicode_addr ;

typedef HWND (WINAPI* _GetClipboardOwner_)( VOID );
extern _GetClipboardOwner_ g_GetClipboardOwner_addr ;

typedef int (WINAPI* _GetClipboardFormatNameA_)(UINT,LPSTR,int);
extern _GetClipboardFormatNameA_ g_GetClipboardFormatNameA_addr ;

typedef int (WINAPI* _GetClipboardFormatNameW_) (UINT, LPWSTR, int);
extern _GetClipboardFormatNameW_ g_GetClipboardFormatNameW_addr ;

typedef BOOL (WINAPI* _ExitWindowsEx_)(UINT,DWORD);
extern _ExitWindowsEx_ g_ExitWindowsEx_addr ;

typedef BOOL (WINAPI* _EndTask_)(HWND,BOOL,BOOL);
extern _EndTask_ g_EndTask_addr ;

typedef LONG (WINAPI* _SetWindowLongW_) (HWND, int, LONG);
extern _SetWindowLongW_ g_SetWindowLongW_addr ;

typedef LONG (WINAPI* _SetWindowLongA_) (HWND, int, LONG);
extern _SetWindowLongA_ g_SetWindowLongA_addr ;

typedef LRESULT (WINAPI* _CallWindowProcW_) (FARPROC, HWND, UINT, WPARAM, LPARAM);
extern _CallWindowProcW_ g_CallWindowProcW_addr ;

typedef LRESULT (WINAPI* _CallWindowProcA_) (FARPROC, HWND, UINT, WPARAM, LPARAM);
extern _CallWindowProcA_ g_CallWindowProcA_addr ;

typedef HWND (WINAPI* _CreateWindowExA_)(DWORD,LPCSTR,LPCSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID);
extern _CreateWindowExA_ g_CreateWindowExA_addr ;

typedef HWND (WINAPI* _CreateWindowExW_)(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID);
extern _CreateWindowExW_ g_CreateWindowExW_addr ;

typedef LRESULT (WINAPI* _DefWindowProcW_) (HWND,UINT,WPARAM,LPARAM);
extern _DefWindowProcW_ g_DefWindowProcW_addr ;

typedef int (WINAPI* _GetWindowTextA_)(HWND,LPSTR,int);
extern _GetWindowTextA_ g_GetWindowTextA_addr ;

typedef int (WINAPI* _GetWindowTextW_)(HWND,LPWSTR,int);
extern _GetWindowTextW_ g_GetWindowTextW_addr ;

typedef int (WINAPI* _InternalGetWindowText_)(HWND,LPWSTR,INT);
extern _InternalGetWindowText_ g_InternalGetWindowText_addr ;

typedef LRESULT (WINAPI* _DefWindowProcA_)(HWND,UINT,WPARAM,LPARAM);
extern _DefWindowProcA_ g_DefWindowProcA_addr ;

typedef UINT (WINAPI* _RealGetWindowClassW_) (HWND, LPWSTR, UINT);
extern _RealGetWindowClassW_ g_RealGetWindowClassW_addr ;

typedef UINT (WINAPI* _RealGetWindowClassA_) (HWND, LPSTR, UINT);
extern _RealGetWindowClassA_ g_RealGetWindowClassA_addr ;

typedef BOOL (WINAPI* _EnumWindows_) (WNDENUMPROC, LPARAM);
extern _EnumWindows_ g_EnumWindows_addr ;

typedef BOOL (WINAPI* _PostMessageW_) (HWND, UINT, WPARAM, LPARAM);
extern _PostMessageW_ g_PostMessageW_addr ;

typedef HHOOK (WINAPI* _SetWindowsHookExW_) (int, HOOKPROC, HINSTANCE, DWORD);
extern _SetWindowsHookExW_ g_SetWindowsHookExW_addr ;

typedef HHOOK (WINAPI* _SetWindowsHookExA_) (int, HOOKPROC, HINSTANCE, DWORD);
extern _SetWindowsHookExA_ g_SetWindowsHookExA_addr ;

typedef BOOL (WINAPI* _GetCursorPos_) (LPPOINT);
extern _GetCursorPos_ g_GetCursorPos_addr ;

typedef VOID (WINAPI* _DragFinish_) (HDROP);
extern _DragFinish_ g_DragFinish_addr ;

typedef DWORD (WINAPI* _GetWindowThreadProcessId_)( HWND, LPDWORD );
extern _GetWindowThreadProcessId_ g_GetWindowThreadProcessId_addr ;

typedef BOOL (WINAPI* _MoveWindow_)(HWND,int,int,int,int,BOOL);
extern _MoveWindow_ g_MoveWindow_addr ;

typedef BOOL (WINAPI* _SetWindowPos_)(HWND,HWND,int,int,int,int,UINT);
extern _SetWindowPos_ g_SetWindowPos_addr ;

typedef HDEVNOTIFY (WINAPI* _RegisterDeviceNotificationA_)(HANDLE,LPVOID,DWORD);
extern _RegisterDeviceNotificationA_ g_RegisterDeviceNotificationA_addr ;

typedef HDEVNOTIFY (WINAPI* _RegisterDeviceNotificationW_)(HANDLE,LPVOID,DWORD);
extern _RegisterDeviceNotificationW_ g_RegisterDeviceNotificationW_addr ;

typedef BOOL (WINAPI* _UnregisterDeviceNotification_)( HDEVNOTIFY );
extern _UnregisterDeviceNotification_ g_UnregisterDeviceNotification_addr ;

typedef ATOM (WINAPI* _RegisterClassA_)(CONST WNDCLASSA*);
extern _RegisterClassA_ g_RegisterClassA_addr ;

typedef ATOM (WINAPI* _RegisterClassW_) (CONST WNDCLASSW *lpWndClass);
extern _RegisterClassW_ g_RegisterClassW_addr ;

typedef LRESULT (WINAPI* _DispatchMessageW_) (CONST MSG *lpmsg);
extern _DispatchMessageW_ g_DispatchMessageW_addr ;

typedef ATOM (WINAPI* _RegisterClassExA_)(CONST WNDCLASSEXA*);
extern _RegisterClassExA_ g_RegisterClassExA_addr ;

typedef ATOM (WINAPI* _RegisterClassExW_)(CONST WNDCLASSEXW*);
extern _RegisterClassExW_ g_RegisterClassExW_addr ;

typedef BOOL (WINAPI* _UnregisterClassA_)(LPCSTR,HINSTANCE);
extern _UnregisterClassA_ g_UnregisterClassA_addr ;

typedef BOOL (WINAPI* _UnregisterClassW_)(LPCWSTR,HINSTANCE);
extern _UnregisterClassW_ g_UnregisterClassW_addr ;

typedef BOOL (WINAPI* _GetClassInfoA_)(HINSTANCE,LPCSTR,LPWNDCLASSA);
extern _GetClassInfoA_ g_GetClassInfoA_addr ;

typedef BOOL (WINAPI* _GetClassInfoW_)(HINSTANCE,LPCWSTR,LPWNDCLASSW);
extern _GetClassInfoW_ g_GetClassInfoW_addr ;

typedef BOOL (WINAPI* _GetClassInfoExA_)(HINSTANCE,LPCSTR,LPWNDCLASSEXA);
extern _GetClassInfoExA_ g_GetClassInfoExA_addr ;

typedef BOOL (WINAPI* _GetClassInfoExW_)(HINSTANCE,LPCWSTR,LPWNDCLASSEXW);
extern _GetClassInfoExW_ g_GetClassInfoExW_addr ;

typedef int (WINAPI* _GetClassNameA_)(HWND,LPSTR,int);
extern _GetClassNameA_ g_GetClassNameA_addr ;

typedef int (WINAPI* _GetClassNameW_)(HWND,LPWSTR,int);
extern _GetClassNameW_ g_GetClassNameW_addr ;

typedef BOOL (WINAPI* _EnumChildWindows_)(HWND,WNDENUMPROC,LPARAM);
extern _EnumChildWindows_ g_EnumChildWindows_addr ;

typedef BOOL (WINAPI* _EnumThreadWindows_)(DWORD,WNDENUMPROC,LPARAM);
extern _EnumThreadWindows_ g_EnumThreadWindows_addr ;

typedef BOOL (WINAPI* _EnumDesktopWindows_)(HDESK,WNDENUMPROC,LPARAM);
extern _EnumDesktopWindows_ g_EnumDesktopWindows_addr ;

typedef HWND (WINAPI* _FindWindowA_)(LPCSTR,LPCSTR);
extern _FindWindowA_ g_FindWindowA_addr ;

typedef HWND (WINAPI* _FindWindowW_)(LPCWSTR,LPCWSTR);
extern _FindWindowW_ g_FindWindowW_addr ;

typedef HWND (WINAPI* _FindWindowExA_)(HWND,HWND,LPCSTR,LPCSTR);
extern _FindWindowExA_ g_FindWindowExA_addr ;

typedef HWND (WINAPI* _FindWindowExW_)(HWND,HWND,LPCWSTR,LPCWSTR);
extern _FindWindowExW_ g_FindWindowExW_addr ;

typedef HWND (WINAPI* _GetShellWindow_)(void);
extern _GetShellWindow_ g_GetShellWindow_addr ;

typedef HANDLE (WINAPI* _GetPropA_)(HWND, LPCSTR);
extern _GetPropA_ g_GetPropA_addr ;

typedef HANDLE (WINAPI* _RemovePropA_)(HWND,LPCSTR);
extern _RemovePropA_ g_RemovePropA_addr ;

typedef HANDLE (WINAPI* _RemovePropW_)(HWND, LPCWSTR);
extern _RemovePropW_ g_RemovePropW_addr ;

typedef LRESULT (WINAPI* _SendMessageA_)(HWND,UINT,WPARAM,LPARAM);
extern _SendMessageA_ g_SendMessageA_addr ;

typedef LRESULT (WINAPI* _SendMessageW_)(HWND,UINT,WPARAM,LPARAM);
extern _SendMessageW_ g_SendMessageW_addr ;

typedef LRESULT (WINAPI* _SendMessageTimeoutA_)(HWND,UINT,WPARAM,LPARAM,UINT,UINT,PDWORD);
extern _SendMessageTimeoutA_ g_SendMessageTimeoutA_addr ;

typedef LRESULT (WINAPI* _SendMessageTimeoutW_)(HWND,UINT,WPARAM,LPARAM,UINT,UINT,PDWORD);
extern _SendMessageTimeoutW_ g_SendMessageTimeoutW_addr ;

typedef BOOL (WINAPI* _SendNotifyMessageA_)(HWND,UINT,WPARAM,LPARAM);
extern _SendNotifyMessageA_ g_SendNotifyMessageA_addr ;

typedef BOOL (WINAPI* _SendNotifyMessageW_)(HWND,UINT,WPARAM,LPARAM);
extern _SendNotifyMessageW_ g_SendNotifyMessageW_addr ;

typedef BOOL (WINAPI* _PostMessageA_)(HWND,UINT,WPARAM,LPARAM);
extern _PostMessageA_ g_PostMessageA_addr ;

typedef LONG (WINAPI* _DispatchMessageA_)(const MSG*);
extern _DispatchMessageA_ g_DispatchMessageA_addr ;

typedef BOOL (WINAPI* _UnhookWindowsHookEx_)(HHOOK);
extern _UnhookWindowsHookEx_ g_UnhookWindowsHookEx_addr ;

typedef HWND (WINAPI* _CreateDialogParamA_)(HINSTANCE,LPCSTR,HWND,DLGPROC,LPARAM);
extern _CreateDialogParamA_ g_CreateDialogParamA_addr ;

typedef HWND (WINAPI* _CreateDialogParamW_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern _CreateDialogParamW_ g_CreateDialogParamW_addr ;

typedef HWND (WINAPI* _CreateDialogIndirectParamA_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern _CreateDialogIndirectParamA_ g_CreateDialogIndirectParamA_addr ;

typedef HWND (WINAPI* _CreateDialogIndirectParamW_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern _CreateDialogIndirectParamW_ g_CreateDialogIndirectParamW_addr ;

typedef HWND (WINAPI* _CreateDialogIndirectParamAorW_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM, DWORD);
extern _CreateDialogIndirectParamAorW_ g_CreateDialogIndirectParamAorW_addr ;

typedef int (WINAPI* _DialogBoxParamA_)(HINSTANCE,LPCSTR,HWND,DLGPROC,LPARAM);
extern _DialogBoxParamA_ g_DialogBoxParamA_addr ;

typedef int (WINAPI* _DialogBoxParamW_)(HINSTANCE,LPCWSTR,HWND,DLGPROC,LPARAM);
extern _DialogBoxParamW_ g_DialogBoxParamW_addr ;

typedef int (WINAPI* _DialogBoxIndirectParamA_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern _DialogBoxIndirectParamA_ g_DialogBoxIndirectParamA_addr ;

typedef int (WINAPI* _DialogBoxIndirectParamW_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern _DialogBoxIndirectParamW_ g_DialogBoxIndirectParamW_addr ;

typedef INT_PTR (WINAPI* _DialogBoxIndirectParamAorW_)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM, DWORD);
extern _DialogBoxIndirectParamAorW_ g_DialogBoxIndirectParamAorW_addr ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL GetUser32dllFuncAddrs( IN HMODULE hModule );


///////////////////////////////   END OF FILE   ///////////////////////////////
