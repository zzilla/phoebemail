// player264demo.h : main header file for the PLAYER264DEMO application
//

#if !defined(AFX_PLAYER264DEMO_H__14E4EBF3_8BB2_4C16_89FC_E0578AAEEE45__INCLUDED_)
#define AFX_PLAYER264DEMO_H__14E4EBF3_8BB2_4C16_89FC_E0578AAEEE45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoApp:
// See player264demo.cpp for the implementation of this class
//

class CPlayer264demoApp : public CWinApp
{
public:
	CPlayer264demoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayer264demoApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPlayer264demoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYER264DEMO_H__14E4EBF3_8BB2_4C16_89FC_E0578AAEEE45__INCLUDED_)
