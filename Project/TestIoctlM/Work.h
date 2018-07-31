#pragma once

#include "CLoadDriver_2.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////////

#define ProteinBoxDrv_LinkName	"ProteinBoxDrv"

extern CDriver* g_drv_ProteinBoxDrv ;

extern CConfigEx* g_Conf ;


//////////////////////////////////////////////////////////////////////////

BOOL
RunPE (
	IN PCHAR szPath,
	IN PCHAR szCmdLine
	);

CDriver* 
InitDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName
	) ;

BOOL
LoadDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName,
	CDriver** drv
	);

VOID 
UnloadDriver(
	IN CDriver* drv
	);

BOOL
Ioctl_StartProcess ( 
	IN CDriver* drv
	) ;

VOID
TestLoadDriver (
	);

BOOL
HandlerConf (
	);

BOOL
Ioctl_HookShadow (
	IN CDriver* drv,
	IN BOOL bHook
	);

BOOL
Ioctl_HookObject (
	IN CDriver* drv,
	IN BOOL bHook
	);