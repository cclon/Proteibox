#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD dwProcessID = 0;

	typedef int(__stdcall * _INSTALLHOOK)();
	typedef int(__stdcall * _UNINSTALLHOOK)();

	HMODULE hHookMod;
	_INSTALLHOOK InstallHook;
	_UNINSTALLHOOK UnInstallHook;

	HWND hwnd = FindWindow(NULL,"驱动加载工具 V1.3版");

	if (hwnd)
	{
		hHookMod = LoadLibrary("SetWinEventHook.dll");
		InstallHook = (_INSTALLHOOK)GetProcAddress(hHookMod,"InstallHook");
		UnInstallHook = (_UNINSTALLHOOK)GetProcAddress(hHookMod,"UnInstallHook");

		InstallHook();
		
		SetForegroundWindow(hwnd);
		while (FindWindow(NULL,"驱动加载工具 V1.3版"))
		{
			Sleep(100);
		}
		UnInstallHook();
	}

	return 0;
}
