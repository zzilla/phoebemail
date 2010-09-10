// SYSINFO.cpp : implementation file
//

#include "stdafx.h"
#include "player264demo.h"
#include "SYSINFO.h"
#include "dhplay.h"
#include "multilanguage.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SYSINFO dialog


SYSINFO::SYSINFO(CWnd* pParent /*=NULL*/)
	: CDialog(SYSINFO::IDD, pParent)
{
	//{{AFX_DATA_INIT(SYSINFO)
	//}}AFX_DATA_INIT
}


void SYSINFO::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SYSINFO)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SYSINFO, CDialog)
	//{{AFX_MSG_MAP(SYSINFO)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SYSINFO message handlers

void SYSINFO::OnButtonRefresh() 
{
	// TODO: Add your control notification handler code here
	CString str ;
	int sys_info = PLAY_GetCaps() ;
	str.Format("%s: %d\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s:  0x%X\n\
%s: %d\n\
%s%d\n\
%s: %d\n\
%s: %d\n\
%s: %d\n\
%s: %d\n\
%s: %s",
		ConvertString("Color Key"), PLAY_GetColorKey(0),
		ConvertString("Is DDRAW Supported"),(sys_info & SUPPORT_DDRAW)?"Yes":"No",
		ConvertString("Is BLT Supported"),(sys_info & SUPPORT_BLT)?"Yes":"No",
		ConvertString("Is FORCC Supported"), (sys_info & SUPPORT_BLTFOURCC)?"Yes":"No",
		ConvertString("Is BLTSHRINKX Supported"), (sys_info & SUPPORT_BLTSHRINKX)?"Yes":"No",
		ConvertString("Is BLTSHRINKY supported"),(sys_info & SUPPORT_BLTSHRINKY)?"Yes":"No",
		ConvertString("Is BLTSTRETCHX supported"),(sys_info & SUPPORT_BLTSTRETCHX)?"Yes":"No",
		ConvertString("Is BLTSTRETCHY supported"), (sys_info & SUPPORT_BLTSTRETCHY)?"Yes":"No",
		ConvertString("Volume"),PLAY_GetVolume(0),
		ConvertString("Max Frame Buf"),PLAY_GetDisplayBuf(0),
		ConvertString("Video Source size(Stream)"),PLAY_GetBufferValue(0,BUF_VIDEO_SRC),
		ConvertString("Audio Source size(Stream)"),PLAY_GetBufferValue(0,BUF_AUDIO_SRC),
		ConvertString("SourceBufferRemain(Stream)"),PLAY_GetSourceBufferRemain(0),
		ConvertString("Video render frames"),PLAY_GetBufferValue(0,BUF_VIDEO_RENDER),
		ConvertString("Audio render frames"),PLAY_GetBufferValue(0,BUF_AUDIO_RENDER),		
		ConvertString("Is OVERLAY supported"),PLAY_GetOverlayMode(0) == 1 ? "Yes":"No"
	);
	
	GetDlgItem(IDC_STATIC_INFO)->SetWindowText(str);
}

void SYSINFO::OnOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

BOOL SYSINFO::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString str ;
	int sys_info = PLAY_GetCaps() ;
	str.Format("%s: %d\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s: %s\n\
%s:  0x%X\n\
%s: %d\n\
%s%d\n\
%s: %d\n\
%s: %d\n\
%s: %d\n\
%s: %d\n\
%s: %s",
		ConvertString("Color Key"), PLAY_GetColorKey(0),
		ConvertString("Is DDRAW Supported"),(sys_info & SUPPORT_DDRAW)?"Yes":"No",
		ConvertString("Is BLT Supported"),(sys_info & SUPPORT_BLT)?"Yes":"No",
		ConvertString("Is FORCC Supported"), (sys_info & SUPPORT_BLTFOURCC)?"Yes":"No",
		ConvertString("Is BLTSHRINKX Supported"), (sys_info & SUPPORT_BLTSHRINKX)?"Yes":"No",
		ConvertString("Is BLTSHRINKY supported"),(sys_info & SUPPORT_BLTSHRINKY)?"Yes":"No",
		ConvertString("Is BLTSTRETCHX supported"),(sys_info & SUPPORT_BLTSTRETCHX)?"Yes":"No",
		ConvertString("Is BLTSTRETCHY supported"), (sys_info & SUPPORT_BLTSTRETCHY)?"Yes":"No",
		ConvertString("Volume"),PLAY_GetVolume(0),
		ConvertString("Max Frame Buf"),PLAY_GetDisplayBuf(0),
		ConvertString("Video Source size(Stream)"),PLAY_GetBufferValue(0,BUF_VIDEO_SRC),
		ConvertString("Audio Source size(Stream)"),PLAY_GetBufferValue(0,BUF_AUDIO_SRC),
		ConvertString("SourceBufferRemain(Stream)"),PLAY_GetSourceBufferRemain(0),
		ConvertString("Video render frames"),PLAY_GetBufferValue(0,BUF_VIDEO_RENDER),
		ConvertString("Audio render frames"),PLAY_GetBufferValue(0,BUF_AUDIO_RENDER),		
		ConvertString("Is OVERLAY supported"),PLAY_GetOverlayMode(0) == 1 ? "Yes":"No"
	);
	
	GetDlgItem(IDC_STATIC_INFO)->SetWindowText(str);

	SetWndStaticText(this) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void SYSINFO::OnDestroy() 
{
	CDialog::OnDestroy();
}
