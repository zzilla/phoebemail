#include "ZKit_BitConverter.h"
BEGIN_ZKIT

std::string BitConverter::ToString( const char* bytes, const size_t length, const size_t maxCount )
{
	if (bytes == NULL)
	{
		return "NULL";
	}
	else
	{
		using namespace std;
		std::ostringstream oss;
		oss << hex << uppercase;
		size_t len = length > maxCount ? maxCount : length;

		if (len == 0)
		{
			len = length;
		}

		for (size_t i = 0; i < len; ++i)
		{
			if (i % 16 == 0)
			{
				oss << "\n";
			}
			oss << setw(2) << setfill('0') << (unsigned short)(unsigned char)bytes[i] << " ";
		}
		if (maxCount < length)
		{
			oss << "\n......";
		}
		return oss.str();
	}
}

//²âÊÔÖ÷»ú×Ö½ÚÐò
bool BitConverter::IsLittleEndian()
{
	union EndianTest
	{
		int i;
		char c;
	};
	EndianTest t;
	t.i = 1;
	return t.c == 1;
}
END_ZKIT