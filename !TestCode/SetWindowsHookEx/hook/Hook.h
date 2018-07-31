#ifdef WX_EXPORTS
#define WX_API __declspec(dllexport)
#else
#define WX_API __declspec(dllimport)
#endif

WX_API DWORD WINAPI InstallHook();
WX_API DWORD WINAPI UnInstallHook();