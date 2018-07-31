#include "stdafx.h"
#include "CLoadDriver_2.h"

#include <windows.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib,"user32.lib") 

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

CDriver::CDriver(LPCTSTR pszDriverPath, LPCTSTR pszLinkName)
{
	strncpy(m_szLinkName, pszLinkName, 55);
	m_bStarted = FALSE;
	m_bCreateService = FALSE;
	m_hSCM = m_hService = NULL;
	m_hDriver = INVALID_HANDLE_VALUE;


	// 安全模式的问题

	//修改SafeBoot,使得安全模式下也可以加载

	HKEY hLicenses = NULL;
	HKEY RegKey;
	DWORD disp;
	LONG Regrt = RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
		0,
		KEY_ALL_ACCESS,
		&hLicenses );

	strcpy(m_szSysName,strrchr(pszDriverPath,'\\')+1);

	Regrt = RegCreateKeyEx (
		hLicenses,
		m_szSysName,
		0,
		"",
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&RegKey,
		&disp );
	CloseHandle(hLicenses);

	// 打开SCM管理器
	m_hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_hSCM == NULL)
	{
		MessageBox(0, "打开服务控制管理器失败\n", 
			"可能是因为您不拥有Administrator权限\n", 0);
		return;
	}

	// 创建或打开服务
	m_hService = ::CreateService((SC_HANDLE)m_hSCM, m_szLinkName, m_szLinkName, SERVICE_ALL_ACCESS, 
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
		pszDriverPath, NULL, 0, NULL, NULL, NULL);
	if(m_hService == NULL)
	{

		// 创建服务失败，可能是因为服务已经存在，所以还要试图打开它
		int nError = ::GetLastError();
		if(nError == ERROR_SERVICE_EXISTS || nError == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			m_hService = ::OpenService((SC_HANDLE)m_hSCM, m_szLinkName, SERVICE_ALL_ACCESS);
		}
	}
	else
	{
		m_bCreateService = TRUE;
	}
}

CDriver::~CDriver()
{

	// 关闭设备句柄
	if(m_hDriver != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hDriver);
	// 如果创建了服务，就将之删除
	if(m_bCreateService)
	{
		StopDriver();
		::DeleteService((SC_HANDLE)m_hService);	
	}
	// 关闭句柄
	if(m_hService != NULL)
		::CloseServiceHandle((SC_HANDLE)m_hService);
	if(m_hSCM != NULL)
		::CloseServiceHandle((SC_HANDLE)m_hSCM);

}

BOOL CDriver::StartDriver()
{
	if(m_bStarted)
		return TRUE;
	if(m_hService == NULL)
	{
		return FALSE;
	}

	char szShow[256];
	wsprintf(szShow,"%X",m_hService);




	// 启动服务
	if(!::StartService((SC_HANDLE)m_hService, 0, NULL))
	{
		int nError = ::GetLastError();
		if(nError == ERROR_SERVICE_ALREADY_RUNNING)
			m_bStarted = TRUE;
		else
			::DeleteService((SC_HANDLE)m_hService);
	}
	else
	{
		// 启动成功后，等待服务进入运行状态
		int nTry = 0;
		SERVICE_STATUS ss;
		::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		while(ss.dwCurrentState == SERVICE_START_PENDING && nTry++ < 80)
		{
			::Sleep(50);
			::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		}
		if(ss.dwCurrentState == SERVICE_RUNNING)
			m_bStarted = TRUE;
	}


	//驱动加载完成,删除注册表中相关内容

	HKEY hKey;
	::RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services",&hKey);
	HKEY hSubKey;
	::RegOpenKey(hKey,m_szLinkName,&hSubKey);
	::RegDeleteKey(hSubKey,"Enum");
	::CloseHandle(hSubKey);
	RegDeleteKey(hKey,m_szLinkName);

	HKEY hSafeBoot;
	::RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",&hSafeBoot);
	::RegDeleteKey(hSafeBoot,m_szSysName);
	::CloseHandle(hSafeBoot);


	return m_bStarted;
}

BOOL CDriver::StopDriver()
{
	if(!m_bStarted)
		return TRUE;
	if(m_hService == NULL)
		return FALSE;
	// 停止服务
	SERVICE_STATUS ss;
	if(!::ControlService((SC_HANDLE)m_hService, SERVICE_CONTROL_STOP, &ss))
	{
		if(::GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
			m_bStarted = FALSE;
	}
	else
	{
		// 等待服务完全停止运行
		int nTry = 0;
		while(ss.dwCurrentState == SERVICE_STOP_PENDING && nTry++ < 80)
		{
			::Sleep(50);
			::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		}
		if(ss.dwCurrentState == SERVICE_STOPPED)
			m_bStarted = FALSE;
	}
	return !m_bStarted;
}

BOOL CDriver::OpenDevice()
{
	if(m_hDriver != INVALID_HANDLE_VALUE)
		return TRUE;

	// "\\.\"是Win32中定义本地计算机的方法，
	// m_szLinkName是设备对象的符号连接名称，后面章节会详细讨论
	char sz[256] = "";
	wsprintf(sz, "\\\\.\\%s", m_szLinkName);
	// 打开驱动程序所控制设备
	m_hDriver = ::CreateFile(sz,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return (m_hDriver != INVALID_HANDLE_VALUE);
}



DWORD CDriver::IoControl(DWORD nCode, PVOID pInBuffer, 
						 DWORD nInCount, PVOID pOutBuffer, DWORD nOutCount)
{
	if(m_hDriver == INVALID_HANDLE_VALUE)
		return -1;
	// 向驱动程序发送控制代码
	DWORD nBytesReturn;
	BOOL bRet = ::DeviceIoControl(m_hDriver, nCode, 
		pInBuffer, nInCount, pOutBuffer, nOutCount, &nBytesReturn, NULL);
	if(bRet)
		return nBytesReturn;
	else
		return -1;
}

BOOL CDriver::MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return FALSE;
	}


	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);  
	return TRUE;
}

BOOL CDriver::WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if(dwSize < dwMinSize)
	{
		return FALSE;
	}
	WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,FALSE);
	return TRUE;
}
