#include <windows.h>
#include <CommCtrl.h>
#include "CommonFunctions.h"
#include "CloudDrive.h"
#include "UIListview.h"
#include "resource.h"

/*******************************************************************
Add exception/error message to the listbox control of the Progress Dialog
Show the Error message info,  show the "Show Detail" button
*******************************************************************/
void AddErrMsgToDlgListbox(HWND hProgressDlg, LPCWSTR lpszErrMsg)
{
	ShowWindow( GetDlgItem( hProgressDlg, IDC_BUTTONHIDE), SW_SHOW);
	ShowWindow( GetDlgItem(hProgressDlg, IDC_STATICERROR), SW_SHOW);
	
	HWND hList = GetDlgItem( hProgressDlg, IDC_LISTMSG);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)lpszErrMsg);  
}

/*********************************************************************
Get all selected node ids in listview control  into  lpszNodeIds;
param:
	lpszNodeIds: [out]   the buffer contains all node ids, each seperated by one space char
*******************************************************************/
void	GetSelectedNodeIds(LPWSTR lpszNodeIds)
{
	ZeroMemory(lpszNodeIds, sizeof(lpszNodeIds));

	LONG lSelectedCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT,  0, 0);
	int iSelected = -1;
	for (int i=0; i<lSelectedCount; i++)
	{
	     iSelected = SendMessage(g_hListView, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);

	     if (iSelected >=0)
	     {
	     	//get the NodeId
		WCHAR szNodeId[LEN_NODEID]={0};
		LVITEM lvItem;
		ZeroMemory(&lvItem, sizeof(LVITEM));
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem =COLUMNINDEX_NODEID;
		lvItem.iItem = iSelected;
		lvItem.pszText = szNodeId;
		lvItem.cchTextMax = LEN_NODEID;
		SendMessage(g_hListView, LVM_GETITEMTEXT, iSelected, (LPARAM)&lvItem);
		
		wcscat(lpszNodeIds,  szNodeId);
		wcscat(lpszNodeIds,  L" ");  //seperated by a space char
	     }
	}
	lpszNodeIds[wcslen(lpszNodeIds)-1]=L'\0';  //get rid of the last space char
}
