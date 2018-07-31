#pragma once

//////////////////////////////////////////////////////////////////////////

// fake_UnhookWindowsHookEx
typedef struct tagSetWindowsHookExInfo // size - 0x18
{ 
/*0x000 */ struct tagSetWindowsHookExInfo * pFlink ;		// 上个节点
/*0x004 */ struct tagSetWindowsHookExInfo * pBlink ;		// 下个节点   
/*0x008 */ HHOOK xx_hook ; // 保存调用函数SetWindowsHookExW(A) 后得到的结果
/*0x00C */ DWORD dwThreadId ;
/*0x010 */ FILETIME CreationTime ;

} SetWindowsHookEx_Little_Info, *LPSetWindowsHookEx_Little_Info ;


typedef struct _SetWindowsHookExInfo_  // size - 0x40
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ ULONG FuckedTag ;
/*0x00C*/ CRITICAL_SECTION Lock ;
/*0x024*/ int idHook ;
/*0x028*/ HOOKPROC lpfn ;
/*0x02C*/ HINSTANCE hMod ;
/*0x030*/ BOOL bIsUnicode ;
/*0x034*/ LIST_ENTRY_EX LittleNode ;

} SetWindowsHookExInfo, *LPSetWindowsHookExInfo ;


typedef struct _SANDBOXNAMEINFO_ 
{
	/*0x000*/
	struct  
	{
		LPSTR szName;
		ULONG  NameLength;
	}SClasseNameA;
	/*0x008*/ 
	struct  
	{
		LPWSTR szName;
		ULONG  NameLength;
	}SClasseNameW; 
	/*0x010*/ 
	struct  
	{
		LPSTR szName;
		ULONG  NameLength;
	}STitleNameA; 
	/*0x018*/ 
	struct  
	{
		LPWSTR szName;
		ULONG  NameLength;
	}STitleNameW; 

} SANDBOXNAMEINFO, *LPSANDBOXNAMEINFO ;


// HandlerBroadcastMessage
typedef struct _BroadcastMessageInfo_ 
{
/*0x000*/ UINT Msg ;
/*0x004*/ UINT wParam ;
/*0x008*/ UINT lParam ;
/*0x00C*/ ULONG FuncAddr1 ;
/*0x010*/ ULONG FuncAddr2 ;
/*0x014*/ HWND hWnd ;
/*0x018*/ int uTimeout ;
/*0x01C*/ int lpdwResult ;

} BroadcastMessageInfo, *LPBroadcastMessageInfo ;


typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO { // size - 0x10
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;  // 0x01 = PROTECT_FROM_CLOSE, 0x02 = INHERIT
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;


typedef struct _MYSYSTEM_HANDLE_INFORMATION 
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[ 1 ];

} MYSYSTEM_HANDLE_INFORMATION, *LPMYSYSTEM_HANDLE_INFORMATION;


typedef struct tagEnumWindowsArgumentInfo 
{
/*0x000 */	WNDENUMPROC lpEnumFunc ;
/*0x004 */  LPARAM lParam ;

} EnumWindows_Argument_Info, *PEnumWindows_Argument_Info ;


//
// 对话框创建时分2中: 普通 & 扩展
//

// (A) 普通
#pragma pack(1)
// typedef struct tagDLGTEMPLATE 
// { 
// /*0x000 */  DWORD style;
// /*0x004 */  DWORD dwExtendedStyle;
// /*0x008 */	WORD cdit;
// /*0x00A */	short x;
// /*0x00C */	short y;
// /*0x00E */	short cx;
// /*0x010 */	short cy;
// 
// } DLGTEMPLATE, *LPDLGTEMPLATE;


// (B) 扩展
typedef struct tagDLGTEMPLATEEX { // size - 0x1A

/*0x000 */	WORD    wDlgVer;
/*0x002 */	WORD    wSignature;
/*0x004 */	DWORD   dwHelpID;
/*0x008 */	DWORD   dwExStyle;
/*0x00C */	DWORD   dwStyle;
/*0x010 */	WORD    cDlgItems;
/*0x012 */	short   x;
/*0x014 */	short   y;
/*0x016 */	short   cx;
/*0x018 */	short   cy;

} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#pragma pack() 


extern ULONG g_offset_jin_Unicode_Length;
extern ULONG g_offset_jin_Ansi_Length;
extern BOOL g_SandboxTitle_NO;
extern BOOL g_SandboxTitle_NO_Ex;
extern LIST_ENTRY_EX g_NodeHead_hhk ;
extern CRITICAL_SECTION g_Lock_Msg ;
extern SANDBOXNAMEINFO g_SandboxNameInfo;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

ULONG WINAPI fake_EnumWindows( WNDENUMPROC lpEnumFunc, LPARAM lParam );

LPSTR RedirectClassNameA( LPSTR szName );
LPWSTR RedirectClassNameW( LPWSTR szName );
LPWSTR RedirectWindowNameW( IN LPWSTR lpWindowName );
LPSTR  RedirectWindowNameA( IN LPSTR lpWindowName );
int GetOrigWindowTitleW (HWND hWnd, LPWSTR lpString, int StringLength);
ULONG GetOrigWindowTitleA ( HWND hWnd,LPSTR lpString,int StringLength);
BOOL IshWndCaptionTitle (IN HWND hWnd );
void HandlerAtoms();
VOID HandlerWindowLong (HWND hWnd,BOOL bFlag );
VOID WalkMsghWnd (LPARAM lParam );
BOOL IsWhitehWnd( HWND hWnd );
BOOL IsWhitehWndEx (HWND hWnd,ULONG *pdwProcessId,ULONG *pdwThreadId);
BOOL IsWhiteClassName( LPWSTR szClassName );
NTSTATUS FindWindowFilter( PVOID lpClassName, ULONG Flag, HWND hWnd );
NTSTATUS FindWindowFilterEx( LPCWSTR lpClassName, ULONG Flag, HWND hWnd );
LPCWSTR GetOrigClassName( LPCWSTR lpClassName );
PVOID RedirectPropString( ULONG lpString );
BOOL Handler_SendMessageW_WM_CAP_DRIVER_DISCONNECT( LPWSTR szName, HWND hWnd, int Msg, WPARAM wParam, PULONG pRet );
ULONG HandlerBroadcastMessage( HWND hWnd, PVOID FuncAddr, UINT Msg, UINT wParam, UINT lParam, int uTimeout, int lpdwResult );
BOOL CALLBACK EnumWndProc_broadcastMessage( HWND hWnd, LPBroadcastMessageInfo pBuffer );
VOID ClearHandlersCache( BOOL bFlag );
VOID ClearHandlersCacheEx( BOOL bFlag, int TickCount );
ULONG GetTargetNameCounts (LPWSTR szTargetName,LPWSTR lpFileName,LPMYSYSTEM_HANDLE_INFORMATION HandleInfo,PBYTE pObjectTypeIndex);
PVOID DIALOG_ParseTemplate32 ( PVOID lpDlgTemplate );
PVOID DIALOG_ParseTemplate32Ex( PVOID lpDlgTemplate );
PBYTE SkipIDorString( LPBYTE pb );
PBYTE CollectDialogHeader( PVOID pdt );
PBYTE CollectDialogChildrenControls(LPBYTE pb,ULONG nCounts);
VOID Copy_and_move (LPBYTE *ptrDest,LPBYTE *ptrSrc,ULONG size);
VOID BuildDialogTemplate ( PVOID lpDlgTemplate_new,PVOID lpDlgTemplate_old );
HHOOK SetWindowsHookExFilter( DWORD idHook, HOOKPROC lpfn, HINSTANCE hMod, BOOL bIsUnicode );
BOOL  SetWindowsHookExFilterEx (PVOID Data,DWORD dwThreadId);
BOOL CALLBACK EnumWndProc_InsertTID( HWND hWnd, LPARAM lParam );
HMODULE Get_hModule_user32DLL();
LRESULT CALLBACK AddressBarProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AddressBarProcA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL AddressBarProcFilter (UINT wMsg,LPARAM lParam,WPARAM wParam,HWND hWnd);
BOOL CALLBACK EnumWindowsProc (HWND hWnd,LPARAM lParam);
BOOL HandlerDropfiles (HWND hWnd,WPARAM wParam);


///////////////////////////////   END OF FILE   ///////////////////////////////
