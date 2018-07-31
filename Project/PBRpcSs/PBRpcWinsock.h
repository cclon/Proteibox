#pragma once


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef enum _WINSOCKTAG_ 
{
	bind_TAG = 0,
	listen_TAG ,
	WSASocketW_TAG ,
	OpenThreadToken_TAG ,
	RegOpenKeyExW_TAG ,
	RegQueryValueExW_TAG ,

};



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL InitHook_pharase2_WinSock();

ULONG WINAPI
fake_bind ( 
	SOCKET s, 
	const struct sockaddr FAR* name,
	int namelen
	);

ULONG WINAPI
fake_listen (
	SOCKET s,
	int backlog
	);

ULONG WINAPI
fake_WSASocketW (
	IN  INT af,
    IN  INT type,
    IN  INT protocol,
    IN  LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN  GROUP g,
    IN  DWORD dwFlags
	);

ULONG WINAPI
fake_OpenThreadToken (
	HANDLE ThreadHandle, 
	int DesiredAccess, 
	BOOL OpenAsSelf, 
	PHANDLE TokenHandle
	);

typedef ULONG (WINAPI* _OpenThreadToken_) (
	HANDLE ThreadHandle, 
	int DesiredAccess, 
	BOOL OpenAsSelf, 
	PHANDLE TokenHandle
	);

ULONG WINAPI
fake_RegOpenKeyExW (
	HKEY hKey,
	LPCWSTR lpSubKey,
	DWORD ulOptions,
	REGSAM samDesired,
	PHKEY phkResult
	);

typedef ULONG (WINAPI* _RegOpenKeyExW_)(
	HKEY hKey,
	LPCWSTR lpSubKey,
	DWORD ulOptions,
	REGSAM samDesired,
	PHKEY phkResult
	);

ULONG WINAPI
fake_RegQueryValueExW (
	HKEY hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData
	);

typedef ULONG (WINAPI* _RegQueryValueExW_)(
	HKEY hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData
	);


///////////////////////////////   END OF FILE   ///////////////////////////////
