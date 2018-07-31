#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

//////////////////////////////////////////////////////////////////////////

#define EWX_SHUTDOWN_REBOOT_POWEROFF 0xB


//////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ExitWindowsEx( EWX_SHUTDOWN_REBOOT_POWEROFF, 0 );
	return 0;
}
