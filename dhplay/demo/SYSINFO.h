#if !defined(AFX_SYSINFO_H__338CC4B6_7D8A_43BA_B48A_7EC8BACF7E19__INCLUDED_)
#define AFX_SYSINFO_H__338CC4B6_7D8A_43BA_B48A_7EC8BACF7E19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SYSINFO.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SYSINFO dialog

class SYSINFO : public CDialog
{
// Construction
public:
	SYSINFO(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SYSINFO)
	enum { IDD = IDD_DIALOG_SYS_INFO };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SYSINFO)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SYSINFO)
	afx_msg void OnButtonRefresh();
	afx_msg void OnOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSINFO_H__338CC4B6_7D8A_43BA_B48A_7EC8BACF7E19__INCLUDED_)
