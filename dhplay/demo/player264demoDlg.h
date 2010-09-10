// player264demoDlg.h : header file
//

#if !defined(AFX_PLAYER264DEMODLG_H__97C5D021_664D_4091_ABAA_7470E715E021__INCLUDED_)
#define AFX_PLAYER264DEMODLG_H__97C5D021_664D_4091_ABAA_7470E715E021__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SYSINFO.h"

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoDlg dialog

class CPlayer264demoDlg : public CDialog
{
// Construction
public:

	void SetWindowSize();
	void PlayFile(CString csFilePath);
	
	CBitmap		m_BMPOverlay;
	CBitmap		m_BMPBlack;
	void		SavePicTmp();
	void		StartDraw();
	WINDOWPLACEMENT m_OldWndplacement;
	BOOL		m_bIsFullScreen;
	void		OnWndFullScreen();
	CRect		GetOnPicRect(CRect rcWnd,CRect rcOnWnd,LONG nPicWidth,LONG nPicHeight);
	CRect		m_rcDraw;
	CPlayer264demoDlg(CWnd* pParent = NULL);	// standard constructor
	bool		m_IsPlaying;
	CPoint		m_StartPoint;
	BOOL		m_bStartDraw;
	bool		m_IsRepeatPlay ;
	CRect		m_rcDisplay;
	BOOL		m_bNeedFileClose;
	typedef enum
	{
		BUTTON_PLAY,BUTTON_PAUSE,
		BUTTON_FAST,BUTTON_SLOW,
		BUTTON_BACKONE,BUTTON_ONE,
		BUTTON_TOBEGIN,BUTTON_TOEND,
		BUTTON_PICCATCH,BUTTON_STOP,
		BUTTON_GETERROR,BUTTON_FULLSCREEN,
		BUTTON_TOTAL		
	}BUTTON_TYPE ;//各个按钮
	CButton* m_button[BUTTON_TOTAL] ;
	CMenu* m_pMainMenu ;
	CBitmap m_bitmap ;
	bool m_pause ;
	bool m_playnormal ;
	enum{PORT = 0} ;
	enum{MAX_INDEX_BUF_LEN = 16*60000} ;
	CString fileName ; //文件名

	BYTE *m_fileindexbuf ;
	DWORD m_fileindexlen ;

	int m_iTimerID;

// Dialog Data
	//{{AFX_DATA(CPlayer264demoDlg)
	enum { IDD = IDD_PLAYER264DEMO_DIALOG };
	CSliderCtrl	m_sliderS;
	CSliderCtrl	m_sliderH;
	CSliderCtrl	m_sliderC;
	CSliderCtrl	m_sliderB;
	CSliderCtrl	m_slidersound;
	CSliderCtrl	m_sliderplay;
	CStatic	m_playregion;
	BOOL	m_playsound;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayer264demoDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPlayer264demoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPlay();
	afx_msg void OnFileOpen();
	afx_msg void OnPause();
	afx_msg void OnFast();
	afx_msg void OnSlow();
	afx_msg void OnBackone();
	afx_msg void OnOne();
	afx_msg void OnTobegin();
	afx_msg void OnToend();
	afx_msg void OnPiccatch();
	afx_msg void OnStop();
	afx_msg void OnFileClose();
	afx_msg void OnAbout();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLocate();
	afx_msg void OnRepeat();
	afx_msg void PlayMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnResetbuf();
	afx_msg void OnExit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg void OnPartDisplay();
	afx_msg void OnSysinfo();
	afx_msg void OnVerticalSync();
	afx_msg void OnCheckSound();
	afx_msg void OnCutfile();
	afx_msg void OnButtonGeterror();
	afx_msg void OnStaticShow();
	afx_msg void OnFullscreen();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCustomdrawSliderPlay(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCustomdrawSliderSound(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDecAudio();
	afx_msg void OnDecVideo();
	afx_msg void OnDecComplex();
	afx_msg void OnDecNone();
	afx_msg void OnLanChinese();
	afx_msg void OnLanEnglish();
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_nPrePlayPosition;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYER264DEMODLG_H__97C5D021_664D_4091_ABAA_7470E715E021__INCLUDED_)
