#ifndef FIFOBUFFER_H
#define FIFOBUFFER_H
#include <windows.h>
#include "decode.h"
// 只要用于内部记录更多信息
struct DhAVFrame 
{
	DhAVFrame *prev;
	DhAVFrame *next;	
	unsigned char *context;	// 数据指针
	int width ;
	int height ;
	int size;
	//Frame bit rate
	int frameLen;
	int frameType;
	//IVS
	int ivsobjnum;
	DH_IVS_OBJ* ivsObjList;
	DH_IVS_PREPOS tIvsPrePos;
};

class FifoBuffer 
{
public:
	FifoBuffer();
	~FifoBuffer();

	int init(int count,int chunksize);
	int reset();
	int resize(int chunksize);
	int clear();
	bool write(DhAVFrame* vframe) ;
	bool read(DhAVFrame* vframe) ;
	int chunkCount();//有效数据 块数
	void setlock(bool iflock) ;
	bool getlock();
	BYTE* GetLastFrame();
	BYTE* GetNextWritePos();

	void SetbReadNull(bool ifReadNull){m_bReadNull = ifReadNull;}
	bool GetIfReadNull(){return m_bReadNull;}

	void GetLastFrameBuf(unsigned char* pBuf){m_dataFrame.context = pBuf;}

private:		
	bool createFreeList();	
	void cleanFreeList();
	void cleanDataList();
	
	DhAVFrame *getFreeNode();
	DhAVFrame *getDataNode();
	
	void appendToFreeList(DhAVFrame *item);
	void appendToDataList(DhAVFrame *item);

private:

	unsigned char *m_mempool;//内存池

	bool m_ifLock ;//如果锁定，则write函数中一直等待有空的缓冲块，在定位操作时，将它置为False,以使write函数尽快返回
	bool m_inited; // 初始化成功标志
	int m_count; // 节点总数
	int m_chunksize;
	DhAVFrame *m_freeQueueFirst;
	DhAVFrame *m_freeQueueLast;
	int m_freeCount;//空的块的数

	DhAVFrame *m_outQueueFirst;
	DhAVFrame *m_outQueueLast;
	int m_outCount;//有效数据块数

	DhAVFrame m_dataFrame;
	CRITICAL_SECTION m_DataAccessLock ;

	bool m_bReadNull;
};

#endif 




















