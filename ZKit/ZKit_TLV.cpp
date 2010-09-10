#include "ZKit_TLV.h"

BEGIN_ZKIT

TLV::TLV() : m_type(0), m_length(0), m_value(NULL)
{

}

TLV::TLV( const uint16 type, const uint32 length, const char* value ) : m_type(type), m_length(length), m_value(NULL)
{
	if (m_length > 0)
	{
		m_value = new char[m_length];
		memcpy(m_value, value, m_length);
	}
}

TLV::~TLV()
{
	delete [] m_value;
	m_value = NULL;
}

uint32 TLV::GetTotalLength() const
{
	return TLV_LEN_TYPE + TLV_LEN_LENGTH + m_length;
}

void TLV::Init( const uint16 type, const uint32 length, const char* value )
{
	m_type = type;
	m_length = length;	
	if (m_value != NULL)
	{
		delete [] m_value;
		m_value = NULL;
	}

	if (m_length > 0)
	{
		m_value = new char[m_length];
		memcpy(m_value, value, m_length);
	}
}

bool TLV::Serialize( std::vector<char>& bytes ) const
{
	BitConverter::AppendValue(bytes, m_type);
	BitConverter::AppendValue(bytes, m_length);
	bytes.insert(bytes.end(), m_value, m_value + m_length);
	return true;
}

bool TLV::Deserialize( const std::vector<char>& bytes )
{
	BitConverter::GetValue(&bytes[0], TLV_IDX_TYPE, m_type);
	BitConverter::GetValue(&bytes[0], TLV_IDX_LENGTH, m_length);
	if (bytes.size() >= GetTotalLength())
	{
		if (m_length == 1)
		{
			m_value = new char;
			*m_value = bytes[TLV_IDX_VALUE];
		}
		else if (m_length >= 1)
		{
			m_value = new char[m_length];
			for (uint32 i = 0; i < m_length; ++i)
			{
				m_value[i] = bytes[TLV_IDX_VALUE + i];
			}
		}

		return true;
	}
	else
	{
		assert(0);
		return false;
	}
}

std::string TLV::ToString() const
{
	std::ostringstream oss;

	oss << *this;

	return oss.str();
}

std::ostream& operator<<( std::ostream& oss, const TLV& tlv )
{
	oss << "\nType: " << tlv.GetType() << "  Length: " << tlv.GetLength() << "  Value: ";

	if (tlv.GetLength() > 0 && tlv.GetValue() != NULL)
	{
		oss << BitConverter::ToString(tlv.GetValue(), tlv.GetLength(), 32);
	}
	else
	{
		oss << "NULL";
	}
	return oss;
}

END_ZKIT