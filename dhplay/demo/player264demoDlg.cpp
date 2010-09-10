
#include "stdafx.h"
#include <mmsystem.h>
#include "player264demo.h"
#include "player264demoDlg.h"

#include "Locate.h"
#include "MultiDisplay.h"
#include "SYSINFO.h"
#include "cutfile.h"
#include "multilanguage.h"

#include "dhplay.h"

#define DEMOVERSION 0x02001208

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_FILE_END WM_USER+100 
#define ISCONVERT TRUE

int m_nDecType = 3;  //Dec CallBack Stream Type
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
void CALLBACK DrawFun(long nPort,HDC hDc,LONG nUser) ;
void CALLBACK FileRefDone(DWORD nReserved,DWORD nUser);
void CALLBACK FileRefDoneEx(DWORD nReserved,BOOL bIndexCreated, DWORD nUser);
void CALLBACK EncTypeChange(long nPort,long User);
void CALLBACK funAudio(long nPort, char * pAudioBuf,long nSize, long nStamp, long nType, long nUser) ;
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2);
void CALLBACK DecCBFunEx(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2);

FILE* f_audio ;
FILE* f_video ;
FILE* f_complex ;

bool g_IndexCreated = false ;
int g_playspeed = 0 ;

extern CString  g_szPropName;   
extern HANDLE   g_hValue;

void CALLBACK timer_proc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CPlayer264demoDlg *pDlg = (CPlayer264demoDlg*)dwUser;
	
	if (!pDlg)
	{
		return;
	}
	
	__try 
	{		
		pDlg->StartDraw();		
	}
	
	__except (0, 1)
	{
		int zgf = 0;
	}
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayer264demoDlg dialog

CPlayer264demoDlg::CPlayer264demoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayer264demoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlayer264demoDlg)
	m_playsound = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_IsPlaying    = false ;
	m_IsRepeatPlay = false ;
	m_pause        = false ;
	m_playnormal   = false ;
	fileName       = ""    ;
	m_fileindexlen = 0     ;
	m_bIsFullScreen = 0    ;
	m_nPrePlayPosition = 0 ;
	m_fileindexbuf = NULL ;
}

void CPlayer264demoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayer264demoDlg)
	DDX_Control(pDX, IDC_SLIDERS, m_sliderS);
	DDX_Control(pDX, IDC_SLIDERH, m_sliderH);
	DDX_Control(pDX, IDC_SLIDERC, m_sliderC);
	DDX_Control(pDX, IDC_SLIDERB, m_sliderB);
	DDX_Control(pDX, IDC_SLIDER_SOUND, m_slidersound);
	DDX_Control(pDX, IDC_SLIDER_PLAY, m_sliderplay);
	DDX_Control(pDX, IDC_STATIC_SHOW, m_playregion);
	DDX_Check(pDX, IDC_CHECK_SOUND, m_playsound);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlayer264demoDlg, CDialog)
	//{{AFX_MSG_MAP(CPlayer264demoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnPlay)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_BUTTON_FAST, OnFast)
	ON_BN_CLICKED(IDC_BUTTON_SLOW, OnSlow)
	ON_BN_CLICKED(IDC_BUTTON_BACKONE, OnBackone)
	ON_BN_CLICKED(IDC_BUTTON_ONE, OnOne)
	ON_BN_CLICKED(IDC_BUTTON_TOBEGIN, OnTobegin)
	ON_BN_CLICKED(IDC_BUTTON_TOEND, OnToend)
	ON_BN_CLICKED(IDC_BUTTON_PICCATCH, OnPiccatch)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnStop)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_LOCATE, OnLocate)
	ON_COMMAND(ID_REPEAT, OnRepeat)
	ON_MESSAGE(WM_FILE_END, PlayMessage)	
	ON_COMMAND(ID_RESETBUF, OnResetbuf)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PART_DISPLAY, OnPartDisplay)
	ON_COMMAND(IDC_SYSINFO, OnSysinfo)
	ON_COMMAND(IDC_VERTICALSYNC, OnVerticalSync)
	ON_BN_CLICKED(IDC_CHECK_SOUND, OnCheckSound)
	ON_COMMAND(IDC_CUTFILE, OnCutfile)
	ON_BN_CLICKED(IDC_BUTTON_GETERROR, OnButtonGeterror)
	ON_BN_CLICKED(IDC_STATIC_SHOW, OnStaticShow)
	ON_BN_CLICKED(IDC_FULLSCREEN, OnFullscreen)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_PLAY, OnCustomdrawSliderPlay)
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SOUND, OnCustomdrawSliderSound)
	ON_COMMAND(ID_DEC_AUDIO, OnDecAudio)
	ON_COMMAND(ID_DEC_VIDEO, OnDecVideo)
	ON_COMMAND(ID_DEC_COMPLEX, OnDecComplex)
	ON_COMMAND(ID_DEC_NONE, OnDecNone)
	ON_COMMAND(ID_MENUITEM_Chinese, OnLanChinese)
	ON_COMMAND(ID_MENUITEM_English, OnLanEnglish)
	ON_WM_CLOSE()
	ON_WM_DROPFILES()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CPlayer264demoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	SetProp(m_hWnd, "dhplaydemo", (HANDLE)1);

	DragAcceptFiles(true);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_pMainMenu = GetMenu() ;

	m_bitmap.LoadBitmap(IDB_BITMAP) ;

	m_button[BUTTON_PLAY]     = (CButton*)GetDlgItem(IDC_BUTTON_PLAY) ;
	m_button[BUTTON_PAUSE]    = (CButton*)GetDlgItem(IDC_BUTTON_PAUSE) ;
	m_button[BUTTON_FAST]     = (CButton*)GetDlgItem(IDC_BUTTON_FAST) ;
	m_button[BUTTON_SLOW]     = (CButton*)GetDlgItem(IDC_BUTTON_SLOW) ;
	m_button[BUTTON_BACKONE]  = (CButton*)GetDlgItem(IDC_BUTTON_BACKONE) ;
	m_button[BUTTON_ONE]      = (CButton*)GetDlgItem(IDC_BUTTON_ONE) ;
	m_button[BUTTON_TOBEGIN]  = (CButton*)GetDlgItem(IDC_BUTTON_TOBEGIN) ;
	m_button[BUTTON_TOEND]    = (CButton*)GetDlgItem(IDC_BUTTON_TOEND) ;
	m_button[BUTTON_PICCATCH] = (CButton*)GetDlgItem(IDC_BUTTON_PICCATCH) ;
	m_button[BUTTON_STOP]     = (CButton*)GetDlgItem(IDC_BUTTON_STOP) ;
	m_button[BUTTON_GETERROR] = (CButton*)GetDlgItem(IDC_BUTTON_GETERROR) ;
	m_button[BUTTON_FULLSCREEN] = (CButton*)GetDlgItem(IDC_FULLSCREEN) ;
	for(int i = 0 ; i < (int)BUTTON_TOTAL; i++)
		m_button[i]->EnableWindow(FALSE) ;

	PLAY_SetFileEndMsg(PORT,m_hWnd,WM_FILE_END);
	PLAY_SetDisplayBuf(PORT,15) ;
	PLAY_RigisterDrawFun(PORT,DrawFun,0) ;

	//Timer
	//SetTimer(1,1000,NULL) ;
	timeBeginPeriod(10);
	m_iTimerID = timeSetEvent(1000, 3, timer_proc, (unsigned int)this, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);

	//File Slider Control
	
	m_sliderplay.SetRangeMin(0);
	m_sliderplay.SetRangeMax(200);
	m_sliderplay.SetPos(0);
	m_sliderplay.SetLineSize(1);
	m_sliderplay.SetPageSize(5);

	//Audio Slider Control
	m_slidersound.SetRangeMin(0);
	m_slidersound.SetRangeMax(0xffff-1);
	int volume = PLAY_GetVolume(PORT) ;
	m_slidersound.SetPos(volume);
	m_slidersound.SetLineSize(1);
	m_slidersound.SetPageSize(5);

	//Set Color
	m_sliderB.SetRange(0, 128);
	m_sliderB.SetPos(64);
	m_sliderC.SetRange(0, 128);
	m_sliderC.SetPos(64);
	m_sliderS.SetRange(0, 128);
	m_sliderS.SetPos(64);
	m_sliderH.SetRange(0, 128);
	m_sliderH.SetPos(64);

	m_BMPOverlay.LoadBitmap(IDB_OVERLAY);
	m_BMPBlack.LoadBitmap(IDB_BITMAP);

	m_bNeedFileClose = 0;
	m_nDecType = 0;		

	f_audio = fopen("c:\\sys_audio.pcm","w+b") ;
	f_video = fopen("C:\\sys_video.264","w+b") ;
	f_complex = fopen("C:\\sys_complex.data","w+b") ;
	
	GetCurrentDirectory(1024, g_strCurDir.GetBuffer(1024)) ;

 	OnLanEnglish() ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPlayer264demoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayer264demoDlg::OnPaint() 
{
	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

	//	SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	};
	
	if(m_IsPlaying)
	{	
		PLAY_RefreshPlay(PORT);
		SetWindowSize();
	}
	else;	
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayer264demoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPlayer264demoDlg::OnPlay() 
{
	// TODO: Add your control notification handler code here
	//PLAY_PlaySound(PORT) ;
	
	m_playsound = FALSE;

	PLAY_SetDecCallBack(PORT,DecCBFun) ;		//Set Dec CallBack

//	PLAY_SetDecCallBack(PORT,DecCBFunEx);
	PLAY_SetDecCBStream(PORT,m_nDecType);	////////////////////////
	
// 	PLAY_SetOverlayMode(PORT,TRUE,RGB(255,0,255));
	PLAY_PlaySound(PORT) ;
	PLAY_Play(PORT,m_playregion.m_hWnd) ;
	PLAY_SetColor(PORT, 0, m_sliderB.GetPos(), m_sliderC.GetPos(), m_sliderS.GetPos(), m_sliderH.GetPos());

	UINT state = m_pMainMenu->GetMenuState(IDC_VERTICALSYNC, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);
	PLAY_VerticalSyncEnable(PORT, (state & MF_CHECKED)? TRUE : FALSE);
	
	m_IsPlaying = true ;
	m_playnormal = true ;
	g_playspeed = 0 ;
	for(int i = 0 ; i < (int)BUTTON_TOTAL; i++)
	{
		if (i == (int)BUTTON_PLAY)
			m_button[i]->EnableWindow(FALSE) ;
		else if ((!g_IndexCreated) && (/*i == (int)BUTTON_ONE || */i == (int)BUTTON_BACKONE))
			m_button[i]->EnableWindow(FALSE) ;
		else m_button[i]->EnableWindow(TRUE) ;
	}
	m_pMainMenu->EnableMenuItem(ID_FILE_CLOSE,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_LOCATE,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_SEARCH,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_RESETBUF,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_PART_DISPLAY,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_DEC_AUDIO,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(ID_DEC_VIDEO,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(ID_DEC_COMPLEX,MF_GRAYED) ;

	
	if((HBITMAP)m_BMPOverlay!=m_playregion.GetBitmap())
		m_playregion.SetBitmap(m_BMPOverlay);
	
	m_bNeedFileClose = 1;

	m_playsound = FALSE;
	UpdateData(0);
	SetWindowSize();
}

void CPlayer264demoDlg::OnFileOpen() 
{
	CString tempfilename ;

	CFileDialog FileChooser(TRUE, 
					NULL,
					NULL, 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"All files(*.*)|*.*|h264 Files (*.264)|*.264|Mpeg4 Files (*.mp4)|*.mp4|LB Files (*.dav)|*.dav|HB Files (*.mpeg)|*.mpeg||");
	//choose file
	if (FileChooser.DoModal()==IDOK)
	{
		tempfilename = FileChooser.GetPathName() ;
	}
	else 
	{
		return ;
	}

	PlayFile(tempfilename);
}

void CPlayer264demoDlg::PlayFile(CString csFilePath)
{
	
	// TODO: Add your command handler code here
	if (m_IsPlaying)
	{		
		OnStop() ;					
	}

	if (m_bNeedFileClose == 1)
	{
		PLAY_CloseFile(PORT) ;
		m_bNeedFileClose = 0;
	}

	g_IndexCreated = false ;
	m_IsPlaying    = false ;
	m_pause        = false ;
	m_playnormal   = false ;		
	m_playsound = 0;
	
	m_button[BUTTON_PLAY]->EnableWindow(FALSE) ;

	UpdateData(0);

//	PLAY_SetFileRefCallBack(PORT,FileRefDone,(DWORD)this);   
	PLAY_SetFileRefCallBackEx(PORT,FileRefDoneEx,(DWORD)this); 
	PLAY_SetEncTypeChangeCallBack(PORT,EncTypeChange,(DWORD)this);
//	PLAY_SetPicQuality(PORT,TRUE);
	//PLAY_SetPicQuality(PORT,FALSE);
//	BOOL ifOk;
//	PLAY_GetPictureQuality(PORT,&ifOk);

//	m_pMainMenu->EnableMenuItem(ID_FILE_OPEN,MF_GRAYED) ;

	if (csFilePath == fileName && m_fileindexlen > 0)//If the file is the same as the last file，and the last file can create index
	{
		//cope the file index of last file 	
		PLAY_SetRefValue(PORT,m_fileindexbuf,m_fileindexlen) ;

		if (!PLAY_OpenFile(PORT,(LPSTR)(LPCTSTR)fileName))
		{			
			AfxMessageBox(ConvertString("File Open Error"));
			return;
		}
	}
	else 
	{
		fileName = csFilePath ;
		if (m_fileindexbuf != NULL)
		{
			delete m_fileindexbuf ;
			m_fileindexbuf = NULL ;
		}
		m_fileindexlen = 0 ;
		if (!PLAY_OpenFile(PORT,(LPSTR)(LPCTSTR)fileName))
		{
			AfxMessageBox(ConvertString("File Open Error"));
			return;
		}
	}
	
	m_pMainMenu->EnableMenuItem(IDC_CUTFILE,MF_ENABLED) ;
	m_pMainMenu->CheckMenuItem(IDC_VERTICALSYNC, MF_UNCHECKED | MF_BYCOMMAND);

	OnPlay() ;	
}

void CPlayer264demoDlg::OnPause() 
{
	// TODO: Add your control notification handler code here
	if (!m_IsPlaying)
	{
		return ;
	}

	m_pause = !m_pause ;
	int z = PLAY_Pause(PORT,(int)m_pause) ;
	
	if (!m_pause && !m_playnormal )
	{
		m_button[BUTTON_PLAY]->EnableWindow(TRUE) ;
	}
	if (m_pause)
	{		
		m_button[BUTTON_PLAY]->EnableWindow(FALSE) ;
		m_button[BUTTON_PAUSE]->SetWindowText(ConvertString("Resume")) ;
	}
	else
	{
		m_button[BUTTON_PAUSE]->SetWindowText(ConvertString("Pause")) ;
	}
	SavePicTmp();
}

void CPlayer264demoDlg::OnFast() 
{
	// TODO: Add your control notification handler code here
	if (m_IsPlaying)
	{
		if (!m_pause)
		{
			m_button[BUTTON_PLAY]->EnableWindow(TRUE) ;
		}

		m_playnormal = false ;
		int z = PLAY_Fast(PORT) ;

		if ( (++g_playspeed) >= 4 )
		{
			g_playspeed = 4 ;
			m_button[BUTTON_FAST]->EnableWindow(FALSE) ;
		}
	
		m_button[BUTTON_SLOW]->EnableWindow(TRUE) ;
	}
}

void CPlayer264demoDlg::OnSlow() 
{
	// TODO: Add your control notification handler code here
	if (m_IsPlaying)
	{
		if (PLAY_Slow(PORT) == FALSE)
		{
			return ;
		}

		if (!m_pause)
		{
			m_button[BUTTON_PLAY]->EnableWindow(TRUE) ;
		}
		m_playnormal = false ;
		
		if ( (--g_playspeed) <= -4 )
		{
			g_playspeed = -4 ;
			m_button[BUTTON_SLOW]->EnableWindow(FALSE) ;
		}
	
		m_button[BUTTON_FAST]->EnableWindow(TRUE) ;
	}
}

void CPlayer264demoDlg::OnBackone() 
{
	// TODO: Add your control notification handler code here 
	if (m_IsPlaying)
	{
		if (!m_pause)
		{
			m_button[BUTTON_PLAY]->EnableWindow(TRUE) ;
		}
		m_playnormal = false ;
		
		PLAY_OneByOneBack(PORT) ;
	}
}

void CPlayer264demoDlg::OnOne() 
{
	// TODO: Add your control notification handler code here
	if (m_IsPlaying)
	{
		if (!m_pause)
		{
			m_button[BUTTON_PLAY]->EnableWindow(TRUE) ;
		}

		if (g_playspeed >= 4)
		{
			m_button[BUTTON_FAST]->EnableWindow(TRUE) ;
		}
		else if (g_playspeed <= -4)
		{
			m_button[BUTTON_SLOW]->EnableWindow(TRUE) ;
		}

		g_playspeed  = 0;
		m_playnormal = false ;

		PLAY_OneByOne(PORT) ;
	}
}

void CPlayer264demoDlg::OnTobegin() 
{
	// TODO: Add your control notification handler code here
	if (!m_IsPlaying)
	{
		return ;
	}
	if (g_IndexCreated)
	{
		BOOL iRet = PLAY_SetCurrentFrameNum(PORT,0) ;
		if (iRet == FALSE)
		{
			PLAY_SetPlayedTimeEx(PORT,0) ;
		}
	}
	else 
	{
		PLAY_SetPlayPos(PORT,0) ;
	}
}

void CPlayer264demoDlg::OnToend() 
{
	// TODO: Add your control notification handler code here
	if (!m_IsPlaying)
	{
		return ;
	}

	if (g_IndexCreated)
	{
		int endframe = PLAY_GetFileTotalFrames(PORT);

		//如果已在最后位置则返回，并避免颜色参数有调整的情况下图像数据被重复修改
		if (PLAY_GetCurrentFrameNum(PORT) == (endframe-1))
		{
			return;
		}
//		while(!PLAY_SetCurrentFrameNum(PORT,endframe --))
//		{
//			//TRACE("FrameNum is :%d\n",nEndFrame);
//			if(endframe==0)
//				break;
//		}	
		BOOL iRet = PLAY_SetCurrentFrameNum(PORT,endframe - 1) ;
		if (iRet == FALSE)
		{
			PLAY_SetPlayedTimeEx(PORT, (PLAY_GetFileTime(PORT)-1)*1000 ) ;
		}
	
	}
	else 
	{
		PLAY_SetPlayPos(PORT,(float)0.99) ;
	}	

}

void CPlayer264demoDlg::OnPiccatch() 
{
	// TODO: Add your control notification handler code here
	static i = 0 ;
	
	if (!m_IsPlaying)
	{
		return ;
	}
	char imagefile[100] ;
	if (g_IndexCreated)
	{
		//sprintf(bmpfile,"c:\\Frame%d.bmp",PLAY_GetCurrentFrameNum(PORT)) ;
		sprintf(imagefile,"c:\\Frame%d.jpg",PLAY_GetCurrentFrameNum(PORT)) ;
	}
	else
	{
		//sprintf(bmpfile,"c:\\aaaaaa\\BMP%d.bmp",i++) ;
		sprintf(imagefile,"c:\\aaaaaa\\jpg%d.jpg",i++) ;
	}

	//PLAY_CatchPic(PORT,bmpfile);
	PLAY_CatchPicEx(PORT,imagefile,PicFormat_JPEG) ;
}

void CPlayer264demoDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	
	if (g_IndexCreated)
	{ 
		PLAY_GetRefValue(PORT,NULL,&m_fileindexlen) ;
		m_fileindexbuf = new BYTE[m_fileindexlen+1] ;
		PLAY_GetRefValue(PORT,m_fileindexbuf,&m_fileindexlen) ;
		
	}
	
	if (m_IsPlaying)
	{
		PLAY_Stop(PORT);
	}
	
	m_playregion.SetBitmap(m_bitmap) ;

	//m_IsPlaying	   = true;
	m_pause = false ;
		m_button[BUTTON_PAUSE]->SetWindowText(ConvertString("Pause")) ;
	m_IsPlaying = false ;
	SetWindowSize();

	for (int i = 0 ; i < (int)BUTTON_TOTAL ; i++)
	{
		if (i == (int)BUTTON_PLAY)
			m_button[i]->EnableWindow(TRUE) ;
		else m_button[i]->EnableWindow(FALSE) ;
	}
	
	m_pMainMenu->EnableMenuItem(ID_LOCATE,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(ID_SEARCH,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(ID_RESETBUF,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(ID_PART_DISPLAY,MF_GRAYED) ;

	m_pMainMenu->EnableMenuItem(ID_DEC_AUDIO,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_DEC_VIDEO,MF_ENABLED) ;
	m_pMainMenu->EnableMenuItem(ID_DEC_COMPLEX,MF_ENABLED) ;
	
	m_pMainMenu->EnableMenuItem(ID_OPEN_LAST_FILE,MF_ENABLED) ;
//	m_playregion.SetBitmap(m_BMPBlack);
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DWORD nVer=PLAY_GetSdkVersion();
	DWORD nVersion=(nVer>>16)&0x000000ff;
	DWORD nBuildVer=(nVer>>8)&0x000000ff;	
	DWORD nMendVer = nVer & 0x000000ff;	
	DWORD nDemoVersion=(DEMOVERSION>>16)&0x0000ffff;
	DWORD nDemoBuild=DEMOVERSION&0x0000ffff;

	CString str ;
//	str.Format("dhplay.dll ver is %04x,build is %02x,mend is %02x\ndhplaydemo ver is%04x,build is %04x\n",
//		nVersion,nBuildVer,nMendVer,nDemoVersion,nDemoBuild) ;
	str.Format("dhplay.dll %d.%d.%d\n", nVersion,nBuildVer,nMendVer) ;

	GetDlgItem(IDC_STATIC_VER)->SetWindowText(str) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayer264demoDlg::OnFileClose() 
{
	// TODO: Add your command handler code here
	
	if (!m_IsPlaying)
	{
		m_pMainMenu->EnableMenuItem(ID_FILE_CLOSE,MF_GRAYED) ;
		m_button[BUTTON_PLAY]->EnableWindow(FALSE) ;
		return ;
	}
	OnStop() ;
	PLAY_CloseFile(PORT) ;
	//m_IsPlaying	   = true;
	//SetWindowSize();
	//m_IsPlaying    = false ;
	g_IndexCreated = false ;	
	m_pause        = false ;
	m_playnormal   = false ;
	m_pMainMenu->EnableMenuItem(ID_FILE_CLOSE,MF_GRAYED) ;
	m_pMainMenu->EnableMenuItem(IDC_CUTFILE,MF_GRAYED) ;
	m_button[BUTTON_PLAY]->EnableWindow(FALSE) ;
	//m_playregion.SetBitmap(m_BMPBlack);
	m_bNeedFileClose = 0;
}

void CPlayer264demoDlg::OnAbout() 
{
	// TODO: Add your command handler code here
	CAboutDlg dlg ;
	dlg.DoModal() ;
}

void CPlayer264demoDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
//	
	if(nIDEvent == 1)
	{
	 	StartDraw();
	}

	CDialog::OnTimer(nIDEvent);
}

void CPlayer264demoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	DWORD nTime;
	int nBrightness, nContrast, nSaturation, nHue;
	int nPlayPosition;

	switch(GetWindowLong(pScrollBar->m_hWnd, GWL_ID))
	{
	case IDC_SLIDER_PLAY:
		if(m_IsPlaying)
		{
			nPlayPosition=m_sliderplay.GetPos();
		
			if (abs(nPlayPosition-m_nPrePlayPosition)<1 && nPlayPosition != 0)
			{
				break;
			}

			if (g_IndexCreated)
			{
				nTime= nPlayPosition * 5 * PLAY_GetFileTime(PORT);			
				PLAY_SetPlayedTimeEx(PORT,nTime);
			}
			else 
			{
				PLAY_SetPlayPos(PORT,nPlayPosition/(200.0)) ;
			}
					
			StartDraw();			
		}
		else;
		break ;		
	case IDC_SLIDERS :
	case IDC_SLIDERB :
	case IDC_SLIDERC :
	case IDC_SLIDERH :
		nBrightness = m_sliderB.GetPos();
		nContrast = m_sliderC.GetPos();
		nSaturation = m_sliderS.GetPos();
		nHue = m_sliderH.GetPos();			
		PLAY_SetColor(PORT, 0, nBrightness, nContrast, nSaturation, nHue);
	
		break ;
	}
		
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPlayer264demoDlg::OnLocate() 
{
	// TODO: Add your command handler code here
	if (!m_IsPlaying)
	{
		AfxMessageBox(ConvertString("Please Open File First")) ;
		return ;
	}
	Locate dlg ;
	dlg.DoModal() ;
}

void CPlayer264demoDlg::OnRepeat() 
{
	// TODO: Add your command handler code here
	if (m_IsRepeatPlay)
	{
		m_pMainMenu->CheckMenuItem(ID_REPEAT,MF_UNCHECKED );
		m_IsRepeatPlay=false;
	}
	else
	{
		m_pMainMenu->CheckMenuItem(ID_REPEAT,MF_CHECKED );
		m_IsRepeatPlay=true;
	}
}
void CPlayer264demoDlg::PlayMessage(WPARAM wParam, LPARAM lParam)
{
	MSG msgVal;
	while(PeekMessage(&msgVal,m_hWnd,WM_FILE_END,WM_FILE_END,PM_REMOVE))
		;
	
	if(m_IsRepeatPlay)
	{
		if (g_IndexCreated)
		{
			BOOL iRet = PLAY_SetCurrentFrameNum(PORT,0);
			if (iRet == FALSE)
			{
				PLAY_SetPlayedTimeEx(PORT, 0) ;
			}
		}
		else PLAY_SetPlayPos(PORT,0) ;
	}
}

void CPlayer264demoDlg::OnResetbuf() 
{
	// TODO: Add your command handler code here
	if (m_IsPlaying)
		PLAY_ResetBuffer(PORT,BUF_VIDEO_RENDER) ;
}


void CPlayer264demoDlg::OnExit() 
{
	// TODO: Add your command handler code here
	CPlayer264demoDlg::OnOK() ;
}

void CPlayer264demoDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
}

void CPlayer264demoDlg::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	// TODO: Add your message handler code here
	if (m_IsPlaying)
		PLAY_RefreshPlay(PORT) ;
	
}
void CALLBACK DrawFun(long nPort,HDC hDc,LONG nUser) 
{
}

void CALLBACK EncTypeChange(long nPort,long User)
{
	CPlayer264demoDlg* pOwner=(CPlayer264demoDlg*)User;
	pOwner->SetWindowSize();
}

void CALLBACK funAudio(long nPort, char * pAudioBuf,long nSize, long nStamp, long nType, long nUser)
{
//	audio_cb.Write(pAudioBuf,nSize) ;
}

void  CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
	TRACE("nPort=%d,TYPE=%d;Width=%d;Height=%d\n",nPort,pFrameInfo->nType,pFrameInfo->nWidth,pFrameInfo->nHeight);
	char fname[100];
	
	switch (m_nDecType)
	{
	case 1:
		fwrite(pBuf, nSize, 1, f_video);
		fflush(f_video) ;
		break;
	case 2:
		fwrite(pBuf, nSize, 1, f_audio) ;
		fflush(f_audio) ;
		break;
	case 3:
		fwrite(pBuf, nSize, 1, f_complex) ;
		fflush(f_complex) ;
		break;
	default:
		return;
	}
}


void  CALLBACK DecCBFunEx(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
	TRACE("nPort=%d,TYPE=%d;Width=%d;Height=%d\n",nPort,pFrameInfo->nType,pFrameInfo->nWidth,pFrameInfo->nHeight);
	char fname[100];
	
	switch (m_nDecType)
	{
	case 1:
		fwrite(pBuf, nSize, 1, f_video);
		fflush(f_video) ;
		break;
	case 2:
		fwrite(pBuf, nSize, 1, f_audio) ;
		fflush(f_audio) ;
		break;
	case 3:
		fwrite(pBuf, nSize, 1, f_complex) ;
		fflush(f_complex) ;
		break;
	default:
		return;
	}

}

void CALLBACK FileRefDone(DWORD nReserved,DWORD nUser)
{
	g_IndexCreated = true ;
	CPlayer264demoDlg * dlg = (CPlayer264demoDlg*)nUser ;

	float fPos = PLAY_GetPlayPos(0);
	PLAY_SetPlayPos(0, fPos);

	dlg->m_button[CPlayer264demoDlg::BUTTON_ONE]->EnableWindow(TRUE) ;
	dlg->m_button[CPlayer264demoDlg::BUTTON_BACKONE]->EnableWindow(TRUE) ;
}

void CALLBACK FileRefDoneEx(DWORD nReserved, BOOL bIndexCreated, DWORD nUser)
{
	g_IndexCreated = bIndexCreated ;
	CPlayer264demoDlg * dlg = (CPlayer264demoDlg*)nUser ;
	
	float fPos = PLAY_GetPlayPos(0);
	PLAY_SetPlayPos(0, fPos);
	
	dlg->m_button[CPlayer264demoDlg::BUTTON_ONE]->EnableWindow(TRUE) ;
	dlg->m_button[CPlayer264demoDlg::BUTTON_BACKONE]->EnableWindow(TRUE) ;
}
void CPlayer264demoDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	int volume = 0xffff - m_slidersound.GetPos() ;
	PLAY_SetVolume(PORT,volume) ;
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPlayer264demoDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	OnClose();
	
	// TODO: Add your message handler code here
	
}

void CPlayer264demoDlg::OnPartDisplay() 
{
	// TODO: Add your command handler code here
	MultiDisplay dlg;
	if (dlg.nRegionNum > 1)
	{
		return;
	}
	dlg.DoModal();
}

void CPlayer264demoDlg::OnSysinfo() 
{
	// TODO: Add your command handler code here
	SYSINFO sysinfodlg ;
	sysinfodlg.DoModal() ;
}

void CPlayer264demoDlg::OnVerticalSync()
{
	UINT state = m_pMainMenu->GetMenuState(IDC_VERTICALSYNC, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);

	if (!m_IsPlaying) return;

	if (state & MF_CHECKED)
	{
		m_pMainMenu->CheckMenuItem(IDC_VERTICALSYNC, MF_UNCHECKED | MF_BYCOMMAND);
		PLAY_VerticalSyncEnable(PORT, FALSE);
	}
	else
	{
		m_pMainMenu->CheckMenuItem(IDC_VERTICALSYNC, MF_CHECKED | MF_BYCOMMAND);
		PLAY_VerticalSyncEnable(PORT, TRUE);
	}
}

void CPlayer264demoDlg::OnCheckSound() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true) ;
	if (m_playsound)
	{
		PLAY_StopSound() ;
	}
	else 
	{
		PLAY_PlaySound(PORT) ;
	}
}

void CPlayer264demoDlg::OnCutfile()
{
	// TODO: Add your command handler code here
	if (g_IndexCreated == false){
		AfxMessageBox(ConvertString("File index not created")) ;
		return ;
	}
	cutfile cutfiledlg(fileName) ;
	cutfiledlg.DoModal() ;
}


void CPlayer264demoDlg::OnButtonGeterror() 
{
	// TODO: Add your control notification handler code here
	int errorid = PLAY_GetLastError(PORT);
	char errorstr[50];
	switch(errorid)
	{
	case 0:
		sprintf(errorstr,"PLAY_NOERROR");
		break;
	case 1:
		sprintf(errorstr,"PLAY_PARA_OVER");
		break;
	case 2:
		sprintf(errorstr,"PLAY_ORDER_ERROR");
		break;
	case 3:
		sprintf(errorstr,"PLAY_TIMER_ERROR");
		break;
	case 4:
		sprintf(errorstr,"PLAY_DEC_VIDEO_ERROR");
		break;
	case 5:
		sprintf(errorstr,"PLAY_DEC_AUDIO_ERROR");
		break;
	case 6:
		sprintf(errorstr,"PLAY_ALLOC_MEMORY_ERROR");
		break;
	case 7:
		sprintf(errorstr,"PLAY_OPEN_FILE_ERROR");
		break;
	case 8:
		sprintf(errorstr,"PLAY_CREATE_OBJ_ERROR");
		break;
	case 9:
		sprintf(errorstr,"PLAY_CREATE_DDRAW_ERROR");
		break;
	case 10:
		sprintf(errorstr,"PLAY_CREATE_OFFSCREEN_ERROR");
		break;
	case 11:
		sprintf(errorstr,"PLAY_BUF_OVER");
		break;
	case 12:
		sprintf(errorstr,"PLAY_CREATE_SOUND_ERROR");
		break;
	case 13:
		sprintf(errorstr,"PLAY_SET_VOLUME_ERROR");
		break;
	case 14:
		sprintf(errorstr,"PLAY_SUPPORT_FILE_ONLY");
		break;
	case 15:
		sprintf(errorstr,"PLAY_SUPPORT_STREAM_ONLY");
		break;
	case 16:
		sprintf(errorstr,"PLAY_SYS_NOT_SUPPORT");
		break;
	case 17:
		sprintf(errorstr,"PLAY_FILEHEADER_UNKNOWN");
		break;
	case 18:
		sprintf(errorstr,"PLAY_VERSION_INCORRECT");
		break;
	case 19:
		sprintf(errorstr,"PLAY_INIT_DECODER_ERROR");
		break;
	case 20:
		sprintf(errorstr,"PLAY_CHECK_FILE_ERROR");
		break;
	case 21:
		sprintf(errorstr,"PLAY_INIT_TIMER_ERROR");
		break;
	case 22:
		sprintf(errorstr,"PLAY_BLT_ERROR");
		break;
	case 23:
		sprintf(errorstr,"PLAY_UPDATE_ERROR");
		break;
	default:
		sprintf(errorstr,"PLAY_NOERROR");
	}
	AfxMessageBox(errorstr);
	
}

void CPlayer264demoDlg::OnWndFullScreen()
{
	if(m_IsPlaying)
	{
		m_bIsFullScreen = !m_bIsFullScreen;
		
	
		CButton *pButton;
		CStatic* pStatic;
		if(m_bIsFullScreen)
		{
			
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PLAY);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PAUSE);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_FAST);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_SLOW);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_BACKONE);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_ONE);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_TOBEGIN);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_TOEND);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PICCATCH);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_STOP);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_GETERROR);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			pButton = (CButton *)GetDlgItem(IDC_FULLSCREEN);		
			pButton->ModifyStyle(WS_VISIBLE,0,0);
			
			m_sliderB.ModifyStyle(WS_VISIBLE,0,0);
			m_sliderC.ModifyStyle(WS_VISIBLE,0,0);
			m_sliderS.ModifyStyle(WS_VISIBLE,0,0);
			m_sliderH.ModifyStyle(WS_VISIBLE,0,0);
			m_sliderplay.ModifyStyle(WS_VISIBLE,0,0);
			m_slidersound.ModifyStyle(WS_VISIBLE,0,0);
			
			pStatic=(CStatic*)GetDlgItem(IDC_STATICB);
			pStatic->ModifyStyle(WS_VISIBLE,0,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICS);
			pStatic->ModifyStyle(WS_VISIBLE,0,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICL);
			pStatic->ModifyStyle(WS_VISIBLE,0,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICD);
			pStatic->ModifyStyle(WS_VISIBLE,0,0);

			GetDlgItem(IDC_GROUPBOX1)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_STATIC1)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_STATIC2)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_STATIC_FRAME)->ModifyStyle(WS_VISIBLE,0,0);
			GetDlgItem(IDC_STATIC_TIME)->ModifyStyle(WS_VISIBLE,0,0);

			GetDlgItem(IDC_CHECK_SOUND)->ModifyStyle(WS_VISIBLE,0,0);



			GetWindowPlacement(&m_OldWndplacement);

			//Remove WS_SIZEBOX windows style. or not the window can't be full-creen.
			ModifyStyle(WS_SIZEBOX,0,0);
			
			CRect WindowRect, ClientRect;
			RECT m_FullScreenRect;

			//ReDraw the window. Get the correct edge value.
			GetWindowRect(&WindowRect);
			WindowRect.left+=1;
			WindowRect.right+=1;
			MoveWindow(CRect(0,0,352,288),TRUE);
			
			GetWindowRect(&WindowRect);
			GetClientRect(&ClientRect);
			ClientToScreen(&ClientRect);
		
			//get the dest window rect.
			int x = GetSystemMetrics(SM_CXSCREEN);
			int y = GetSystemMetrics(SM_CYSCREEN);
			m_FullScreenRect.left = WindowRect.left - ClientRect.left;
			m_FullScreenRect.top = WindowRect.top - ClientRect.top;
			m_FullScreenRect.right = WindowRect.right - ClientRect.right + x;
			m_FullScreenRect.bottom = WindowRect.bottom - ClientRect.bottom + y;

//			m_FullScreenRect.left = 0;
//			m_FullScreenRect.top = 0; 
//			m_FullScreenRect.right = GetSystemMetrics(SM_CXSCREEN);
//			m_FullScreenRect.bottom = GetSystemMetrics(SM_CYSCREEN);

			//Move the main window to the dest rect.
			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof(WINDOWPLACEMENT);
			wndpl.flags = 0;
			wndpl.showCmd = SW_SHOWNORMAL;
			wndpl.rcNormalPosition = m_FullScreenRect;
			SetWindowPlacement(&wndpl);

			//Move the view winow to full-screen.
			RECT rc;
			GetClientRect(&rc);
			GetDlgItem(IDC_STATIC_SHOW)->MoveWindow(&rc,TRUE);

		}
		else
		{
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PLAY);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PAUSE);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_FAST);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_SLOW);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_BACKONE);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_ONE);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_TOBEGIN);	
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_TOEND);	
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_PICCATCH);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_STOP);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_BUTTON_GETERROR);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			pButton = (CButton *)GetDlgItem(IDC_FULLSCREEN);		
			pButton->ModifyStyle(0,WS_VISIBLE,0);
			
			m_sliderB.ModifyStyle(0,WS_VISIBLE,0);
			m_sliderC.ModifyStyle(0,WS_VISIBLE,0);
			m_sliderS.ModifyStyle(0,WS_VISIBLE,0);
			m_sliderH.ModifyStyle(0,WS_VISIBLE,0);
			m_sliderplay.ModifyStyle(0,WS_VISIBLE,0);
			m_slidersound.ModifyStyle(0,WS_VISIBLE,0);
			
			pStatic=(CStatic*)GetDlgItem(IDC_STATICB);
			pStatic->ModifyStyle(0,WS_VISIBLE,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICS);
			pStatic->ModifyStyle(0,WS_VISIBLE,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICL);
			pStatic->ModifyStyle(0,WS_VISIBLE,0);
			pStatic=(CStatic*)GetDlgItem(IDC_STATICD);
			pStatic->ModifyStyle(0,WS_VISIBLE,0);

			GetDlgItem(IDC_GROUPBOX1)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_GROUPBOX2)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_STATIC1)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_STATIC2)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_STATIC_FRAME)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_STATIC_TIME)->ModifyStyle(0,WS_VISIBLE,0);
			GetDlgItem(IDC_CHECK_SOUND)->ModifyStyle(0,WS_VISIBLE,0);
			ModifyStyle(0,WS_SIZEBOX,0);
			
			
			SetWindowPlacement(&m_OldWndplacement);
			RECT rc;
			GetClientRect(&rc);
			rc.right  -= 1;
			rc.bottom -= 132;
			GetDlgItem(IDC_STATIC_SHOW)->MoveWindow(&rc,TRUE);
			SetWindowSize();
		}

		this->RedrawWindow();

		PLAY_RefreshPlay(PORT);
	}
	else;
}

void CPlayer264demoDlg::OnStaticShow() 
{
	// TODO: Add your control notification handler code here	
}

void CPlayer264demoDlg::OnFullscreen() 
{
	// TODO: Add your control notification handler code here
	OnWndFullScreen();
}

void CPlayer264demoDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect pt;
	ClientToScreen(&point);
	GetDlgItem(IDC_STATIC_SHOW)->GetWindowRect(&pt);
	if( point.x > pt.left && point.x < pt.right && point.y > pt.top && point.y < pt.bottom)
	{
		OnWndFullScreen();
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CPlayer264demoDlg::OnCustomdrawSliderPlay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CPlayer264demoDlg::StartDraw()
{
	if (m_IsPlaying)
	{
		int volume = PLAY_GetVolume(PORT) ;
		m_slidersound.SetPos(0xffff - volume) ;

		float fTime = PLAY_GetPlayedTimeEx(PORT)/1000.0;
		float fDotNum = fTime - ((int)fTime/10)*10- (int)fTime%10;
		fTime = fDotNum >= 0.50? ((int)fTime)+1.0 : fTime;
		DWORD nCTime = fTime;
	
		DWORD nHour = (nCTime / 3600) % 24;
		DWORD nMinute = (nCTime % 3600) / 60;
		DWORD nSecond = nCTime % 60;
		int nPos = 0;
		DWORD m_nMaxFileTime = PLAY_GetFileTime(PORT);

		DWORD total_hour = m_nMaxFileTime/3600;
		DWORD total_min = (m_nMaxFileTime%3600)/60;
		DWORD total_sec = m_nMaxFileTime%60;
		
		DWORD nCFrame = PLAY_GetCurrentFrameNum(PORT)+1;

		if (PLAY_GetFileTime(PORT) && g_IndexCreated)
		{			
			nPos = (int)(fTime*200/PLAY_GetFileTime(PORT));
			
			if (m_nPrePlayPosition<nPos)
			{
				nPos = fTime*200/PLAY_GetFileTime(PORT);		
			}	
		}
		else
		{
			nPos = PLAY_GetPlayPos(0) * 200;
		}

		m_sliderplay.SetPos(nPos);

		UINT uiLangstate = m_pMainMenu->GetMenuState(ID_MENUITEM_Chinese, MF_BYCOMMAND);
		
		CString str1,str2 ;	
		
		if (g_IndexCreated)
		{
			if (uiLangstate & MF_CHECKED)
			{
				str1.Format("当前帧数:%d   总帧数:%d", PLAY_GetCurrentFrameNum(PORT)+1, PLAY_GetFileTotalFrames(PORT)) ;		
				str2.Format("当前时间 %d:%d:%d  总时间 %d:%d:%d",nHour,nMinute,nSecond,total_hour,total_min,total_sec) ;
			}
			else
			{
				str1.Format("current frame number:%d   total frame number:%d", PLAY_GetCurrentFrameNum(PORT)+1, PLAY_GetFileTotalFrames(PORT)) ;		
				str2.Format("current time %d:%d:%d  total time %d:%d:%d",nHour,nMinute,nSecond,total_hour,total_min,total_sec) ;
			}
		}
		else
		{
			if (uiLangstate & MF_CHECKED)
			{
				str1.Format("已播放帧数:   %d",PLAY_GetPlayedFrames(PORT)) ;
				str2.Format("当前帧率:   %d",PLAY_GetCurrentFrameRate(PORT)) ;	
			}
			else
			{
				str1.Format("played frame number:   %d",PLAY_GetPlayedFrames(PORT)) ;
				str2.Format("current frame rate:   %d",PLAY_GetCurrentFrameRate(PORT)) ;	
			}
		}	
		
		CString csCaption("dhplaydemo");
		CString csBitRate;
		double frameBitRate = 0.0;
		PLAY_GetRealFrameBitRate(PORT, &frameBitRate);
		csBitRate.Format("Frame Bit Rate: %.3fKbps", frameBitRate);
		csCaption = csCaption + "   " + csBitRate;
		SetWindowText(csCaption);

		GetDlgItem(IDC_STATIC_FRAME)->SetWindowText(str1) ;
		GetDlgItem(IDC_STATIC_TIME)->SetWindowText(str2) ;	
	}
	else
	{
		m_sliderplay.SetPos(0) ;
		SetWindowText("dhplaydemo");
		GetDlgItem(IDC_STATIC_FRAME)->SetWindowText("") ;
		GetDlgItem(IDC_STATIC_TIME)->SetWindowText("") ;	
	}

}

void CPlayer264demoDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
}

void CPlayer264demoDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CPlayer264demoDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CPlayer264demoDlg::SavePicTmp() 
{
	// TODO: Add your message handler code here and/or call default

}

void CPlayer264demoDlg::OnCustomdrawSliderSound(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CPlayer264demoDlg::OnDecAudio() 
{
	// TODO: Add your command handler code here
	m_pMainMenu->CheckMenuItem(ID_DEC_NONE,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_AUDIO,MF_CHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_VIDEO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_COMPLEX,MF_UNCHECKED );
	m_nDecType = 2;
}

void CPlayer264demoDlg::OnDecVideo() 
{
	// TODO: Add your command handler code here
	m_pMainMenu->CheckMenuItem(ID_DEC_NONE,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_AUDIO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_VIDEO,MF_CHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_COMPLEX,MF_UNCHECKED );
	m_nDecType = 1;
}

void CPlayer264demoDlg::OnDecComplex() 
{
	// TODO: Add your command handler code here
	m_pMainMenu->CheckMenuItem(ID_DEC_NONE,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_AUDIO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_VIDEO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_COMPLEX,MF_CHECKED );
	m_nDecType = 3;
}

void CPlayer264demoDlg::OnDecNone() 
{
	// TODO: Add your command handler code here
	m_pMainMenu->CheckMenuItem(ID_DEC_NONE,MF_CHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_AUDIO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_VIDEO,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_DEC_COMPLEX,MF_UNCHECKED );
	m_nDecType = 0;
}

void CPlayer264demoDlg::SetWindowSize()
{
	if(m_IsPlaying)
	{
		if (!m_bIsFullScreen)
		{
				CRect rcWindow,rcClient;
				GetWindowRect(&rcWindow);
				if (rcWindow.top < -2000)
				{
					return;
				}

				DWORD oldx = (rcWindow.TopLeft()).x;
				DWORD oldy = (rcWindow.TopLeft()).y;

				GetClientRect(&rcClient);
				int m_nDlgEdge=(rcWindow.Width()-rcClient.Width())>>1;
				ClientToScreen(&rcClient);
				int m_nDlgTopSize=rcClient.top-rcWindow.top;
				long m_nWidth,m_nHeight;
				
				int ret = PLAY_GetPictureSize(PORT,&m_nWidth,&m_nHeight);
				
				
//				m_nWidth *=2;
//				m_nHeight *=2;
				static long width = m_nWidth ;
				static long height = m_nHeight;
				if (ret==0)
				{
					m_nWidth = width;
					m_nHeight = height;
				}
				else;
				
				DWORD m_nWindowHeight =m_nHeight+180;
				DWORD m_nWindowWidth = 705+m_nDlgEdge;	

				long x = 0;
				if (m_nWindowWidth <= m_nWidth)
				{
					x = (m_nWidth - m_nWindowWidth)/2;
					m_nWindowWidth = m_nWidth;
				}

				long y = m_nHeight;
				GetDlgItem(IDC_SLIDER_PLAY)->MoveWindow(x+5,y+5,695,20,TRUE);

				GetDlgItem(IDC_BUTTON_PLAY)->MoveWindow(x+5,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_PAUSE)->MoveWindow(x+5,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_FAST)->MoveWindow(x+70,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_SLOW)->MoveWindow(x+70,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_BACKONE)->MoveWindow(x+135,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_ONE)->MoveWindow(x+135,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_TOBEGIN)->MoveWindow(x+200,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_TOEND)->MoveWindow(x+200,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_PICCATCH)->MoveWindow(x+265,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_STOP)->MoveWindow(x+265,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_GETERROR)->MoveWindow(x+328,y+34,64,20,TRUE);
				GetDlgItem(IDC_FULLSCREEN)->MoveWindow(x+328,y+58,64,20,TRUE);
				
				GetDlgItem(IDC_GROUPBOX1)->MoveWindow(x+395,y+28,244,50,TRUE);
				GetDlgItem(IDC_STATICB)->MoveWindow(x+400,y+42,40,15,TRUE);
				GetDlgItem(IDC_SLIDERS)->MoveWindow(x+440,y+42,80,15,TRUE);
				GetDlgItem(IDC_STATICL)->MoveWindow(x+525,y+42,40,15,TRUE);
				GetDlgItem(IDC_SLIDERB)->MoveWindow(x+555,y+42,80,15,TRUE);
				GetDlgItem(IDC_STATICD)->MoveWindow(x+400,y+60,40,15,TRUE);
				GetDlgItem(IDC_SLIDERC)->MoveWindow(x+440,y+60,80,15,TRUE);
				GetDlgItem(IDC_STATICS)->MoveWindow(x+525,y+60,40,15,TRUE);
				GetDlgItem(IDC_SLIDERH)->MoveWindow(x+555,y+60,80,15,TRUE);

				GetDlgItem(IDC_STATIC1)->MoveWindow(x+5,y+80,320,40,TRUE);
				GetDlgItem(IDC_STATIC_FRAME)->MoveWindow(x+10,y+95,310,20,TRUE);
				GetDlgItem(IDC_STATIC2)->MoveWindow(x+330,y+80,310,40,TRUE);
				GetDlgItem(IDC_STATIC_TIME)->MoveWindow(x+335,y+95,300,20,TRUE);
				
				GetDlgItem(IDC_GROUPBOX2)->MoveWindow(x+645,y+28,50,92,TRUE);
				GetDlgItem(IDC_SLIDER_SOUND)->MoveWindow(x+660,y+40,15,60,TRUE);
				GetDlgItem(IDC_CHECK_SOUND)->MoveWindow(x+650,y+100,40,18,TRUE);

				GetDlgItem(IDC_STATIC_SHOW)->MoveWindow(((m_nWidth>705)? 0:x+(705-m_nWidth)/2),0,m_nWidth,m_nHeight,TRUE);
				
				MoveWindow(	oldx, oldy, m_nWindowWidth,	m_nWindowHeight, TRUE);
				
		}
		else;
	}
	else
	{
		if (!m_bIsFullScreen)
		{
				CRect rcWindow,rcClient;
				GetWindowRect(&rcWindow);
				DWORD oldx = (rcWindow.TopLeft()).x;
				DWORD oldy = (rcWindow.TopLeft()).y;

				GetClientRect(&rcClient);
				int m_nDlgEdge=(rcWindow.Width()-rcClient.Width())>>1;
				ClientToScreen(&rcClient);
				int m_nDlgTopSize=rcClient.top-rcWindow.top;
				int m_nWidth,m_nHeight;
				
				m_nWidth = 704;
				m_nHeight = 576;
				static int width = m_nWidth ;
				static int height = m_nHeight;
				
				DWORD m_nWindowHeight =m_nHeight+180;
				DWORD m_nWindowWidth = 705+m_nDlgEdge;	

				int y = m_nHeight;
				GetDlgItem(IDC_SLIDER_PLAY)->MoveWindow(5,y+5,695,20,TRUE);

				GetDlgItem(IDC_BUTTON_PLAY)->MoveWindow(5,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_PAUSE)->MoveWindow(5,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_FAST)->MoveWindow(70,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_SLOW)->MoveWindow(70,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_BACKONE)->MoveWindow(135,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_ONE)->MoveWindow(135,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_TOBEGIN)->MoveWindow(200,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_TOEND)->MoveWindow(200,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_PICCATCH)->MoveWindow(265,y+34,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_STOP)->MoveWindow(265,y+58,60,20,TRUE);
				GetDlgItem(IDC_BUTTON_GETERROR)->MoveWindow(328,y+34,64,20,TRUE);
				GetDlgItem(IDC_FULLSCREEN)->MoveWindow(328,y+58,64,20,TRUE);

				
				GetDlgItem(IDC_GROUPBOX1)->MoveWindow(395,y+28,244,50,TRUE);
				GetDlgItem(IDC_STATICB)->MoveWindow(400,y+42,40,15,TRUE);
				GetDlgItem(IDC_SLIDERS)->MoveWindow(440,y+42,80,15,TRUE);
				GetDlgItem(IDC_STATICL)->MoveWindow(525,y+42,40,15,TRUE);
				GetDlgItem(IDC_SLIDERB)->MoveWindow(555,y+42,80,15,TRUE);
				GetDlgItem(IDC_STATICD)->MoveWindow(400,y+60,40,15,TRUE);
				GetDlgItem(IDC_SLIDERC)->MoveWindow(440,y+60,80,15,TRUE);
				GetDlgItem(IDC_STATICS)->MoveWindow(525,y+60,40,15,TRUE);
				GetDlgItem(IDC_SLIDERH)->MoveWindow(555,y+60,80,15,TRUE);

				GetDlgItem(IDC_STATIC1)->MoveWindow(5,y+80,320,40,TRUE);
				GetDlgItem(IDC_STATIC_FRAME)->MoveWindow(10,y+95,310,20,TRUE);
				GetDlgItem(IDC_STATIC2)->MoveWindow(330,y+80,310,40,TRUE);
				GetDlgItem(IDC_STATIC_TIME)->MoveWindow(335,y+95,300,20,TRUE);
				
				GetDlgItem(IDC_GROUPBOX2)->MoveWindow(645,y+28,50,92,TRUE);
				GetDlgItem(IDC_SLIDER_SOUND)->MoveWindow(660,y+40,15,60,TRUE);
				GetDlgItem(IDC_CHECK_SOUND)->MoveWindow(650,y+100,40,18,TRUE);

				GetDlgItem(IDC_STATIC_SHOW)->MoveWindow((705-m_nWidth)/2,0,m_nWidth,m_nHeight,TRUE);
				
				MoveWindow(	oldx, oldy, m_nWindowWidth,	m_nWindowHeight, TRUE);
				
		}
		else;
	}
	//CPlayer264demoDlg::OnPaint();

}

void CPlayer264demoDlg::OnLanChinese() 
{
	// TODO: Add your command handler code here
	if (strcmp("Chinese", g_pCurInterface.pLanguage.LangName) == 0)
	{
		return ;
	}

	m_pMainMenu->CheckMenuItem(ID_MENUITEM_Chinese,MF_CHECKED);
	m_pMainMenu->CheckMenuItem(ID_MENUITEM_English,MF_UNCHECKED );
	

	strcpy(g_pCurInterface.pLanguage.LangName,"Chinese") ;
	strcpy(g_pCurInterface.pLanguage.IniFile,"dhchs_play.ini") ;

	SetMenuStaticText(m_pMainMenu) ;
	SetWndStaticText(this) ;
	DrawMenuBar() ;
}

void CPlayer264demoDlg::OnLanEnglish() 
{
	// TODO: Add your command handler code here
	if (strcmp("English", g_pCurInterface.pLanguage.LangName) == 0)
	{
		return ;
	}

	m_pMainMenu->CheckMenuItem(ID_MENUITEM_Chinese,MF_UNCHECKED );
	m_pMainMenu->CheckMenuItem(ID_MENUITEM_English,MF_CHECKED );
	
	strcpy(g_pCurInterface.pLanguage.LangName,"English") ;
	strcpy(g_pCurInterface.pLanguage.IniFile,"dheng_play.ini") ;

 	SetMenuStaticText(m_pMainMenu) ;
	SetWndStaticText(this) ;
 	DrawMenuBar() ;
}

void CPlayer264demoDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	PLAY_Stop(PORT) ;
	if(m_bNeedFileClose == 1)
	{
		PLAY_CloseFile(PORT) ;
		m_bNeedFileClose = 0;
	}
	
	fclose(f_complex) ;
	fclose(f_audio) ;
	fclose(f_video) ;
	if (m_fileindexbuf != NULL)
	{
		delete m_fileindexbuf ;
		m_fileindexbuf = NULL ;
	}

	timeKillEvent(m_iTimerID);
	timeEndPeriod(10);
	
	CDialog::OnClose();
}

void CPlayer264demoDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR *lpszFileName = new TCHAR[MAX_PATH];
	
	UINT nDragNum;
	UINT nFileNum;
	
	nDragNum = ::DragQueryFile(hDropInfo, 0xFFFFFFFF,lpszFileName, MAX_PATH);
	
	for (int i = 0; i < 1/*nDragNum*/; i++)
	{
		::DragQueryFile(hDropInfo, i, lpszFileName, MAX_PATH);
		
		CString csFilePath;
		csFilePath.Format("%s", lpszFileName);
		PlayFile(csFilePath);			
	}
	
	::DragFinish (hDropInfo); //释放内存
	delete []lpszFileName;
	lpszFileName = NULL;
}
