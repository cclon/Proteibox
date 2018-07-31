#ifdef WX_EXPORTS
#define WX_API __declspec(dllexport)
#else
#define WX_API __declspec(dllimport)
#endif

WX_API VOID WINAPI Test();


DWORD FindProcessID(char* szName) ;
VOID RemoteThread(char* szName) ;