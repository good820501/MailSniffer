// MailFinder.h : main header file for the MAILFINDER application
//

#if !defined(AFX_MAILFINDER_H__367F28EC_2129_4044_B555_4F22FFF9BB89__INCLUDED_)
#define AFX_MAILFINDER_H__367F28EC_2129_4044_B555_4F22FFF9BB89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMailFinderApp:
// See MailFinder.cpp for the implementation of this class
//

class CMailFinderApp : public CWinApp
{
public:
	CMailFinderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMailFinderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMailFinderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAILFINDER_H__367F28EC_2129_4044_B555_4F22FFF9BB89__INCLUDED_)
