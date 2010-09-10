#include "AX_strncpy.h"

namespace AX_OS
{
	size_t strncpy(char* dst, const char* src, size_t siz)
	{
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

//下面代码为AX_OS::strncpy的测试函数，如果有修改，请运行下面函数来检查
/*

void run_verify_strncpy(char* dst, const char * src, const size_t siz,
const size_t expRes,const size_t expDstLen,const bool mustCheckDstLen, const bool hasNULL,const char* expDstStr)
{
size_t res=AX_OS::strncpy(dst,src,siz);
assert(expRes==res);
if(0<expDstLen)
{
size_t len=strlen(dst);
assert(expDstLen==len);
}
if(NULL!=dst && 0<siz)
{
size_t len=strlen(dst);
assert(expDstLen==len);
}
if(mustCheckDstLen)
{
size_t len=strlen(dst);
assert(expDstLen==len);
}
if(hasNULL)
{
size_t len=strlen(dst);
assert(siz>len);
}
if(NULL!=expDstStr)
{
assert(0==strcmp(dst,expDstStr));
}
}


void check_strncpy(char* buf)
{
run_verify_strncpy(NULL		,"aaa"			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,1		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,2		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,3		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,4		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,5		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,6		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,"aaa"			,7		,0		,0		,false		,false		,NULL);

run_verify_strncpy(NULL		,NULL			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,1		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,2		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,3		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,4		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,5		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,6		,0		,0		,false		,false		,NULL);
run_verify_strncpy(NULL		,NULL			,7		,0		,0		,false		,false		,NULL);


run_verify_strncpy(buf		,NULL			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,NULL			,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,2		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,3		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,4		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,5		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,6		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,NULL			,7		,0		,0		,true		,true		,"");


run_verify_strncpy(buf		,""				,7		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,7		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,7		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123"			,7		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"1234"			,7		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"12345"		,7		,5		,5		,true		,true		,"12345");
run_verify_strncpy(buf		,"123456"		,7		,6		,6		,true		,true		,"123456");
run_verify_strncpy(buf		,"1234567"		,7		,6		,6		,true		,true		,"123456");
run_verify_strncpy(buf		,"1234567890"	,7		,6		,6		,true		,true		,"123456");


run_verify_strncpy(buf		,""				,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"1"			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"12"			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"123"			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"1234"			,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"12345"		,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"123456"		,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"1234567"		,0		,0		,0		,false		,false		,NULL);
run_verify_strncpy(buf		,"12345607890"	,0		,0		,0		,false		,false		,NULL);


run_verify_strncpy(buf		,""				,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"12"			,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"123"			,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1234"			,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"12345"		,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"123456"		,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1234567"		,1		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"12345607890"	,1		,0		,0		,true		,true		,"");


run_verify_strncpy(buf		,""				,2		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"123"			,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"1234"			,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12345"		,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"123456"		,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"1234567"		,2		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12345607890"	,2		,1		,1		,true		,true		,"1");



run_verify_strncpy(buf		,""				,3		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,3		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123"			,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"1234"			,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"12345"		,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123456"		,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"1234567"		,3		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"12345607890"	,3		,2		,2		,true		,true		,"12");


run_verify_strncpy(buf		,""				,4		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,4		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,4		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123"			,4		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"1234"			,4		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"12345"		,4		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"123456"		,4		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"1234567"		,4		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"12345607890"	,4		,3		,3		,true		,true		,"123");


run_verify_strncpy(buf		,""				,5		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,5		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,5		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123"			,5		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"1234"			,5		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"12345"		,5		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"123456"		,5		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"1234567"		,5		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"12345607890"	,5		,4		,4		,true		,true		,"1234");



run_verify_strncpy(buf		,""				,6		,0		,0		,true		,true		,"");
run_verify_strncpy(buf		,"1"			,6		,1		,1		,true		,true		,"1");
run_verify_strncpy(buf		,"12"			,6		,2		,2		,true		,true		,"12");
run_verify_strncpy(buf		,"123"			,6		,3		,3		,true		,true		,"123");
run_verify_strncpy(buf		,"1234"			,6		,4		,4		,true		,true		,"1234");
run_verify_strncpy(buf		,"12345"		,6		,5		,5		,true		,true		,"12345");
run_verify_strncpy(buf		,"123456"		,6		,5		,5		,true		,true		,"12345");
run_verify_strncpy(buf		,"1234567"		,6		,5		,5		,true		,true		,"12345");
run_verify_strncpy(buf		,"12345607890"	,6		,5		,5		,true		,true		,"12345");
}

void batch_check_strncpy()
{
char buf[7];
memset(buf,'1',7);
check_strncpy(buf);


memset(buf,'1',7);
memset(buf,'0',4);
check_strncpy(buf);



memset(buf,'1',7);
*buf=0;
check_strncpy(buf);

memset(buf,'1',7);
*(buf+6)=0;
check_strncpy(buf);

}

int main(int argc,char* argv[] )
{	
batch_check_strncpy();
return 0;
}



*/