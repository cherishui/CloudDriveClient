/***************************************************************
Functions related to download files/folders from Adobe share sevice to local computer

Author: Jerry wang (王建华)
All right reserved. 
***************************************************************/
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <shlobj.h>
#include <math.h>
#include "ThreadParam.h"
#include "CloudDrive.h"
#include "UIListview.h"
#include "DialogProcedure.h"
#include "ThreadCallbackFunction.h"
#include "ShareAPI.h"
#include "ShareAPINode.h"
#include "UIMenuToolbarStatusbar.h"
#include "ShareAPIException.h"
#include "WinErrorException.h"
#include "DownloadFile.h"
#include "CommonFunctions.h"
#include "resource.h"
/*****************************************************************
Convert file size string like "49KB" into  int type
Private method, only used in this file
****************************************************************/
int getFileSizeFromSizeStr(LPCWSTR szFileSize)
{
	//get int file size from szFileSize
	WCHAR tmpStr[LEN_SIZE] ={0};
	wcscpy_s(tmpStr, LEN_SIZE,  szFileSize);
	tmpStr[lstrlen(tmpStr)-2] ='\0'; 
	int nSize =  _wtoi(tmpStr);
	
	return nSize *1024;
}


/*************************************************************************
The thread procedure to open/edit/save online file.
downloading file  and open/edit  the file thread, 
if the file is modified, upload the modified file to the online storage

Params:
	pvoid:  the ThreadParam type params from the caller.
*************************************************************************/
 void fileDownloadAndOpenThread(PVOID pvoid)
 {
	ThreadParam* pParam = (ThreadParam*)pvoid;

	LPCWSTR lpszFileNode = pParam->m_szNodeId;
	LPCWSTR lpszFileName = pParam->m_szName;
	HWND hMsg = pParam->m_hMsg;
	
	WCHAR szFilePath[MAX_PATH]={0};
	try  	
	{
		g_pShareAPI->GetFile(lpszFileNode, lpszFileName, szFilePath, hMsg);	
	}		
	catch (std::exception &exp)
	{
		int nDimension = strlen(exp.what()) +1;
		WCHAR* lpszExceptionMsg = new WCHAR[nDimension]();
		MultiByteToWideChar(CP_UTF8, 0, exp.what(), -1, lpszExceptionMsg, nDimension);

		AddErrMsgToDlgListbox( hMsg, lpszExceptionMsg);
	
		SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
		return;
	}
	catch (...)
	{
		AddErrMsgToDlgListbox( hMsg, L"File opne failed: unknown reason");
		SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
		return;
	}
	
	
	if (!g_bUserCanceled)
		ShellExecute(NULL,NULL,szFilePath,NULL,NULL,SW_SHOWNORMAL);  //open the file
		
	SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
	//ExitProcessingThreadPROGRESS(pParam);
}


/*********************************************************************************
Open the file
Start a new thread to download the file from Share,
then choose the right program to open it.

*******************************************************************************/
void openOnlineFile(void)
{
	if (g_bIsBusy ) 
		return; 
		
	//::SetCursor(::LoadCursor(NULL,  IDC_APPSTARTING));
	int iSelect = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	
	WCHAR szFileNodeId[LEN_NODEID]={0};  
	
	LVITEM lvItem;
	memset(&lvItem, 0, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem =COLUMNINDEX_NODEID;
	lvItem.pszText = szFileNodeId;
	lvItem.cchTextMax = LEN_NODEID;
	lvItem.iItem = iSelect;
	SendMessage(g_hListView,LVM_GETITEMTEXT,  iSelect, (LPARAM)&lvItem); 

	WCHAR szFileName[LEN_NAME]={0}; 
	
	memset(&lvItem, 0, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = COLUMNINDEX_NAME;
	lvItem.pszText = szFileName;
	lvItem.cchTextMax = LEN_NAME;
	lvItem.iItem = iSelect;
	SendMessage(g_hListView, LVM_GETITEMTEXT, iSelect, (LPARAM)&lvItem);
	
	WCHAR szFileSize[LEN_SIZE]= {0};
	
	memset(&lvItem, 0, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = COLUMNINDEX_SIZE;
	lvItem.pszText = szFileSize;
	lvItem.cchTextMax = LEN_SIZE;
	lvItem.iItem = iSelect;
	SendMessage(g_hListView, LVM_GETITEMTEXT, iSelect, (LPARAM)&lvItem);
	

	HWND hProgressDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGPROGRESS), g_hwnd,  ProgressDlgProc);
	if (hProgressDlg != NULL)
	{
		SetDlgItemText(hProgressDlg, IDC_STATICNAME,  szFileName);
		SetDlgItemText(hProgressDlg, IDC_STATICSIZE,  szFileSize);
		
		//get int file size from szFileSize
		int nSize = getFileSizeFromSizeStr(szFileSize);
	
		//set the progress bar's step and range
		HWND hProgressBar = GetDlgItem(hProgressDlg, IDC_PROGRESSBAR);
		
		if (nSize <= BUFFERSIZE)
		{
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0,1));
		}
		else
		{
			int nRange = ceil( (float)nSize/BUFFERSIZE);
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nRange));
			//SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nSize/BUFFERSIZE));
		}
		SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);
		
		ShowWindow(hProgressDlg, SW_SHOW);
	}
	
	//disable menu item, toobar button and context menu
	g_bIsBusy = TRUE;
	SetMenuToolbarStatusbar();

	//start the download thread
	ThreadParam*  pParam = new ThreadParam(szFileNodeId, szFileName, hProgressDlg);    
	_beginthread(fileDownloadAndOpenThread, 0, (PVOID)pParam); 
}


/******************************************************************
When double click the file,  download the file from online storage and open it.
when double click the folder,  enter the subfolder , and show all the items in it.
*********************************************************************/
void OpenFileOrFolder()
{
	int iSelect = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (iSelect == -1)
		return;

	//get the selected item's type (Folder or File)
	const int iTypeSize =64;
	WCHAR itemType[iTypeSize] ={0};
	
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = iSelect;
	lvItem.iSubItem =COLUMNINDEX_TYPE;
	lvItem.pszText = itemType;
	lvItem.cchTextMax = iTypeSize;   

	SendMessage(g_hListView, LVM_GETITEM, 0, (LPARAM)&lvItem);

	if (lstrcmp(itemType, L"Folder") == 0)   //folder: enter the folder
	{
		//get NodeId of the selected item.
		WCHAR szNodeId[LEN_NODEID] = {0};
		
		LVITEM li;
		li.mask = LVIF_TEXT;
		li.iItem = iSelect;
		li.iSubItem = COLUMNINDEX_NODEID;
		li.pszText = szNodeId;
		li.cchTextMax = LEN_NAME; 

		SendMessage(g_hListView, LVM_GETITEM, 0, (LPARAM)&li);
		
		//Change the global variable  g_szCurrentFolderId!
		//wcscpy_s(g_szCurrentFolderId, LEN_NODEID, szNodeId);
		
		//refresh the new folder's list view
		RefreshListView(szNodeId);
	}
	else   //file:  download and open the file
	{	
		//download and open the selected file. 
		openOnlineFile();
	}
}


/****************************************************************************
download file , show the downloading progress
 this function is called by saveFolder() and saveFilesThread()
 Param:
	lpszFileNodeId:  current file's node id
	lpszFileName: current file name
	lpszFileSize:  file size string, KB , like " 49KB";
	hProgressDlg: handle of the progress dialogbox
	lpszFilePath: the destination path of the file
******************************************************************************/
static int  iFileCount ;  
static int  iCurrentIdx; 

void saveFile( LPCWSTR lpszFileNodeId, LPCWSTR lpszFileName, LONG lFileSize, HWND hProgressDlg, LPCWSTR lpszFilePath )
{
	iCurrentIdx ++;  //每次调用这个方法，iCurrentIdx都应该+1；
	
	//set the progress message dialog box
	if (hProgressDlg != NULL)
	{
		SetDlgItemInt(hProgressDlg, IDC_STATICCURRENT, iCurrentIdx, FALSE);
		SetDlgItemInt(hProgressDlg, IDC_STATICALL, iFileCount, FALSE);
			
		SetDlgItemText(hProgressDlg, IDC_STATICNAME,  lpszFileName);
		SetDlgItemInt (hProgressDlg, IDC_STATICSIZE,  lFileSize, FALSE);
		
		//set the progress bar's step and range
		HWND hProgressBar = GetDlgItem(hProgressDlg, IDC_PROGRESSBAR);
		if (lFileSize <= BUFFERSIZE)
		{
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0,1));
		}
		else
		{
			int nRange = ceil( (float)lFileSize/BUFFERSIZE);
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nRange));
		}
		SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);
	}
		
	//start downloading!
	try
	{
		g_pShareAPI->SaveFileTo(lpszFileNodeId, lpszFilePath, hProgressDlg);  
	}
	catch (std::exception &exp)  
	{
		int nDimension = strlen(exp.what()) +1;
		WCHAR* lpszExceptionMsg = new WCHAR[nDimension]();
		MultiByteToWideChar(CP_UTF8, 0, exp.what(), -1, lpszExceptionMsg, nDimension);
		
		WCHAR* szErrMsg = new WCHAR[nDimension+MAX_PATH]();
		wcscpy_s(szErrMsg, nDimension+MAX_PATH, lpszFileName);
		wcscat_s(szErrMsg, nDimension+MAX_PATH, L"  Failed!:    " );
		wcscat_s(szErrMsg, nDimension+MAX_PATH, lpszExceptionMsg);
		
		//SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szErrMsg);  
		AddErrMsgToDlgListbox( hProgressDlg, szErrMsg);
		
		delete [] lpszExceptionMsg;
		delete [] szErrMsg;
	}
	catch (...)
	{
		WCHAR* szErrMsg = new WCHAR[MAX_PATH]();
		wcscpy_s(szErrMsg, MAX_PATH, lpszFileName);
		wcscat_s(szErrMsg, MAX_PATH, L"  Failed!:  Unknown reason" );
		
		AddErrMsgToDlgListbox( hProgressDlg, szErrMsg);
		
		delete [] szErrMsg;
	}
}


/*****************************************************************
Save the whole folder
params:
	lpszFolderNodeId: the node id of the folder
	lpszDestPath: the full local path of the folder to be saved to.
	hProgressDlg: the handle of the progress dialog.
*******************************************************************/
void saveFolder(LPCWSTR lpszFolderNodeId, LPCWSTR lpszDestPath, HWND hProgressDlg)
{
	//if the folder does not exist, create it
	BOOL bResult = CreateDirectory(lpszDestPath, NULL);
	
	if (!bResult)
	{
		DWORD dwErr = GetLastError();
		if (dwErr == ERROR_ALREADY_EXISTS)
		{
			AddErrMsgToDlgListbox( hProgressDlg, L"Folder already exists.");
			return; 
		}
		if (dwErr == ERROR_PATH_NOT_FOUND)
		{
			AddErrMsgToDlgListbox( hProgressDlg, L"Folder creation failed.");
			return;
		}
	}
	
	MSXML2::IXMLDOMNodeListPtr pNodeList = g_pShareAPI->GetNodeList(lpszFolderNodeId)->selectNodes(_bstr_t("response/children/node"));
	
	iFileCount--;
	iFileCount += pNodeList->length;
	
	for ( long i=0; i<pNodeList->length; i++)
	{
		if (g_bUserCanceled)
			return;
		
		ShareAPINode* pNode = new ShareAPINode( pNodeList->Getitem(i) );
		
		WCHAR szNodeId[LEN_NODEID]={0}; 
		wcscpy_s(szNodeId, LEN_NODEID, pNode->GetNodeId());
		
		WCHAR szName[MAX_PATH] ={0};
		wcscpy_s(szName, MAX_PATH, pNode->GetName());
		
		if (pNode->IsDirectory() )    //folder
		{
			WCHAR szSubFolderPath[MAX_PATH]={0};
			wcscpy_s(szSubFolderPath, MAX_PATH, lpszDestPath);
			if (szSubFolderPath[wcslen(szSubFolderPath)-2] != L'\\')
				wcscat_s(szSubFolderPath, MAX_PATH, L"\\"); 
			wcscat_s(szSubFolderPath, MAX_PATH, szName);

			saveFolder(szNodeId, szSubFolderPath, hProgressDlg);
		}
		else   //file
		{
			WCHAR szFilePath[MAX_PATH] ={0};
			wcscpy_s(szFilePath, MAX_PATH, lpszDestPath);
			if (szFilePath[wcslen(szFilePath)-2]!= L'\\')
				wcscat_s(szFilePath, MAX_PATH, L"\\");
			wcscat_s(szFilePath, MAX_PATH, szName);
			
			saveFile(szNodeId, szName, pNode->GetFileSize(), hProgressDlg, szFilePath);
		}
	}
}


/*****************************************************************
the thread procedure to save online files/folder to local
for folders,  call saveFolder()

private method, called by SaveFilesTo()

************************************************************************/
void saveFilesThread(PVOID pvoid)
{
	ThreadParamSave*  pParam = (ThreadParamSave*)pvoid;
	
	LONG lSelectedCount = pParam->m_lCount;
	WCHAR* szNodeIds = pParam->m_szNodeIds;
	LPCWSTR szPath = pParam->m_szDestPath;
	HWND hProgressDlg = pParam->m_hMsg;
				
	iFileCount =0;
	iCurrentIdx =0;
	
	iFileCount += lSelectedCount;
	
	WCHAR* lpszNodeId = NULL;
	
	lpszNodeId = wcstok(szNodeIds, L" ");
	while(lpszNodeId != NULL)
	{
		if (g_bUserCanceled)
		{
			break;
		}

		if (wcslen(lpszNodeId)>3)
		{
			WCHAR szXPath[LEN_NODEID+20]={0};
			wcscpy_s(szXPath, LEN_NODEID+20, L"response/children/node[@nodeid=\"");
			wcscat_s(szXPath, LEN_NODEID+20, lpszNodeId);
			wcscat_s(szXPath, LEN_NODEID+20, L"\"]");
			
			MSXML2::IXMLDOMNodePtr  nodePtr =  g_CurrentFolderDomDocPtr->selectSingleNode(_bstr_t(szXPath));
			
			if (nodePtr == NULL)
			{
				g_CurrentFolderDomDocPtr =  g_pShareAPI->GetNodeList(g_szCurrentFolderId);
				nodePtr =  g_CurrentFolderDomDocPtr->selectSingleNode(_bstr_t(szXPath));
			} 
			
			if (nodePtr != NULL)
			{
				ShareAPINode* pNode = new ShareAPINode(nodePtr);
				
				WCHAR szNodeId[LEN_NODEID] ={0};
				wcscpy_s(szNodeId, LEN_NODEID, pNode->GetNodeId());
				
				//get file/folder name
				WCHAR szName[MAX_PATH] ={0};
				wcscpy_s(szName, MAX_PATH, pNode->GetName());
				
				WCHAR szDestPath[MAX_PATH] = {0};
				wcscpy_s(szDestPath, MAX_PATH, szPath);
				if (szDestPath[wcslen(szDestPath)-2]!= L'\\')
					wcscat_s(szDestPath, MAX_PATH, L"\\");
					
				wcscat_s(szDestPath, MAX_PATH, szName); 
				
				if (pNode->IsDirectory())  //folder
				{
					saveFolder(szNodeId, szDestPath, hProgressDlg ); 
				}
				else //file
				{
					saveFile(szNodeId, szName, pNode->GetFileSize(), hProgressDlg, szDestPath);	
				}
			}
		}
		lpszNodeId = wcstok(NULL, L" ");
	}
	
	//quit
	SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
}


void SaveFilesTo()
{
	if (g_bIsBusy)  
		return; 
	
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));

	bi.hwndOwner = g_hwnd;
	bi.lpszTitle=L"Select the folder:";
	bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_USENEWUI;

	PIDLIST_ABSOLUTE  pidl = SHBrowseForFolder(&bi);
	
	if (pidl == NULL)  //canceled
	{	
		return; 
	}
	
	//disable menu item, toobar button and context menu
	g_bIsBusy = TRUE;
	SetMenuToolbarStatusbar();

	WCHAR szPath[MAX_PATH];
	SHGetPathFromIDList(pidl, szPath);
	
	//add all selected items nodeid to a string, each nodeid is seprated by a " " (space char);
	LONG lSelectedCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT, 0, 0);
	WCHAR* szNodeIds = new WCHAR[lSelectedCount*LEN_NODEID]();
	
	//get all selected node ids into  szNodeIds, each seperated with a space char.
	GetSelectedNodeIds(szNodeIds); 
		
	HWND hProgressDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGPROGRESS), g_hwnd,  ProgressDlgProc);
	if (hProgressDlg != NULL)
	{
		ShowWindow(hProgressDlg, SW_SHOW);
	}
	
	//start the download and save thread
	ThreadParamSave*  pParam = new ThreadParamSave(lSelectedCount, szNodeIds, szPath, hProgressDlg);
	_beginthread(saveFilesThread, 0, (PVOID)pParam);
	
	delete [] szNodeIds;
}


