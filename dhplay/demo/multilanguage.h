////////////////////////////////////////////////////////////////////////////////////
// MultiLanguage.h

//******************************* Supplementary Explanation    *******************//
// The initialization process in the multiple-language environment is shwon as below £º
// Language type link list initialization £½>>Interface blue print link list initialization 
// £½>> Select current interface blue print

#ifndef _MULTILANGUAGE_H
#define _MULTILANGUAGE_H

#include "AFXTEMPL.H"

/////////////////////////////////////////////////////////////////////////////////
// The paths in the language and interface configuration file all adopt relative
// path of current work	relative path of catalog(E.g£ºg_strCurDir)
// The following content is configuration information of one language
typedef struct _LANGUAGE_TYPE
{
	char IniFile[64];		//Language configuration file path
	char LangName[64];	    //Language name
	char AuthorName[64];    //Author name
}LANGUAGE_TYPE;
//The following content is configuration information of one interface blue print
typedef struct _INTERFACE_TYPE						   
{													 
	char ConfigFile[1024];	//Interface configuration file path
	char SolutionName[64];	//Interface blue print name	
	char SolutionDir[128];  //Interface blue print working path
	LANGUAGE_TYPE  pLanguage;	//Interface language environment
}INTERFACE_TYPE;
////////////////////////////////////////////////////////////////////////////////

extern CString g_strCurDir;
//Currently used interface blue print
extern INTERFACE_TYPE  g_pCurInterface;
//"English string" switch to "Current language type string"				
extern CString ConvertString(CString strText);                 
//Set static text in dialogue box  (English-->Cureent language)              
extern void SetWndStaticText(CWnd * pWnd);   

extern void SetMenuStaticText(CMenu* pMenu) ;

#endif //_MULTILANGUAGE_H