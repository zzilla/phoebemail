// SnapPicture.h: interface for the CSnapPicture class.
// 抓图类
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_SNAPPICTURE_H__1C7BD09A_8154_47BA_A5F1_3F246BCA1B3D__INCLUDED_)
#define AFX_SNAPPICTURE_H__1C7BD09A_8154_47BA_A5F1_3F246BCA1B3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netsdk.h"
#include "../dvr/kernel/afkinc.h"

class CManager;


class CSnapPicture  
{
public:
	CSnapPicture(CManager *pManager);
	virtual ~CSnapPicture();

	bool SnapPictureQuery(LONG lLoginID, SNAP_PARAMS par);

	void SetSnapRevCallBack(fSnapRev OnSnapRevMessage, DWORD dwUser);

public:
	static	int __stdcall		ReceiveData(
										afk_handle_t object,	//数据提供者
										unsigned char *data,	//数据体
										unsigned int datalen,	//数据长度
										void *param,			//回调参数
										void *udata);

private:
	fSnapRev m_RecievCallBack;
	DWORD m_dwuserdata;

	CManager*		m_pManager;
};


#endif // !defined(AFX_SNAPPICTURE_H__1C7BD09A_8154_47BA_A5F1_3F246BCA1B3D__INCLUDED_)

