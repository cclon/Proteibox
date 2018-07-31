#pragma once
#ifndef _parsecmd_h_
#define _parsecmd_h_

//////////////////////////////////////////////////////////////////////////

extern HANDLE g_new_hToken ;
extern HANDLE g_new_ImpersonationToken ;
extern BOOL g_bFlag_system ;
extern BOOL g_bFlag_Start_RunAsAdmin ;
extern LPWSTR g_lpCurrentDirectory ;

//////////////////////////////////////////////////////////////////////////

class CParseCMD  
{
public:
	static CParseCMD& GetInstance()
	{
		static CParseCMD _instance;
		return _instance;
	}

	CParseCMD();
	virtual ~CParseCMD();

	BOOL ParaseCommandLine();
	BOOL StartPBCtrl();
	BOOL VistaRunAsAdmin( IN BOOL bWork );
	BOOL InitProcess();
	LPWSTR GetCommandLineWEx( IN LPWSTR lpCmdLine );
	BOOL INIGetConf( IN LPWSTR Key );

protected:
	LPWSTR ParseInternetShortcutURL( IN LPWSTR lpData );
	BOOL ParaseCommandLine_internet_explorer( IN LPWSTR lpCmdLineL );
	BOOL ParaseCommandLine_default_browser( IN LPWSTR lpCmdLineL );
	BOOL ParaseCommandLine_mail_agent( IN LPWSTR lpCmdLineL );
	BOOL ParaseCommandLine_delete_sandbox( IN LPWSTR lpCmdLineL );
	VOID DeleteSandbox( IN BOOL bSilent, IN ULONG phrase );
	VOID DeleteSandbox_phrase0();
	VOID DeleteSandbox_phrase1();
	VOID DeleteSandbox_phrase2();
	VOID DeleteSandbox_phrase2_dep( IN LPWSTR szBoxName );
	VOID DeleteSandbox_phrase2_depth( IN LPWSTR szBoxPath );
	LPWSTR Split_joint_filePath( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort );
	BOOL SetFileNormalAttribute( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort );
	BOOL HandlerFreakFile( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort, LPWSTR szBoxPath );
	VOID WaitforFiletobeFree( IN LPWSTR szPath, IN int nCounts );
	LPWSTR QueryBoxPath( IN LPWSTR szBoxName );
	BOOL CreateProcess2DeleteSandbox( IN LPWSTR lpData, IN BOOL bWait );
	BOOL DisableForceProcess( IN LPWSTR lpCmdLineL );
	BOOL GetForcePath( IN LPWSTR lpCmdLineL, OUT LPWSTR* szForcePath );

public:
	BOOL m_bNoPBCtrl ; // 启动沙箱进程时,是否启动界面程序PBCtrl.exe
	BOOL m_bIsSystem ; // 
	BOOL m_bIsElevate ; //
	BOOL m_bNeedStartCOM ;
	BOOL m_bSilent ;

	HANDLE m_hEvent_DeleteSandbox ;

protected:

};

extern LPWSTR g_CommandLine_strings ;

//////////////////////////////////////////////////////////////////////////
#endif 
