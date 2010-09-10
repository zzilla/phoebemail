#ifndef DSS_AX_STRNCPY_H_INCLUDED
#define DSS_AX_STRNCPY_H_INCLUDED

#include "platform.h"
#include <string.h>

namespace AX_OS
{
	//安全的字符串copy函数
	//根据传入的siz大小，从src copy最多siz-1个字符到dst,并在copy完成的字符串后面加'\0'
	//对于传入的dst=NULL或者src=NULL都加了检查
	//传入值siz为dst的buffer的大小
	//返回结果，实际copy到dst中的字符的数目，不包括'\0'字符
	size_t strncpy(char* dst, const char* src, size_t siz);
}

#endif//