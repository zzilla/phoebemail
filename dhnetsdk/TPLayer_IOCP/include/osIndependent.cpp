
#include "osIndependent.h"

//////////////////////////////////////////////////////////////////////////

COSEvent::COSEvent()
{
	m_bCreate = FALSE;
	m_bManualReset = FALSE;
#ifdef WIN32
	m_hEvent = NULL;
#endif
}

COSEvent::~COSEvent()
{
	if (m_bCreate)
	{
#ifdef WIN32
		if (m_hEvent != NULL)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
#else
		sem_destroy(&m_hEvent);
#endif
		m_bCreate = FALSE;
	}
}


COSThread::COSThread()
{
	m_bCreate = FALSE;
#ifdef WIN32
	m_hThread = NULL;
#endif
}

COSThread::~COSThread()
{
	if (m_bCreate)
	{
#ifdef WIN32
		if (m_hThread != NULL)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
#endif
		m_bCreate = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////

int		CreateEventEx(OS_EVENT &event, BOOL bManualReset, BOOL bInitialState)
{
#ifdef WIN32
	event.m_hEvent = CreateEvent(NULL, bManualReset, bInitialState, NULL);
	if (event.m_hEvent == NULL)
	{
		event.m_bCreate = FALSE;
		return -1;
	}
	event.m_bCreate = TRUE;
	event.m_bManualReset = bManualReset;
	
	return 0;
#else
	unsigned int nValue = bInitialState?1:0;
	int ret = sem_init(&event.m_hEvent, 0, nValue);
	if (ret != 0)
	{
		event.m_bCreate = FALSE;
		return -1;
	}
	event.m_bCreate = TRUE;
	event.m_bManualReset = bManualReset;

	return 0;
#endif
}


void	CloseEventEx(OS_EVENT &event)
{
	if (event.m_bCreate)
	{
#ifdef WIN32
		if (event.m_hEvent != NULL)
		{
			CloseHandle(event.m_hEvent);
			event.m_hEvent = NULL;
		}
#else
		sem_destroy(&event.m_hEvent);
#endif
		event.m_bCreate = FALSE;
	}
}


BOOL	SetEventEx(OS_EVENT &event)
{
	BOOL bRet = FALSE;
	if (event.m_bCreate)
	{
#ifdef WIN32
		if (event.m_hEvent != NULL)
		{
			bRet = SetEvent(event.m_hEvent);
		}
#else
		int ret = sem_post(&event.m_hEvent);
		bRet = (ret == 0) ? TRUE:FALSE;
#endif
	}

	return bRet;
}


BOOL	ResetEventEx(OS_EVENT &event)
{
	BOOL bRet = FALSE;
	if (event.m_bCreate)
	{
#ifdef WIN32
		if (event.m_hEvent != NULL)
		{
			bRet = ResetEvent(event.m_hEvent);
		}
#else
		int sval = 0;
		do 
		{
			sem_trywait(&event.m_hEvent);
			sem_getvalue(&event.m_hEvent, &sval);
		} while(sval > 0);

		bRet = TRUE;
#endif
	}

	return bRet;
}

DWORD	WaitForSingleObjectEx(OS_EVENT &event, DWORD dwMilliseconds)
{
	DWORD dwRet = 0xFFFFFFFF;
	if (event.m_bCreate)
	{
#ifdef WIN32
		if (event.m_hEvent != NULL)
		{
			dwRet = WaitForSingleObject(event.m_hEvent, dwMilliseconds);
		}
#else
		if (dwMilliseconds == 0)
		{
			int ret = sem_trywait(&event.m_hEvent);
			if (ret == 0)
			{
				dwRet = 0;

				if (event.m_bManualReset)
				{
					sem_post(&event.m_hEvent);
				}
			}
		}
		else if (dwMilliseconds == INFINITE)
		{
			sem_wait(&event.m_hEvent);
			dwRet = 0;

			if (event.m_bManualReset)
			{
				sem_post(&event.m_hEvent);
			}
		}
		else
		{
			int roopMax = dwMilliseconds/10;
			do 
			{
				usleep(10*1000);
				int ret = sem_trywait(&event.m_hEvent);
				if (ret == 0)
				{
					dwRet = 0;

					if (event.m_bManualReset)
					{
						sem_post(&event.m_hEvent);
					}

					break;
				}
				
				roopMax--;
			} while(roopMax > 0);
		}
#endif
	}

	return dwRet;
}


DWORD	WaitForSingleObjectEx(OS_THREAD &thread, DWORD dwMilliseconds)
{
	DWORD dwRet = 0xFFFFFFFF;
	if (thread.m_bCreate)
	{
#ifdef WIN32
		if (thread.m_hThread != NULL)
		{
			dwRet = WaitForSingleObject(thread.m_hThread, dwMilliseconds);
		}
#else
		pthread_join(thread.m_hThread, NULL);
		dwRet = 0;
#endif
	}

	return dwRet;
}


int		CreateThreadEx(OS_THREAD &thread, DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
#ifdef WIN32
	thread.m_hThread = CreateThread(NULL, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	if (thread.m_hThread == NULL)
	{
		thread.m_bCreate = FALSE;
		return -1;
	}
	thread.m_bCreate = TRUE;

	return 0;
#else
	typedef void* (*ln_start_routine)(LPVOID lpThreadParameter);
	ln_start_routine lpThreadAddress = (ln_start_routine)lpStartAddress;

	int ret = pthread_create(&thread.m_hThread, NULL, lpThreadAddress, lpParameter);
	if (ret != 0)
	{
		thread.m_bCreate = FALSE;
		return -1;
	}
	thread.m_bCreate = TRUE;

	return 0;
#endif
}


void	CloseThreadEx(OS_THREAD &thread)
{
	if (thread.m_bCreate)
	{
#ifdef WIN32
		if (thread.m_hThread != NULL)
		{
			CloseHandle(thread.m_hThread);
			thread.m_hThread = NULL;
		}
#endif
		thread.m_bCreate = FALSE;
	}
}


BOOL	TerminateThreadEx(OS_THREAD &thread, DWORD dwExitCode)
{
	BOOL bRet = FALSE;
	if (thread.m_bCreate)
	{
#ifdef WIN32
		if (thread.m_hThread != NULL)
		{
			bRet = TerminateThread(thread.m_hThread, dwExitCode);
		}
#else
		int ret = pthread_cancel(thread.m_hThread);
		bRet = (ret == 0) ? TRUE:FALSE;
#endif
	}

	return bRet;
}


DWORD	GetTickCountEx()
{
#ifdef WIN32
	return GetTickCount();
#else
	tms tm;
    return times(&tm) * 10;
#endif
}


LONG	InterlockedIncrementEx(LN_LIFECOUNT *lpAddend)
{
#ifdef WIN32
	return InterlockedIncrement(lpAddend);
#else
	return atomic_add_return(1, lpAddend);
#endif 
}


LONG	InterlockedDecrementEx(LN_LIFECOUNT *lpAddend)
{
#ifdef WIN32
	return InterlockedDecrement(lpAddend);
#else
	return atomic_sub_return(1, lpAddend);
#endif 
}


void	InterlockedSetEx(LN_LIFECOUNT *lpAddend, int value)
{
#ifdef WIN32
	*lpAddend = value;
#else
	atomic_set(lpAddend, value);
#endif 
}


//////////////////////////////////////////////////////////////////////////
#ifndef WIN32

char*	_strupr(char *string)
{
	for (char *p = string; p < string + strlen(string); p++)
	{
		if (islower(*p))
		{
			*p = toupper(*p);
		}
	}

	return string;
}

char*	_strlwr(char *string)
{
	for (char *p = string; p < string + strlen(string); p++)
	{
		if (isupper(*p))
		{
			*p = tolower(*p);
		}
	}

	return string;
}

int		_stricmp(const char *string1, const char *string2)
{
	int strLen1 = strlen(string1);
	int strLen2 = strlen(string2);

	char *szString1 = new char[strLen1 + 1];
	char *szString2 = new char[strLen2 + 1];

	strcpy(szString1, string1);
	strcpy(szString2, string2);

	_strupr(szString1);
	_strupr(szString2);

	int nRet = strcmp(szString1, szString2);

	delete[] szString1;
	delete[] szString2;

	return nRet;
}

char*	_strrev(char *string)
{
	if (!string)
		return "";

    int i = strlen(string);
    int t = !(i%2)? 1 : 0;
    for(int j = i-1, k = 0; j > (i/2 - t); j--)
    {
        char ch = string[j];
        string[j] = string[k];
        string[k++] = ch;
    }

    return string;
}

char*	itoa(int value, char *string, int radix)
{
	int  rem = 0;
    int  pos = 0;
    char ch  = '!';
    do
    {
        rem    = value % radix;
        value /= radix;
        if (16 == radix)
        {
            if(rem >= 10 && rem <= 15)
            {
                switch(rem)
                {
				case 10:
					ch = 'a';
					break;
				case 11:
					ch ='b';
					break;
				case 12:
					ch = 'c';
					break;
				case 13:
					ch ='d';
					break;
				case 14:
					ch = 'e';
					break;
				case 15:
					ch ='f';
					break;
                }
            }
        }
        if('!' == ch)
        {
            string[pos++] = (char)(rem + 0x30);
        }
        else
        {
            string[pos++] = ch;
        }
    }while(value != 0);
    string[pos] = '\0';
	
    return _strrev(string);
}

#endif

//////////////////////////////////////////////////////////////////////////
/*
short	__ConvertBigLittle(short sNum)
{
#ifdef __BIG_ENDIAN__
	short sResult = 0;

	sResult |= sNum & 0xFF;
	sResult <<= 8;
	sResult |= sNum

	return sResult;
#else
	return sNum;
#endif
}

int		__ConvertBigLittle(int nNum)
{
#ifdef __BIG_ENDIAN__

#else
	return nNum;
#endif
}

long	__ConvertBigLittle(long lNum)
{
#ifdef __BIG_ENDIAN__

#else
	return lNum;
#endif
}

void	__ConvertBigLittle(char *pSource, char *pDestination, int size)
{
	if (pSource == NULL || pDestination == NULL)
	{
		return;
	}

#ifdef __BIG_ENDIAN__

#else
	memcpy(pDestination, pSource, size);
#endif
}
*/

//////////////////////////////////////////////////////////////////////////



