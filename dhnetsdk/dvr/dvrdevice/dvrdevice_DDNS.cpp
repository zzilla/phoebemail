
#include "../def.h"
#include "../Net/TcpSocket.h"
#include "dvrpacket_comm.h"
#include "dvrdevice_DDNS.h"
#include "dvrsearchchannel_DDNS.h"


CDvrDevice_DDNS::CDvrDevice_DDNS(afk_dvrdevice_info stuDevInfo, CTcpSocket *pTcpSocket,
                               const char *szIp, int nPort, const char *szUser, const char *szPassword,
                               fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata)
: CDvrDevice(stuDevInfo, pTcpSocket, szIp, nPort, szUser, szPassword, cbDisconnect, cbDevEvent, userdata)
{
    
}

CDvrDevice_DDNS::~CDvrDevice_DDNS()
{
}

/*
 * 摘要：打开通道
 */
void* CDvrDevice_DDNS::device_open_channel(int type, void *parm)
{
    if (device_channel_is_opened(type, parm))
    {
        return NULL;
    }

    CDvrSearchChannel_DDNS *DDNS_channel = NULL;
    afk_search_channel_param_s *sc_parm = NULL; 

    switch (type)
    {
    //查询通道
    case AFK_CHANNEL_TYPE_SEARCH:
		{
			void *p = 0;
			sc_parm = (afk_search_channel_param_s*)parm;
        
			switch (sc_parm->type)
			{
			//按名字查询
			case AFK_CHANNEL_SEARCH_DDNSIP_BYNAME:
				if (sendQueryIPByName_comm(this, (char *)sc_parm->webName, sc_parm->webNamelen))
				{
					DDNS_channel = new CDvrSearchChannel_DDNS((CDvrDevice*)this, AFK_CHANNEL_TYPE_SEARCH, parm);
					if (DDNS_channel)
					{
						CReadWriteMutexLock lock(m_cssearch);

						m_search_channel_list.push_back(DDNS_channel);
					}
					else
					{
						//have error
					}

					p = DDNS_channel;
				}
				break;
			default:
				break;
			}

			return p;
		}
		break;
    default:
        break;
    }
    return NULL;
}


