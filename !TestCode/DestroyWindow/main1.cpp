#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

//////////////////////////////////////////////////////////////////////////

#define ARRAYSIZEOF(x) sizeof (x) / sizeof (x[0])

static LPSTR g_Array[] = 
{
	"驱动加载工具 V1.3版",
	"打开",
	"Driver Logical Tester",
};


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd = NULL ;

	for ( int i=0; i<ARRAYSIZEOF(g_Array); i++ )
	{
		hwnd = FindWindow( NULL, g_Array[i] );
		
		if (hwnd)
		{
			SetForegroundWindow( hwnd );
			DestroyWindow( hwnd ) ;
		}
	}

	return 0;
}
