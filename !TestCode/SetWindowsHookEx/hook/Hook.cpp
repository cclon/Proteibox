#define WX_EXPORTS
#define  _WIN32_WINNT 0x410
#include <windows.h>
#include "Hook.h"

HMODULE hMyModule;
HHOOK	g_Hook =NULL;

DWORD WINAPI CallWndProcW(int nCode,WPARAM wParam,LPARAM lParam)
{
	HWND hwnd = FindWindow(NULL,"驱动加载工具 V1.3版");
	DWORD Pid;

	if (hwnd)
	{
		GetWindowThreadProcessId(hwnd,&Pid);
		if (Pid == GetCurrentProcessId())
		{
			ExitProcess(0);
		}
	}
	return CallNextHookEx(g_Hook,nCode,wParam,lParam);
}

WX_API DWORD WINAPI InstallHook()
{
	DWORD dwResult = 0;

	g_Hook = SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)CallWndProcW,hMyModule,0);
	if (g_Hook == NULL)
	{
		dwResult = GetLastError();
	}

	return dwResult;
}

WX_API DWORD WINAPI UnInstallHook()
{
	DWORD dwResult = 0;

	if (UnhookWindowsHookEx(g_Hook) == 0)
	{
		dwResult = GetLastError();
	}
	
	return dwResult;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hMyModule = hModule;
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}
