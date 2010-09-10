/*********************************************************
                      DllDeinterlace.h
*********************************************************/
#ifndef _DLLDEINTERLACE_H_
#define _DLLDEINTERLACE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
    
#if defined(WIN32)
#if !defined(USELIB)
#define HI_API __declspec( dllexport ) 
#define HI_APICALL __stdcall
#else
#define HI_API
#define HI_APICALL
#endif
#elif defined(LINUX)
#define HI_API
#define HI_APICALL
#else
#error "Please specify a design-in platform!"
#endif


//错误码
#define HI_SUCCESS_DEINTERLACE  0   //成功
#define HI_ERR_MALLOC           -1  //malloc时失败
#define HI_ERR_PITCH            -2  //输入、输出图像的跨度设置不对
#define HI_ERR_POINT_NULL       -3  //输入指针为空
#define HI_ERR_FIELD_FLAG       -4  //输入的不是场标志

typedef enum hiPIC_TYPE_E
{
    PIC_PROGRESSIVE = 0,    // 帧
    PIC_INTERLACED_ODD,     // 奇场
    PIC_INTERLACED_EVEN     // 偶场
} PIC_TYPE_E;

typedef struct hiDEINTERLACE_PARA_S
{
    int iFieldWidth;    //场宽
    int iFieldHeight;   //场高
    int iSrcYPitch;     //输入场图像Y分量的跨度
    int iSrcUVPitch;    //输入场图像UV分量的跨度
    int iDstYPitch;     //输出帧图像Y分量的跨度
    int iDstUVPitch;    //输出帧图像UV分量的跨度
}DEINTERLACE_PARA_S;

typedef struct hiDEINTERLACE_FRAME_S
{
    unsigned char *pszY;  //YUV420图像Y分量
    unsigned char *pszU;  //YUV420图像U分量
    unsigned char *pszV;  //YUV420图像V分量
}DEINTERLACE_FRAME_S;

/***************************************************************************
           初始化deinterlace函数
  功能：调用HI_Deinterlace函数前需调用此函数
        以完成deinterlace的初始化工作
  参数：
      **pHandle    : 输出参数，Deinterlace句柄,不需要做任何操作,只需
                               按照例子输入参数即可.
      struPara     : 输入参数，配置输入YUV420场图像高宽跨度参数，
                               配置输出YUV420帧图像的跨度参数.
  返回值:
      HI_SUCCESS_DEINTERLACE:  成功
      HI_ERR_MALLOC         :  开辟空间失败
      HI_ERR_PITCH          :  输入输出的图像跨度设置不对，比如跨度比宽度还小
/***************************************************************************/
HI_API int HI_InitDeinterlace(void **pHandle, DEINTERLACE_PARA_S struPara);

/***************************************************************************
         释放deinterlace函数
  功能：释放Deinterlace资源
  参数：
      *pHandle    : 输入参数，Deinterlace句柄,不需要做任何操作,只需
                    按照实例输入参数即可.
  返回值:
         HI_SUCCESS_DEINTERLACE:  成功
         HI_ERR_POINT_NULL     ： 输入pHandle指针为空
***************************************************************************/
HI_API int HI_ReleaseDeinterlace(void *pHandle);

/**************************************************************************
             deinterlace调用接口
 功能： 此函数完成每两场输出一帧图像的功能
 参数：
     *pHandle    : 输入参数，deinterlace句柄
     struDstFrame：输出参数，输出帧YUV420图像
     *pszSrcY    ：输入参数，输入场YUV420图像Y分量
     *pszSrcU    ：输入参数，输入场YUV420图像U分量
     *pszSrcV    ：输入参数，输入场YUV420图像V分量
     tFieldFlag  ：输入参数，输入顶底场标志值
 返回值：
     HI_SUCCESS_DEINTERLACE： 成功
     HI_ERR_POINT_NULL     ： 输入pszDstFrame或pszSrcY指针为空
     HI_ERR_FIELD_FLAG     ： 输入的tFieldFlag不是顶底场标志值
**************************************************************************/
HI_API int HI_Deinterlace(void *pHandle,
                          DEINTERLACE_FRAME_S struDstFrame,
                          unsigned char *pszSrcY,
                          unsigned char *pszSrcU,
                          unsigned char *pszSrcV,
                          PIC_TYPE_E tFieldFlag);

/***************************************************************************
         获取deinterlace版本号函数
  功能：获取deinterlace版本号
  参数：
         *pszVersion : 输出参数，输出版本号字符串.
  返回值:
         HI_SUCCESS_DEINTERLACE:  成功
         HI_ERR_POINT_NULL     ： 输入pszVersion指针为空
***************************************************************************/
HI_API int HI_GetVersion(char **pszVersion);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif   //_DLLDEINTERLACE_H_
