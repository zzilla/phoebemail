#ifndef _ZKit_TLV_h_
#define _ZKit_TLV_h_

#include "ZKit_Protocol.h"
#include "ZKit_ISerializable.h"
#include "ZKit_BitConverter.h"
#include "ZKit_Mutex.h"
#include "ZKit_Locker.h"

BEGIN_ZKIT

class TLV : public ISerializable
{
public:
	TLV();

	TLV(const uint16 type, const uint32 length, const char* value);

	virtual ~TLV();

	//属性.
	uint16 GetType() const 
	{ 
		return m_type; 
	}

	uint32 GetLength() const 
	{ 
		return m_length; 
	}

	const char* GetValue() const
	{
		return m_value;
	}

	bool Check() const
	{
		return GetType() > 0 && GetLength() > 0 && GetValue() != NULL;
	}

	uint32 GetTotalLength() const;

	//标准初始化函数
	void Init(const uint16 type, const uint32 length, const char* value);

	//如果参数只是简单数值类型如int, short, 可以采用此函数来初始化tlv
	template<typename T>
	void Init(const uint16 type, const T& value)
	{
		char buf[sizeof(T)];
		BitConverter::ToBytes(value, buf);
		Init(type, sizeof(T), buf);
	}

	//如果参数是简单数值类型的数组如int[], 可以采用此函数来初始化tlv
	template<typename T>
	void Init(const uint16 type, const T* p, size_t size)
	{
		std::vector<char> v;
		v.reserve(size * sizeof(T));

		for (size_t i = 0; i < size; ++i)
		{
			BitConverter::AppendValue(v, p[i]);
		}

		Init(type, (uint32)v.size(), &v[0]);
	}

	template<typename T>
	void GetArg(T& value)
	{
		BitConverter::GetValue(m_value, 0, value);
	}

	TLV& operator = (const TLV& rhs)
	{
		if (&rhs != this)
		{
			Init(rhs.GetType(), rhs.GetLength(), rhs.GetValue());
		}
		return *this;
	}

	TLV(const TLV& rhs) : m_type(0), m_length(0), m_value(NULL)
	{
		*this = rhs;
	}

	bool Equals(const TLV& rhs) const
	{
		return m_type == rhs.m_type && m_length == rhs.m_length && 0 == memcmp(m_value, rhs.m_value, m_length);
	}

	bool operator == (const TLV& rhs) const
	{
		return this->Equals(rhs);
	}

	std::string ToString() const;

	virtual bool Serialize(std::vector<char>& bytes) const;

	virtual bool Deserialize(const std::vector<char>& bytes);

private:
	uint16  m_type;
	uint32  m_length;
	char*   m_value;
};

//注意<< 和 >> 不应该是成员函数, 但它们是类接口
std::ostream& operator << (std::ostream& os, const TLV& tlv);

END_ZKIT
#endif // _ZKit_TLV_h_
