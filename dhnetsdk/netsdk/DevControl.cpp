
#include "StdAfx.h"
#include "DevControl.h"
#include "Manager.h"
#include "netsdktypes.h"
#include "../dvr/kernel/afkinc.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "DevConfig.h"
#include "Utils_StrParser.h"
#include "devconfigex.h"
#include "decoderdevice.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevControl::CDevControl(CManager *pManager)
	: m_pManager(pManager)
{
}

CDevControl::~CDevControl()
{
}

int CDevControl::Init()
{
	return 0;
}

int CDevControl::Uninit()
{
	int nRet = 0;
	{
		m_csTCI.Lock();

		list<st_TransCom_Info*>::iterator it = m_lstTCI.begin();
		while(it != m_lstTCI.end())
		{
			if (*it)
			{
				bool b = (*it)->channel->close((*it)->channel);
				if (false == b)
				{
					nRet = -1;
				}

				delete (*it);
			}

			m_lstTCI.erase(it++);
		}

		m_csTCI.UnLock();
	}

	{
		m_csUI.Lock();

		list<st_Upgrade_Info*>::iterator it = m_lstUI.begin();
		while(it != m_lstUI.end())
		{
			if (*it)
			{
				bool b = (*it)->channel->close((*it)->channel);
				if (false == b)
				{
					nRet = -1;
				}

				if ((*it)->pcsLock)
				{
					delete (*it)->pcsLock;
				}
				
				CloseEventEx((*it)->hRecEvent);

				delete (*it);
			}

			m_lstUI.erase(it++);
		}

		m_csUI.UnLock();
	}
	return nRet;
}

int CDevControl::CloseChannelOfDevice(afk_device_s* device)
{
	int nRet = 0;

	{
		m_csTCI.Lock();

		list<st_TransCom_Info*>::iterator it = m_lstTCI.begin();
		while(it!=m_lstTCI.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
				if (_device == device)
				{
					bool b = (*it)->channel->close((*it)->channel);
					if (false == b)
					{
						nRet = -1;
					}

					delete (*it);
					m_lstTCI.erase(it++);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		m_csTCI.UnLock();
	}

	{
		m_csUI.Lock();

		list<st_Upgrade_Info*>::iterator it = m_lstUI.begin();
		while(it!=m_lstUI.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
				if (_device == device)
				{
					bool b = (*it)->channel->close((*it)->channel);
					if (false == b)
					{
						nRet = -1;
					}

					delete (*it);
					m_lstUI.erase(it++);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		m_csUI.UnLock();
	}

	return nRet;
}

int __stdcall TransComFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
	CDevControl::st_TransCom_Info* pTCI = (CDevControl::st_TransCom_Info*)udata;
	if (!pTCI)
	{
		return -1;
	}

	InterlockedIncrementEx(&(pTCI->life));

	pTCI->pcsLock->Lock();

	afk_device_s *device = 0;
	afk_channel_s *channel = 0;
	
    channel = (afk_channel_s*)object;
	if (!channel)
	{
		goto e_out;
	}

    device = (afk_device_s*)channel->get_device(channel);
	if (!device)
	{
		goto e_out;
	}
    
    if (pTCI->cbFunc)
    { 
        pTCI->cbFunc((LONG)device, (LONG)channel, 
            (char*)data, datalen, pTCI->dwUser);
    }	

	pTCI->pcsLock->UnLock();
	if (InterlockedDecrementEx(&(pTCI->life)) <= 0)
	{
		delete pTCI->pcsLock;
		delete pTCI;
	}
    return 1;
e_out:
	pTCI->pcsLock->UnLock();
	if (InterlockedDecrementEx(&(pTCI->life)) <= 0)
	{
		delete pTCI->pcsLock;
		delete pTCI;
	}
	return 0;
}

LONG CDevControl::CreateTransComChannel(LONG lLoginID, int TransComType, 
                                      unsigned int baudrate, unsigned int databits,
                                      unsigned int stopbits, unsigned int parity,
                                      fTransComCallBack cbTransCom, DWORD dwUser)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

    afk_device_s *device = (afk_device_s*)lLoginID;
	afk_channel_s *pchannel = 0;
    afk_trans_channel_param_s transchannel = {0};

	st_TransCom_Info* pTCI = new st_TransCom_Info;
	if (!pTCI)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clear;
	}

	pTCI->pcsLock = new DEVMutex;
	if (!pTCI->pcsLock)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clear;
	}
	
	InterlockedSetEx(&pTCI->life, 1);

    pTCI->device = device;
    pTCI->channel = 0;
    pTCI->cbFunc = cbTransCom;
    pTCI->dwUser = dwUser;
    pTCI->transComtype = TransComType;

    transchannel.base.func = TransComFunc;
	transchannel.base.udata= pTCI;

    transchannel.trans_device_type = TransComType;
    transchannel.baudrate = baudrate;
    transchannel.databits = databits;
    transchannel.stopbits = stopbits;
    transchannel.parity = parity;

    pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_TRANS, &transchannel);
    if (pchannel)
    {
        pTCI->channel = pchannel;

		m_csTCI.Lock();
		m_lstTCI.push_back(pTCI);
		m_csTCI.UnLock();
    }
	else
	{
		goto e_clear;
	}

    return (LONG)pchannel;

e_clear:
	if (pchannel) 
	{
		pchannel->close(pchannel);
		pchannel = 0;
	}

	if (pTCI)
	{
		if (pTCI->pcsLock)
		{
			delete pTCI->pcsLock;
			pTCI->pcsLock = 0;
		}
		delete pTCI;
		pTCI = 0;
	}
	return 0;
}

int CDevControl::SendTransComData(LONG lTransComChannel, char *pBuffer, DWORD dwBufSize)
{
	if (!pBuffer)
	{
		return NET_ERROR;
	}

	int nRet = NET_ERROR;
    afk_channel_s *channel = (afk_channel_s*)lTransComChannel;

	m_csTCI.Lock();
	list<st_TransCom_Info*>::iterator it = 
		find_if(m_lstTCI.begin(),m_lstTCI.end(),SearchTCI(lTransComChannel));
	
	if (it != m_lstTCI.end())
	{
        afk_trans_info_s transinfo = {0};
        transinfo.data = pBuffer;
        transinfo.datalen = dwBufSize;
		int r = (*it)->channel->set_info((*it)->channel, (*it)->transComtype, (void*)&transinfo);
        if (r)
        {
            nRet = NET_NOERROR;
        }
        else
        {
            nRet = NET_ERROR;
        }		
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

	m_csTCI.UnLock();
	return nRet;
}

int CDevControl::DestroyTransComChannel(LONG lTransComChannel)
{
	int nRet = NET_ERROR;
    afk_channel_s *channel = (afk_channel_s*)lTransComChannel;

	m_csTCI.Lock();
	list<st_TransCom_Info*>::iterator it = 
		find_if(m_lstTCI.begin(),m_lstTCI.end(),SearchTCI(lTransComChannel));
	
	if (it != m_lstTCI.end())
	{
		//与回调函数互斥
		DEVMutex *pcs = (*it)->pcsLock;
		pcs->Lock();

		bool b = (*it)->channel->close((*it)->channel);
		if (b)
		{
			nRet = NET_NOERROR;
			if (InterlockedDecrementEx(&((*it)->life)) <= 0)
			{
				delete (*it);
				pcs->UnLock();
				delete pcs;
				m_lstTCI.erase(it);
				goto END;
			}
			m_lstTCI.erase(it);
		}
		else
		{
			nRet = NET_CLOSE_CHANNEL_ERROR;
		}

		pcs->UnLock();
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

END:
	m_csTCI.UnLock();
	return nRet;
}

/*现在已经废掉了Reset接口，该函数实现可能有问题*/
int CDevControl::Reset(LONG lLoginID, BOOL bReset)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return -1;
    }

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_control_channel_param_s controlchannel;
    controlchannel.type1 = AFK_CONTROL_TYPE_RESET;
    controlchannel.parm1 = !bReset;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONTROL, &controlchannel);
    if (pchannel)
    {
        pchannel->close(pchannel);
        return 0;
    }

    return -1;
}

int __stdcall UpgradeFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
	CDevControl::st_Upgrade_Info* pUI = (CDevControl::st_Upgrade_Info*)udata;
	if (!pUI)
	{
		return -1;
	}

	InterlockedIncrementEx(&(pUI->life));

	pUI->pcsLock->Lock();

	afk_channel_s *channel = 0;
	afk_device_s *device = 0;
    channel = (afk_channel_s*)object;
	if (!channel)
	{
		goto e_out;
	}

    device = (afk_device_s*)channel->get_device(channel);
	if (!device)
	{
		goto e_out;
	}

	switch((long)param)
	{
	case 0: //升级中
		if (pUI->cbUpgrade)
		{
			pUI->cbUpgrade((LONG)device, (LONG)channel, 
				(long)data, datalen, pUI->dwUser);
		}
		break;
	case 1: //可以传送升级数据
		SetEventEx(pUI->hRecEvent);
		break;
	case 2: //升级完成
		if (pUI->cbUpgrade)
		{
			pUI->cbUpgrade((LONG)device, (LONG)channel, 
				0, -1, pUI->dwUser);
		}
		break;
	case 3: //升级出错
		if (pUI->cbUpgrade)
		{
			pUI->cbUpgrade((LONG)device, (LONG)channel, 
				0, -2, pUI->dwUser);
		}
		break;
	case 4: //升级进度
		if (pUI->cbUpgrade)
		{
			pUI->cbUpgrade((LONG)device, (LONG)channel,
				-1, *data, pUI->dwUser);
		}
		break;
	default:
		break;

		/*
		case 10: //数据错乱
		break;
		case 11: //系统忙，暂时无法升级
		break;
		case 12: //升级包校验失败
		break;
		case 13:  //升级包非法（不同机器等）
		break;
		case 14: //升级失败
		break;
		case 15: //该操作员无权升级设备
		break;
		default:
		break;
		*/
	}

	pUI->pcsLock->UnLock();
	if (InterlockedDecrementEx(&(pUI->life)) <= 0)
	{
		CloseEventEx(pUI->hRecEvent);
		delete pUI->pcsLock;
		delete pUI;
	}
    return 1;
e_out:
	pUI->pcsLock->UnLock();
	if (InterlockedDecrementEx(&(pUI->life)) <= 0)
	{
		CloseEventEx(pUI->hRecEvent);
		delete pUI->pcsLock;
		delete pUI;
	}
	return 0;
}

LONG CDevControl::StartUpgrade(LONG lLoginID, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if (!pchFileName)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

    afk_device_s *device = (afk_device_s*)lLoginID;

    st_Upgrade_Info* pUI = new st_Upgrade_Info;
	if (!pUI)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}

	afk_channel_s *pchannel = 0;
	afk_upgrade_channel_param_s upgradechannel = {0};

	FILE *file = fopen(pchFileName, "rb");
    if (file)
    {
        fpos_t pos;
        fseek(file, 0, SEEK_END);
        fgetpos(file, &pos);
#ifdef WIN32
        upgradechannel.size = pos;
#else	//linux
        upgradechannel.size = pos.__pos;
#endif
    }
    else
    {
		delete pUI;
		m_pManager->SetLastError(NET_ERROR);
        return 0;
    }

	//判断升级文件类型，应用程序也应该做这部分工作，以免升错文件
	//...新协议中不区分升级类型，升级类型由设备去判断。升级包的头两个字节("DH"和"PK")
	int ret = 0;
	int nUpgradeFlag = 0;
	char tmpbuf[3];
	memset(tmpbuf, 0, 3);
	fseek(file, 0, SEEK_SET);
	fread(tmpbuf, 1, 2, file);
	if ((tmpbuf[0] == 'D' && tmpbuf[1] == 'H') || (tmpbuf[0] == 'P' && tmpbuf[1] == 'K'))
	{
		nUpgradeFlag = 1;	//bios
	}
	else if (tmpbuf[0] == 'U')	
	{
		nUpgradeFlag = 2;	//Web
	}
	else if (tmpbuf[0] == (char)0xC0 && tmpbuf[1] == (char)0x80)
	{
		nUpgradeFlag = 3;//boot
		if (upgradechannel.size <= 1202180) 
		{
			 //boot
		}
		else
		{
			//boot and bios
		}
	}
	else
	{
		goto e_clearup;
	}
	fclose(file);
	file = NULL;
	
	upgradechannel.filetype = nUpgradeFlag;
	upgradechannel.type = AFK_CHANNEL_UPLOAD_UPGRADE;

	pUI->pcsLock = new DEVMutex;
	if (!pUI->pcsLock)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
	ret = CreateEventEx(pUI->hRecEvent, TRUE, FALSE);
	if (ret < 0)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
	InterlockedSetEx(&pUI->life, 1);

	pUI->device = device;
	pUI->channel = 0;
	pUI->cbUpgrade = cbUpgrade;
	pUI->dwUser = dwUser;

    upgradechannel.base.func = UpgradeFunc; 
    upgradechannel.base.udata = pUI;
    strcpy(upgradechannel.filename, pchFileName);

    pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_UPGRADE, &upgradechannel);
    if (pchannel)
    {
		DWORD dwRet = WaitForSingleObjectEx(pUI->hRecEvent, 10*WAIT_TIME);
        ResetEventEx(pUI->hRecEvent);
        if (dwRet == WAIT_OBJECT_0)
        {
            pUI->channel = pchannel;

			m_csUI.Lock();
			m_lstUI.push_back(pUI);
			m_csUI.UnLock();
        }
		else
		{
			goto e_clearup;
		}
    }
	else
	{
		goto e_clearup;
	}

    return (LONG)pchannel;

e_clearup:
	if (file)
	{
		fclose(file);
		file = 0;
	}

	if (pchannel)
	{
		pchannel->close(pchannel);
		pchannel = 0;
	}
	
	if (pUI)
	{
		if (pUI->pcsLock)
		{
			delete pUI->pcsLock;
			pUI->pcsLock = 0;
		}
		
		CloseEventEx(pUI->hRecEvent);
		
		delete pUI;
		pUI = 0;
	}

	return 0;
}

LONG CDevControl::StartUpgradeEx(LONG lLoginID, EM_UPGRADE_TYPE emType, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if (!pchFileName)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

    afk_device_s *device = (afk_device_s*)lLoginID;

    st_Upgrade_Info* pUI = new st_Upgrade_Info;
	if (!pUI)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}

	int ret = 0;
	afk_channel_s *pchannel = 0;
	afk_upgrade_channel_param_s upgradechannel = {0};

	FILE *file = fopen(pchFileName, "rb");
    if (file)
    {
        fpos_t pos;
        fseek(file, 0, SEEK_END);
        fgetpos(file, &pos);
#ifdef WIN32
        upgradechannel.size = pos;
#else	//linux
        upgradechannel.size = pos.__pos;
#endif
    }
    else
    {
		delete pUI;
		m_pManager->SetLastError(NET_ERROR);
        return 0;
    }

	fclose(file);
	file = NULL;
	
	upgradechannel.filetype = (int)emType;
	upgradechannel.type = AFK_CHANNEL_UPLOAD_UPGRADE;

	pUI->pcsLock = new DEVMutex;
	if (!pUI->pcsLock)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
	ret = CreateEventEx(pUI->hRecEvent, TRUE, FALSE);
	if (ret < 0)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
	InterlockedSetEx(&pUI->life, 1);

	pUI->device = device;
	pUI->channel = 0;
	pUI->cbUpgrade = cbUpgrade;
	pUI->dwUser = dwUser;

    upgradechannel.base.func = UpgradeFunc; 
    upgradechannel.base.udata = pUI;
    strcpy(upgradechannel.filename, pchFileName);

    pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_UPGRADE, &upgradechannel);
    if (pchannel)
    {
		DWORD dwRet = WaitForSingleObjectEx(pUI->hRecEvent, 10*WAIT_TIME);
        ResetEventEx(pUI->hRecEvent);
        if (dwRet == WAIT_OBJECT_0)
        {
            pUI->channel = pchannel;

			m_csUI.Lock();
			m_lstUI.push_back(pUI);
			m_csUI.UnLock();
        }
		else
		{
			goto e_clearup;
		}
    }
	else
	{
		goto e_clearup;
	}

    return (LONG)pchannel;

e_clearup:
	if (file)
	{
		fclose(file);
		file = 0;
	}

	if (pchannel)
	{
		pchannel->close(pchannel);
		pchannel = 0;
	}

	if (pUI)
	{
		if (pUI->pcsLock)
		{
			delete pUI->pcsLock;
			pUI->pcsLock = 0;
		}
		
		CloseEventEx(pUI->hRecEvent);

		delete pUI;
		pUI = 0;
	}

	return 0;
}

int CDevControl::SendUpgrade(LONG lUpgradeID)
{
	int iRet = NET_ERROR;
    afk_channel_s *channel = (afk_channel_s*)lUpgradeID;
//	EnterCriticalSection(&m_csUI);
	m_csUI.Lock();

	list<st_Upgrade_Info*>::iterator it = 
		find_if(m_lstUI.begin(),m_lstUI.end(),SearchUI(lUpgradeID));

	if (it != m_lstUI.end())
	{
		int r = (*it)->channel->set_info((*it)->channel, 0, 0);
        if (r > 0)
        {
            iRet = NET_NOERROR;
        }
	}
//	LeaveCriticalSection(&m_csUI);
	m_csUI.UnLock();
	return iRet;
}

int CDevControl::StopUpgrade(LONG lUpgradeID)
{
    afk_channel_s *channel = (afk_channel_s*)lUpgradeID;
	if (!lUpgradeID)
	{
		return NET_INVALID_HANDLE;
	}
	
	int nRet = NET_ERROR;

	m_csUI.Lock();
	list<st_Upgrade_Info*>::iterator it = 
		find_if(m_lstUI.begin(),m_lstUI.end(),SearchUI(lUpgradeID));
	if (it != m_lstUI.end())
	{
		if ((*it))
		{
			//与回调函数互斥
			DEVMutex *pcs = (*it)->pcsLock;

			pcs->Lock();
			
			bool b = (*it)->channel->close((*it)->channel);
			if (b)
			{
				nRet = 0;
				if (InterlockedDecrementEx(&((*it)->life)) <= 0)
				{
					pcs->UnLock();
					delete pcs;
					CloseEventEx((*it)->hRecEvent);
					delete (*it);
					m_lstUI.erase(it);
					goto END;
				}
				m_lstUI.erase(it);
			}
			else
			{
				nRet = NET_CLOSE_CHANNEL_ERROR;
			}
			
			pcs->UnLock();
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
END:
	m_csUI.UnLock();
	return nRet;
}

/* 设备重启或关闭返回成功与否的通用回调函数 */ 
int __stdcall DevControlFunc(
						   afk_handle_t object,	/* 数据提供者 */
						   unsigned char *data,	/* 没定义：0 */
						   unsigned int datalen,	/* 没定义：0  */
						   void *param,			/* 回调参数 ：转换成整型值，0－成功，1-无权限, 2-暂时无法执行*/
						   void *udata)
{
	receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1; 
    }
	
	receivedata->result = (int)param;

	SetEventEx(receivedata->hRecEvt);

    return 1;
}

/* 配置通道简单返回正确或者失败的通用回调函数 */
int __stdcall ConfigCBFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 没定义：0 */
	unsigned int datalen,	/* 操作结果: 0 成功，非零 失败  */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	receivedata->result = (datalen==1)?0:-1;

	SetEventEx(receivedata->hRecEvt);

    return 1;
}

int CDevControl::MakeKeyFrame(LONG lLoginID, int nChannelID, int nSubChannel)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = NET_ERROR;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_config_channel_param_s configchannel = {0};
    receivedata_s receivedata;// = {0};

	//receivedata.hRecEvt = m_hRecEvent;
	receivedata.result = 0; //-1;

	configchannel.no = nChannelID;
	configchannel.type = AFK_CHANNEL_CONFIG_FORCE_IFRAME;
	configchannel.chngetIframe.subchn = (unsigned char)nSubChannel;
	configchannel.base.func = ConfigCBFunc;
    configchannel.base.udata = &receivedata;
   
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        DWORD dwRet = WAIT_OBJECT_0; //WaitForSingleObjectEx(receivedata.hRecEvt, WAIT_TIME);
		pchannel->close(pchannel);
        ResetEventEx(receivedata.hRecEvt);
        if (WAIT_OBJECT_0 == dwRet)
        {
            if (-1 == receivedata.result)
            {
				nRet = NET_RETURN_DATA_ERROR;	
            }
            else
            {
				nRet = NET_NOERROR;
            }
        }
		else
		{
			nRet = NET_NETWORK_ERROR;
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

	return nRet;
}

int CDevControl::SetMaxFlux(LONG lLoginID, WORD wFlux)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = NET_ERROR;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_config_channel_param_s configchannel = {0};
    receivedata_s receivedata;// = {0};

	//receivedata.hRecEvt = m_hRecEvent;
	receivedata.result = -1;

	configchannel.limitflux = wFlux;
	configchannel.type = CONFIG_TYPE_LIMIT_BR;
	configchannel.base.func = ConfigCBFunc;
    configchannel.base.udata = &receivedata;
   
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, WAIT_TIME);
        pchannel->close(pchannel);
        ResetEventEx(receivedata.hRecEvt);
		if (WAIT_OBJECT_0 == dwRet)
        {
            if (-1 == receivedata.result)
            {
				nRet = NET_RETURN_DATA_ERROR;	
            }
            else
            {
				nRet = NET_NOERROR;
            }
        }
		else
		{
			nRet = NET_NETWORK_ERROR;
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}
	
	return nRet;
}

int CDevControl::RebootDev(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = NET_ERROR;
	
    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_control_channel_param_s controlchannel = {0};
	receivedata_s receivedata;// = {0};
	
	//receivedata.hRecEvt = m_hRecEvent;
	receivedata.result = -1;

    controlchannel.type1 = AFK_CONTROL_TYPE_DEVCONTROL;
    controlchannel.type2 = 0; 
	controlchannel.parm1 = 1;//重启
	controlchannel.base.func = DevControlFunc;
	controlchannel.base.udata = &receivedata;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONTROL, &controlchannel);
    if (pchannel)
    {
        DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, WAIT_TIME);
        pchannel->close(pchannel);
        ResetEventEx(receivedata.hRecEvt);
        if (WAIT_OBJECT_0 == dwRet)
        {
			switch(receivedata.result)
			{
			case -1:
				nRet = NET_RETURN_DATA_ERROR; //系统出错
				break;
			case 0:
				nRet = NET_NOERROR;
				break;
			case 1:
				nRet = NET_NOT_AUTHORIZED;
				break;
			case 2:
				nRet = NET_NOT_NOW;
				break;
			default:
				nRet = NET_RETURN_DATA_ERROR; //系统出错
				break;
			}
        }
		else
		{
			nRet = NET_NETWORK_ERROR; //超时
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet; 
}

int CDevControl::ShutDownDev(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	int nRet = NET_ERROR;
	
    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_control_channel_param_s controlchannel = {0};
	receivedata_s receivedata;// = {0};
	
	//receivedata.hRecEvt = m_hRecEvent;
	receivedata.result = -1;
	
    controlchannel.type1 = AFK_CONTROL_TYPE_DEVCONTROL;
    controlchannel.type2 = 0; 
	controlchannel.parm1 = 2; //关闭
	controlchannel.base.func = DevControlFunc;
	controlchannel.base.udata = &receivedata;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONTROL, &controlchannel);
    if (pchannel)
    {
        DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, WAIT_TIME);
        pchannel->close(pchannel);
        ResetEventEx(receivedata.hRecEvt);
		if (WAIT_OBJECT_0 == dwRet)
        {
            switch(receivedata.result)
			{
			case -1:
				nRet = NET_RETURN_DATA_ERROR; //系统出错
				break;
			case 0:
				nRet = NET_NOERROR;
				break;
			case 1:
				nRet = NET_NOT_AUTHORIZED;
				break;
			case 2:
				nRet = NET_NOT_NOW;
				break;
			default:
				nRet = NET_RETURN_DATA_ERROR; //系统出错
				break;
			}
        }
		else
		{
			nRet = NET_NETWORK_ERROR; //超时
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int CDevControl::ControlDevice(LONG lLoginID, CtrlType type, void *param, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = NET_ERROR;
	
    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_control_channel_param_s controlchannel = {0};
	receivedata_s receivedata;// = {0};
	
	//receivedata.hRecEvt = m_hRecEvent;
	receivedata.result = -1;

	switch(type)
	{
	case CTRL_TYPE_REBOOT:
		{
			controlchannel.type2 = 0; //关闭、重启及硬盘操作
			controlchannel.parm1 = 1;//重启
		}
		break;
	case CTRL_TYPE_SHUTDOWN:
		{
			controlchannel.type2 = 0; //关闭、重启及硬盘操作
			controlchannel.parm1 = 2;//关闭
		}
		break;
	case CTRL_TYPE_DISK:
		{
			DISKCTRL_PARAM *diskParam = (DISKCTRL_PARAM *)param;
			if (NULL ==diskParam)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int	nDiskIndex = QueryDiskIndex(lLoginID, diskParam->nIndex);
			if (nDiskIndex < 0)
			{
				return NET_QUERY_DISKINFO_FAILED;
			}

			controlchannel.type2 = 0;	//关闭、重启及硬盘操作
			controlchannel.parm1 = 3;	//硬盘操作
			controlchannel.parm2 = (nDiskIndex & 0x0F);		//目标硬盘
			controlchannel.parm3 = diskParam->ctrlType;		//操作类型
			controlchannel.pParm4 = (char*)nDiskIndex;
		}
		break;
	case CTRL_TYPE_DISK_SUBAREA:
		{
			DISKCTRL_SUBAREA *pDiskSubarea = (DISKCTRL_SUBAREA *)param;
			if (NULL ==pDiskSubarea)
			{
				return NET_ILLEGAL_PARAM;
			}

			DISK_SUBAREA_EN stuAbility = {0}; 
			int nRetLen = 0;
			int nRet = m_pManager->GetDevConfig().GetDevFunctionInfo(lLoginID, ABILITY_DISK_SUBAREA, (char *)&stuAbility, sizeof(DISK_SUBAREA_EN), &nRetLen, waittime);
			if(nRet <= 0)
			{
				return -1;
			}
			
			if(stuAbility.bSupported <= 0)
			{
				return NET_DONT_SUPPORT_SUBAREA;
			}
			if(stuAbility.nSupportNum < pDiskSubarea->bSubareaNum)
			{
				return NET_ILLEGAL_PARAM;
			}

			int	nDiskIndex = QueryDiskIndex(lLoginID, pDiskSubarea->bIndex);
			if (nDiskIndex < 0)
			{
				return NET_QUERY_DISKINFO_FAILED;
			}
			
			char szPart[1024] = {0};
			int nPos = 0;
			for(int i = 0; i < pDiskSubarea->bSubareaNum; i++)
			{
				sprintf(szPart+nPos, "PartSize:%d\r\n", pDiskSubarea->bSubareaSize[i]);
				nPos = strlen(szPart);
			}
			strcat(szPart, "\r\n");
			
			controlchannel.type2 = 0;	//关闭、重启及硬盘操作
			controlchannel.parm1 = 3;	//硬盘操作
			controlchannel.parm2 = (nDiskIndex & 0x0F);
			controlchannel.parm3 = 6;		//操作类型
			controlchannel.pParm4 = szPart;
		}
		break;
	case CTRL_TYPE_SDCARD:
		{
			DISKCTRL_PARAM *diskParam = (DISKCTRL_PARAM *)param;
			if (NULL ==diskParam)
			{
				return NET_ILLEGAL_PARAM;
			}
			controlchannel.type2 = 0; //关闭、重启及硬盘操作
			controlchannel.parm1 = 7; //SD卡操作
			controlchannel.parm2 = diskParam->nIndex;		//目标设备
			controlchannel.parm3 = diskParam->ctrlType;		//操作类型
		}
		break;
	case CTRL_TYPE_TRIGGER_ALARM_IN:
		{
			controlchannel.type2 = 2;  //触发、停止报警输入输出
			ALARMCTRL_PARAM *alcParam = (ALARMCTRL_PARAM *)param;
			if (NULL == alcParam)
			{
				return NET_ILLEGAL_PARAM;
			}
			controlchannel.parm1 = 0;//set alarm in
			controlchannel.parm2 = alcParam->nAction;
			controlchannel.parm3 = alcParam->nAlarmNo;
		}
		break;
	case CTRL_TYPE_TRIGGER_ALARM_OUT:
		{
			controlchannel.type2 = 2;  //触发、停止报警输入输出
			ALARMCTRL_PARAM *alcParam = (ALARMCTRL_PARAM *)param;
			if (NULL == alcParam)
			{
				return NET_ILLEGAL_PARAM;
			}
			controlchannel.parm1 = 1;//set alarm out
			controlchannel.parm2 = alcParam->nAction;
			controlchannel.parm3 = alcParam->nAlarmNo;
		}
		break;
	case CTRL_TYPE_TRIGGER_ALARM_WIRELESS:
		{
			controlchannel.type2 = 12;//无线报警
			ALARMCTRL_PARAM *alcParam = (ALARMCTRL_PARAM *)param;
			if (NULL == alcParam)
			{
				return NET_ILLEGAL_PARAM;
			}
			controlchannel.parm2 = alcParam->nAction;
			controlchannel.parm3 = alcParam->nAlarmNo;
		}
		break;
	case CTRL_TYPE_MATRIX:
		{
			controlchannel.type2 = 3;  //操作矩阵
			MATRIXCTRL_PARAM *mtxcParam = (MATRIXCTRL_PARAM *)param;
			if (NULL == mtxcParam)
			{
				return NET_ILLEGAL_PARAM;
			}

			controlchannel.parm1 = mtxcParam->nChannelNo;
			controlchannel.parm2 = mtxcParam->nMatrixNo;			
		}
		break;
	case CTRL_TYPE_BURNING_START:	//开始刻录
		{
			BURNNG_PARM *burnParm = (BURNNG_PARM*)param;
			controlchannel.type2 = 4;
			controlchannel.parm1 = 1;
			controlchannel.parm2 = burnParm->channelMask;
			controlchannel.parm3 = burnParm->devMask;
		}
		break;
	case CTRL_TYPE_BURNING_STOP:	//停止刻录
		{
			BURNNG_PARM *burnParm = (BURNNG_PARM*)param;
			controlchannel.type2 = 4;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = burnParm->channelMask;
			controlchannel.parm3 = burnParm->devMask;
		}
		break;
	case CTRL_TYPE_BURNING_POSTPONE:	//刻录顺延
		{
			BURNNG_PARM *burnParm = (BURNNG_PARM*)param;
			controlchannel.type2 = 4;
			controlchannel.parm1 = 4;
			controlchannel.parm2 = burnParm->channelMask;
			controlchannel.parm3 = burnParm->devMask;
		}
		break;
	case CTRL_TYPE_BURNING_ATTACH:
		{
			BURNING_PARM_ATTACH *burnParamAttach = (BURNING_PARM_ATTACH *)param;
			if(NULL == burnParamAttach)
			{
				return -1;
			}
			
			BOOL bEnable = burnParamAttach->bAttachBurn;
			char szBuf[1024] = {0};
			int nRetLen = 0;
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_BURN_ATTACH, NULL, szBuf, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				memset(szBuf, 0 , 1024);
				nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_SET_BURN_ATTACH, (void *)&bEnable, szBuf, 1024, &nRetLen, waittime);
				if(nRet >= 0 && nRetLen > 0)
				{
					return 1;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		break;
	case CTRL_TYPE_BURNING_PAUSE:  //暂停刻录
		{
			BURNNG_PARM *burnParm = (BURNNG_PARM*)param;
			controlchannel.type2 = 4;
			controlchannel.parm1 = 2;
			controlchannel.parm2 = burnParm->channelMask;
			controlchannel.parm3 = burnParm->devMask;			
		}
		break;
	case CTRL_TYPE_BURNING_CONTINUE:
		{	
			BURNNG_PARM *burnParm = (BURNNG_PARM*)param;
			controlchannel.type2 = 4;
			controlchannel.parm1 = 3;
			controlchannel.parm2 = burnParm->channelMask;
			controlchannel.parm3 = burnParm->devMask;
		}
		break;
	case CTRL_TYPE_BURNING_ADDPWD:		//叠加密码
		{
			char *szBurningPwd = (char *)param;
			if(szBurningPwd == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}

			controlchannel.type2 = 5;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = szBurningPwd;
		}
		break;
	case CTRL_TYPE_BURNING_HEADADD:		//叠加片头
		{
			char *szBurningHead = (char *)param;
			if(szBurningHead == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			controlchannel.type2 = 6;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = szBurningHead;
		}
		break;
	case CTRL_TYPE_BURNING_ADDSIGN:		//叠加打点
		{	
			controlchannel.type2 = 7;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = NULL;
		}
		break;
	case CTRL_TYPE_BURNING_ADDCURSTOMINFO:		//自定义叠加
		{
			char *szBurningCurstom = (char *)param;
			if(szBurningCurstom == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			controlchannel.type2 = 8;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = szBurningCurstom;
		}
		break;
	case CTRL_TYPE_RESTOREDEFAULT:		//恢复默认设置
		{
			DWORD *pCtrlFlag = (DWORD*)param;
			if(pCtrlFlag == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}

			controlchannel.type2 = 9;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = (char*)pCtrlFlag;
		}
		break;
	case CTRL_TYPE_CAPTURE_START:			//开始抓图
		{
			NET_SNAP_COMMANDINFO *pSnapInfo = (NET_SNAP_COMMANDINFO*)param;
			if (pSnapInfo == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}

			controlchannel.type2 = 10;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			int nLen = sizeof(pSnapInfo->szCardInfo);
			pSnapInfo->szCardInfo[nLen-1] = '\0';
			controlchannel.pParm4 = pSnapInfo->szCardInfo;
		}
		break;
	case CTRL_TYPE_CLEARLOG:				//清除日志
		{
			LOG_QUERY_TYPE *pSubType = (LOG_QUERY_TYPE*)param;
			if (pSubType == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}

			controlchannel.type2 = 11;
			controlchannel.parm1 = (int)*pSubType;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = NULL;
		}
		break;
	case CTRL_TYPE_MARK_IMPORTANT_RECORD:			//设置重要录像文件
		{
			NET_RECORDFILE_INFO *pRecordInfo = (NET_RECORDFILE_INFO*)param;
			if (pRecordInfo == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nRetLen = 0;
			bool bSupport = false;
			
			DEV_ENABLE_INFO stDevEn = {0};
			//查看能力
			nRet = m_pManager->GetDevConfig().GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, 1000);
			if (nRet >= 0 && nRetLen > 0)
			{
				if (stDevEn.IsFucEnable[EN_MARK_IMPORTANTRECORD] != 0)
				{
					bSupport = true;
				}
			}
			
			if (!bSupport)
			{
				//return -1;
			}
			
			IMPORTANT_REC_DIRINFO stuRecordInfo = {0};

			if (4 == pRecordInfo->nRecordFileType)
			{
				stuRecordInfo.DirInfo.usAorV = 0;
			}
			else
			{
				stuRecordInfo.DirInfo.usAorV = 2;
				stuRecordInfo.DirInfo.usAlarm = pRecordInfo->nRecordFileType;
			}

			stuRecordInfo.DirInfo.usCardNo = pRecordInfo->ch;
			stuRecordInfo.DirStartTime.year = pRecordInfo->starttime.dwYear - 2000;
			stuRecordInfo.DirStartTime.month = pRecordInfo->starttime.dwMonth; 
			stuRecordInfo.DirStartTime.day = pRecordInfo->starttime.dwDay; 
			stuRecordInfo.DirStartTime.hour = pRecordInfo->starttime.dwHour; 
			stuRecordInfo.DirStartTime.minute = pRecordInfo->starttime.dwMinute; 
			stuRecordInfo.DirStartTime.second = pRecordInfo->starttime.dwSecond; 

			stuRecordInfo.DirEndTime.year = pRecordInfo->endtime.dwYear - 2000;
			stuRecordInfo.DirEndTime.month = pRecordInfo->endtime.dwMonth; 
			stuRecordInfo.DirEndTime.day = pRecordInfo->endtime.dwDay; 
			stuRecordInfo.DirEndTime.hour = pRecordInfo->endtime.dwHour; 
			stuRecordInfo.DirEndTime.minute = pRecordInfo->endtime.dwMinute; 
			stuRecordInfo.DirEndTime.second = pRecordInfo->endtime.dwSecond; 

			stuRecordInfo.DriveNo = pRecordInfo->driveno;
			stuRecordInfo.StartCluster = pRecordInfo->startcluster;
			stuRecordInfo.FileSize = pRecordInfo->size;

			stuRecordInfo.ImportantRecID = pRecordInfo->bImportantRecID;
			stuRecordInfo.HInt = pRecordInfo->bHint;
		
			controlchannel.type2 = 13;
			controlchannel.parm1 = 0;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = (char *)&stuRecordInfo;
		}
		break;
	case CTRL_TYPE_CTRL_OEMCTRL:
		{
			BOOL *bOpen = (BOOL*)param;
			if (bOpen == NULL)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			controlchannel.type2 = 14;
			controlchannel.parm1 = *bOpen;
			controlchannel.parm2 = 0;
			controlchannel.parm3 = 0;
			controlchannel.pParm4 = NULL;
		}
		break;
	case CTRL_TYPE_BACKUP_START:
		{
			LPBACKUP_RECORD lpRecords = (LPBACKUP_RECORD) param;
			if(NULL == lpRecords)
			{
				return -1;
			}
			if (lpRecords->nRecordNum <= 0)
			{
				return NET_ILLEGAL_PARAM;
			}
			if (lpRecords->szDeviceName[0] == '\0')
			{
				return NET_ILLEGAL_PARAM;
			}
			
			REQUEST_BACKUP_RECORD * lpstuRecord = new REQUEST_BACKUP_RECORD;
			if (lpstuRecord == NULL)
			{
				return -1;
			}
			memset(lpstuRecord, 0, sizeof(REQUEST_BACKUP_RECORD));
			REQUEST_BACKUP_RECORD & stuRecord = *lpstuRecord;
			strncpy(stuRecord.szDeviceName, lpRecords->szDeviceName, MAX_BACKUP_NAME);
			stuRecord.nRecordNum = lpRecords->nRecordNum;
			for (int i = 0; i < stuRecord.nRecordNum; i++)
			{
				stuRecord.stuRecordInfo[i].byType = lpRecords->stuRecordInfo[i].nRecordFileType;
				
				stuRecord.stuRecordInfo[i].DirStartTime.year = lpRecords->stuRecordInfo[i].starttime.dwYear - 2000;
				stuRecord.stuRecordInfo[i].DirStartTime.month = lpRecords->stuRecordInfo[i].starttime.dwMonth;
				stuRecord.stuRecordInfo[i].DirStartTime.day = lpRecords->stuRecordInfo[i].starttime.dwDay;
				stuRecord.stuRecordInfo[i].DirStartTime.hour = lpRecords->stuRecordInfo[i].starttime.dwHour;
				stuRecord.stuRecordInfo[i].DirStartTime.minute = lpRecords->stuRecordInfo[i].starttime.dwMinute;
				stuRecord.stuRecordInfo[i].DirStartTime.second = lpRecords->stuRecordInfo[i].starttime.dwSecond;
				
				stuRecord.stuRecordInfo[i].DirEndTime.year = lpRecords->stuRecordInfo[i].endtime.dwYear - 2000;
				stuRecord.stuRecordInfo[i].DirEndTime.month = lpRecords->stuRecordInfo[i].endtime.dwMonth;
				stuRecord.stuRecordInfo[i].DirEndTime.day = lpRecords->stuRecordInfo[i].endtime.dwDay;
				stuRecord.stuRecordInfo[i].DirEndTime.hour = lpRecords->stuRecordInfo[i].endtime.dwHour;
				stuRecord.stuRecordInfo[i].DirEndTime.minute = lpRecords->stuRecordInfo[i].endtime.dwMinute;
				stuRecord.stuRecordInfo[i].DirEndTime.second = lpRecords->stuRecordInfo[i].endtime.dwSecond;

				stuRecord.stuRecordInfo[i].nChannel = lpRecords->stuRecordInfo[i].ch;
				stuRecord.stuRecordInfo[i].nDriveNo = lpRecords->stuRecordInfo[i].driveno;
				stuRecord.stuRecordInfo[i].nFileSize = lpRecords->stuRecordInfo[i].size;
				stuRecord.stuRecordInfo[i].nHInt = lpRecords->stuRecordInfo[i].bHint;
				stuRecord.stuRecordInfo[i].nImportantRecID = lpRecords->stuRecordInfo[i].bImportantRecID;
				stuRecord.stuRecordInfo[i].nStartCluster = lpRecords->stuRecordInfo[i].startcluster;
			}

			int nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_BACKUP_START, &stuRecord, waittime);
			
			if (lpstuRecord != NULL)
			{
				delete lpstuRecord;
				lpstuRecord = NULL;
			}
			if(nRet >= 0 )
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
		break;
	case CTRL_TYPE_BACKUP_STOP: 
		{
			if(NULL == param || '\0' == *(char *) param)
			{
				return NET_ILLEGAL_PARAM;
			}

			int nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_BACKUP_STOP, param, waittime);
			if(nRet >= 0 )
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
		break;
	default:
		if (type >= CTRL_TYPE_KEYBOARD_POWER && type <= CTRL_TYPE_KEYBOARD_JMPUP)
		{
			controlchannel.type2 = 1;
			//如果CtrlType枚举变动，则此处也应随之变动
			controlchannel.parm1 = type - 3;
		}
		else
		{
			return NET_ILLEGAL_PARAM;
		}
		break;
	}
	controlchannel.type1 = AFK_CONTROL_TYPE_DEVCONTROL;
	controlchannel.base.func = DevControlFunc;
	controlchannel.base.udata = &receivedata;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONTROL, &controlchannel);
    if (pchannel)
    {
		if ((type >= CTRL_TYPE_KEYBOARD_POWER && type <= CTRL_TYPE_KEYBOARD_JMPUP)
			|| CTRL_TYPE_BURNING_START == type || CTRL_TYPE_BURNING_STOP == type 
			|| CTRL_TYPE_BURNING_ADDPWD == type || CTRL_TYPE_BURNING_HEADADD == type
			|| CTRL_TYPE_BURNING_ADDSIGN == type || CTRL_TYPE_BURNING_ADDCURSTOMINFO == type
			|| CTRL_TYPE_CLEARLOG == type || CTRL_TYPE_MARK_IMPORTANT_RECORD == type
			|| CTRL_TYPE_BURNING_PAUSE == type || CTRL_TYPE_BURNING_CONTINUE == type
			|| CTRL_TYPE_BURNING_POSTPONE == type || CTRL_TYPE_CTRL_OEMCTRL == type) //网络前面板命令、刻录控制,设置重要录像暂无返回值
		{
			pchannel->close(pchannel);
			nRet = 0;
		}
		else
		{
			DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			pchannel->close(pchannel);
			ResetEventEx(receivedata.hRecEvt);
			if (WAIT_OBJECT_0 == dwRet)
			{
				switch(receivedata.result)
				{
				case -1:
					nRet = NET_RETURN_DATA_ERROR; //系统出错
					break;
				case 0:
					nRet = NET_NOERROR;
					break;
				case 1:
					nRet = NET_NOT_AUTHORIZED;	//没有权限
					break;
				case 2:
					nRet = NET_NOT_NOW;			//暂时无法执行
					break;
				default:
					nRet = NET_RETURN_DATA_ERROR; //系统出错
					break;
				}
			}
			else
			{
				nRet = NET_NETWORK_ERROR; //超时
			}
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}
	
    return nRet; 
}

int	CDevControl::QueryDiskIndex(LONG lLoginID, int nIndex)
{
	int	nDiskIndex = -1;
	afk_device_s *device = (afk_device_s*)lLoginID;

	// 取硬盘信息，将目标硬盘序号转换为实际的硬盘标识
	int nRetLen = 0;
	int nBufSize = sizeof(IDE_INFO64) + 256*sizeof(DRIVER_INFO);
	char *pDiskBuf = new char[nBufSize];
	if (NULL == pDiskBuf)
	{
		return nDiskIndex;
	}
	memset(pDiskBuf, 0, nBufSize);
	
	int ret = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_DISK_DRIVER, pDiskBuf, nBufSize, &nRetLen, 2000);

	int nDiskNum = ((IDE_INFO *)pDiskBuf)->ide_num;

	if (ret >= 0 && nBufSize >= sizeof(IDE_INFO) && (nDiskNum == (nBufSize-sizeof(IDE_INFO))/sizeof(DRIVER_INFO)))
	{
		IDE_INFO *pIDE = (IDE_INFO *)pDiskBuf;

		if (nIndex >= 0 && nIndex < (nRetLen-sizeof(IDE_INFO))/sizeof(DRIVER_INFO))
		{
			DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO) + nIndex*sizeof(DRIVER_INFO));

			nDiskIndex = pDisk_Info->index;
		}
	}
	else if (ret >= 0 && nRetLen >= sizeof(IDE_INFO64) && (nDiskNum == (nRetLen-sizeof(IDE_INFO64))/sizeof(DRIVER_INFO)))
	{
		IDE_INFO64 *pIDE = (IDE_INFO64 *)pDiskBuf;

		if (nIndex >= 0 && nIndex < (nRetLen-sizeof(IDE_INFO64))/sizeof(DRIVER_INFO))
		{
			DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO64) + nIndex*sizeof(DRIVER_INFO));

			nDiskIndex = pDisk_Info->index;
		}
	}

	if (pDiskBuf != NULL)
	{
		delete[] pDiskBuf;
	}

	return nDiskIndex;
}





