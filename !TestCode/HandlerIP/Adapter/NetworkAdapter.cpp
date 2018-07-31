//////////////////////////////////////////////////////////////////////////
// 文件名：NetworkAdapter.cpp
// 功能描述：网络适配器接口实现

#include "StdAfx.h"
#include "NetworkAdapter.h"
#include <Iphlpapi.h>
#include <Setupapi.h>
#include <Regstr.h>
#include <cfgmgr32.h>   // DDK
#include <Rpc.h>

#pragma comment( lib, "Iphlpapi.lib" )
#pragma comment( lib, "Setupapi.lib" )
#pragma comment( lib, "Rpcrt4.lib" )

#ifdef UNICODE
#define RPC_TSTR    RPC_WSTR
#else
#define RPC_TSTR    RPC_CSTR
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////

typedef struct _DeviceInfoSet_Personal_ /* HDEVINFO */
{
	DWORD magic; /* SETUP_DEV_INFO_SET_MAGIC */
	/* If != GUID_NULL, only devices of this class can be in the device info set */
	GUID ClassGuid;
	/* Local or distant HKEY_LOCAL_MACHINE registry key */
	HKEY HKLM;
	/* Used when dealing with CM_* functions */
	HMACHINE hMachine;

	/* Reserved Field points to a struct DriverInfoElement */
	SP_DEVINSTALL_PARAMS_W InstallParams;

	/* List of struct DriverInfoElement (if no driver has been
	* searched/detected, this list is empty) */
	LIST_ENTRY DriverListHead;

	/* List of struct DeviceInfoElement */
	LIST_ENTRY ListHead;
// 	struct DeviceInfoElement *SelectedDevice;
// 
// 	/* Used by SetupDiGetClassInstallParamsW/SetupDiSetClassInstallParamsW */
// 	struct ClassInstallParams ClassInstallParams;
// 
// 	/* Contains the name of the remote computer ('\\COMPUTERNAME' for example),
// 	* or NULL if related to local machine. Points into szData field at the
// 	* end of the structure */
// 	PCWSTR MachineName;
// 
// 	/* Variable size array (contains data for MachineName) */
// 	WCHAR szData[ANYSIZE_ARRAY];
} DeviceInfoSet_Personal, *PDeviceInfoSet_Personal ;


//////////////////////////////////////////////////////////////////////////



// 全局函数
BOOL ChangeStatus( DWORD dwNewStatus, DWORD dwSelectedItem, HDEVINFO hDevInfo );
BOOL ControlAdapter( DWORD dwStatus );
BOOL GetDeviceRegistryProperty( HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDeviceInfoData, DWORD dwProperty, CString& strProperty );

CNetworkAdapter::CNetworkAdapter(void)
{
	m_nCounts = 0 ;

    InitLocalAdapterInfo();
}

CNetworkAdapter::~CNetworkAdapter(void)
{
}

void CNetworkAdapter::InitLocalAdapterInfo()
{
 //   m_vecAdapterInfo.clear();
    ULONG ulOutBufLen = 0;
	int Index = 0 ;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    if ( ERROR_SUCCESS != GetAdaptersInfo( pAdapterInfo, &ulOutBufLen ) )
    {
        pAdapterInfo = ( PIP_ADAPTER_INFO )new BYTE[ ulOutBufLen ];
        if ( NULL == pAdapterInfo )
        {
            return;
        }
    }

    DWORD dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );
    if ( ERROR_SUCCESS == dwRetVal )
    {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while ( pAdapter )
        {
            if ( MIB_IF_TYPE_ETHERNET == pAdapter->Type )
            {
                CString strText;

				// 先清空内容
				m_vecAdapterInfo[Index].m_strMAC.Empty() ;

                for ( UINT i = 0; i < pAdapter->AddressLength; ++i )
                {
                    strText.Format( _T("%02X"), pAdapter->Address[i] );
                    m_vecAdapterInfo[Index].m_strMAC += strText;
                }

                m_vecAdapterInfo[Index].m_strIP = pAdapter->IpAddressList.IpAddress.String		;
                m_vecAdapterInfo[Index].m_strNetMask = pAdapter->IpAddressList.IpMask.String	;
                m_vecAdapterInfo[Index].m_strAdapterName = pAdapter->AdapterName				;
				m_vecAdapterInfo[Index].m_strDescription = pAdapter->Description				;

				Index++ ;
            }
			
            pAdapter = pAdapter->Next;
        }

		m_nCounts = Index ;
    }
    else
    {
        TRACE( _T("\nGet network info failed.\n") );
        ASSERT( 0 );
    }

    delete[] pAdapterInfo;

}

CString CNetworkAdapter::GetIPAddress(int n) const
{
    CString strIP;
//    if ( !m_vecAdapterInfo.empty() )
    {
        strIP = m_vecAdapterInfo[n].m_strIP;
    }

    return strIP;
}


ULONG CNetworkAdapter::GetDescriptionCounts() const
{
	ULONG nCounts ;

	nCounts = m_nCounts ;

	return nCounts;
}

CString CNetworkAdapter::GetDescription(int n) const
{
	CString strDescription;
	{
		strDescription = m_vecAdapterInfo[n].m_strDescription;
	}

	return strDescription;
}

CString CNetworkAdapter::GetMACAddress(int n) const
{
    CString strMAC;
    {
        strMAC = m_vecAdapterInfo[n].m_strMAC;
    }

    return strMAC;
}

void CNetworkAdapter::SetIPAddress( CString strIP, int n )
{
    {
        m_vecAdapterInfo[n].m_strIP = strIP;
    }
}

void CNetworkAdapter::SetMACAddress( CString strMAC, int n )
{
    {
        m_vecAdapterInfo[n].m_strMAC = strMAC;
    }
}

BOOL CNetworkAdapter::RegSetIP( CString strAdapterName, CString strIPAddress, CString strNetMask )
{
    BOOL bRet = TRUE;
    HKEY hKey;
    CString strKeyName = _T("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\");
    strKeyName += strAdapterName;
    if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_WRITE, &hKey ) )
    {
        TRACE( _T("\nRegOpenKeyEx failed\n") );
        return FALSE;
    }

    const int MAX_SIZE = 100;
    TCHAR mszIPAddress[ MAX_SIZE ];
    TCHAR mszNetMask[ MAX_SIZE ];

    _tcsncpy_s( mszIPAddress, strIPAddress, MAX_SIZE - 2 );
    _tcsncpy_s( mszNetMask, strNetMask, MAX_SIZE - 2 );

    mszIPAddress[ MAX_SIZE - 2 ] = 0;
    mszNetMask[ MAX_SIZE - 2 ] = 0;

    int nIP = 0, nMask = 0;
    nIP = _tcslen( mszIPAddress );
    nMask = _tcslen( mszNetMask );

    *( mszIPAddress + nIP + 1 ) = 0x00;         // REG_MULTI_SZ数据需要在后面再加个0
    nIP += 2;

    *( mszNetMask + nMask + 1 ) = 0x00;
    nMask += 2;

    nIP = nIP * sizeof( TCHAR );
    nMask = nMask * sizeof( TCHAR );

    if ( ERROR_SUCCESS != RegSetValueEx( hKey, _T("IPAddress"), 0, 
        REG_MULTI_SZ, ( unsigned char* )mszIPAddress, nIP ) )
    {
        bRet = FALSE;
    }

    if ( ERROR_SUCCESS != RegSetValueEx( hKey, _T("SubnetMask"), 0,
        REG_MULTI_SZ, ( unsigned char* )mszNetMask, nMask ) )
    {
        bRet = FALSE;
    }

    RegCloseKey( hKey );

    return bRet;
}

BOOL CNetworkAdapter::RegSetMAC( CString strAdapterName, CString strMACAddress )
{
    BOOL bRet = FALSE;
    HKEY hKey;
    CString strKeyName = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\");
    if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyName, 0, 
        KEY_WRITE | KEY_READ, &hKey ) ) 
    {
        TRACE( _T("\nRegOpenKeyEx failed.\n") );
        return FALSE;
    }

    const int BUFSIZE = 255;
    TCHAR szSubKeyName[ BUFSIZE ];
    DWORD dwIndex = 0, dwSubKeyName = BUFSIZE;
    FILETIME fileTime;

    while ( ERROR_SUCCESS == RegEnumKeyEx( hKey, dwIndex, szSubKeyName, 
        &dwSubKeyName, NULL, NULL, NULL, &fileTime ) )
    {
        dwIndex ++;
        dwSubKeyName = BUFSIZE;

        HKEY hSubKey;
        CString strSubKeyName;
        strSubKeyName = strKeyName + CString( szSubKeyName ) + _T("\\");
        if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, strSubKeyName, 0, 
            KEY_WRITE | KEY_READ, &hSubKey ) )
        {
            continue;
        }

        TCHAR szValue[ BUFSIZE ];
        DWORD dwType = 0, dwValueSize = sizeof( szValue );
        if ( ERROR_SUCCESS == RegQueryValueEx( hSubKey, _T("NetCfgInstanceId"), NULL, 
            &dwType, ( unsigned char* )szValue, &dwValueSize ) )
        {
            CString strValueText( szValue );
            if ( strValueText == strAdapterName )
            {
                _tcsncpy_s( szValue, strMACAddress, BUFSIZE );
                DWORD dwDataSize = ( strMACAddress.GetLength() + 1 ) * sizeof( TCHAR );

                if ( ERROR_SUCCESS == RegSetValueEx( hSubKey, _T("NetworkAddress"), 0, 
                    REG_SZ, ( unsigned char* )szValue, dwDataSize ) )
                {
                    bRet = TRUE;
                }
                RegCloseKey( hSubKey );
                break;
            }
        }

        RegCloseKey( hSubKey );
    }

    RegCloseKey( hKey );

    return bRet;
}

BOOL CNetworkAdapter::MakeChange(int n)
{
    BOOL bRet = TRUE;

    CString strAdapterName, strIPAddress, strNetMask, strMACAddress;
    {
        strAdapterName = m_vecAdapterInfo[n].m_strAdapterName;
        strIPAddress = m_vecAdapterInfo[n].m_strIP;
        strNetMask = m_vecAdapterInfo[n].m_strNetMask;
        strMACAddress = m_vecAdapterInfo[n].m_strMAC;
    }

	OutputDebugStringA( "开始修改注册表 \n" );

    if ( !RegSetIP( strAdapterName, strIPAddress, strNetMask ) )
    {
		OutputDebugStringA( "RegSetIP() 失败 \n" );
        bRet = FALSE;
    }

    if ( !RegSetMAC( strAdapterName, strMACAddress ) )
    {
		OutputDebugStringA( "RegSetMAC() 失败 \n" );
        bRet = FALSE;
    }

    if ( !ControlAdapter( DICS_PROPCHANGE ) )
    {
		OutputDebugStringA( "ControlAdapter() 失败 \n" );
        bRet = FALSE;
    }

    return bRet;
}

BOOL ChangeStatus( DWORD dwNewStatus, DWORD dwSelectedItem, HDEVINFO hDevInfo )
{
    BOOL bRet = TRUE;

    TRY 
    {
        SP_DEVINFO_DATA DeviceInfoData;
        ZeroMemory( &DeviceInfoData, sizeof( SP_DEVINFO_DATA ) );
        DeviceInfoData.cbSize = sizeof( SP_DEVINFO_DATA );

        // Get a handle to the Selected Item
        if ( !SetupDiEnumDeviceInfo( hDevInfo, dwSelectedItem, &DeviceInfoData ) )
        {
            bRet = FALSE;
        }
        else
        {
            SP_PROPCHANGE_PARAMS PropChangeParams;
            ZeroMemory( &PropChangeParams, sizeof( SP_PROPCHANGE_PARAMS ) );
            PropChangeParams.ClassInstallHeader.cbSize = sizeof( SP_CLASSINSTALL_HEADER );

            // Set the PropChangeParams structure
            PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            PropChangeParams.Scope = DICS_FLAG_GLOBAL;
            PropChangeParams.StateChange = dwNewStatus;

            if ( !SetupDiSetClassInstallParams( hDevInfo, &DeviceInfoData, ( SP_CLASSINSTALL_HEADER* )&PropChangeParams, sizeof( PropChangeParams ) ) )
            {
                bRet = FALSE;
            }
            else
            {
                // Call the ClassInstaller and perform the change
                if ( !SetupDiCallClassInstaller( DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData ) )
                {
                    bRet = FALSE;
                }
                else
                {
                    // 判断是否需要重新启动
                    SP_DEVINSTALL_PARAMS devParams;
                    ZeroMemory( &devParams, sizeof( SP_DEVINSTALL_PARAMS ) );
                    devParams.cbSize = sizeof( SP_DEVINSTALL_PARAMS );

                    if ( !SetupDiGetDeviceInstallParams( hDevInfo, &DeviceInfoData, &devParams ) )
                    {
                        bRet = FALSE;
                    }
                    else
                    {
                        if ( devParams.Flags & ( DI_NEEDRESTART | DI_NEEDREBOOT ) )
                        {
                            TRACE( _T("\nNeed Restart Computer\n") );
                        }
                    }
                }
            }
        }
    }
    CATCH_ALL( e )
    {
    	e->ReportError();
        bRet = FALSE;
    }
    END_CATCH_ALL

    return bRet;
}

BOOL GetDeviceRegistryProperty( HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDeviceInfoData, DWORD dwProperty, CString& strProperty )
{
    BOOL bRet = TRUE;
    DWORD dwDataType = 0;
    LPTSTR pszBuf = NULL;
    DWORD dwBuffersize = 0;
    
    while ( !SetupDiGetDeviceRegistryProperty( hDevInfo, pDeviceInfoData, dwProperty, &dwDataType, ( PBYTE )pszBuf, dwBuffersize, &dwBuffersize ) )
    {
        if ( ERROR_INVALID_DATA == GetLastError() )
        {
            // 不存在 Device desc
            bRet = FALSE;
            break;
        }
        else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            // buffer size 不对
            if ( NULL != pszBuf )
            {
                LocalFree( LocalHandle( pszBuf ) );
                pszBuf = NULL;
            }

            pszBuf = ( LPTSTR )LocalAlloc( LPTR, dwBuffersize );
            if ( NULL == pszBuf )
            {
                bRet = FALSE;
                break;
            }
        }
        else
        {
            // 未知错误
            bRet = FALSE;
            break;
        }
    }

    if ( bRet )
    {
        strProperty = pszBuf;
    }

    if ( NULL != pszBuf )
    {
        LocalFree( LocalHandle( pszBuf ) );
        pszBuf = NULL;
    }

    return bRet;
}

BOOL ControlAdapter( DWORD dwStatus )
{
    BOOL bRet = FALSE;
	char pBufferTmp[255] = "" ;

	OutputDebugStringA( "进入函数ControlAdapter() 内部 \n" );

    if ( 0 == dwStatus )
    {
		OutputDebugStringA( "error! | ControlAdapter() | 0 == dwStatus  \n" );
        return FALSE;
    }
    
    TCHAR* GUIDString = _T("4D36E972-E325-11CE-BFC1-08002BE10318");
    GUID guid;
    ZeroMemory( &guid, sizeof( GUID ) );
    if ( RPC_S_OK != UuidFromString( (RPC_TSTR)GUIDString, &guid ) )
    {
		OutputDebugStringA( "error! | ControlAdapter() | UuidFromString() failed  \n" );
        bRet = FALSE;
    }
    else
    {
        HDEVINFO hDevInfo = NULL;
        hDevInfo = SetupDiGetClassDevs( &guid, REGSTR_KEY_PCIENUM, NULL, DIGCF_PRESENT );
        if ( INVALID_HANDLE_VALUE == hDevInfo )
        {
			OutputDebugStringA( "error! | ControlAdapter() | INVALID_HANDLE_VALUE == hDevInfo  \n" );
            bRet = FALSE;
        }
        else
        {
            DWORD i = 0;
            SP_DEVINFO_DATA DeviceInfoData;
            ZeroMemory( &DeviceInfoData, sizeof( SP_DEVINFO_DATA ) );
            DeviceInfoData.cbSize = sizeof( SP_DEVINFO_DATA );

			//
			//
			//

			PDeviceInfoSet_Personal list = (PDeviceInfoSet_Personal) hDevInfo ;

			RtlZeroMemory( pBufferTmp, 255 );
			sprintf( 
				pBufferTmp,
				"hDevInfo: [magic - %d ] \n",
				list->magic
			//	list->ClassGuid
				);
			OutputDebugStringA( pBufferTmp );

			OutputDebugStringA( "准备调用函数 SetupDiEnumDeviceInfo() \n" );

            for ( i = 0; SetupDiEnumDeviceInfo( hDevInfo, i, &DeviceInfoData ); ++i )
            {
				RtlZeroMemory( pBufferTmp, 255 );
				sprintf( pBufferTmp, "准备调用函数CM_Get_DevNode_Status() [%d]\n", i );
				OutputDebugStringA( pBufferTmp );

                // 获得设备的状态
                DWORD dwProblem = 0, dwDeviceStatus = 0;
                if ( CR_SUCCESS != CM_Get_DevNode_Status( &dwDeviceStatus, &dwProblem, DeviceInfoData.DevInst, 0 ) )
                {
                    continue;
                }

                // 获取设备注册表项描述
                CString strText;
                if ( !GetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_CLASS, strText ) )
                {
                    continue;
                }

                TRACE( _T("\n The %d device instance handle : %d, Class : %s\n"), i, DeviceInfoData.DevInst, strText );

                if ( 0 == lstrcmp( strText, _T("Net") ) )
                {
                    TRACE( _T("This is the adapter device that I want.\n") );

                    //////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
                    if ( GetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, strText ) )
                    {
                        TRACE( _T("SPDRP_DEVICEDESC : %s\n"), strText );
                    }
#endif
                    //////////////////////////////////////////////////////////////////////////

                    if ( ChangeStatus( dwStatus, i, hDevInfo ) )
                    {
                        bRet = TRUE;
                    }
                }
            }

            // 释放 device information set
            bRet = SetupDiDestroyDeviceInfoList( hDevInfo );
        }
    }

	OutputDebugStringA( "离开函数 ControlAdapter() \n" );
    return bRet;
}