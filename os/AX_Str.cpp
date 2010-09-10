#include "AX_Str.h"
#include <stdio.h>
#include <string.h>

namespace AX_OS
{

#if defined(WIN32) || defined(WINCE)
	Fun_snprintf snprintf=::_snprintf;
	Fun_stricmp stricmp=::_stricmp;
#else
	Fun_snprintf snprintf=::snprintf;
	Fun_stricmp stricmp=::strcasecmp;
#endif

	size_t strncat(char* dest, const char* src,const size_t srcBufLen)
	{
		if(0==dest || 0==src || 0>=srcBufLen)
			return 0;
		size_t pos=0;
		char* to=dest;
		char* end=dest+srcBufLen-1;
		while(end>to && 0!=*to)
			++to;

		if(end<=to)//dest buffer已满
		{
			*end=0;//设置dest的最后一个字符为0
			return 0;
		}
		const char* from=src;
		while(end>to && 0!=*from)
		{
			*to=*from;
			++to;
			++from;
		}
		*to=0;
		return from-src;
	}

	size_t strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount)
	{
		size_t siz=dstBufLen<=srcCopyCount?dstBufLen:srcCopyCount+1;
		if(0!=dst && 0!=src &&1<siz)
		{
			char *d = dst;
			const char *s = src;
			size_t n = siz;

			while (0!=--n)
			{
				if ('\0'==(*d++ = *s++))
					return d-dst-1;
			}
			*d = '\0';
			return d-dst;
		}
		if(0!=dst && 1==siz)
			*dst=0;
		if(0==src && 0!=dst && 0<siz)
			*dst=0;
		return 0;
	}
}