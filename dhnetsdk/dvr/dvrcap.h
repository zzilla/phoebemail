///////////////////////////////////////////////////////////////////////////////////////////
// 文件名: dvrcap.h
// 创建者: 刘杰
// 创建时间: 
// 内容描述: dvr能力
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef DVRCAP_H
#define DVRCAP_H

//得到报警输入端口个数
int alarminputCount(int type, unsigned char *buf);

//得到报警输出端口个数
int alarmoutputCount(int type, unsigned char *buf);

//得到透明串口能力
bool transability(int type);

//得到对讲能力
bool talkability(int type);

//得到多画面预览能力
//bool previewability(int type);

#endif


