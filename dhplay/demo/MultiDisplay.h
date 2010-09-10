#if !defined(AFX_MULTIDISPLAY_H__2D9AB4E6_009E_4DA1_8264_69402ECD1C55__INCLUDED_)
#define AFX_MULTIDISPLAY_H__2D9AB4E6_009E_4DA1_8264_69402ECD1C55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MultiDisplay dialog

class MultiDisplay : public CDialog
{
// Construction
public:
	static int NUM ;
	MultiDisplay(CWnd* pParent = NULL);   // standard constructor
	int nRegionNum ;
// Dialog Data
	//{{AFX_DATA(MultiDisplay)
	enum { IDD = IDD_DIALOG_MultiDisplay };
	UINT	m_nBottom;
	UINT	m_nLeft;
	UINT	m_nRight;
	UINT	m_nTop;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MultiDisplay)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MultiDisplay)
	afx_msg void OnButtonDisplay();
	virtual void OnOK();
	afx_msg void OnButtonRefresh();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bIsplay;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIDISPLAY_H__2D9AB4E6_009E_4DA1_8264_69402ECD1C55__INCLUDED_)
