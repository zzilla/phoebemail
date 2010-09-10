#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ZKit_File.h"
BEGIN_ZKIT

File::File()
:m_fil(NULL)
,m_rptr(0)
,m_wptr(0)
{
}


File::~File()
{
	fclose();
}


bool File::fopen(const std::string& path, const std::string& mode)
{
	m_path = path;
	m_mode = mode;
	m_fil = ::fopen(path.c_str(), mode.c_str());

	return m_fil ? true : false;
}


void File::fclose() const
{
	if (m_fil)
	{
		::fclose(m_fil);
		m_fil = NULL;
	}
}



size_t File::fread(char *ptr, size_t size, size_t nmemb) const
{
	size_t r = 0;
	if (m_fil)
	{
		fseek(m_fil, m_rptr, SEEK_SET);
		r = ::fread(ptr, size, nmemb, m_fil);
		m_rptr = ftell(m_fil);
	}
	return r;
}


size_t File::fwrite(const char *ptr, size_t size, size_t nmemb)
{
	size_t r = 0;
	if (m_fil)
	{
		fseek(m_fil, m_wptr, SEEK_SET);
		r = ::fwrite(ptr, size, nmemb, m_fil);
		m_wptr = ftell(m_fil);
	}
	return r;
}



char *File::fgets(char *s, int size) const
{
	char *r = NULL;
	if (m_fil)
	{
		fseek(m_fil, m_rptr, SEEK_SET);
		r = ::fgets(s, size, m_fil);
		m_rptr = ftell(m_fil);
	}
	return r;
}


void File::fprintf(const char *format, ...)
{
	if (!m_fil)
		return;
	va_list ap;
	va_start(ap, format);
	fseek(m_fil, m_rptr, SEEK_SET);
	vfprintf(m_fil, format, ap);
	m_rptr = ftell(m_fil);
	va_end(ap);
}


off_t File::size() const
{
	struct stat st;
	if (stat(m_path.c_str(), &st) == -1)
	{
		return 0;
	}
	return st.st_size;
}


bool File::eof() const
{
	if (m_fil)
	{
		if (feof(m_fil))
			return true;
	}
	return false;
}


void File::reset_read() const
{
	m_rptr = 0;
}


void File::reset_write()
{
	m_wptr = 0;
}



//////////////////////////////////////////////////////////////////////////
//成功, 然会实际读取的字节数, 失败, 返回负数.
int FileUtil::LoadDataFormFile( char* buffer, size_t size, const std::string& file, const std::string& mode /*= "rb"*/ )
{
	File f;
	if (!f.fopen(file, mode))
	{
		assert(0);
		return -1;
	}

	const size_t fileLength = f.size();

	if (fileLength > size)
	{
		assert(0);
		return -2;
	}

	f.fread(buffer, 1, fileLength);

	f.fclose();

	return (int)fileLength;
}

int FileUtil::LoadDataFormFile( std::vector<char>& data, const std::string& file, const std::string& mode /*= "rb"*/ )
{
	File f;
	if (!f.fopen(file, mode))
	{
		assert(0);
		return -1;
	}

	const size_t fileLength = f.size();

	if (fileLength == 0)
	{
		assert(0);
		return -2;
	}

	char* buffer = new char[fileLength];//

	f.fread(buffer, 1, fileLength);

	f.fclose();

	for (size_t i = 0; i < fileLength; ++i)
	{
		data.push_back(buffer[i]);
	}

	delete [] buffer;

	return 0;
}

int FileUtil::SaveDataToFile( const char* buffer, const size_t length, const std::string& file, const std::string& mode /*= "wb"*/ )
{
	File f;
	if (!f.fopen(file, mode))
	{
		assert(0);
		return -1;
	}

	f.fwrite((const char*)buffer, 1, length);
	f.fclose();
	return 0;
}

END_ZKIT