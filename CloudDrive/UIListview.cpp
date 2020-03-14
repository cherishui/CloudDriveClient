/****************************************************************
the functions in this file handle ListView control operations
Author: jerry wang (王建华)
all right reserved.
*************************************************************/
#include <windows.h>
#include <commctrl.h>
#include <atlcomtime.h>
#include <math.h>
#include <process.h>
#include "CloudDrive.h"
#include "UIListview.h"
#include "resource.h"
#include "ThreadParam.h"
#include "ShareAPINode.h"
#include "DialogProcedure.h"
#include "ShareAPI.h"
#include "ThreadCallbackFunction.h"
#include "UIMenuToolbarStatusbar.h"
#include "DownloadFile.h"

/****************************************************************
Create ListView control
************************************************************/
void CreateListView(void)
{
	//register specific common controls classes from the common control DLL
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icce);

	//Create the Listview control with Report(detail) View
	RECT rc;
	GetClientRect(g_hwnd, &rc);
	g_hListView = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW , WC_LISTVIEW, NULL, 
				WS_CHILD|WS_VISIBLE|LVS_REPORT| WS_HSCROLL|WS_VSCROLL|LVS_EDITLABELS,
				0,0, rc.right-rc.left, rc.bottom-rc.top, g_hwnd, (HMENU)ID_LISTVIEW, g_hInstance, NULL);
	
	SendMessage(g_hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}


/*************************************************************************
initialize the columns of the listview control
The nodeid column is hidden by set its width to 0;
*************************************************************************/
void InitListViewColumns()
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;

	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = TEXT("Name");
	lvc.iSubItem =COLUMNINDEX_NAME;
	lvc.cx = 160;
	SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_NAME, (LPARAM)&lvc);

	lvc.pszText = TEXT("Size");
	lvc.iSubItem =COLUMNINDEX_SIZE;
	lvc.cx = 80;
	SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_SIZE, (LPARAM)&lvc);

	lvc.pszText = TEXT("Type");
	lvc.iSubItem = COLUMNINDEX_TYPE;
	lvc.cx = 160;
	SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_TYPE, (LPARAM)&lvc);

	lvc.pszText = TEXT("CreateDate");
	lvc.iSubItem = COLUMNINDEX_CREATEDATE;
	lvc.cx = 160;
	SendMessage(g_hListView, LVM_INSERTCOLUMN,COLUMNINDEX_CREATEDATE, (LPARAM)&lvc);

	lvc.pszText = TEXT("ModifyDate");
	lvc.iSubItem = COLUMNINDEX_MODIFYDATE;
	lvc.cx = 160;
	SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_MODIFYDATE, (LPARAM)&lvc);

	//lvc.pszText = TEXT("Status");                 //Shared, Encrypted.....
	//lvc.iSubItem =COLUMNINDEX_STATUS;
	//lvc.cx = 100;
	//SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_STATUS, (LPARAM)&lvc);
	
	lvc.fmt = LVCFMT_FIXED_WIDTH|LVCFMT_NO_TITLE;
	lvc.pszText = TEXT("NodeId");
	lvc.iSubItem = COLUMNINDEX_NODEID;                                          //nodeid is hidden !!!!!!
	lvc.cx = 1;
	SendMessage(g_hListView, LVM_INSERTCOLUMN, COLUMNINDEX_NODEID, (LPARAM)&lvc);
}

/*******************************************************
get system imagelist to use on our app
Using system api to get the system imageList, this imagelist is being used on g_hListView.
****************************************************************/
VOID InitListViewImageLists()
{
	SHFILEINFO sfi;
	HIMAGELIST hLargeImageList = (HIMAGELIST)SHGetFileInfo(TEXT("."), NULL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX|SHGFI_LARGEICON);
	HIMAGELIST hSmallImageList  = (HIMAGELIST)SHGetFileInfo(TEXT("."), NULL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX|SHGFI_SMALLICON);

	ListView_SetImageList(g_hListView, hLargeImageList, LVSIL_NORMAL);
	ListView_SetImageList(g_hListView, hSmallImageList, LVSIL_SMALL);
}


/************************************************************************
Add one  file/folder node to the listview 

for default: we add new node as the last item of the listview,   this can avoid  flashing.
************************************************************************/
void AddNodeToListView(MSXML2::IXMLDOMNodePtr nodePtr)
{
	ShareAPINode * pNode = new ShareAPINode(nodePtr);  //convert IXmlDomNodePtr to ShareAPINode  to easy using. 
	
	LRESULT iItemCount = SendMessage(g_hListView, LVM_GETITEMCOUNT, 0,0);
	
	//Name Column
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvItem.iItem = iItemCount; 
	lvItem.iSubItem =COLUMNINDEX_NAME; 
	
	WCHAR szName[MAX_PATH] ={0};
	wcscpy_s(szName, MAX_PATH, pNode->GetName());
	lvItem.pszText =szName;
	lvItem.lParam = iItemCount;   
	
	SHFILEINFO sfi;
	if (pNode->IsDirectory())
	{
		if(SHGetFileInfo(szName, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX )>0)
			lvItem.iImage=sfi.iIcon;
	}
	else
	{
		if(SHGetFileInfo(szName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX )>0)
			lvItem.iImage=sfi.iIcon;
	}
			
	SendMessage(g_hListView, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

	
	// Size Column
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = COLUMNINDEX_SIZE;
	if (pNode->IsDirectory())
	{
		lvItem.pszText = L"";
	}
	else
	{
		WCHAR szFileSize[LEN_SIZE] = {0};
		int iFileSize = ceil( (float)pNode->GetFileSize()/1024);  //get the KB size of file
		wsprintf(szFileSize, TEXT("%dKB"), iFileSize);
		lvItem.pszText = szFileSize;
	}
	SendMessage(g_hListView, LVM_SETITEM, 0, (LPARAM)&lvItem);

	// Type Column
	lvItem.iSubItem = COLUMNINDEX_TYPE;
	if (pNode->IsDirectory())
	{
		lvItem.pszText = L"Folder";
	}
	else
	{
		if (SHGetFileInfo( szName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME )>0)
		{
			lvItem.pszText = sfi.szTypeName;
		}
		else
		{
			lvItem.pszText = L" ";
		}
	}
	SendMessage(g_hListView, LVM_SETITEM, 0, (LPARAM)&lvItem);

	// CreateDate column
	lvItem.iSubItem =COLUMNINDEX_CREATEDATE;
	
	WCHAR szTime[64] = {0};
	time_t  ttCreate= pNode->GetCreatedDate()/1000; 
	wcsftime(szTime, 64, L"%x  %X", gmtime(&ttCreate));
	lvItem.pszText = szTime;
	SendMessage(g_hListView, LVM_SETITEM, 0, (LPARAM)&lvItem);

	// ModifyDate
	lvItem.iSubItem = COLUMNINDEX_MODIFYDATE;
	time_t ttModi = pNode->GetModifiedDate()/1000;
	wcsftime(szTime,64, L"%x  %X", gmtime(&ttModi));
	lvItem.pszText = szTime;
	SendMessage(g_hListView, LVM_SETITEM, 0, (LPARAM)&lvItem);
	
	//nodeid (hidden column)
	lvItem.iSubItem =COLUMNINDEX_NODEID;
	WCHAR szNodeId[LEN_NODEID] ={0};
	wcscpy_s(szNodeId, LEN_NODEID, pNode->GetNodeId());
	lvItem.pszText = szNodeId;     //nodeId (hidden)
	
	SendMessage(g_hListView, LVM_SETITEM, 0, (LPARAM)&lvItem);
	
	delete pNode;
}


 /************************************************
use the new DomNodeList to refresh the Listview control
this function is only called by refreshListViewThread()
 **********************************************************/
 void refreshLV(MSXML2::IXMLDOMNodeListPtr  nodeListPtr)
 {
 	SendMessage(g_hListView, LVM_DELETEALLITEMS, 0, 0);  //clear all items.
	
	for (long i=0; i<nodeListPtr->length; i++)
	{
		AddNodeToListView(nodeListPtr->Getitem(i));
	}
}


/******************************************************************
call ShareAPI's getNodeList  to get all the node  from Share Service.
then call  refreshLV to show xml node info to Listview control.

called by RefrshListView()

********************************************************************/
void refreshListViewThread(PVOID pvoid)
{
	ThreadParam* pParam = (ThreadParam* )pvoid;
	
	LPCWSTR lpszFolderId = (LPCWSTR) (pParam->m_szNodeId);
	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	try
	{
		g_CurrentFolderDomDocPtr =  g_pShareAPI->GetNodeList(lpszFolderId);
	}
	catch(...)
	{
		//reset the connection and try again
		g_pShareAPI->ResetConnection();
		
		try
		{
			g_CurrentFolderDomDocPtr =  g_pShareAPI->GetNodeList(lpszFolderId);
		}
		catch(...)
		{
			////if still has problem, return!
			SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
			CoUninitialize();
			
			MessageBox(g_hwnd, L"Get file/folder information failed. Please retry ", L"Error",  MB_OK|MB_ICONERROR);
			return;
		}
	}
	
	//Change the global variable  g_szCurrentFolderId!
	wcscpy_s(g_szCurrentFolderId, LEN_NODEID, lpszFolderId);
	
	
	MSXML2::IXMLDOMNodeListPtr  pNodeList;
	pNodeList = g_CurrentFolderDomDocPtr->selectNodes(_bstr_t("response/children/node"));
	//refresh listview control
	refreshLV(pNodeList);	

	SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
	
	CoUninitialize();
}


/******************************************************************
Refresh current listview control.
 Only call this funtion when the folder changes.   
 (Upload files, delete, add folder, rename ... do not need to call this thread!)
********************************************************************/
void RefreshListView(LPCWSTR lpszFolderNodeId)
{
	//create and  show the message dialog  (modalesss)
	HWND hRefreshMsgDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGMSG),  g_hwnd, WaitingMessageDlgProc);
	if (hRefreshMsgDlg != NULL)
	{	
		SetDlgItemText(hRefreshMsgDlg, IDC_MESSAGE, L" Getting files information ...");
		ShowWindow(hRefreshMsgDlg, SW_SHOW);
	}
	
	g_bIsBusy = TRUE;
	SetMenuToolbarStatusbar();
	
	//refresh list view 
	ThreadParam*  pParam = new ThreadParam(lpszFolderNodeId, NULL, hRefreshMsgDlg);
	_beginthread(refreshListViewThread, 0, (PVOID)pParam);
}



/*************************************************************************
private function. only used in listViewCompareProc();
************************************************************************/
void getListViewSubItemText(int itemIndex,  int subItemIndex,  LPWSTR szText, int iSize)
{
	LVFINDINFO  lvFi;
	ZeroMemory(&lvFi, sizeof(LVFINDINFO));
	lvFi.flags = LVFI_PARAM;
	lvFi.lParam = itemIndex;
	int idx = SendMessage(g_hListView, LVM_FINDITEM, -1,  (LPARAM)&lvFi);

	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = idx;
	lvItem.iSubItem = subItemIndex;
	lvItem.pszText = szText;
	lvItem.cchTextMax = iSize;
	SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);
	
	//if the item is a folder, always to be the smallest.  "Folder"
	WCHAR szItemType[128]={0};
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = idx;
	lvItem.iSubItem = COLUMNINDEX_TYPE;
	lvItem.pszText = szItemType;
	lvItem.cchTextMax = 128;
	SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);
	
	if ( (lstrcmp(szItemType, L"Folder")==0 ) &&  (subItemIndex == COLUMNINDEX_NAME ||subItemIndex == COLUMNINDEX_SIZE ||subItemIndex== COLUMNINDEX_TYPE )  )  //is folder!
	{
		WCHAR szItemName[MAX_PATH-4]={0};
		LVITEM lvItem;
		ZeroMemory(&lvItem, sizeof(LVITEM));
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = idx;
		lvItem.iSubItem = COLUMNINDEX_NAME;
		lvItem.pszText = szItemName;
		lvItem.cchTextMax = MAX_PATH;
		SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);

		WCHAR szFolderName[MAX_PATH]={L' ', L' '};  
		wcscat(szFolderName, szItemName);
		wcscpy_s(szText, iSize, szFolderName);
	}
}


/**************************************************************************************
LVM_SORTITEMS 的 procedure
private function, only used in  HandleListViewNotification
***************************************************************************************/
int CALLBACK listViewCompareProc(LPARAM lParam1, LPARAM lParam2,  LPARAM   lParamSort)
{
	int idx1 = (int)lParam1;
	int idx2 =  (int)lParam2;
	int iResult=0;

	switch (lParamSort)
	{
		case COLUMNINDEX_NAME:
		{			
			WCHAR szText1[MAX_PATH] ={0};  
			WCHAR szText2[MAX_PATH] ={0};

			getListViewSubItemText(idx1, COLUMNINDEX_NAME, szText1, MAX_PATH);
			getListViewSubItemText(idx2, COLUMNINDEX_NAME, szText2, MAX_PATH);
			
			iResult = lstrcmp(szText1, szText2);
			break;
		}
		case COLUMNINDEX_SIZE:
		{
			WCHAR szText1[32] ={0};  
			WCHAR szText2[32] ={0};

			getListViewSubItemText(idx1, COLUMNINDEX_SIZE, szText1, 32);
			getListViewSubItemText(idx2, COLUMNINDEX_SIZE, szText2, 32);
			
			WCHAR szText1Aligned[32] ={0};
			WCHAR szText2Aligned[32] ={0};
			
			swprintf(szText1Aligned, L"%030s", szText1);
			swprintf(szText2Aligned, L"%030s", szText2);
			
			iResult = lstrcmp(szText1Aligned, szText2Aligned);
			break;
		}
		
		case COLUMNINDEX_TYPE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			getListViewSubItemText(idx1, COLUMNINDEX_TYPE, szText1, 64);
			getListViewSubItemText(idx2, COLUMNINDEX_TYPE, szText2, 64);
			
			iResult = lstrcmp(szText1, szText2);
			break;
		}
		
		case COLUMNINDEX_CREATEDATE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			getListViewSubItemText(idx1, COLUMNINDEX_CREATEDATE, szText1, 64);
			getListViewSubItemText(idx2, COLUMNINDEX_CREATEDATE, szText2, 64);
			
			COleDateTime cDt1;
			cDt1.ParseDateTime(szText1);
			COleDateTime cDt2;
			cDt2.ParseDateTime(szText2);
			
			COleDateTimeSpan  dts = cDt1-cDt2;
			if (dts.GetStatus() == COleDateTimeSpan::invalid)
			{
				//ASSERT(FALSE);
				iResult =0 ;
			}
			
			iResult = dts.GetTotalSeconds();
			
			break;
		}
		
		case COLUMNINDEX_MODIFYDATE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			getListViewSubItemText(idx1, COLUMNINDEX_MODIFYDATE, szText1, 64);
			getListViewSubItemText(idx2, COLUMNINDEX_MODIFYDATE, szText2, 64);
			
			COleDateTime cDt1;
			cDt1.ParseDateTime(szText1);
			COleDateTime cDt2;
			cDt2.ParseDateTime(szText2);
			
			COleDateTimeSpan  dts = cDt1-cDt2;
			if (dts.GetStatus() == COleDateTimeSpan::invalid)
			{
				//ASSERT(FALSE);
				iResult =0 ;
			}
			
			iResult = dts.GetTotalSeconds();
			
			break;
		}
	
		default:
		{
			iResult=0;
		}
	}
	
	if (!g_bSortAscending)
		iResult = -iResult;
	
	return iResult;
}


/******************************************************************************
the process of the WM_NOTIFY from listview control
including Right click, double click, rename ... etc.
******************************************************************************/
void 	HandleListViewNotification(LPNMHDR pnmh)
{
        static int nSortColumn =-1;  
	static WCHAR szOldFileName[LEN_NAME]= L"";            
      
	SetMenuToolbarStatusbar();  //

	if (pnmh->code == NM_RCLICK)     //right click,  popup context menu
	{
		POINT ptCursor;
		GetCursorPos(&ptCursor);

		HMENU hPopupMenu = GeneratePopupMenu();
		//show the menu
		SetForegroundWindow(g_hwnd);
		TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN, ptCursor.x, ptCursor.y, 0,g_hwnd, NULL);
	}

	if (pnmh->code == NM_DBLCLK )   //double click on the listview, open the file or folder
	{
		if (!g_bIsBusy )
		{
			OpenFileOrFolder();
		}
	}
	
	if(pnmh->code == LVN_BEGINLABELEDIT)           //before renameing the file/folder,  save the oldName!
	{
		HWND hEdit=ListView_GetEditControl(g_hListView);
		GetWindowText(hEdit, szOldFileName, LEN_NAME);
	}

	if (pnmh->code == LVN_ENDLABELEDIT)              
	{
		int iSelect = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
		if (iSelect ==-1)
			return;

		WCHAR szNewName[LEN_NAME]=L"";
		HWND hEdit = ListView_GetEditControl(g_hListView);
		GetWindowText(hEdit,szNewName, LEN_NAME);
		
		//if name is not modified, or the newname is same as old one, return.
		if (lstrcmp(szOldFileName, szNewName) ==0 )
		{
			return;
		}
		
		//get NodeId of the selected item.
		WCHAR szNodeId[LEN_NODEID] = {0};
		
		LVITEM li;
		li.mask = LVIF_TEXT;
		li.iItem = iSelect;
		li.iSubItem = COLUMNINDEX_NODEID;
		li.pszText = szNodeId;
		li.cchTextMax = LEN_NAME; 
		SendMessage(g_hListView, LVM_GETITEM, 0, (LPARAM)&li);
		
		BOOL isSucceed = FALSE;
		try
		{
			isSucceed = g_pShareAPI->RenameNode(szNodeId, szNewName);
		}
		catch (...)  //error
		{
			return;
		}
		
		if (isSucceed) 
		{
			LVITEM lv;
			lv.iSubItem =0;
			lv.pszText =szNewName;
			SendMessage(g_hListView, LVM_SETITEMTEXT, (WPARAM)iSelect, (LPARAM)&lv);
		}
	}
	
	if (pnmh->code == LVN_COLUMNCLICK )       // click head column to sort the data
	{
		LPNMLISTVIEW pnmv =(LPNMLISTVIEW)pnmh;
		
		if(pnmv->iSubItem == nSortColumn)			
		{
		    g_bSortAscending =!g_bSortAscending;
		}
		else
		{
		    g_bSortAscending = TRUE;
		}
		nSortColumn = pnmv->iSubItem;
		
		SendMessage(pnmh->hwndFrom, LVM_SORTITEMS,  (WPARAM)pnmv->iSubItem, (LPARAM)listViewCompareProc);
	}
}
