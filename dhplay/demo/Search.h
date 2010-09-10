#if !defined(AFX_SEARCH_H__A9BAEA1F_7430_4F63_A683_D82934803155__INCLUDED_)
#define AFX_SEARCH_H__A9BAEA1F_7430_4F63_A683_D82934803155__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Search.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Search dialog

class Search : public CDialog
{
// Construction
public:
	Search(CWnd* pParent = NULL);   // standard constructor
	typedef enum
	{
		BYFRAME,
		BYTIME
	} SEARCHTYPE ;
	SEARCHTYPE m_searchtype ;

	int m_MDFrameNum ;
// Dialog Data
	//{{AFX_DATA(Search)
	enum { IDD = IDD_DIALOG_SEARCH };
	int		m_left;
	int		m_right;
	int		m_top;
	int		m_bottom;
	int		m_value;
	CString	m_info;
	int		m_ValueBegin;
	int		m_ValueEnd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Search)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Search)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonPreMDFrame();
	afx_msg void OnButtonNextMDFrame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCH_H__A9BAEA1F_7430_4F63_A683_D82934803155__INCLUDED_)
