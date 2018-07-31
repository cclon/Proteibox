/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/17 [17:5:2010 - 14:14]
* MODULE : d:\Work\Program\Coding\…≥œ‰\SandBox\Code\Project\ProteinBoxDrv\Version.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"

//////////////////////////////////////////////////////////////////////////

SYSTEM_VERSION_INFO g_Version_Info = { 0 } ;


//////////////////////////////////////////////////////////////////////////


BOOL
GetVersion(
	) 
{
	BOOL  bRet = TRUE ;
	ULONG MajorVersion, MinorVersion ;
	CHAR  szInfo[MAX_PATH] = "" ;

	PsGetVersion( &g_Version_Info.MajorVersion, &g_Version_Info.MinorVersion, &g_Version_Info.BuildNumber, 0 );
	
	MajorVersion = g_Version_Info.MajorVersion ;
	MinorVersion = g_Version_Info.MinorVersion ;

	if ( 5 == MajorVersion && 0 == MinorVersion )
	{
		g_Version_Info.IS___win2k = TRUE ;
		g_Version_Info.CurrentVersion = __win2k ;
		strcpy( szInfo, "Win2k \n" );
	} 
	else if ( 5 == MajorVersion && 1 == MinorVersion )
	{
		g_Version_Info.IS___xp = TRUE ;
		g_Version_Info.CurrentVersion = __winxp ;
		strcpy( szInfo, "Winxp \n" );
	}
	else if ( 5 == MajorVersion && 2 == MinorVersion )
	{
		g_Version_Info.IS___win2003 = TRUE ;
		g_Version_Info.CurrentVersion = __win2003 ;
		strcpy( szInfo, "Win2003 \n" );
	}
	else if ( 6 == MajorVersion && 0 == MinorVersion )
	{
		// Vista or Windows Server 2008
		g_Version_Info.IS___vista = TRUE ;
		g_Version_Info.CurrentVersion = __vista ;
		strcpy( szInfo, "Vista \n" );
	}
	else if ( 6 == MajorVersion && 1 == MinorVersion )
	{
		// Windows 7 or Windows Server 2008 R2
		g_Version_Info.IS___win7 = TRUE ;
		g_Version_Info.CurrentVersion = __win7 ;
		strcpy( szInfo, "Win7 \n" );
	}
	else
	{
		bRet = FALSE ;
		g_Version_Info.CurrentVersion = __unknow ;
		strcpy( szInfo, "!!! unknow system version \n" );
	}

	if ( MajorVersion < 6 || g_Version_Info.BuildNumber <= 0x1770 )
	{
		g_Version_Info.IS_before_vista = TRUE ;
	}

	dprintf( "[Vesrion] %s", szInfo );
	return bRet ;
}







///////////////////////////////   END OF FILE   ///////////////////////////////