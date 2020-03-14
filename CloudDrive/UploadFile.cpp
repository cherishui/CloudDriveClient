#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <math.h>
#include "CloudDrive.h"
#include "UploadFile.h"
#include "ThreadParam.h"
#include "UIListView.h"
#include "UIMenuToolbarStatusbar.h"
#include "ThreadCallbackFunction.h"
#include "DialogProcedure.h"
#include "CommonFunctions.h"
#include "ShareAPI.h"
#include "resource.h"

/**************************************************************************************
upload one file, set the progress dialog and listview control
Param: 
	lpszFileFullPathName: the full path name of the file
	lpszFolderNodeId: the node id of the folder in which the file is added. 
	iCurrent:  the current files index
	hProgress: the handle of the progress dialog
*************************************************************************************/
MSXML2::IXMLDOMNodePtr UploadOneFile(LPWSTR lpszFileFullPathName, LPCWSTR lpszFolderNodeId, int iCurrent, HWND hProgressDlg)
{
	if (hProgressDlg != NULL)
	{
		
		SetDlgItemInt(hProgressDlg, IDC_STATICCURRENT, iCurrent, FALSE);
		SetDlgItemText(hProgressDlg, IDC_STATICNAME,  lpszFileFullPathName);
		
		//get filesize
		HANDLE hFile = CreateFile(lpszFileFullPathName, GENERIC_READ, FILE_SHARE_READ, 0,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		
		SetDlgItemInt(hProgressDlg, IDC_STATICSIZE,  dwFileSize, FALSE);
		
		//set the progress bar's step and range
		HWND hProgressBar = GetDlgItem(hProgressDlg, IDC_PROGRESSBAR);
		if (dwFileSize <= BUFFERSIZE)
		{
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0,1));
		}
		else
		{
			int nRange = ceil( (float)dwFileSize/BUFFERSIZE);
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nRange));
		}
		SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);
	}

	//do upload
	MSXML2::IXMLDOMNodePtr pUploadedNode = NULL;
	try
	{
		pUploadedNode = g_pShareAPI->UploadFile(lpszFileFullPathName, lpszFolderNodeId, NULL, hProgressDlg);
	}
	catch(std::exception &exp)
	{
		//add error message to the listbox , Format like:  abc.txt failed:    connection timeout.....
		int nDimension = strlen(exp.what()) +1;
		WCHAR* lpszExceptionMsg = new WCHAR[nDimension]();
		MultiByteToWideChar(CP_UTF8, 0, exp.what(), -1, lpszExceptionMsg, nDimension);
		
		WCHAR* szErrMsg = new WCHAR[nDimension+MAX_PATH]();
		wcscpy_s(szErrMsg, nDimension+MAX_PATH, lpszFileFullPathName);
		wcscat_s(szErrMsg, nDimension+MAX_PATH, L"  Failed!:    " );
		wcscat_s(szErrMsg, nDimension+MAX_PATH, lpszExceptionMsg);

		AddErrMsgToDlgListbox( hProgressDlg, szErrMsg);		
			
		delete [] lpszExceptionMsg;
		delete [] szErrMsg;
		
		Sleep(1000);  
		
		return NULL;
	}
	catch(...)
	{
		WCHAR* szErrMsg = new WCHAR[MAX_PATH]();
		wcscpy_s(szErrMsg, MAX_PATH, lpszFileFullPathName);
		wcscat_s(szErrMsg, MAX_PATH, L"  Failed!:  Unknown reason" );
		
		AddErrMsgToDlgListbox( hProgressDlg, szErrMsg);
		
		delete [] szErrMsg;
		
		return NULL;
	}
	
	return pUploadedNode;	
}


/********************************************************************************
upload file thread callback procedure.
When user click the "Upload" button or the menuitem , this is called. (not including drag&drop)
only called by UploadFiles();
**********************************************************************************/
void uploadFilesThread(PVOID pvoid)
{
	WCHAR szPath[MAX_PATH];

	ThreadParamUpload* pParam = (ThreadParamUpload*) pvoid;
	
	LPWSTR szFileNames = pParam->m_szFileNames;
	DWORD nFileOffset = pParam->m_nOffset;
	HWND hProgressDlg = pParam->m_hMsg;
	
	lstrcpyn(szPath, szFileNames,nFileOffset); 
	szPath[nFileOffset] = '\0';

	if (szPath[lstrlen(szPath)-1] != '\\')          
		lstrcat(szPath, L"\\");
		
	WCHAR* q;
	q= szFileNames + nFileOffset;
	int iCount =0;
	while (*q)
	{	
		iCount++;
		q += lstrlen(q)+1; 
	}
	
	if (hProgressDlg != NULL)
	{
		//WCHAR szCount[8] ={0};
		//_itow(iCount, szCount, 10);
		SetDlgItemInt(hProgressDlg, IDC_STATICALL, iCount, FALSE);		
	}


	WCHAR* p;
	p=szFileNames +nFileOffset;  
	
	int i =0;
	while (*p)
	{
		if (g_bUserCanceled)
			break;;

		i++;
		
		WCHAR lpszFileFullPathName[MAX_PATH] ={0};
		
		lstrcat(lpszFileFullPathName, szPath);
		lstrcat(lpszFileFullPathName, p);  
		
		//upload one file and show the progress
		MSXML2::IXMLDOMNodePtr pUploadedNode = UploadOneFile(lpszFileFullPathName, g_szCurrentFolderId, i, hProgressDlg);
		
		//∞—pUploadedNodeº”»Îlistview
		if (pUploadedNode != NULL)
		{
			AddNodeToListView(pUploadedNode);			
		}		
		
		p += lstrlen(p) +1;  //next file
	}	
	
	//quite the thread
	SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
	//ExitProcessingThreadPROGRESS(pParam);
}


/***************************************************************
 Upload a file or multiple files
 Open the file selection dialogbox to let the user select the files to be uploaded.
 then call ShareAPI to upload all the files and refresh the listview.
 Drag&Drop is in another function.
 **************************************************************/
void UploadFiles()
{
	WCHAR* szFileNames=  new WCHAR[MAX_PATH+4096]();

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hwnd;
	ofn.lpstrFilter = L"All File(*.*)\0*.*\0";
	ofn.lpstrFile = szFileNames;
	ofn.nMaxFile = MAX_PATH+4096;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT;

	if (GetOpenFileName(&ofn))
	{
		g_bIsBusy = TRUE;
		SetMenuToolbarStatusbar();
		
		//progress dialog
		HWND hProgressDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGPROGRESS), g_hwnd,  ProgressDlgProc);
		if (hProgressDlg != NULL)
		{
			ShowWindow(hProgressDlg, SW_SHOW);
		}
		
		ThreadParamUpload* pParam = new ThreadParamUpload(szFileNames, ofn.nFileOffset, hProgressDlg);
		_beginthread(uploadFilesThread, 0, (PVOID)pParam);
	}
}
