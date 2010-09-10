#ifndef _ZKit_Config_h_
#define _ZKit_Config_h_
//此文件定义ZKit库需要依赖的外部头文件
//ZKit库中每个头文件都要直接或间接包含此头文件

#ifdef  NO_NAME_SPACE
#define BEGIN_ZKIT
#define END_ZKIT
#define USING_ZKIT
#else
#define BEGIN_ZKIT namespace ZKit{
#define END_ZKIT   }
#define USING_ZKIT using namespace ZKit;
#endif

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "platform.h"

//e.g. char dst[10];
#define strncpy_safe(dst, src)   do {strncpy(dst, src, sizeof(dst) - 1); dst[sizeof(dst) - 1] = 0;} while (0)

//e.g. char* p = new char[10];
#define strncpy_safe_x(p, src, psize)   do {strncpy(dst, src, psize - 1); p[psize - 1] = 0;} while (0)

#define delete_safe(p)   do {delete p;p = 0;} while (0)

#define delete_safe_x(p)   do {delete [] p;p = 0;} while (0)

#define array_size(a) (sizeof(a)/sizeof(a[0]))

#ifdef  USE_OLD_CASTS
#define STATIC_CAST(T, V)      (T)(V)
#define DYNAMIC_CAST(T, V)     (T)(V)
#define CONST_CAST(T, V)       (T)(V)
#define REINTERPRET_CAST(T, V) (T)(V)
#else
#define STATIC_CAST(T, V)      static_cast<T>(V)
#define DYNAMIC_CAST(T, V)     dynamic_cast<T>(V)
#define CONST_CAST(T, V)       const_cast<T>(V)
#define REINTERPRET_CAST(T, V) reinterpret_cast<T>(V)
#endif

//stl算法库中由于历史原因没有实现copy_if, 这里自己实现一个
template<typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin, InputIterator end, OutputIterator destBegin, Predicate p) 
{
	while (begin != end) 
	{
		if (p(*begin))
			*destBegin++ = *begin;
		++begin;
	}
	return destBegin;
}

#endif // _ZKit_Config_h_
