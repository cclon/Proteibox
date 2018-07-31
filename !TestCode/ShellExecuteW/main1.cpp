#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ShellExecuteW( NULL, L"explore", L"D:\\WinRAR", NULL, NULL, SW_SHOWNORMAL );
	return 0;
}
