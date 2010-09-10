// Search.cpp : implementation file
//

#include "stdafx.h"
#include "player264demo.h"
#include "Search.h"
#include "dhplay.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Search dialog

Search::Search(CWnd* pParent /*=NULL*/)
	: CDialog(Search::IDD, pParent)
{
	//{{AFX_DATA_INIT(Search)
	m_left = 0;
	m_right = 100;
	m_top = 0;
	m_bottom = 100;
	m_value = 100;
	m_info = _T("");
	m_ValueBegin = 0;
	m_ValueEnd = 0;
	//}}AFX_DATA_INIT
	m_searchtype = BYFRAME ;
	m_MDFrameNum = 0 ;
}


void Search::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Search)
	DDX_Text(pDX, IDC_EDIT_LEFT, m_left);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_right);
	DDX_Text(pDX, IDC_EDIT_TOP, m_top);
	DDX_Text(pDX, IDC_EDIT_BUTTOM, m_bottom);
	DDX_Text(pDX, IDC_EDIT_VALUE, m_value);
	DDX_Text(pDX, IDC_STATIC_INFO, m_info);
	DDX_Text(pDX, IDC_EDIT_BEGIN_VALUE, m_ValueBegin);
	DDX_Text(pDX, IDC_EDIT_END_VALUE, m_ValueEnd);
	DDX_Radio(pDX, IDC_RADIO_SEARCH_TYPE, (int&)m_searchtype);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Search, CDialog)
	//{{AFX_MSG_MAP(Search)
	ON_BN_CLICKED(IDC_BUTTON_PREMDFRAME, OnButtonPreMDFrame)
	ON_BN_CLICKED(IDC_BUTTON_NEXTMDFRAME, OnButtonNextMDFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Search message handlers
BOOL Search::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DWORD totalframes = PLAY_GetFileTotalFrames(0) ;
	DWORD totaltimes   = PLAY_GetFileTime(0) ;
	m_info.Format("Ö¡Êý·¶Î§ : 0 -- %d\nÊ±¼ä·¶Î§ : 0 -- %d",totalframes, totaltimes) ;

	m_ValueBegin = totalframes/4 ;
	m_ValueEnd   = totalframes * 3 / 4 ;
	m_MDFrameNum = m_ValueBegin ;

	RECT MDRect ;
	MDRect.left = m_left ;
	MDRect.right = m_right ;
	MDRect.top = m_top ;
	MDRect.bottom = m_bottom ;

	PLAY_SetMDRange(0, &MDRect, m_ValueBegin, m_ValueEnd, m_searchtype+1) ;
	PLAY_SetMDThreShold(0,m_value) ;

	UpdateData(false) ;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void Search::OnButtonPreMDFrame() 
{
	// TODO: Add your control notification handler code here
	int preMDValue = m_value ;
	int preValueBegin = m_ValueBegin ;
	int preValueEnd    = m_ValueEnd ;
	RECT preMDRect ;
	preMDRect.left = m_left ;
	preMDRect.right = m_right ;
	preMDRect.top  = m_top ;
	preMDRect.bottom = m_bottom ;

	if (UpdateData(TRUE) == 0)
	{
		return ;
	}

	if (preMDValue != m_value)
	{
		PLAY_SetMDThreShold(0, m_value) ;
	}

	if (preValueBegin != m_ValueBegin || preValueEnd != m_ValueEnd
		|| preMDRect.left != m_left || preMDRect.right != m_right 
		|| preMDRect.top != m_top || preMDRect.bottom != m_bottom)
	{
		preMDRect.left = m_left ;
		preMDRect.right = m_right ;
		preMDRect.top  = m_top ;
		preMDRect.bottom = m_bottom ;
		if (m_searchtype == BYFRAME)
		{
			PLAY_SetMDRange(0,&preMDRect,m_ValueBegin, m_ValueEnd, m_searchtype+1) ;
		}
		else if (m_searchtype == BYTIME)
		{
			PLAY_SetMDRange(0,&preMDRect,m_ValueBegin*1000, m_ValueEnd*1000, m_searchtype+1) ;
		}
	}

	DWORD MDValue ;
	m_MDFrameNum = PLAY_GetMDPosition(0,0,m_MDFrameNum,&MDValue) ;
	if (m_MDFrameNum != 0)
	{
		PLAY_SetCurrentFrameNum(0, m_MDFrameNum) ;
	}
}

void Search::OnButtonNextMDFrame() 
{
	// TODO: Add your control notification handler code here
	int preMDValue = m_value ;
	int preValueBegin = m_ValueBegin ;
	int preValueEnd    = m_ValueEnd ;
	RECT preMDRect ;
	preMDRect.left = m_left ;
	preMDRect.right = m_right ;
	preMDRect.top  = m_top ;
	preMDRect.bottom = m_bottom ;

	if (UpdateData(TRUE) == 0)
	{
		return ;
	}

	if (preMDValue != m_value)
	{
		PLAY_SetMDThreShold(0, m_value) ;
	}

	if (preValueBegin != m_ValueBegin || preValueEnd != m_ValueEnd
		|| preMDRect.left != m_left || preMDRect.right != m_right 
		|| preMDRect.top != m_top || preMDRect.bottom != m_bottom)
	{
		preMDRect.left = m_left ;
		preMDRect.right = m_right ;
		preMDRect.top  = m_top ;
		preMDRect.bottom = m_bottom ;
		if (m_searchtype == BYFRAME)
		{
			PLAY_SetMDRange(0,&preMDRect,m_ValueBegin, m_ValueEnd, m_searchtype+1) ;
		}
		else if (m_searchtype == BYTIME)
		{
			PLAY_SetMDRange(0,&preMDRect,m_ValueBegin*1000, m_ValueEnd*1000, m_searchtype+1) ;
		}
	}

	DWORD MDValue ;
	DWORD nCurrentFrame = PLAY_GetCurrentFrameNum(0) ;
	DWORD nFrame = (m_MDFrameNum > nCurrentFrame) ? m_MDFrameNum : nCurrentFrame ;
	m_MDFrameNum = PLAY_GetMDPosition(0,1,nFrame,&MDValue) ;

	if (m_MDFrameNum != 0)
	{
		PLAY_SetCurrentFrameNum(0, m_MDFrameNum) ;
	}
}
