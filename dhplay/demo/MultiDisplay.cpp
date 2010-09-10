/*
**  ************************************************************************
**                                     VEC
**                               Video Encoder Card 
**
**         (c) Copyright 1992-2006, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: MultiDisplay.cpp
**	Modification	: 2006/4/28		zhougf		Create the file
**  ************************************************************************
*/

#include "stdafx.h"
#include "player264demo.h"
#include "MultiDisplay.h"
#include "dhplay.h"
#include "multilanguage.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MultiDisplay dialog
int MultiDisplay::NUM = 1 ;

MultiDisplay::MultiDisplay(CWnd* pParent /*=NULL*/)
	: CDialog(MultiDisplay::IDD, pParent)
{
	//{{AFX_DATA_INIT(MultiDisplay)
	m_nBottom = 100;
	m_nLeft = 0;
	m_nRight = 100;
	m_nTop = 0;
	//}}AFX_DATA_INIT		

	if (NUM >= 16)
		nRegionNum = -1;
	else
		nRegionNum = NUM;

	NUM++;
}


void MultiDisplay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MultiDisplay)
	DDX_Text(pDX, IDC_EDIT_BOTTOM, m_nBottom);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_nLeft);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_nRight);
	DDX_Text(pDX, IDC_EDIT_TOP, m_nTop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MultiDisplay, CDialog)
	//{{AFX_MSG_MAP(MultiDisplay)
	ON_BN_CLICKED(IDC_BUTTON_DISPLAY, OnButtonDisplay)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MultiDisplay message handlers

void MultiDisplay::OnButtonDisplay() 
{
	// TODO: Add your control notification handler code here
	
	if (UpdateData(true) == 0)
		return ;

	RECT m_rcRect;
	
	m_rcRect.left   = m_nLeft ;
	m_rcRect.bottom = m_nBottom ;
	m_rcRect.right  = m_nRight ;
	m_rcRect.top    = m_nTop ;
	long width,height ;
	PLAY_GetPictureSize(0,&width,&height) ;
	if (m_nLeft < 0 || m_nLeft >= m_nRight || m_nRight >= width 
		||m_nTop < 0 || m_nTop >= m_nBottom || m_nBottom >= height)
	{
		AfxMessageBox(ConvertString("Input number error")) ;
		return  ;
	}
	if (PLAY_SetDisplayRegion(0,nRegionNum,&m_rcRect,GetDlgItem(IDC_STATIC_PART_DISPLAY)->m_hWnd,TRUE))
	{
		m_bIsplay = TRUE;

		//20090929 bug: when pause, button display has no effect
		PLAY_RefreshPlayEx(0,nRegionNum) ;
	}	
}

void MultiDisplay::OnOK() 
{	
	CDialog::OnOK();
}

void MultiDisplay::OnButtonRefresh() 
{
	// TODO: Add your control notification handler code here
	PLAY_RefreshPlayEx(0,nRegionNum) ;
}

BOOL MultiDisplay::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_bIsplay = FALSE;

	SetWndStaticText(this) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MultiDisplay::OnClose() 
{
	CDialog::OnClose();
	
	PLAY_SetDisplayRegion(0,nRegionNum,NULL,NULL,FALSE);

	if (NUM > 1)
	{
		NUM-- ;
	}
}

void MultiDisplay::OnDestroy() 
{
	CDialog::OnDestroy();
	
	OnClose();
}