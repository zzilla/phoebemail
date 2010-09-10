
#include "../StdAfx.h"
#include "dvrchannel.h"
#include "../dvrinterface.h"


CDvrChannel::CDvrChannel(CDvrDevice *pDvrDevice, int nMainCommand)
{
    get_device	= interface_get_device;
    close		= interface_close_ch;
    pause		= interface_pause_ch;
	get_info	= interface_get_info_ch;
	set_info	= interface_set_info_ch;
    
    m_nMainCommand = nMainCommand;
	m_nChannelType = 0;
    m_pDvrDevice = pDvrDevice;
	
    m_bStatiscEn = FALSE;
    m_nStatistic = 0;

	InterlockedSetEx(&m_llifecount, 1);
}

CDvrChannel::~CDvrChannel()
{
    get_device	= NULL;
    close		= NULL;
    pause		= NULL;
	get_info	= NULL;
	set_info	= NULL;
}

void CDvrChannel::startstatisc()
{
    m_bStatiscEn = TRUE;
    m_nStatistic = 0;
}

void  CDvrChannel::statiscing(unsigned int nLen)
{
    if (m_bStatiscEn)
    {
        m_nStatistic += nLen;
    }
}

unsigned int CDvrChannel::statisc()
{
    unsigned int nStatistic = m_nStatistic; 
	
    m_bStatiscEn = FALSE;
    m_nStatistic = 0;

    return nStatistic;
}

int CDvrChannel::OnRespond(unsigned char *pBuf, int nLen)
{
    if (nLen < 0)
    {
        return -1;
    }

    statiscing(nLen);

	return 1;
}

int CDvrChannel::channel_addRef()
{
	int nLifeCount = InterlockedIncrementEx(&m_llifecount);
	if (nLifeCount <= 1)
	{
		InterlockedDecrementEx(&m_llifecount);
		return 0;
	}
	
	return nLifeCount;
}

int CDvrChannel::channel_decRef()
{
	int nLifeCount = InterlockedDecrementEx(&m_llifecount);
	if (nLifeCount <= 0)
	{
		delete this;
		return 0;
	}
	
	return nLifeCount;
}

