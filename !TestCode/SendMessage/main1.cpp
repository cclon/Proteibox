#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INPUT inputs[4] = {0};
	DWORD dwPid;
	HANDLE hProc;

	HWND hwnd = FindWindow(NULL,"驱动加载工具 V1.3版");

	if (hwnd)
	{
		SetForegroundWindow(hwnd);

		SendMessage(hwnd,WM_CLOSE,0,0);

		while (FindWindow(NULL,"驱动加载工具 V1.3版"))
		{
			Sleep(100);
		}

	}

	return 0;
}

		/*
		hProc = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION,0,dwPid);
		CreateRemoteThread(hProc,NULL,0,(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"),"ExitProcess"),NULL,0,NULL);

		SendMessage(hwnd,WM_CLOSE,0,0);
		SendNotifyMessage(hwnd,WM_CLOSE,0,0);
		SendMessageTimeout(hwnd,WM_CLOSE,0,0,SMTO_NORMAL,2000,NULL);
		SendMessageCallback(hwnd,WM_CLOSE,0,0,NULL,0);
		PostMessage(hwnd,WM_CLOSE,0,0);

		SendMessage(hwnd,WM_SYSCOMMAND,SC_CLOSE,0);
		SendNotifyMessage(hwnd,WM_SYSCOMMAND,SC_CLOSE,0);
		SendMessageTimeout(hwnd,WM_SYSCOMMAND,SC_CLOSE,0,SMTO_NORMAL,2000,NULL);
		SendMessageCallback(hwnd,WM_SYSCOMMAND,SC_CLOSE,0,NULL,0);
		PostMessage(hwnd,WM_SYSCOMMAND,SC_CLOSE,0);

		keybd_event(VK_RETURN,0,0,0);
		keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);

		keybd_event(VK_ESCAPE,0,0,0);
		keybd_event(VK_ESCAPE,0,KEYEVENTF_KEYUP,0);

		keybd_event(VK_MENU,0,0,0);
		keybd_event(VK_F4,0,0,0);
		keybd_event(VK_F4,0,KEYEVENTF_KEYUP,0);
		keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);

		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = VK_RETURN;
		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.wVk = VK_RETURN;
		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(2,inputs,sizeof(INPUT));
		*/
