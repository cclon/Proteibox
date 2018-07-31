/////////////////////////////////////////////////
// FirstDialog.cpp文件



#include <windows.h>
#include "resource.h"

BOOL __stdcall DlgProc(HWND, UINT, WPARAM, LPARAM);

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	int nResult = ::DialogBoxParam(
		hInstance,		// 实例句柄
		(LPCTSTR)IDD_MAIN,	// 对话框资源ID号
		NULL,			// 父窗口句柄
		DlgProc,		// 消息处理函数
		NULL); 			// 对话框初始化的值，在WM_INITDIALOG消息的lParam参数中取出
	
// 	if(nResult == IDOK)
// 		::MessageBox(NULL, "用户选择了OK按钮", "07FirstDialog", MB_OK);
// 	else
// 		::MessageBox(NULL, "用户选择了CANCEL按钮", "07FirstDialog", MB_OK);

	return 0;
}

BOOL __stdcall DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{	
	case WM_INITDIALOG: // 初始化对话框
	//	::SetWindowText(hDlg,"第一个对话框！");
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			::EndDialog(hDlg, IDOK);
			break;
		case IDCANCEL:
			::EndDialog (hDlg, IDCANCEL);
			break;
		}
		break;
	}
	return 0;
}


