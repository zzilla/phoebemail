#ifndef __JPEGENCODER_H__
#define __JPEGENCODER_H__

#include "JpegInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

int JpegEncode(unsigned char * JPEGBuffer, unsigned char * YUVBuffer, int *outsize, long width, long height, int YUVtype, int quality);

#ifdef __cplusplus
}
#endif


#endif