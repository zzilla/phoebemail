#include "StdAfx.h"
#include "Utils_StrParser.h"

#define W_NEW(var,classname) {var = new classname;}
#define W_DELETE(p) if(NULL != (p)) {delete((p)); (p)=NULL; }
#define W_FREE(p) if(NULL != (p)) {free((p)); (p)=NULL; }
#define EMPTY_STRING ""

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x) 
#endif
/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CStrParse::Initialize()
{
	m_iPosition = 0;
	m_vItem.clear();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CStrParse::CStrParse()
{
	m_bTrim = true;
	m_strSpliter = ":";
	Initialize();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CStrParse::CStrParse(
    const std::string &strSrc,
    const std::string &strSpliter)
{
	m_bTrim = true;
	m_strSpliter = strSpliter;

	Parse(strSrc);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

bool CStrParse::Parse(const std::string &strSrc)
{
	Initialize();

	m_strSrc = strSrc;

	return doParse();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CStrParse::~CStrParse()
{
	// No need, but write here
	m_vItem.clear();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

bool CStrParse::doParse()
{
	char *buf;
	char *buf_left;
	char *buf_right;

	if (m_strSrc.empty() || m_strSpliter.empty())
	{
		DEB(  printf("strSrc or strSpliter empty\n");  )

		return false;
	}

	//
	// malloc is taste good in uClinux
	// so do not use new / delete here
	//
	//buf_left = new char[m_strSrc.size()+1];
	buf_left = (char *)malloc(m_strSrc.size()+1);

	if (NULL == buf_left)
	{
		DEB(  printf("alloc memory failed.\n");  )
		return false;
	}
	//buf_right = new char[m_strSrc.size()+1];
	buf_right = (char *)malloc(m_strSrc.size()+1);
	if (NULL == buf_right)
	{
		DEB(  printf("alloc memory failed.\n");  )
		//delete []buf_left;
		W_FREE(buf_left);
		return false;
	}

	strcpy(buf_right, m_strSrc.c_str());
	buf = buf_right;

	m_vItem.clear();

	while (0
	       == split_str(
	           buf,
	           buf_left,
	           buf_right,
	           m_strSpliter.c_str(),
	           0))
	{
		if (m_bTrim)
		{
			trim_blank(buf_left, 0);
		}

		m_vItem.push_back(buf_left);
		buf = buf_right;
	} // while

	//
	// last word is seperator, we add a item.
	//
	if ( m_strSrc.size() >= m_strSpliter.size()
		&& 0 == _stricmp(m_strSrc.c_str()+m_strSrc.size()-m_strSpliter.size(), 
						m_strSpliter.c_str()) )
	{
		m_vItem.push_back("");
	}

	//delete []buf_left;
	//delete []buf_right;
	W_FREE(buf_left);
	W_FREE(buf_right);

	return true;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CStrParse::setSpliter(const std::string &strSpliter)
{
	m_strSpliter = strSpliter;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

std::string CStrParse::getSpliter()
{
	return m_strSpliter;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CStrParse::setTrim(bool isTrim )
{
	m_bTrim = isTrim;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

std::string CStrParse::getWord(int iPos)
{
	if (iPos >= 0)
	{
		m_iPosition = iPos+1;
	}
	else
	{
		m_iPosition++;
	}

	if (m_iPosition > m_vItem.size())
	{
		return EMPTY_STRING;
	}

	return m_vItem[m_iPosition-1];
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

long CStrParse::getValue(int iPos)
{
	return atol(getWord(iPos).c_str());
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

int CStrParse::Size()
{
	return m_vItem.size();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 * split_str - 
 * @: 
 * @: 
 *
 * find func based strstr, it find from left
 */
int split_str(
    const char * const src,
    char *left,
    char *right,
    const char * const spliter,
    size_t src_len)
{
	char *p;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !left || !right)
	{
		return -1000;
	}

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	strncpy(right, src, src_len);
	right[src_len] = '\0';

	/* get right pos */
	p = strstr(right, spliter);
	/* not found spliter */
	if (!p)
	{
#if 0
		// ret error here
		*left = '\0';
		*right = '\0';
		return -1;
#else
		// we load src to lefp
		// ret ok here
		strcpy(left, src);
		*right = '\0';
		return 0;
#endif

	}
	/* left */
	strncpy(left, right, p-right);
	left[p-right] = '\0';
	/* right */
	strcpy(right, p+strlen(spliter));

	return 0;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t trim_blank(char *src, size_t src_len)
{
	size_t pass_len;
	size_t real_len;
	char *p = src;

	/* check input*/
	if (!src || !*src)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	while ((*p != '\0')
	       && ( (*p == ' ')
	            || (*p == '\t')
	            || (*p == '\r')
	            || (*p == '\n') ) )
	{
		p++;
	}

	pass_len = p-src;
	src_len = src_len-pass_len;

	memmove(src, p, src_len);
	src[src_len] = '\0';

	p = src + src_len - 1;
	while (p >= src)
	{
		if ( (*p == ' ')
		     || (*p == '\t')
		     || (*p == '\r')
		     || (*p == '\n') )
		{
			*p-- = 0;
			pass_len++;
		}
		else
		{
			break;
		}
	}

	return (size_t)(pass_len);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
