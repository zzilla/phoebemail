#include "AX_sstream.h"
#include "AX_Str.h"

#ifdef WIN32
#define snscanf  _snscanf
#pragma warning(disable : 4996)
#endif

AX_stringstream::AX_stringstream(uint32 step)
:_step(step)
{
	if(0>=_step)
		_step=1024;
	_capacity=_step;
	_writePos=0;
	_readPos=0;
	_buf=new char[_capacity];
	memset(_buf,0,_capacity);
}
AX_stringstream::~AX_stringstream()
{
	if(NULL!=_buf)
		delete[]_buf;
	_buf=NULL;
	_step=0;
	_capacity=0;
	_writePos=0;
	_readPos=0;
}

//////////////////////////////////////////////////////////////////////////
void AX_stringstream::reserve(uint32 capacity)
{
	applySpace(capacity);
}
void AX_stringstream::applySpace(uint32 len)
{
	if(0<len && _capacity-_writePos<=len)
	{
		uint32 ex_len=_step;
		if(ex_len<=len)
			ex_len+=len;
		uint32 new_capacity=_capacity+ex_len;
		char* new_buf=new char[new_capacity];
		memset(new_buf,0,new_capacity);
		memcpy(new_buf,_buf,_capacity);
		delete[]_buf;
		_buf=new_buf;
		_capacity=new_capacity;
	}
}
const std::string AX_stringstream::str()
{
	return _buf;
}
//////////////////////////////////////////////////////////////////////////

AX_stringstream& AX_stringstream::operator<<(const int8& i8)
{
	applySpace(5);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%d",i8);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const uint8& ui8)
{
	applySpace(5);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%u",ui8);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const int16& i16)
{
	applySpace(10);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%hd",i16);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const uint16& ui16)
{
	applySpace(10);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%hu",ui16);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const int32& i32)
{
	applySpace(20);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%d",i32);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const uint32& ui32)
{
	applySpace(20);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%u",ui32);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const int64& i64)
{
	applySpace(30);
#if defined(WIN32)||defined(WINCE)
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%I64d",i64);
#else
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%lld",i64);
#endif
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const uint64& ui64)
{
	applySpace(30);
#if defined(WIN32)||defined(WINCE)
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%I64u",ui64);
#else
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%llu",ui64);
#endif
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const float32& f32)
{
	applySpace(50);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%f",f32);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const float64& f64)
{
	applySpace(50);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%f",f64);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const char* str)
{
	if(NULL!=str)
	{
		applySpace(uint32(strlen(str)+1));
		int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%s",str);
		if(0<len)
			_writePos+=len;
	}
	else
	{
		static const char *nullStr="NULL";
		applySpace(5);
		int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%s",nullStr);
		if(0<len)
			_writePos+=len;
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const std::string& str)
{
	applySpace(uint32(str.length()+1));
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%s",str.c_str());
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const void* p)
{
	applySpace(20);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%p",p);
	if(0<len)
		_writePos+=len;
	return *this;
}
AX_stringstream& AX_stringstream::operator<<(const size_t& sv)
{
	applySpace(30);
	int32 len=AX_OS::snprintf(_buf+_writePos,_capacity-_writePos,"%lu",sv);
	if(0<len)
		_writePos+=len;
	return *this;
}



//////////////////////////////////////////////////////////////////////////

#ifdef WIN32

AX_stringstream& AX_stringstream::operator>>(int8& i8)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%d",&i8);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(uint8& ui8)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%u",&ui8);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(int16& i16)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%hd",&i16);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(uint16& ui16)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%hu",&ui16);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(int32& i32)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%d",&i32);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(uint32& ui32)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%u",&ui32);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(int64& i64)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%I64d",&i64);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(uint64& ui64)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%I64u",&ui64);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(float32& f32)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%f",&f32);
	}
	return *this;
}
AX_stringstream& AX_stringstream::operator>>(float64& f64)
{
	if(_readPos<_writePos)
	{
		snscanf(_buf+_readPos,_writePos-_readPos,"%f",&f64);
	}
	return *this;
}

AX_stringstream& AX_stringstream::operator>>(std::string& str)
{
	if(_readPos<_writePos)
	{
		uint32 len=_writePos-_readPos;
		if(str.capacity()<len)
			str.reserve(len);
		snscanf(_buf+_readPos,_writePos-_readPos,"%s",(char*)str.data());
	}
	return *this;
}

#endif

