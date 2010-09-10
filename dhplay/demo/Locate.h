#if !defined(AFX_LOCATE_H__C962B323_7ACD_4F63_8F6B_BDCB220D9CD4__INCLUDED_)
#define AFX_LOCATE_H__C962B323_7ACD_4F63_8F6B_BDCB220D9CD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Locate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Locate dialog

class Locate : public CDialog
{
// Construction
public:
	Locate(CWnd* pParent = NULL);   // standard constructor
	typedef enum
	{
		BYFRAME,
		BYTIME
	} LOCATETYPE ;
	LOCATETYPE m_locatetype ;
// Dialog Data
	//{{AFX_DATA(Locate)
	enum { IDD = IDD_DIALOG1 };
	int		m_locatevalue;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Locate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Locate)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	afx_msg void OnLocate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATE_H__C962B323_7ACD_4F63_8F6B_BDCB220D9CD4__INCLUDED_)
