#ifndef _ZKit_Uninheritable_h_
#define _ZKit_Uninheritable_h_

#include "ZKit_Config.h"

BEGIN_ZKIT
//if u don't wanna your class to be inherited, use this, just like final in Java and sealed in C#.
//by qguo. 2010.04.30
template <class OnlyFriend> 
class Uninheritable 
{ 
	friend class OnlyFriend;
private:
	Uninheritable(void) {};
};

/*example:
class NotABase : virtual public Uninheritable<NotABase>
{ 
	// WHATEVER 
};

class NotADerived: public NotABase 
{ 
	// THIS CLASS GENERATES A COMPILER ERROR 
};
*/

END_ZKIT
#endif // _ZKit_Uninheritable_h_
