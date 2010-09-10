#ifndef _ZKit_Uncopyable_h_
#define _ZKit_Uncopyable_h_

#include "ZKit_Config.h"

BEGIN_ZKIT
//if u don't wanna your class to be copied, use this.
//by qguo. 2010.04.30
class Uncopyable
{
protected:
	Uncopyable(){};

	~Uncopyable(){};
private:
	Uncopyable(const Uncopyable&);

	Uncopyable& operator = (const Uncopyable&);
};

/*example:
class UncopyableExample : private Uncopyable  
{  
    //THIS CLASS IS UNCOPYABLE  
};
*/

END_ZKIT
#endif // _ZKit_Uncopyable_h_
