#if !defined(_YW_PTZ_MENU_H_)
#define _YW_PTZ_MENU_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PtzMenu.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPtzMenu dialog

class CPtzMenu : public CDialog
{
	LONG m_DeviceID;
	int m_Channel;
// Construction
public:
	CPtzMenu(CWnd* pParent = NULL);   // standard constructor

	void SetPtzParam(LONG iHandle, int iChannel);
	void PtzMemuControl(DWORD dwCommand);
// Dialog Data
	//{{AFX_DATA(CPtzMenu)
	enum { IDD = IDD_YWPTZ_MENU };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPtzMenu)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPtzMenu)
	afx_msg void OnOprCancel();
	afx_msg void OnOprClosemenu();
	afx_msg void OnOprDown();
	afx_msg void OnOprLeft();
	afx_msg void OnOprOk();
	afx_msg void OnOprOpenmenu();
	afx_msg void OnOprRight();
	afx_msg void OnOprUp();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_YW_PTZ_MENU_H_)
