///////////////////////////////////////////////////////////////////////////////////////////
// 文件名: dvrcap.cpp
// 创建者: 刘杰
// 创建时间: 
// 内容描述: dvr能力
///////////////////////////////////////////////////////////////////////////////////////////
#include "dvrcap.h"
#include "def.h"

//得到报警输入端口个数
int alarminputCount(int type, unsigned char *buf)
{
	switch (type)
    {
	case PRODUCT_DVR_NONREALTIME_MACE:		//非实时MACE
		return 8;
	case PRODUCT_DVR_NONREALTIME:			//非实时
		return 4;
		break;
	case PRODUCT_NVS_MPEG1:					//网络视频服务器
		break;
	case PRODUCT_DVR_MPEG1_2:				//MPEG1二路录像机
		return 4;
	case PRODUCT_DVR_MPEG1_8:				//MPEG1八路录像机
		return 8;
	case PRODUCT_DVR_MPEG4_8:				//MPEG4八路录像机
		return 8;
	case PRODUCT_DVR_MPEG4_16:				//MPEG4十六路录像机
		return 16;
	case PRODUCT_DVR_MPEG4_SX2:			    //MPEG4视新十六路录像机
		return 8;
	case PRODUCT_DVR_MEPG4_ST2:				//MPEG4视通录像机
		return 8;
	case PRODUCT_DVR_MEPG4_SH2:				//MPEG4视豪录像机
		return 8;
    case PRODUCT_DVR_MPEG4_GBE:              //MPEG4视通二代增强型录像机
        return 16;
    case PRODUCT_DVR_MPEG4_NVSII:            //PRODUCT_DVR_MPEG4_NVSII
        return 4;
	case PRODUCT_DVR_STD_NEW:				//新标准配置协议
		break;
	default:
		return 8;
	}

	return 8;
}

//得到报警输出端口个数
int alarmoutputCount(int type, unsigned char *buf)
{
	switch (type)
    {
	case PRODUCT_DVR_NONREALTIME_MACE:		//非实时MACE
		break;
	case PRODUCT_DVR_NONREALTIME:			//非实时
		return 3;
	case PRODUCT_NVS_MPEG1:					//网络视频服务器
		break;
	case PRODUCT_DVR_MPEG1_2:				//MPEG1二路录像机
		return 3;
	case PRODUCT_DVR_MPEG1_8:				//MPEG1八路录像机
		break;
	case PRODUCT_DVR_MPEG4_8:				//MPEG4八路录像机
		break;
	case PRODUCT_DVR_MPEG4_16:				//MPEG4十六路录像机
		break;
	case PRODUCT_DVR_MPEG4_SX2:			    //MPEG4视新十六路录像机
		return 3;
	case PRODUCT_DVR_MEPG4_ST2:				//MPEG4视通录像机
		return 3;
	case PRODUCT_DVR_MEPG4_SH2:				//MPEG4视豪录像机
		return 3;
    case PRODUCT_DVR_MPEG4_GBE:              //MPEG4视通二代增强型录像机
        return 3;//20?
    case PRODUCT_DVR_MPEG4_NVSII:            //PRODUCT_DVR_MPEG4_NVSII
        return 2;
	case PRODUCT_DVR_STD_NEW:				//新标准配置协议
		break;
	default:
		return 3;
	}

	return 3;
}

//得到透明串口能力
bool transability(int type)
{
    switch (type)
    {
	case PRODUCT_DVR_NONREALTIME_MACE:		//非实时MACE
		break;
	case PRODUCT_DVR_NONREALTIME:			//非实时
		break;
	case PRODUCT_NVS_MPEG1:					//网络视频服务器
		break;
	case PRODUCT_DVR_MPEG1_2:				//MPEG1二路录像机
		break;
	case PRODUCT_DVR_MPEG1_8:				//MPEG1八路录像机
		break;
	case PRODUCT_DVR_MPEG4_8:				//MPEG4八路录像机
		break;
	case PRODUCT_DVR_MPEG4_16:				//MPEG4十六路录像机
		break;
	case PRODUCT_DVR_MPEG4_SX2:			    //MPEG4视新十六路录像机
		return true;
	case PRODUCT_DVR_MEPG4_ST2:				//MPEG4视通录像机
		return true;
	case PRODUCT_DVR_MEPG4_SH2:				//MPEG4视豪录像机
		return true;
    case PRODUCT_DVR_MPEG4_GBE:              //MPEG4视通二代增强型录像机
        return true;
    case PRODUCT_DVR_MPEG4_NVSII:            //PRODUCT_DVR_MPEG4_NVSII
        return true;
	case PRODUCT_DVR_STD_NEW:				//新标准配置协议
		break;
	default:
		return false;
	}

	return false;
}

//得到对讲能力
bool talkability(int type)
{
	return true;
	
    switch (type)
    {
	case PRODUCT_DVR_NONREALTIME_MACE:		//非实时MACE
		break;
	case PRODUCT_DVR_NONREALTIME:			//非实时
		break;
	case PRODUCT_NVS_MPEG1:					//网络视频服务器
		break;
	case PRODUCT_DVR_MPEG1_2:				//MPEG1二路录像机
		break;
	case PRODUCT_DVR_MPEG1_8:				//MPEG1八路录像机
		break;
	case PRODUCT_DVR_MPEG4_8:				//MPEG4八路录像机
		break;
	case PRODUCT_DVR_MPEG4_16:				//MPEG4十六路录像机
		break;
	case PRODUCT_DVR_MPEG4_SX2:			    //MPEG4视新十六路录像机
		return true;
	case PRODUCT_DVR_MEPG4_ST2:				//MPEG4视通录像机
		return true;
	case PRODUCT_DVR_MEPG4_SH2:				//MPEG4视豪录像机
		return true;
    case PRODUCT_DVR_MPEG4_GBE:              //MPEG4视通二代增强型录像机
        return true;
    case PRODUCT_DVR_MPEG4_NVSII:            //PRODUCT_DVR_MPEG4_NVSII
        return true;
	case PRODUCT_DVR_STD_NEW:				//新标准配置协议
		return true;
	default:
		return false;
	}

	return false;
}

//得到多画面预览能力
/*bool previewability(int type)
{
    switch (type)
    {
	case PRODUCT_DVR_NONREALTIME_MACE:		//非实时MACE
		break;
	case PRODUCT_DVR_NONREALTIME:			//非实时
		break;
	case PRODUCT_NVS_MPEG1:					//网络视频服务器
		break;
	case PRODUCT_DVR_MPEG1_2:				//MPEG1二路录像机
		break;
	case PRODUCT_DVR_MPEG1_8:				//MPEG1八路录像机
		break;
	case PRODUCT_DVR_MPEG4_8:				//MPEG4八路录像机
		break;
	case PRODUCT_DVR_MPEG4_16:				//MPEG4十六路录像机
		break;
	case PRODUCT_DVR_MPEG4_SX2:			    //MPEG4视新十六路录像机
		return true;
	case PRODUCT_DVR_MEPG4_ST2:				//MPEG4视通录像机
		return true;
	case PRODUCT_DVR_MEPG4_SH2:				//MPEG4视豪录像机
		return false;
    case PRODUCT_DVR_MPEG4_GBE:              //MPEG4视通二代增强型录像机
        return true;
    case PRODUCT_DVR_MPEG4_NVSII:            //PRODUCT_DVR_MPEG4_NVSII
        return true;
	case PRODUCT_DVR_STD_NEW:				//新标准配置协议
		return true;
	default:
		return false;
	}

	return false;
}*/

