#ifndef _ZKit_File_h_
#define _ZKit_File_h_

#include "ZKit_IFile.h"

BEGIN_ZKIT

//文件操作具体类, by qguo.
class File : public IFile
{
public:
	File();
	~File();

	bool fopen(const std::string&, const std::string&);
	void fclose() const;

	size_t fread(char *, size_t, size_t) const;
	size_t fwrite(const char *, size_t, size_t);

	char *fgets(char *, int) const;
	void fprintf(const char *format, ...);

	off_t size() const;
	bool eof() const;

	void reset_read() const;
	void reset_write();

private:
	File(const File& ) {} // copy constructor
	File& operator=(const File& ) { return *this; } // assignment operator

	std::string m_path;
	std::string m_mode;
	mutable FILE *m_fil;
	mutable long m_rptr;
	long m_wptr;
};

class FileUtil
{
public:
	static int LoadDataFormFile( char* buffer, size_t size, const std::string& file, const std::string& mode = "rb" );
	static int LoadDataFormFile( std::vector<char>& data, const std::string& file, const std::string& mode = "rb" );
	static int SaveDataToFile(const char* buffer, const size_t length, const std::string& file, const std::string& mode = "wb");
};

END_ZKIT
#endif // _ZKit_File_h_