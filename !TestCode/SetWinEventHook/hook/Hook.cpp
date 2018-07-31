#define WX_EXPORTS
#define  WINVER 0x500
#include <windows.h>
#include <Winuser.h>
#include <Windef.h>
#include "Hook.h"

HMODULE hMyModule;
HWINEVENTHOOK	g_Hook =NULL;

VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook,
  DWORD event,
  HWND hwnd,
  LONG idObject,
  LONG idChild,
  DWORD dwEventThread,
  DWORD dwmsEventTime)
{
	HWND hwnd1 = FindWindow(NULL,"驱动加载工具 V1.3版");
	DWORD Pid;

	if (hwnd1)
	{
		GetWindowThreadProcessId(hwnd1,&Pid);
		if (Pid == GetCurrentProcessId())
		{
			ExitProcess(0);
		}
	}
}

WX_API DWORD WINAPI InstallHook()
{
	DWORD dwResult = 0;

	g_Hook = SetWinEventHook(EVENT_MIN,EVENT_MAX,hMyModule,(WINEVENTPROC)WinEventProc,0,0,WINEVENT_INCONTEXT | WINEVENT_SKIPOWNPROCESS);
	if (g_Hook == NULL)
	{
		dwResult = GetLastError();
	}

	return dwResult;
}

WX_API DWORD WINAPI UnInstallHook()
{
	DWORD dwResult = 0;

	if (UnhookWinEvent(g_Hook) == 0)
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
