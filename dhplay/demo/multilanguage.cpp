////////////////////////////////////////////////////////////////////////////////////
// MultiLanguage.cpp

#include "stdafx.h"
#include "MultiLanguage.h"
#include "Resource.h"

CString g_strCurDir =".\\";			
INTERFACE_TYPE  g_pCurInterface;	

CString ConvertString(CString strText);    
void SetWndStaticText(CWnd * pWnd);                
void SetMenuStaticText(CMenu* pMenu)  ;

CString ConvertString(CString strText)
{
	char val[300];
	CString strIniPath,strRet;

	strIniPath.Format("%s\\%s",g_strCurDir,g_pCurInterface.pLanguage.IniFile);
	memset(val,0,sizeof(val));
	GetPrivateProfileString("String",strText,"",
		val,sizeof(val),strIniPath);
	strRet=val;
	if(strRet.GetLength()==0)
	{
		strRet=strText;
	}
	return strRet;
}

void SetMenuStaticText(CMenu* pMenu) 
{
	CString strCaption,strText;

	int MenuItemNum = pMenu->GetMenuItemCount() ;

	for (int i = 0 ; i < MenuItemNum ; i++)
	{		
		MENUITEMINFO info;
		memset(&info, 0 , sizeof(MENUITEMINFO)) ;
	    info.cbSize = sizeof (MENUITEMINFO); // must fill up this field
	    info.fMask = MIIM_STATE;             // get the state of the menu item

		pMenu->GetMenuString(i, strCaption, MF_BYPOSITION);

		strText=ConvertString(strCaption);

		UINT ID = pMenu->GetMenuItemID(i) ;

		if (ID != -1)
		{
			pMenu->GetMenuItemInfo(ID, &info) ;
			pMenu->ModifyMenu(ID, MF_BYCOMMAND|MF_STRING, ID, strText);
			SetMenuItemInfo(pMenu->m_hMenu, ID, FALSE, &info) ;
		}
		else
		{
			pMenu->ModifyMenu(i, MF_BYPOSITION| MF_STRING, 0, strText);
		}
	
		CMenu* subMenu = pMenu->GetSubMenu(i) ;
		
		if (subMenu != NULL)
		{			
			SetMenuStaticText(subMenu) ;
		}
	}
}

void SetWndStaticText(CWnd * pWnd)
{
	CString strCaption,strText;

	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText=ConvertString(strCaption);
		pWnd->SetWindowText(strText);
	}

	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")
		{
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}
		//////////////////////////////////////////////////////////////////////////
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption);
		pChild->SetWindowText(strText);
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}