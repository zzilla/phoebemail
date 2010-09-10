#if !defined(AFX_CUTFILE_H__470D1590_0DF3_4CBA_9AA4_22F6B3D27C5F__INCLUDED_)
#define AFX_CUTFILE_H__470D1590_0DF3_4CBA_9AA4_22F6B3D27C5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cutfile.h : header file
//
#include "dhplay.h"
/////////////////////////////////////////////////////////////////////////////
// cutfile dialog

class cutfile : public CDialog
{
// Construction
public:
	cutfile(CString originfile, CWnd* pParent = NULL);   // standard constructor
	FRAME_POS m_RealBegin;
	FRAME_POS m_RealEnd;
	DWORD m_nMaxTime ;
	DWORD m_nMaxFrameNum ;
	CString m_originfile ;
// Dialog Data
	//{{AFX_DATA(cutfile)
	enum { IDD = IDD_DIALOG_CUTFILE };
	int		m_start_posion;
	int		m_startposition;
	int		m_end_position;
	int		m_endposition;
	int		m_type;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cutfile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cutfile)
	afx_msg void OnButtonSavefile();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUTFILE_H__470D1590_0DF3_4CBA_9AA4_22F6B3D27C5F__INCLUDED_)
