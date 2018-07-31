/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/21 [21:1:2012 - 15:36]
* MODULE : Code\Project\ProteinBoxDLL\PBMsidll\PBMsidll.cpp
* 
* Description:
*
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "../common.h"
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "PBMsidll.h"

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI fake_MsiOpenPackageA();
ULONG WINAPI fake_MsiOpenPackageW();
ULONG WINAPI fake_MsiOpenPackageExA();
ULONG WINAPI fake_MsiOpenPackageExW();
ULONG WINAPI fake_MsiInstallProductA(LPCSTR szPackagePath, LPCSTR szCommandLine);
ULONG WINAPI fake_MsiInstallProductW(LPCWSTR szPackagePath, LPCWSTR szCommandLine);
ULONG WINAPI fake_MsiReinstallFeatureA(LPCSTR szProduct, LPCSTR szFeature, DWORD dwReinstallMode);
ULONG WINAPI fake_MsiReinstallFeatureW(LPCWSTR szProduct, LPCWSTR szFeature, DWORD dwReinstallMode);
ULONG WINAPI fake_MsiAdvertiseProductA(LPCSTR szPackagePath, LPCSTR szScriptfilePath, LPCSTR szTransforms, LANGID lgidLanguage);
ULONG WINAPI fake_MsiAdvertiseProductW(LPCWSTR szPackagePath, LPCWSTR szScriptfilePath, LPCWSTR szTransforms, LANGID lgidLanguage);
ULONG WINAPI fake_MsiConfigureProductExA(LPCSTR szProduct, int iInstallLevel, INSTALLSTATE eInstallState, LPCSTR szCommandLine);
ULONG WINAPI fake_MsiConfigureProductExW(LPCWSTR szProduct, int iInstallLevel, INSTALLSTATE eInstallState, LPCWSTR szCommandLine);
ULONG WINAPI fake_MsiApplyPatchA(LPCSTR szPatchPackage, LPCSTR szInstallPackage, int eInstallType, LPCSTR szCommandLine);
ULONG WINAPI fake_MsiApplyPatchW(LPCWSTR szPatchPackage, LPCWSTR szInstallPackage, int eInstallType, LPCWSTR szCommandLine);
ULONG WINAPI fake_MsiDoActionA(MSIHANDLE hInstall, LPSTR szAction);
ULONG WINAPI fake_MsiDoActionW(MSIHANDLE hInstall, LPCWSTR szAction);


typedef enum _Msi_ 
{
	MsiOpenPackageA_TAG = 0,
	MsiOpenPackageW_TAG ,
	MsiOpenPackageExA_TAG ,
	MsiOpenPackageExW_TAG ,
	MsiInstallProductA_TAG ,
	MsiInstallProductW_TAG ,
	MsiReinstallFeatureA_TAG ,
	MsiReinstallFeatureW_TAG ,
	MsiAdvertiseProductA_TAG ,
	MsiAdvertiseProductW_TAG ,
	MsiConfigureProductExA_TAG ,
	MsiConfigureProductExW_TAG ,
	MsiApplyPatchA_TAG ,
	MsiApplyPatchW_TAG ,
	MsiDoActionA_TAG ,
	MsiDoActionW_TAG ,
};


static HOOKINFOLittle g_HookInfoMsi_Array [] = 
{
	{ Nothing_TAG, MsiOpenPackageA_TAG, "MsiOpenPackageA", 0, NULL, fake_MsiOpenPackageA },
	{ Nothing_TAG, MsiOpenPackageW_TAG, "MsiOpenPackageW", 0, NULL, fake_MsiOpenPackageW },
	{ Nothing_TAG, MsiOpenPackageExA_TAG, "MsiOpenPackageExA", 0, NULL, fake_MsiOpenPackageExA },
	{ Nothing_TAG, MsiOpenPackageExW_TAG, "MsiOpenPackageExW", 0, NULL, fake_MsiOpenPackageExW },
	{ Nothing_TAG, MsiInstallProductA_TAG, "MsiInstallProductA", 0, NULL, fake_MsiInstallProductA },
	{ Nothing_TAG, MsiInstallProductW_TAG, "MsiInstallProductW", 0, NULL, fake_MsiInstallProductW },
	{ Nothing_TAG, MsiReinstallFeatureA_TAG, "MsiReinstallFeatureA", 0, NULL, fake_MsiReinstallFeatureA },
	{ Nothing_TAG, MsiReinstallFeatureW_TAG, "MsiReinstallFeatureW", 0, NULL, fake_MsiReinstallFeatureW },
	{ Nothing_TAG, MsiAdvertiseProductA_TAG, "MsiAdvertiseProductA", 0, NULL, fake_MsiAdvertiseProductA },
	{ Nothing_TAG, MsiAdvertiseProductW_TAG, "MsiAdvertiseProductW", 0, NULL, fake_MsiAdvertiseProductW },
	{ Nothing_TAG, MsiConfigureProductExA_TAG, "MsiConfigureProductExA", 0, NULL, fake_MsiConfigureProductExA },
	{ Nothing_TAG, MsiConfigureProductExW_TAG, "MsiConfigureProductExW", 0, NULL, fake_MsiConfigureProductExW },
	{ Nothing_TAG, MsiApplyPatchA_TAG, "MsiApplyPatchA", 0, NULL, fake_MsiApplyPatchA },
	{ Nothing_TAG, MsiApplyPatchW_TAG, "MsiApplyPatchW", 0, NULL, fake_MsiApplyPatchW },
	{ Nothing_TAG, MsiDoActionA_TAG, "MsiDoActionA", 0, NULL, fake_MsiDoActionA },
	{ Nothing_TAG, MsiDoActionW_TAG, "MsiDoActionW", 0, NULL, fake_MsiDoActionW },
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID
MsiDll_Filter (
	IN int nIndex,
	IN ULONG dwErrorCode
	)
{
	if ( nIndex && ERROR_INSTALL_SERVICE == dwErrorCode )
	{
		PB_CanElevateOnVista();
		Sleep( 1000 );
	}
	else
	{
		PB_StartBoxedService( L"MSIServer" );
	}

	return;
}


ULONG WINAPI fake_MsiOpenPackageA()
{
	typedef int (WINAPI* _MsiOpenPackageA_)();
	_MsiOpenPackageA_ addr = (_MsiOpenPackageA_) g_HookInfoMsi_Array[ MsiOpenPackageA_TAG ].OrignalAddress ;

	MsiDll_Filter( 0, 0 );
	return  addr();
}


ULONG WINAPI fake_MsiOpenPackageW()
{
	typedef int (WINAPI* _MsiOpenPackageW_)();
	_MsiOpenPackageW_ addr = (_MsiOpenPackageW_) g_HookInfoMsi_Array[ MsiOpenPackageW_TAG ].OrignalAddress ;

	MsiDll_Filter( 0, 0 );
	return  addr();
}


ULONG WINAPI fake_MsiOpenPackageExA()
{
	typedef int (WINAPI* _MsiOpenPackageExA_)();
	_MsiOpenPackageExA_ addr = (_MsiOpenPackageExA_) g_HookInfoMsi_Array[ MsiOpenPackageExA_TAG ].OrignalAddress ;

	MsiDll_Filter( 0, 0 );
	return  addr();
}


ULONG WINAPI fake_MsiOpenPackageExW()
{
	typedef int (WINAPI* _MsiOpenPackageExW_)();
	_MsiOpenPackageExW_ addr = (_MsiOpenPackageExW_) g_HookInfoMsi_Array[ MsiOpenPackageExW_TAG ].OrignalAddress ;

	MsiDll_Filter( 0, 0 );
	return  addr();
}


ULONG WINAPI fake_MsiInstallProductA(LPCSTR szPackagePath, LPCSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiInstallProductA_)(LPCSTR, LPCSTR);
	_MsiInstallProductA_ addr = (_MsiInstallProductA_) g_HookInfoMsi_Array[ MsiInstallProductA_TAG ].OrignalAddress ;

	do
	{
		ret = addr( szPackagePath, szCommandLine );
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiInstallProductW(LPCWSTR szPackagePath, LPCWSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiInstallProductW_)(LPCWSTR, LPCWSTR);
	_MsiInstallProductW_ addr = (_MsiInstallProductW_) g_HookInfoMsi_Array[ MsiInstallProductW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szPackagePath, szCommandLine);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );
	
	return ret;
}


ULONG WINAPI fake_MsiReinstallFeatureA(LPCSTR szProduct, LPCSTR szFeature, DWORD dwReinstallMode)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiReinstallFeatureA_)(LPCSTR, LPCSTR, DWORD);
	_MsiReinstallFeatureA_ addr = (_MsiReinstallFeatureA_) g_HookInfoMsi_Array[ MsiReinstallFeatureA_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szProduct, szFeature, dwReinstallMode);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiReinstallFeatureW(LPCWSTR szProduct, LPCWSTR szFeature, DWORD dwReinstallMode)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiReinstallFeatureW_)(LPCWSTR, LPCWSTR, DWORD);
	_MsiReinstallFeatureW_ addr = (_MsiReinstallFeatureW_) g_HookInfoMsi_Array[ MsiReinstallFeatureW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szProduct, szFeature, dwReinstallMode);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiAdvertiseProductA(LPCSTR szPackagePath, LPCSTR szScriptfilePath, LPCSTR szTransforms, LANGID lgidLanguage)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiAdvertiseProductA_)(LPCSTR, LPCSTR, LPCSTR, LANGID);
	_MsiAdvertiseProductA_ addr = (_MsiAdvertiseProductA_) g_HookInfoMsi_Array[ MsiAdvertiseProductA_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szPackagePath, szScriptfilePath, szTransforms, lgidLanguage);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiAdvertiseProductW(LPCWSTR szPackagePath, LPCWSTR szScriptfilePath, LPCWSTR szTransforms, LANGID lgidLanguage)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiAdvertiseProductW_)(LPCWSTR, LPCWSTR, LPCWSTR, LANGID);
	_MsiAdvertiseProductW_ addr = (_MsiAdvertiseProductW_) g_HookInfoMsi_Array[ MsiAdvertiseProductW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szPackagePath, szScriptfilePath, szTransforms, lgidLanguage);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiConfigureProductExA(LPCSTR szProduct, int iInstallLevel, INSTALLSTATE eInstallState, LPCSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiConfigureProductExA_)(LPCSTR, int, INSTALLSTATE, LPCSTR);
	_MsiConfigureProductExA_ addr = (_MsiConfigureProductExA_) g_HookInfoMsi_Array[ MsiConfigureProductExA_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szProduct, iInstallLevel, eInstallState, szCommandLine);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiConfigureProductExW(LPCWSTR szProduct, int iInstallLevel, INSTALLSTATE eInstallState, LPCWSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiConfigureProductExW_)(LPCWSTR, int, INSTALLSTATE, LPCWSTR);
	_MsiConfigureProductExW_ addr = (_MsiConfigureProductExW_) g_HookInfoMsi_Array[ MsiConfigureProductExW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szProduct, iInstallLevel, eInstallState, szCommandLine);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiApplyPatchA(LPCSTR szPatchPackage, LPCSTR szInstallPackage, int eInstallType, LPCSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiApplyPatchA_)(LPCSTR, LPCSTR, int, LPCSTR);
	_MsiApplyPatchA_ addr = (_MsiApplyPatchA_) g_HookInfoMsi_Array[ MsiApplyPatchA_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szPatchPackage, szInstallPackage, eInstallType, szCommandLine);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiApplyPatchW(LPCWSTR szPatchPackage, LPCWSTR szInstallPackage, int eInstallType, LPCWSTR szCommandLine)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiApplyPatchW_)(LPCWSTR, LPCWSTR, int, LPCWSTR);
	_MsiApplyPatchW_ addr = (_MsiApplyPatchW_) g_HookInfoMsi_Array[ MsiApplyPatchW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(szPatchPackage, szInstallPackage, eInstallType, szCommandLine);
		if ( ret != ERROR_INSTALL_SERVICE ) { break; }

		MsiDll_Filter( nIndex++, ERROR_INSTALL_SERVICE );
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiDoActionA(MSIHANDLE hInstall, LPSTR szAction)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiDoActionA_)(MSIHANDLE, LPSTR);
	_MsiDoActionA_ addr = (_MsiDoActionA_) g_HookInfoMsi_Array[ MsiDoActionA_TAG ].OrignalAddress ;

	do
	{
		ret = addr(hInstall, szAction);
		if ( ret != ERROR_INSTALL_SERVICE && ret != ERROR_INSTALL_FAILURE ) { break; }

		MsiDll_Filter(nIndex++, ret);
	}
	while ( nIndex < 6 );

	return ret;
}


ULONG WINAPI fake_MsiDoActionW(MSIHANDLE hInstall, LPCWSTR szAction)
{
	ULONG nIndex = 0, ret = 0 ;
	typedef int (WINAPI* _MsiDoActionW_)(MSIHANDLE, LPCWSTR);
	_MsiDoActionW_ addr = (_MsiDoActionW_) g_HookInfoMsi_Array[ MsiDoActionW_TAG ].OrignalAddress ;

	do
	{
		ret = addr(hInstall, szAction);
		if ( ret != ERROR_INSTALL_SERVICE && ret != ERROR_INSTALL_FAILURE ) { break; }

		MsiDll_Filter(nIndex++, ret);
	}
	while ( nIndex < 6 );

	return ret;
}


BOOL WINAPI Hook_pharase4_msidll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	int i = 0, TotalCounts = 0 ;

	if ( NULL == hModule )
	{
		if ( g_hEvent_PB_WindowsInstallerInUse )
		{
			CloseHandle(g_hEvent_PB_WindowsInstallerInUse);
			g_hEvent_PB_WindowsInstallerInUse = hModule ;
		}

		return TRUE;
	}

	if (   wcsicmp( g_BoxInfo.ProcessName, g_PBRpcSs_exe ) 
		&& wcsicmp( g_BoxInfo.ProcessName, g_PBDcomLaunch_exe )
		&& NULL == g_hEvent_PB_WindowsInstallerInUse
		)
	{
		g_hEvent_PB_WindowsInstallerInUse = CreateEventW( 0, 1, 0, L"PB_WindowsInstallerInUse" );
	}

	// 获取函数原始地址
	LPHOOKINFOLittle pArray = g_HookInfoMsi_Array;
	TotalCounts = ARRAYSIZEOF( g_HookInfoMsi_Array );
	for( i=0; i<TotalCounts; i++ )
	{
		pArray[i].OrignalAddress = (PVOID) GetProcAddress( hModule, pArray[i].FunctionName );
	}

	// 进行Hook
	for( i=0; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[i] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase4_msidll() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
			return FALSE ;
		}
	}

	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////