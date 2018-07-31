#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

//////////////////////////////////////////////////////////////////////////

char g_szName1 [100] = "Driver Logical Tester" ;
char g_szName2 [100] = "[#] Driver Logical Tester [#]" ;


//////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CHAR cls[128];
	
	MessageBox( NULL, "Go!", "- -", MB_OK );

	HWND hWnd = FindWindow( NULL, g_szName1 );
	if(hWnd == NULL)
		return 0;

	GetClassName( hWnd, cls, 128 );
	MessageBox( NULL, cls, "GetClassName", MB_OK );

	HWND hParent = CreateWindow("BUTTON","Test",0,0,0,100,100,GetDesktopWindow(),0,hInstance,NULL);
	SetParent(hWnd,hParent);

	DestroyWindow(hParent);
	return 0;
}
