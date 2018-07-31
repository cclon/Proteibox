#pragma once

//////////////////////////////////////////////////////////////////////////

class CPBStart
{
public:
	CPBStart();
	virtual ~CPBStart();

	static int KillProcess( int ExitCode );
	static BOOL GetFullPath( IN OUT LPWSTR lpData );
	static int RunSandboxedProc();

protected:

};



///////////////////////////////   END OF FILE   ///////////////////////////////