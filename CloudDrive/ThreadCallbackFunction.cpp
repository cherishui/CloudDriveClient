#include <windows.h>
#include <process.h>
#include <commctrl.h>
#include "ThreadCallbackFunction.h"
#include "ThreadParam.h"
#include "CloudDrive.h"
#include "UIListview.h"
#include "UIMenuToolbarStatusbar.h"
#include "DialogProcedure.h"
#include "ShareAPI.h"
#include "resource.h"


/*****************************************************************************
Create folder thread call back procedure

******************************************************************************/
void CreateFolderThread(PVOID pvoid)
{
	ThreadParam* pParam = (ThreadParam*) pvoid;
	
	LPCWSTR lpszParentFolderId = pParam->m_szNodeId;
	LPCWSTR lpszNewSubFolderName = pParam->m_szName;
	HWND hMsg = pParam->m_hMsg;
	
	MSXML2::IXMLDOMNodePtr  folderNodePtr;
	try
	{
		folderNodePtr = g_pShareAPI->AddFolder(lpszParentFolderId, lpszNewSubFolderName, NULL);
	}
	catch(...)  //failed!
	{
		SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
		MessageBox(g_hwnd, L"Create Folder Failed!", L"Error",  MB_OK|MB_ICONERROR);
		return;
	}
	
	//add to the listview control
	if (wcscmp(lpszParentFolderId, g_szCurrentFolderId) == 0)  
	{
		if (folderNodePtr != NULL)
		{
			AddNodeToListView(folderNodePtr);
		}
	}
	
	//quit the thread
	SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
}


/*************************************************************
get the index of the listview item  whose nodeid equals to the given string

if no item found, -1 returned.
***************************************************************/
int getIndexFromNodeId(LPCWSTR lpszNodeIdPattern)
{
	LONG lCount = SendMessage(g_hListView, LVM_GETITEMCOUNT, 0, 0);
	for (int i=0; i<lCount; i++)
	{
		WCHAR  szNodeId[LEN_NODEID] ={0};
		
		LVITEM lvItem;
		memset(&lvItem, 0, sizeof(LVITEM));
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = COLUMNINDEX_NODEID;
		lvItem.iItem = i;
		lvItem.pszText = szNodeId;
		lvItem.cchTextMax = LEN_NODEID;
		SendMessage(g_hListView, LVM_GETITEMTEXT, i, (LPARAM)&lvItem);
		
		if (wcscmp(lpszNodeIdPattern, szNodeId) == 0)  //find it
		{
			return i;
		}
	}
	return -1;
}

/*****************************************************************
In this thread callback function,  selected files are deleted both on Server side and on Listview control

****************************************************************/
void DeleteFilesThread(PVOID pvoid)
{
	ThreadParamDelete* pParam = (ThreadParamDelete*)pvoid;
	
	WCHAR* lpszNodeId = NULL;
	lpszNodeId= wcstok(pParam->m_szNodeIds, L" ");
	while (lpszNodeId != NULL)
	{
		try
		{
			g_pShareAPI->DeleteNode(lpszNodeId);
		}
		catch(...)
		{
			lpszNodeId = wcstok(NULL, L" ");
			continue;
		}
		
		int idx = getIndexFromNodeId(lpszNodeId);
		ListView_DeleteItem(g_hListView, idx);
		
		lpszNodeId = wcstok(NULL, L" ");
	}
	//quit
	SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
}
//
//	
////Share file thread callback function 
//void ShareFileThread(PVOID pvoid)
//{
//	ThreadParam* pParam = (ThreadParam*) pvoid;
//	
//	LPCWSTR lpszFileNodeId = pParam->m_szNodeId;
//	LPWSTR lpszUserEmails = pParam->m_szName;
//	HWND hMsg = pParam->m_hwndMsg;
//	
//	try
//	{
//		g_pShareAPI->ShareFile(lpszFileNodeId, lpszUserEmails, L"Share File with you", L" You can view or download the file by clicking the link", 2);
//	}
//	catch(std::exception &exp)
//	{
//		//ExitProcessingThreadMSG(pParam);
//		SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
		//MessageBox(g_hwnd, L"Share File Failed", L"Error", MB_OK|MB_ICONERROR);
//		return ;
//	}
//	catch(...)
//	{
//		SendMessage(g_hwnd, UWM_EXITTHREADMSG, 0, (LPARAM)pParam);
//		//ExitProcessingThreadMSG(pParam);
//		MessageBox(g_hwnd, L"Share File Failed", L"Error", MB_OK|MB_ICONERROR);
//		return ;
//	}
//
//	if (hMsg != NULL)
//	{
//		ShowWindow(hMsg, SW_HIDE);
//		DestroyWindow(hMsg);
//	}
//	//set the gloabal var  to indicate FREE
//	g_bIsBusy = FALSE;                
//	if (g_bUserCanceled)
//		g_bUserCanceled = FALSE;
//	//set menu, toolbar, statusbar's state	SetMenuToolbarStatusbar();
//	
//	delete pParam;  //release thread param
//	pParam = NULL;
//	
//	//refreshListview
//	//SendMessage(g_hwnd, WM_COMMAND, ID_VIEW_REFRESH, 0);
//	
//	
//	_endthread(); 	
//	
//	return;		
//}