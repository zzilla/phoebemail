#ifndef _UTILS_STRPARSER_H_
#define _UTILS_STRPARSER_H_

#include <string>
#include <vector>

//#include "Utils.h"

class CStrParse
{
public:
	CStrParse();
	CStrParse(
	    const std::string &strSrc,
	    const std::string &strSpliter = ":");

	virtual ~CStrParse();

	void Initialize();

	// set the split string what you want
	void setSpliter(const std::string &strSpliter=":");
	std::string getSpliter();

	// trim item or not
	void setTrim(bool isTrim = true);

	// parse a new string
	bool Parse(const std::string &strSrc);

	// when iPos<0 get word one by one
	std::string getWord(int iPos=-1);

	// when iPos<0 get Value one by one
	long getValue(int iPos=-1);

	// How many items have been parsed
	int Size();
	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
private:

	bool doParse();

	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
	std::string m_strSrc;
	std::string m_strSpliter;
	bool m_bTrim;
	size_t m_iPosition;
	std::vector<std::string> m_vItem;
};

int split_str(
		const char * const src,
		char *left,
		char *right,
		const char * const spliter,
		size_t src_len);

size_t trim_blank(char *src, size_t src_len);

#endif

