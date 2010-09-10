#ifndef _G723_H_
#define _G723_H_

typedef enum 
{
	G723_63, 
	G723_53
} eG723Mode;


int g723enc_init(void** handle,eG723Mode mode);	//编码初始化 
int g723dec_init(void** handle);	//解码初始化
int g723enc_mode(void* handle, eG723Mode mode);	//设置编码模式
int g723enc(void* handle, short* sample, char* bits, int src_len, int* out_len);	//编码
int g723dec(void* handle, char* bits,  short* out, int bits_len, int* out_len);		//解码
int g723enc_close(void** handle);	//关闭编码
int g723dec_close(void** handle);	//关闭解码

#endif

