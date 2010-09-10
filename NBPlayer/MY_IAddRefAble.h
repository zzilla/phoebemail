#ifndef __MY_IADDREFABLE_INC__
#define __MY_IADDREFABLE_INC__

#include "ax_iaddrefable.h"

#ifdef _DEBUG

class MY_IAddRefAble : public AX_IAddRefAble
{
public:
	MY_IAddRefAble(void);
	virtual ~MY_IAddRefAble(void);
};

#else

#define MY_IAddRefAble AX_IAddRefAble

#endif //_DEBUG

#endif //__MY_IADDREFABLE_INC__
