#ifndef _ZKit_ISerializable_h_
#define _ZKit_ISerializable_h_

#include "ZKit_Config.h"

BEGIN_ZKIT
class ISerializable
{
public:
	virtual ~ISerializable(){};
	virtual bool Serialize(std::vector<char>& bytes) const = 0;
	virtual bool Deserialize(const std::vector<char>& bytes) = 0;
};
END_ZKIT
#endif // _ZKit_ISerializable_h_