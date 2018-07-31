// TestIoctlM.h : main header file for the TESTIOCTLM application
//

#if !defined(AFX_TESTIOCTLM_H__7388EFD2_0FF3_4B93_A3AA_6B050943C834__INCLUDED_)
#define AFX_TESTIOCTLM_H__7388EFD2_0FF3_4B93_A3AA_6B050943C834__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMApp:
// See TestIoctlM.cpp for the implementation of this class
//

class CTestIoctlMApp : public CWinApp
{
public:
	CTestIoctlMApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestIoctlMApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestIoctlMApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTIOCTLM_H__7388EFD2_0FF3_4B93_A3AA_6B050943C834__INCLUDED_)
