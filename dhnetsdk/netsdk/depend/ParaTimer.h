
#ifndef PARATIMER_H
#define PARATIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

//定时器的回调函数类型
typedef void (* TimerProc_Para)(long nId,void* lpPara);

//设定一个定时器
//标识：			nId
//间隔：			uElapse
//定时器回调函数：	lpfn
//定时器参数：		lpPara
//返回值：－1 添加定时器出错
long SetTimer_Para(long nId,unsigned long uElapse,TimerProc_Para lpfn,void* lpPara);

//删除一个定时器
//标识：	nId
void KillTimer_Para(long nId);


#ifdef __cplusplus
}
#endif

#endif

