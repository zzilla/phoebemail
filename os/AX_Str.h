#ifndef DH_DSS_COMPONENT_AX_API_AX_STR_H_INCLUDED
#define DH_DSS_COMPONENT_AX_API_AX_STR_H_INCLUDED

#include "platform.h"

namespace AX_OS
{


	typedef int (*Fun_snprintf)(char *buffer,size_t count,const char *format ,... );
	extern Fun_snprintf snprintf;
	
	typedef int (*Fun_stricmp)(const char* str1, const char* str2);
	extern Fun_stricmp stricmp;
	
	//dest:目标buffer,如果目标buffer没有'\0'结束，会设最后一个字符为'\0',返回0
	//src:
	//srcBufLen:目标buffer的总长度,copy的数目不会超过这个srcBufLen-1
	//执行完成后，如果有copy，那么，Dest一定可以以0结束
	//返回的结果是从src copy到dest中的字符数目
	size_t strncat(char* dest, const char* src, const size_t srcBufLen);

	//安全的字符串copy函数
	//根据传入的siz大小，从src copy最多siz-1个字符到dst,并在copy完成的字符串后面加'\0'
	//对于传入的dst=NULL或者src=NULL都加了检查
	//传入值siz为dst的buffer的大小
	//返回结果，实际copy到dst中的字符的数目，不包括'\0'字符
	size_t strncpy(char* dst, const char* src, size_t siz);

	//安全的字符串copy函数
	//dst,目标buffer
	//dstBufLen,目标buffer空间大小,该函数最多写入dstBufLen-1个字符，并且在写入字符后面添加'\0'字符
	//src,源buffer
	//要copy的字符数码,在dstBufLen-1空间允许的情况下，最多copy的字符数目为srcCopyCount,并且在后面添加'\0'字符
	size_t strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount);

}

#endif//DH_DSS_COMPONENT_AX_API_AX_STR_H_INCLUDED
