#ifndef _LUMA_STRETCH_H_
#define _LUMA_STRETCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
	function name:	LumaStretch
    purpose:		增加图象的通透性
    input param:	unsigned char* pSrc:源YUV图象地址					
					unsigned char* pDst：目标YUV图象地址,如果pDst为NULL，则数据直接写到pSrc上					
					int nWidth：图象宽
					int nHeight：图象高
    output param:	NULL
    return:			-1：failed	0:success
    created:		13:3:2008   16:05
	author:			winton	
	
*********************************************************************/
int LumaStretch(unsigned char* pSrc,unsigned char* pDst,int nWidth,int nHeight);

#ifdef __cplusplus
}
#endif
#endif