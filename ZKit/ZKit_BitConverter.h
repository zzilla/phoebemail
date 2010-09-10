#ifndef _ZKit_BitConverter_h_
#define _ZKit_BitConverter_h_

#include "ZKit_Config.h"

BEGIN_ZKIT
//////////////////////////////////////////////////////////////////////////
//用于字节转化等处理的类
class BitConverter
{
public:

	// @brief 将字节数组转化为数值
	// @param bytes: bigendian的字节数组
	// @param startIndex: 起始位置
	// @param value: 结果 [out]
	// @return void
	// @note 如果你的bytes是一个vector<char>, 则使用&bytes[0]
	template<typename T>
	static void GetValue(const char* bytes, const size_t startIndex, T& value)
	{
		value = 0;
		for (size_t i = startIndex; i < startIndex + sizeof(T); ++i)
		{
			value = value << 8;
			value += (T)((byte)bytes[i]);			
		}		
	}

	template<typename T>
	static void ToBytes(const T v, char* bytes)
	{
		T value = v;
		const size_t length = sizeof(T);

		for (size_t i = 0; i < length; ++i)
		{
			bytes[length -1 -i] = (char)(value & 0xFF);
			value = value >> 8;
		}
	}

	template<typename T>
	static void AppendValue(std::vector<char>& bytes, const T value)
	{
		const size_t size = sizeof(T);
		char buf[size];
		ToBytes(value, buf);
		bytes.insert(bytes.end(), buf, buf + size);
	}

	//maxCount = 0表示不限定
	static std::string ToString(const char* bytes, const size_t length, const size_t maxCount = 0);

	static bool IsLittleEndian();
};
END_ZKIT
#endif // _ZKit_BitConverter_h_