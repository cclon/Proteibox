#if !defined(_outputdebug_H_)
#define _outputdebug_H_

#include <wtypes.h>

// 可以在Release版本中单独打开此开关，此时注意自行关闭

#ifdef _DEBUG
#define _MYDEBUG
#endif

#ifdef _MYDEBUG

#define _MYDEBUGFLAG_A "[PB] "
#define _MYDEBUGFLAG_W L"[PB] "

//void _cdecl MyAtlTraceW(LPCWSTR lpszFormat, ...);

void _cdecl MyAtlTraceA(LPCSTR lpszFormat, ...);

#ifdef _UNICODE
    #define MYTRACE MyAtlTraceW
#else
    #define MYTRACE MyAtlTraceA
#endif

#else//////////////

#define MyAtlTraceA (void)0
#define MyAtlTraceW (void)0
#define MYTRACE (void)0

#endif

#endif // !defined(_outputdebug_H_)