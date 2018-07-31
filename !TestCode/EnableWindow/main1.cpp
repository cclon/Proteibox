#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd = FindWindow(NULL,"驱动加载工具 V1.3版");

	if (hwnd)
	{
		SetForegroundWindow(hwnd);

		EnableWindow( hwnd, FALSE ) ; // 禁止当前窗口
	}

	return 0;
}
