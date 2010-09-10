/*
**  ************************************************************************
**                                     VEC
**                               Video Encoder Card 
**
**         (c) Copyright 1992-2006, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: Locate.cpp
**	Modification	: 2006/4/28		zhougf		Create the file
**  ************************************************************************
*/

#include "stdafx.h"
#include "player264demo.h"
#include "Locate.h"
#include "dhplay.h"
#include "multilanguage.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Locate dialog
BOOL CheckString(CString str) 
{ 
	BOOL bIsDigit = TRUE; 
	int nCount = str.GetLength(); 
	for ( int i = 0; i < nCount; i ++ ) 
	{ 
		if ( 0 == isdigit( str.GetAt(i) ) ) //check if the character is a digit  
		{ 
			bIsDigit = FALSE; 
			break;
		} 
	} 
	
	return bIsDigit;
}

Locate::Locate(CWnd* pParent /*=NULL*/)
	: CDialog(Locate::IDD, pParent)
{
	//{{AFX_DATA_INIT(Locate)
		// NOTE: the ClassWizard will add member initialization here
	m_locatevalue = 0;
	m_locatetype = BYFRAME ;
	//}}AFX_DATA_INIT
}


void Locate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Locate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_EDIT_RANGEVALUE, m_locatevalue);
	DDX_Radio(pDX, IDC_RADIO_FRAME, (int&)m_locatetype);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Locate, CDialog)
	//{{AFX_MSG_MAP(Locate)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_BUTTON_LOCATE, OnLocate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL Locate::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString str ;
	str.Format("%s         1--%d\n%s(Sec)   0--%d",
		ConvertString("Frame Range"), PLAY_GetFileTotalFrames(0),
		ConvertString("Time Range"),PLAY_GetFileTime(0));
	
	GetDlgItem(IDC_STATIC_RANGE)->SetWindowText(str);

	SetWndStaticText(this) ;

	return TRUE ;
}

void Locate::OnLocate()
{
// 	if (UpdateData(TRUE) == 0)
// 		return ;
	
	CString strLocateValue;
	GetDlgItem(IDC_EDIT_RANGEVALUE)->GetWindowText(strLocateValue);

	if (!CheckString(strLocateValue))
	{
		AfxMessageBox(ConvertString("Input number error")) ;
		return ;
	}

	m_locatevalue = atoi(strLocateValue.GetBuffer(0));
	strLocateValue.ReleaseBuffer();

	CButton* pRadioBN = (CButton*)GetDlgItem(IDC_RADIO_FRAME);
	m_locatetype = (pRadioBN->GetCheck() == BST_CHECKED)? BYFRAME:BYTIME;

	if (m_locatetype == BYFRAME){
		if ( m_locatevalue <= 0 || m_locatevalue > PLAY_GetFileTotalFrames(0))
		{
			AfxMessageBox(ConvertString("Input number error")) ;
			return ;
		}
		PLAY_SetCurrentFrameNum(0,m_locatevalue-1) ;
	}
	else 
	{
		if (m_locatevalue < 0 || m_locatevalue > PLAY_GetFileTime(0))
		{
			AfxMessageBox(ConvertString("Input number error")) ;
			return ;
		}
		PLAY_SetPlayedTimeEx(0,m_locatevalue*1000) ;
	}

}
/////////////////////////////////////////////////////////////////////////////
// Locate message handlers





















