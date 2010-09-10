
#ifndef _DVRSEARCHCHANNELDDNS_H_
#define _DVRSEARCHCHANNELDDNS_H_

#include "dvrsearchchannel.h"
class CDvrDevice;


class CDvrSearchChannel_DDNS : public CDvrSearchChannel
{
public:
	CDvrSearchChannel_DDNS(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrSearchChannel_DDNS();
	
public:
	/* ¥¶¿Ì√¸¡Ó */
	virtual int		OnRespond(unsigned char *pBuf, int nLen);
};


#endif // _DVRSEARCHCHANNELDDNS_H_



