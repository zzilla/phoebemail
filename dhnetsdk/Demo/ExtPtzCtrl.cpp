// ExtPtzCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "netsdkdemo.h"
#include "ExtPtzCtrl.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExtPtzCtrl dialog


CExtPtzCtrl::CExtPtzCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CExtPtzCtrl::IDD, pParent)
{
	m_DeviceID = 0;
	m_Channel = 0;
	m_presetPoint = 0;
	m_cruiseGroup = 0;
	m_modeNo = 0;
	//{{AFX_DATA_INIT(CExtPtzCtrl)
	//}}AFX_DATA_INIT
}


void CExtPtzCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExtPtzCtrl)
	DDX_Control(pDX, IDC_COMBO_AUX_NO, m_auxNosel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExtPtzCtrl, CDialog)
	//{{AFX_MSG_MAP(CExtPtzCtrl)
	ON_BN_CLICKED(IDC_PRESET_ADD, OnPresetAdd)
	ON_BN_CLICKED(IDC_PRESET_DELE, OnPresetDele)
	ON_BN_CLICKED(IDC_PRESET_SET, OnPresetSet)
	ON_BN_CLICKED(IDC_CRUISE_ADD_POINT, OnCruiseAddPoint)
	ON_BN_CLICKED(IDC_CRUISE_DEL_POINT, OnCruiseDelPoint)
	ON_BN_CLICKED(IDC_CRUISE_DEL_GROUP, OnCruiseDelGroup)
	ON_BN_CLICKED(IDC_START_CRUISE, OnStartCruise)
	ON_BN_CLICKED(IDC_STOP_CRUISE, OnStopCruise)
	ON_BN_CLICKED(IDC_LAMP_ACTIVATE, OnLampActivate)
	ON_BN_CLICKED(IDC_LAMP_DEACTIVATE, OnLampDeactivate)
	ON_BN_CLICKED(IDC_ROTATE_START, OnRotateStart)
	ON_BN_CLICKED(IDC_ROTATE_STOP, OnRotateStop)
	ON_BN_CLICKED(IDC_LINE_SET_LEFT, OnLineSetLeft)
	ON_BN_CLICKED(IDC_LINE_SET_RIGHT, OnLineSetRight)
	ON_BN_CLICKED(IDC_LINE_START, OnLineStart)
	ON_BN_CLICKED(IDC_LINE_STOP, OnLineStop)
	ON_BN_CLICKED(IDC_MODE_SET_BEGIN, OnModeSetBegin)
	ON_BN_CLICKED(IDC_MODE_SET_DELETE, OnModeSetDelete)
	ON_BN_CLICKED(IDC_MODE_SET_END, OnModeSetEnd)
	ON_BN_CLICKED(IDC_MODE_START, OnModeStart)
	ON_BN_CLICKED(IDC_MODE_STOP, OnModeStop)
	ON_BN_CLICKED(IDC_QUERY_ALARM, OnQueryAlarm)
	ON_BN_CLICKED(IDC_FAST_GO, OnFastGo)
	ON_BN_CLICKED(IDC_AUX_OPEN, OnAuxOpen)
	ON_BN_CLICKED(IDC_AUX_CLOSE, OnAuxClose)
	ON_BN_CLICKED(IDC_LIGHT_OPEN, OnLightOpen)
	ON_BN_CLICKED(IDC_LIGHT_CLOSE, OnLightClose)
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_BN_CLICKED(IDC_TEST2, OnTest2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExtPtzCtrl message handlers

BOOL CExtPtzCtrl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	SetDlgItemInt(IDC_PRESET_DATA,1);
	SetDlgItemInt(IDC_CRUISE_GROUP,1);
	SetDlgItemInt(IDC_MODE_NO, 1);
	SetDlgItemInt(IDC_POS_X, 0);
	SetDlgItemInt(IDC_POS_Y, 0);
	SetDlgItemInt(IDC_POS_ZOOM, 0);
	
	m_auxNosel.SetCurSel(0);
	return TRUE; 
}

void CExtPtzCtrl::SetExtPtzParam(LONG iHandle, int iChannel)
{
	m_DeviceID = iHandle;
	m_Channel = iChannel;
}


void CExtPtzCtrl::OnPresetAdd() 
{
	PtzExtControl(PTZ_POINT_SET_CONTROL);
}

void CExtPtzCtrl::OnPresetDele() 
{
	PtzExtControl(PTZ_POINT_DEL_CONTROL);
}

void CExtPtzCtrl::OnPresetSet() 
{
	PtzExtControl(PTZ_POINT_MOVE_CONTROL);
}

void CExtPtzCtrl::PtzExtControl(DWORD dwCommand, DWORD dwParam)
{
	BOOL ret = FALSE;
	BYTE bParam[3] = {0};
	DWORD dwPm[3] = {0};

	if(!m_DeviceID)
	{
		MessageBox(ConvertString(MSG_PTZCTRL_NOCHANNEL));
	}
	
	switch(dwCommand) 
	{
	case PTZ_POINT_MOVE_CONTROL:     //转至
    case PTZ_POINT_SET_CONTROL:     //设置
    case PTZ_POINT_DEL_CONTROL:      //删除
		m_presetPoint = GetDlgItemInt(IDC_PRESET_DATA);
		bParam[1] = (BYTE)m_presetPoint;
		break;
	case EXTPTZ_ADDTOLOOP:
	case EXTPTZ_DELFROMLOOP:
		m_presetPoint = GetDlgItemInt(IDC_PRESET_DATA);
		m_cruiseGroup = GetDlgItemInt(IDC_CRUISE_GROUP);
		bParam[0] = (BYTE)m_cruiseGroup;
		bParam[1] = (BYTE)m_presetPoint;
		break;
	case EXTPTZ_CLOSELOOP:
		m_cruiseGroup = GetDlgItemInt(IDC_CRUISE_GROUP);
		bParam[0] = (BYTE)m_cruiseGroup;
		break;
	case PTZ_POINT_LOOP_CONTROL:
		if (0 == dwParam) 
		{
			m_cruiseGroup = GetDlgItemInt(IDC_CRUISE_GROUP);
			bParam[0] = (BYTE)m_cruiseGroup;
			bParam[2] = 76;
		}
		else
		{
			m_cruiseGroup = GetDlgItemInt(IDC_CRUISE_GROUP);
			bParam[0] = (BYTE)m_cruiseGroup;
			bParam[2] = 96;
		}
		break;
	case PTZ_LAMP_CONTROL:
		if (0 == dwParam) 
		{
			//open
			bParam[0] = 0x01;
		}
		else
		{
			//close
			bParam[0] = 0x00;
		}
		break;
	case EXTPTZ_LIGHTCONTROL:
		bParam[0] = GetDlgItemInt(IDC_LIGHT_CONTROLLER);
		bParam[1] = GetDlgItemInt(IDC_LIGHT_NO);
		if (0 == dwParam) 
		{
			//open
			bParam[2] = 1;
		}
		else
		{
			//close
			bParam[2] = 0;
		}
		break;
	case EXTPTZ_STARTPANCRUISE:		//开始水平旋转	
	case EXTPTZ_STOPPANCRUISE:		//停止水平旋转
	case EXTPTZ_SETLEFTBORDER:		//设置左边界
	case EXTPTZ_SETRIGHTBORDER:		//设置右边界
	case EXTPTZ_STARTLINESCAN:		//开始线扫
	case EXTPTZ_CLOSELINESCAN:		//停止线扫	
	case EXTPTZ_SETMODESTART:		//设置模式开始	
		break;
	case EXTPTZ_SETMODESTOP:		//设置模式结束
	case EXTPTZ_RUNMODE:			//运行模式
	case EXTPTZ_STOPMODE:			//停止模式
	case EXTPTZ_DELETEMODE:			//清除模式
		m_modeNo = GetDlgItemInt(IDC_MODE_NO);
		bParam[0] = (BYTE)m_modeNo;
		break;
//	case EXTPTZ_REVERSECOMM:
//		break;
	case EXTPTZ_AUXIOPEN:
	case EXTPTZ_AUXICLOSE:
		switch(m_auxNosel.GetCurSel())
		{
		case 0:
			m_auxNo = 23;
			break;
		case 1:
			m_auxNo = 24;
			break;
		case 2:
			m_auxNo = 27;
			break;
		case 3:
			m_auxNo = 41;
			break;
		case 4:
			m_auxNo = 43;
			break;
		default:
			return;		
		}
		bParam[0] = m_auxNo;
		break;
	case EXTPTZ_FASTGOTO:
		{
			m_pos_x = GetDlgItemInt(IDC_POS_X);
			m_pos_y = GetDlgItemInt(IDC_POS_Y);
			m_pos_zoom = GetDlgItemInt(IDC_POS_ZOOM);
			dwPm[0] = m_pos_x;
			dwPm[1] = m_pos_y;
			dwPm[2] = m_pos_zoom;
			ret = CLIENT_YWPTZControlEx(m_DeviceID, m_Channel,dwCommand ,dwPm[0],dwPm[1],dwPm[2],FALSE);
			if(!ret)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();//Peng Dongfeng 06.11.24
				MessageBox(ConvertString(MSG_PTZCTRL_CTRLFAILED));
			}
		}
		break;
	case EXTPTZ_UP_TELE:
		bParam[0] = dwParam;
		break;
	default: 
		return;
	}
	ret = CLIENT_YWPTZControlEx(m_DeviceID, m_Channel,dwCommand ,bParam[0],bParam[1],bParam[2],FALSE);
	if(!ret)
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Peng Dongfeng 06.11.24
		MessageBox(ConvertString(MSG_PTZCTRL_CTRLFAILED));
	}
}

void CExtPtzCtrl::OnCruiseAddPoint() 
{
	PtzExtControl(EXTPTZ_ADDTOLOOP);
}

void CExtPtzCtrl::OnCruiseDelPoint() 
{
	PtzExtControl(EXTPTZ_DELFROMLOOP);
}

void CExtPtzCtrl::OnCruiseDelGroup() 
{
	PtzExtControl(EXTPTZ_CLOSELOOP);
}

void CExtPtzCtrl::OnStartCruise() 
{
	PtzExtControl(PTZ_POINT_LOOP_CONTROL, 0);
}

void CExtPtzCtrl::OnStopCruise() 
{
	PtzExtControl(PTZ_POINT_LOOP_CONTROL, 1);
}

void CExtPtzCtrl::OnLampActivate() 
{
	PtzExtControl(PTZ_LAMP_CONTROL, 0);
}

void CExtPtzCtrl::OnLampDeactivate() 
{
	PtzExtControl(PTZ_LAMP_CONTROL, 1);
}

void CExtPtzCtrl::OnRotateStart() 
{
	PtzExtControl(EXTPTZ_STARTPANCRUISE);
}

void CExtPtzCtrl::OnRotateStop() 
{
	PtzExtControl(EXTPTZ_STOPPANCRUISE);
}

void CExtPtzCtrl::OnLineSetLeft() 
{
	PtzExtControl(EXTPTZ_SETLEFTBORDER);
}

void CExtPtzCtrl::OnLineSetRight() 
{
	PtzExtControl(EXTPTZ_SETRIGHTBORDER);
}

void CExtPtzCtrl::OnLineStart() 
{
	PtzExtControl(EXTPTZ_STARTLINESCAN);	
}

void CExtPtzCtrl::OnLineStop() 
{
	PtzExtControl(EXTPTZ_CLOSELINESCAN);
}

void CExtPtzCtrl::OnModeSetBegin() 
{
	PtzExtControl(EXTPTZ_SETMODESTART);
}

void CExtPtzCtrl::OnModeSetDelete() 
{
	PtzExtControl(EXTPTZ_DELETEMODE);
}

void CExtPtzCtrl::OnModeSetEnd() 
{
	PtzExtControl(EXTPTZ_SETMODESTOP);
}

void CExtPtzCtrl::OnModeStart() 
{
	PtzExtControl(EXTPTZ_RUNMODE);
}

void CExtPtzCtrl::OnModeStop() 
{
	PtzExtControl(EXTPTZ_STOPMODE);
}

void CExtPtzCtrl::OnQueryAlarm() 
{
//	PtzExtControl(EXTPTZ_REVERSECOMM);
}

void CExtPtzCtrl::OnFastGo() 
{
	PtzExtControl(EXTPTZ_FASTGOTO);
}

void CExtPtzCtrl::OnAuxOpen() 
{
	PtzExtControl(EXTPTZ_AUXIOPEN);
}

void CExtPtzCtrl::OnAuxClose() 
{
	PtzExtControl(EXTPTZ_AUXICLOSE);
}

void CExtPtzCtrl::OnLightOpen() 
{
	PtzExtControl(EXTPTZ_LIGHTCONTROL, 0);
}

void CExtPtzCtrl::OnLightClose() 
{
	PtzExtControl(EXTPTZ_LIGHTCONTROL, 1);
}

void CExtPtzCtrl::OnTest() 
{
	PtzExtControl(EXTPTZ_UP_TELE, 5);
}

void CExtPtzCtrl::OnTest2() 
{
	PtzExtControl(PTZ_LAMP_CONTROL, 1);
}
