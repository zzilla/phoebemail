
#ifndef DVRPACKETDDNS_H
#define DVRPACKETDDNS_H

#include "../kernel/afkinc.h"

class CDvrDevice;
class CDvrSearchChannel;

//通过名字查询IP
bool sendQueryIPByName_DDNS(CDvrDevice* device, char *name, int namelen);

#endif

