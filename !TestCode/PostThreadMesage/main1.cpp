#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD pid;
	HWND hWnd = FindWindow(NULL, "驱动加载工具 V1.3版");
	DWORD tid = GetWindowThreadProcessId( hWnd, &pid);

	PostThreadMessage(tid,WM_QUIT,0,0);

	return 0;
}