#ifndef _STDMP4PARSE_H
#define _STDMP4PARSE_H

//extern int ParseStdMp4(unsigned char* pBuf, int len, int* width, int* height);
extern int Mpeg4_Analyse(unsigned char* pBuf,int nSize,int* nWidth,int* nHeight,int*framerate);
extern int H264_Analyse(unsigned char* pBuf,int nSize,int* nWidth,int* nHeight,int* framerate);
#endif
