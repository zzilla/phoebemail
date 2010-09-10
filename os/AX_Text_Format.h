#ifndef _AX_Text_Format_h_
#define _AX_Text_Format_h_

#ifdef _WIN32
#define snprintf _snprintf
#endif

//用于文本格式化的类
//注意buf是由外界分配后传入的, 所以析构函数中并不需要释放它

class AX_Text_Format
{
public:
	AX_Text_Format(const char *buf, int len)
		:_pos((char*)buf),_buf((char*)buf),_len(len)
	{
	}
	~AX_Text_Format()
	{
	}
public:
	AX_Text_Format& operator<<(char c)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%c",c);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned char uc)
	{
		if(0<_len)
		{
			int ret=snprintf(_pos,_len,"%c",uc);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(short s)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%hd",s);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned short us)
	{
		if(0<_len)
		{
			int ret=snprintf(_pos,_len,"%hu",us);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(int i)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%d",i);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned int ui)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%u",ui);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(long l)
	{
		if(0<_len)
		{
			int ret=snprintf(_pos,_len,"%ld",l);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned long ul)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%lu",ul);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(float f)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%f",f);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(double d)
	{
		if(0<_len)
		{

			int ret=snprintf(_pos,_len,"%f",d);

			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(const char * str)
	{
		if(0<_len)
		{
			if(NULL!=str)
			{

				int ret=snprintf(_pos,_len,"%s",str);

				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{

				int ret=snprintf(_pos,_len,"NULL");

				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//add by qguo.
	AX_Text_Format& operator<<(const std::string& str)
	{
		if(0<_len)
		{
			if(0<str.length())
			{
				int ret=snprintf(_pos,_len,"%s",str.c_str());
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{
				int ret=snprintf(_pos,_len,"NULL");
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
		}
		return *this;
	}

	AX_Text_Format& operator<<(void* p)
	{
		if(0<_len)
		{
			if(NULL!=p)
			{
				int ret=snprintf(_pos,_len,"%p",p);
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{
				int ret=snprintf(_pos,_len,"NULL");
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
		}
		return *this;
	}
	//////////////////////////////////////////////////////////////////////////

protected:
	char* _pos;
	char* _buf;
	int _len;
};

//////////////////////////////////////////////////////////////////////////
class AX_ObjKey
{
protected:
	AX_ObjKey()
	{
	}
	~AX_ObjKey()
	{
	}
protected:
	char __objKey[64];
};

#define AX_SET_OBJKEY(str)	\
{	\
	AX_Text_Format txtFormat(__objKey,sizeof(__objKey));	\
	txtFormat<<str;	\
}

#endif // _AX_Text_Format_h_