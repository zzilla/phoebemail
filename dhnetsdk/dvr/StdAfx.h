// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8D89C208_8E35_4E61_89A9_C3CECE9F04D5__INCLUDED_)
#define AFX_STDAFX_H__8D89C208_8E35_4E61_89A9_C3CECE9F04D5__INCLUDED_

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#pragma warning(disable:4786)

#else	//linux

#endif

#include "versionctl.h"
#include "osIndependent.h"
#include "kernel/afkdef.h"
#include "kernel/afkplugin.h"
#include "dvrdevice/dvr2cfg.h"
#include "def.h"

#include <vector>
#include <list>
#include <queue>
#include "ReadWriteMutex.h"
using namespace SPTools;

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8D89C208_8E35_4E61_89A9_C3CECE9F04D5__INCLUDED_)

