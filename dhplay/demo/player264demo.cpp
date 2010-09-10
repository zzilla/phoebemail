// player264demo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "player264demo.h"
#include "player264demoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoApp

BEGIN_MESSAGE_MAP(CPlayer264demoApp, CWinApp)
	//{{AFX_MSG_MAP(CPlayer264demoApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoApp construction

CPlayer264demoApp::CPlayer264demoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPlayer264demoApp object

CPlayer264demoApp theApp;

/////////////////////////////////////////////////////////////////////////////
CString  g_szPropName("dhplaydemo");   
HANDLE   g_hValue = (HANDLE)1;

BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam)
{   
	HANDLE h = GetProp(hwnd, g_szPropName);   
	if (h == g_hValue)
	{   
		*(HWND*)lParam = hwnd;   
		return FALSE;   
	}   
	return TRUE;   
}

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoApp initialization

BOOL CPlayer264demoApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	HWND oldHWnd = NULL;   
	EnumWindows(EnumWndProc, (LPARAM)&oldHWnd);   //enumerates all top-level windows on the screen  
	if (oldHWnd != NULL)   
	{   
		//AfxMessageBox("The program is already running!");   
		::ShowWindow(oldHWnd,SW_SHOWNORMAL);   
		::SetForegroundWindow(oldHWnd);   
		return FALSE;   
	} 

	CPlayer264demoDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CPlayer264demoApp::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CWinApp::PreTranslateMessage(pMsg);
}
