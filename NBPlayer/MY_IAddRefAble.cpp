#include "MY_IAddRefAble.h"
#include "AX_Mutex.h"
#include <set>
#include "SingleZenoLock.h"

#ifdef _DEBUG

AX_Mutex g_mtxRefObj;
set<MY_IAddRefAble*> g_setRefObj;

MY_IAddRefAble::MY_IAddRefAble(void)
{
	CSingleZenoLock lock(g_mtxRefObj);
	g_setRefObj.insert(this);
}

MY_IAddRefAble::~MY_IAddRefAble(void)
{
	CSingleZenoLock lock(g_mtxRefObj);
	g_setRefObj.erase(this);
}

#endif