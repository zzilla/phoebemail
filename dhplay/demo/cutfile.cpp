/*
**  ************************************************************************
**                                     VEC
**                               Video Encoder Card 
**
**         (c) Copyright 1992-2006, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: cutfile.cpp
**	Modification	: 2006/4/28		zhougf		Create the file
**  ************************************************************************
*/
#include "stdafx.h"
#include "player264demo.h"
#include "cutfile.h"
#include "multilanguage.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cutfile dialog


cutfile::cutfile(CString originfile,CWnd* pParent /*=NULL*/)
	: CDialog(cutfile::IDD, pParent)
{
	//{{AFX_DATA_INIT(cutfile)
	m_start_posion  = 0;
	m_startposition = 0;
	m_end_position  = 0;
	m_endposition   = 0;
	m_type          = 0;
	//}}AFX_DATA_INIT
	m_nMaxFrameNum = 0 ; 
	m_nMaxTime     = 0 ;
	m_originfile   = originfile ;
}


void cutfile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cutfile)
	DDX_Text(pDX, IDC_EDIT_START_POSITION, m_start_posion);
	DDX_Text(pDX, IDC_EDIT_startposition, m_startposition);
	DDX_Text(pDX, IDC_EDIT_END_POSITION, m_end_position);
	DDX_Text(pDX, IDC_EDIT_endposition, m_endposition);
	DDX_Radio(pDX, IDC_RADIO_BYFRAME, m_type);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(cutfile, CDialog)
	//{{AFX_MSG_MAP(cutfile)
	ON_BN_CLICKED(IDC_BUTTON_SAVEFILE, OnButtonSavefile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cutfile message handlers

void cutfile::OnButtonSavefile() 
{
	// TODO: Add your control notification handler code here
	if (UpdateData(true) == 0)
		return;
	
	if (m_type == 0)  //By Frame number
	{
		if (m_start_posion < 0 || m_start_posion > m_nMaxFrameNum
			|| m_end_position < 0 || m_end_position > m_nMaxFrameNum
			|| m_start_posion > m_end_position)
		{
			AfxMessageBox(ConvertString("Input number error")) ;
			return ;
		}
	}
	else
	{
		if (m_start_posion < 0 || m_start_posion > m_nMaxTime
			|| m_end_position < 0 || m_end_position > m_nMaxTime
			|| m_start_posion > m_end_position)
		{
			AfxMessageBox(ConvertString("Input number error")) ;
			return ;
		}	
	}
	
	PLAY_GetKeyFramePos(0,m_start_posion * ((m_type == 0)?1:1000) ,m_type + 1,&m_RealBegin);
	PLAY_GetNextKeyFramePos(0,m_end_position * ((m_type == 0)?1:1000),m_type + 1,&m_RealEnd) ;

	m_startposition = (m_type == 0)?m_RealBegin.nFrameNum:(m_RealBegin.nFrameTime/1000) ; 
	m_endposition   = (m_type == 0)?(m_RealEnd.nFrameNum-1):((m_RealEnd.nFrameTime/1000)-1) ;
	UpdateData(false) ;	

	DWORD newfilelen = m_RealEnd.nFilePos+ m_RealEnd.nFrameLen - m_RealBegin.nFilePos ;


	TCHAR szFilters[]= _T("dav Files (*.dav)|*.dav|All Files (*.*)|*.*||");
	CFileDialog FileChooser (FALSE, _T("dav"), _T("*.dav"), OFN_FILEMUSTEXIST,szFilters, this);

	CString savefile ;
		
	//Choose File
	if (FileChooser.DoModal()==IDOK)
	{
		savefile = FileChooser.GetPathName() ;
	}
	else return ;

	CFile file_in(m_originfile,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) ;
	CFile file_out(savefile,CFile::modeWrite|CFile::typeBinary|CFile::modeCreate) ;

	int fileheaderlen = PLAY_GetFileHeadLength() ;
	
	const int nSize = 2048 ;
	BYTE pBuf[nSize] ;
	ZeroMemory(pBuf,nSize) ;
	DWORD nBlock = newfilelen / nSize ;

	file_in.Read(pBuf,fileheaderlen) ;
	
	const BYTE dahuaheader[6] = "DAHUA" ;

	if (memcmp(pBuf, dahuaheader, 5) == 0)
	{
		file_out.Write(pBuf,fileheaderlen) ;
	}
	
	if (m_RealBegin.nFilePos < (1<<31))
		file_in.Seek(m_RealBegin.nFilePos,SEEK_SET) ;
	else
	{
		DWORD zgf = file_in.Seek(m_RealBegin.nFilePos/2,SEEK_SET) ;
		zgf = file_in.Seek(m_RealBegin.nFilePos - m_RealBegin.nFilePos/2,SEEK_CUR) ;
	}
	
	for (DWORD i = 0 ; i < nBlock ; i++ )
	{
		file_in.Read(pBuf,nSize) ;
		file_out.Write(pBuf,nSize) ;
	}
	DWORD nRemain = newfilelen - nBlock*nSize ;
	if (nRemain > 0)
	{
		file_in.Read(pBuf,nRemain) ;
		file_out.Write(pBuf,nRemain) ;
	}	
	
	file_out.Close() ;	
}

BOOL cutfile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_nMaxTime=PLAY_GetFileTime(0) - 1;
	m_nMaxFrameNum=	PLAY_GetFileTotalFrames(0) - 1;
	CString s_range;
	s_range.Format("%s:           %d--%d\r\%s(Sec):     %d--%d\r\n",
		ConvertString("Frame Range"), 0,m_nMaxFrameNum,ConvertString("Time Range"), 0,m_nMaxTime);
	GetDlgItem(IDC_STATIC_RANGE)->SetWindowText(s_range);
	SetWndStaticText(this) ;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
