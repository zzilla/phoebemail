#include "../Net/TcpSocket.h"
#include "dvrdevice.h"
#include "dvrpacket_DDNS.h"
#include "dvrpacket_DDNS.h"
#include "../def.h"

/****************************************************************************/


/****************************************************************************/

bool sendQueryIPByName_DDNS(CDvrDevice* device, char *name, int namelen)
{
	unsigned char data[HEADER_SIZE + 64];
	memset(data, 0, HEADER_SIZE + 64);
	
	data[0] = 0x81;
	DWORD datalen = (namelen <= 64) ? namelen : 64;
	(*(DWORD*)(data+4)) = datalen;//datalen;
	data[8] = 50;
	data[9] = 0; //0.单记录查询  1.查询所有在线记录
    memcpy(data+HEADER_SIZE, name, datalen);
	
    CTcpSocket *tcp = device->GetTcpSocket();
    if (tcp)
    {
        if (tcp->WriteData((char*)data, HEADER_SIZE + 64) >= 0)
        {
            return true;
        }
    }
    return false;
}

