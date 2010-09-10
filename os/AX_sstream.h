#ifndef AX_SSTREAM_H_INCLUDED
#define AX_SSTREAM_H_INCLUDED

#include <string>
#include "platform.h"

class AX_stringstream
{
public:
	explicit AX_stringstream(uint32 step=1024);
	~AX_stringstream();
public:
	void reserve(uint32 capacity);
public:
	AX_stringstream& operator<<(const int8& i8);
	AX_stringstream& operator<<(const uint8& ui8);
	AX_stringstream& operator<<(const int16& i16);
	AX_stringstream& operator<<(const uint16& ui16);
	AX_stringstream& operator<<(const int32& i32);
	AX_stringstream& operator<<(const uint32& ui32);
	AX_stringstream& operator<<(const int64& i64);
	AX_stringstream& operator<<(const uint64& ui64);
	AX_stringstream& operator<<(const float32& f32);
	AX_stringstream& operator<<(const float64& f64);
	AX_stringstream& operator<<(const char* str);
	AX_stringstream& operator<<(const std::string& str);
	AX_stringstream& operator<<(const void* p);
	AX_stringstream& operator<<(const size_t& sv);
public:
	AX_stringstream& operator>>(int8& i8);
	AX_stringstream& operator>>(uint8& ui8);
	AX_stringstream& operator>>(int16& i16);
	AX_stringstream& operator>>(uint16& ui16);
	AX_stringstream& operator>>(int32& i32);
	AX_stringstream& operator>>(uint32& ui32);
	AX_stringstream& operator>>(int64& i64);
	AX_stringstream& operator>>(uint64& ui64);
	AX_stringstream& operator>>(float32& f32);
	AX_stringstream& operator>>(float64& f64);
	AX_stringstream& operator>>(std::string& str);
public:
	const std::string str();
protected:
	void applySpace(uint32 len);
	char* _buf;
	uint32 _capacity;
	uint32 _writePos;
	uint32 _step;

	uint32 _readPos;
};


#endif//AX_SSTREAM_H_INCLUDED
