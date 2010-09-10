#if !defined _RENDERMANAGER_H
#define _RENDERMANAGER_H

#include "StdAfx.h"

class CVideoRender;
class CManager;

typedef struct _RenderResource
{
	CVideoRender *	render;
	BOOL				bAvailable;
	HWND				hwindow;
}RenderResource;

class CRenderManager
{
public:
	CRenderManager(CManager *pManager);
	virtual ~CRenderManager();

public:
	
	CVideoRender*	GetRender(HWND hwnd);
	void			ReleaseRender(CVideoRender* rls_render);
	BOOL			Init();
	BOOL			Uninit();
protected:
private:
	list<RenderResource *>		m_renders;
	DEVMutex						m_csRenders;

	CManager*		m_pManager;

};

#endif


