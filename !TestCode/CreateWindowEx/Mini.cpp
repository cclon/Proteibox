// Mini.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <tlhelp32.h>


// 消息处理函数
BOOL CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}


HWND hDialog;
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HINSTANCE   hInst = GetModuleHandle( NULL );

// 	hDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc);
// 	MoveWindow(hDialog,1,1,1,1,TRUE);

	HWND hwndFullScreen = CreateWindowEx(
		WS_EX_TOPMOST,
		TEXT("Sandbox__DefaultBox__:sudami"),
		TEXT("kvp [#]"),
		WS_POPUP | WS_CAPTION,
		0, 0, 0, 0,
		NULL, 
		NULL,
		hInst, 
		NULL 
		);

	MoveWindow( hDialog, 100, 100, 150 , 150, TRUE );

	//----------------------------------------------------------------------------------------
	ShowWindow(hDialog, nCmdShow);
	UpdateWindow(hDialog);
	ShowCursor( FALSE );
    SetFocus( hwndFullScreen );
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}




