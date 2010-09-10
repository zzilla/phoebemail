#ifndef GDIRENDER_H
#define GDIRENDER_H

#include "videorender.h"

#include <windows.h>

class GDIRender : public VideoRender
{
public:
	GDIRender();
	virtual ~GDIRender();

	virtual int init(int index, HWND hWnd, int width, int height, draw_callback cb);

	virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
		int width, int height,RECT*rc);

	virtual int clean();
	
	virtual void resize();

	virtual void VerticalSyncEnable(BOOL bEnable) {};

private:
	inline void colorConvert(unsigned char *py, unsigned char *pu, 
		unsigned char *pv, int width, int height);

	inline void YUV_TO_RGB24(unsigned char *puc_y, int stride_y, 
		unsigned char *puc_u, unsigned char *puc_v, int stride_uv, 
		unsigned char *puc_out, int width_y, int height_y,int stride_out);
	
	void ShowString(HDC hDC);

	void GetRenderRect(HWND hWnd, RECT &rcRect);

private:
	int m_index;
	HWND m_hWnd;
	draw_callback m_cb;

	bool m_flip;
	bool m_rgbReverseOrder;

	struct RGBPlane 
	{
        RGBPlane();
        ~RGBPlane();
        
        unsigned char * data;
        int w, h;
        
        void resize(int width,  int height);
        unsigned char * getLine(int y);
    }  m_rgb;
};

#endif /* GDIRENDER_H */
