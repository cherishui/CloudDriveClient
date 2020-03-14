/********************************************************************
this file includes function releated to Drag and Drop,
Till now , only Drop is supported.  So all functions now are about uploading.

Author: jerry wang (王建华)
All rights reserved.
**********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <process.h>
#include <math.h>
#include "CloudDrive.h"
#include "UIListview.h"
#include "threadParam.h"
#include "UIMenuToolbarStatusbar.h"
#include "DialogProcedure.h"
#include "resource.h"
#include "helperfunctions.h"
#include "threadcallbackfunction.h"
#include "DataObject.h"
#include "DragAndDrop.h"
#include "CommonFunctions.h"
#include "UploadFile.h"
#include "ShareAPI.h"
#include "ShareAPINode.h"

static int iCurrent = 0; 

int countFiles(LPCWSTR lpszFolderPath)
{
	int iCount =0;
	WIN32_FIND_DATA  fileInfo;
	WCHAR strPattern[MAX_PATH]={0};
	
	wcscpy_s(strPattern, MAX_PATH, lpszFolderPath);
	wcscat_s(strPattern, MAX_PATH, L"\\*.*");
	
	HANDLE  hFile = FindFirstFile(strPattern, &fileInfo);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if( (_wcsicmp(fileInfo.cFileName,L".")!=0)  && (_wcsicmp (fileInfo.cFileName,L"..")!=0) &&(_wcsicmp(fileInfo.cFileName,L"") !=0) )
			{
				if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					WCHAR szSubPath[MAX_PATH]={0};
					wcscpy_s(szSubPath, MAX_PATH, lpszFolderPath);
					wcscat_s(szSubPath, MAX_PATH, L"\\");
					wcscat_s(szSubPath, MAX_PATH, fileInfo.cFileName);
					
					iCount +=  countFiles(szSubPath);
				}
				else
				{
					if (! (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) )
					{
						++iCount;
					}
				}
			}
		} while (FindNextFile(hFile, &fileInfo)==TRUE); 
		
		FindClose(hFile);
	}
	return  iCount;
}

VOID calculateFileNumThread(PVOID pvoid)
{
	ThreadParamCalculate*  pParam = (ThreadParamCalculate*) pvoid;
	HDROP hDrop = pParam->m_hDrop;
	HWND hProgress = pParam->m_hMsg;
	
	UINT uNumFiles = DragQueryFile( hDrop, -1, NULL, 0);   //retrieve the names of dropped files that result from a successful drag&drop opeeration
	WCHAR szFileName[MAX_PATH];
	
	UINT uFileCount =0;
	
	for (UINT i=0; i<uNumFiles; i++)
	{
		if (DragQueryFile(hDrop, i, szFileName, MAX_PATH) >0) 
		{
			if (GetFileAttributes(szFileName) & FILE_ATTRIBUTE_DIRECTORY)  //folder
			{
				uFileCount += countFiles( szFileName );
				SetDlgItemInt(hProgress, IDC_STATICALL,uFileCount, FALSE);
			}
			else  //file
			{
				if ( ! (GetFileAttributes(szFileName) & FILE_ATTRIBUTE_TEMPORARY))
				{
					uFileCount ++;
					//set the Progress Dialog 
					SetDlgItemInt(hProgress, IDC_STATICALL,uFileCount, FALSE);
				}
			}	
		}
	}
	_endthread();
}

/************************************************************************************
Param:
	szFolderPath: the full path name of the local folder
	lpszCurrentFolderId:  the node id of the current folder that the newly folder are going to be added to. that is , the added folder's parent folder's node id.
	hProgressDlg: the handle of progress dialog.				
************************************************************************************/
MSXML2::IXMLDOMNodePtr  uploadFolder(LPCWSTR szFolderPath, LPCWSTR lpszCurrentFolderId, HWND hProgressDlg)
{
	MSXML2::IXMLDOMNodePtr  pFolderNode = NULL;
	
	//get the foldername from szFolderPath
	WCHAR szFolderName[64]={0};
	HelperFunctions::GetFileNameFromFilePath(szFolderPath, szFolderName, 64);
	
	try
	{
		pFolderNode = g_pShareAPI->AddFolder(lpszCurrentFolderId, szFolderName, NULL);  
	}
	catch (...)
	{
		WCHAR szErr[MAX_PATH] ={0};
		wcscpy_s(szErr, MAX_PATH, szFolderPath);
		wcscat_s(szErr, MAX_PATH, L"  Folder create  failed");
		
		AddErrMsgToDlgListbox(hProgressDlg, szErr);  //in Commonfunctions.h
		
		return NULL;
	}
	
	//if the folder is the direct child ,  add it to the listview 
	if (pFolderNode != NULL)
	{
		WCHAR  szFolderNodeId[LEN_NODEID] ={0};
		wcscpy_s( szFolderNodeId,  LEN_NODEID, ((ShareAPINode)pFolderNode).GetNodeId());
		
		GetUplevelId(szFolderNodeId); //get the parent folder id.
		
		if (  wcscmp(szFolderNodeId, g_szCurrentFolderId) ==0)
		{
			AddNodeToListView(pFolderNode);
		}
	}
	
	WIN32_FIND_DATA fileInfo;
	WCHAR szPattern[MAX_PATH]={0};

	wcscpy_s(szPattern, MAX_PATH, szFolderPath);
	wcscat_s(szPattern, MAX_PATH, L"\\*.*");
	
	HANDLE  hFile = FindFirstFile(szPattern, &fileInfo);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (g_bUserCanceled)
				break;
		
			if( (_wcsicmp(fileInfo.cFileName,L".")!=0)  && (_wcsicmp (fileInfo.cFileName,L"..")!=0) &&(_wcsicmp(fileInfo.cFileName,L"") !=0) )
			{
				WCHAR szFilePathName[MAX_PATH]={0};
				wcscpy_s(szFilePathName, MAX_PATH, szFolderPath);
				wcscat_s(szFilePathName, MAX_PATH, L"\\");
				wcscat_s(szFilePathName, MAX_PATH, fileInfo.cFileName);
				
				ShareAPINode * pNode = new ShareAPINode(pFolderNode);
				WCHAR szNodeId[LEN_NODEID]={0};
				wcscpy_s(szNodeId, LEN_NODEID, pNode->GetNodeId());
				
				if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					uploadFolder(szFilePathName,szNodeId,  hProgressDlg);			
				}
				else
				{  
					UploadOneFile(szFilePathName, szNodeId , ++iCurrent, hProgressDlg);
				}
				delete pNode;	
			}
		} while (FindNextFile(hFile, &fileInfo)==TRUE); 
		
		FindClose(hFile);
	}	
	return pFolderNode;
}

/***************************************************************************************************
Thread callback function
//paste  to the listview control from the clipboard
or drop files to the listview control
This thread is started in PasteOrDropFiles()

********************************************************************************************************/
VOID pasteFilesThread(PVOID pvoid)
{
	OleInitialize(0);	
	
	iCurrent =0;

	ThreadParamDrop* pParam = (ThreadParamDrop*)pvoid;
	
	LPCWSTR lpszCurrentFolderId = pParam->m_szFolderId;
	HWND hProgressDlg = pParam->m_hMsg;
	LPSTREAM pStream = pParam->m_pStream;
	
	IDataObject* pDataObject = NULL;  //要从m_pStream中unmarshal出来
	CoGetInterfaceAndReleaseStream(pStream, IID_IDataObject, (LPVOID*)&pDataObject);
	
	
	WCHAR szFileName[MAX_PATH];
	FORMATETC fmtetc = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stgmed;

	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
		{
			HDROP hDrop = (HDROP)GlobalLock(stgmed.hGlobal);  //locks a blobal memory and returns a poingtr to the first byte of the memory block
			
			ThreadParamCalculate  threadParam(hDrop, hProgressDlg);
			HANDLE  hCalThread = (HANDLE)_beginthread(calculateFileNumThread, 0, (PVOID)&threadParam); 
			
			UINT uNumFiles = DragQueryFile( hDrop, -1, NULL, 0);   //retrieve the names of dropped files that result from a successful drag&drop opeeration
			
			MSXML2::IXMLDOMNodePtr  pNode = NULL;
			for (UINT i=0; i<uNumFiles; i++)
			{
				if (g_bUserCanceled)
					break;
				
				if (DragQueryFile(hDrop, i, szFileName, MAX_PATH) >0) 
				{
					if (GetFileAttributes(szFileName) & FILE_ATTRIBUTE_DIRECTORY)  //folder
					{
						pNode = uploadFolder(szFileName, lpszCurrentFolderId, hProgressDlg);  
					}
					else  //file
					{
						pNode  = UploadOneFile(szFileName, lpszCurrentFolderId, ++iCurrent,  hProgressDlg);
						if (pNode != NULL)
						{
							AddNodeToListView(pNode);
						}
					}						
					
					//if (pNode != NULL)
					//{
					//	AddNodeToListView(pNode);
					//}
				}
			}
			
			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);
		}
	}

	//quit 
	OleUninitialize();  
	
	SendMessage(g_hwnd, UWM_EXITTHREADPROGRESS, 0, (LPARAM)pParam);
	//ExitProcessingThreadPROGRESS(pParam);
};



/***********************************************************************************
When paste file to the app window, or Drop files on the app window, this method is called
************************************************************************************/
VOID PasteOrDropFiles(IDataObject* pDataObject)
{
	if (g_bIsBusy)
		return;
		
	g_bIsBusy= TRUE;
	SetMenuToolbarStatusbar();
	
	//progress dialog
	HWND hProgressDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGPROGRESS), g_hwnd,  ProgressDlgProc);
	if (hProgressDlg != NULL)
	{
		ShowWindow(hProgressDlg, SW_SHOW);
	}
	
	ThreadParamDrop* pParam = new ThreadParamDrop(g_szCurrentFolderId, pDataObject, hProgressDlg);
	_beginthread(pasteFilesThread, 0, (PVOID)pParam);
}	
	
//VOID PasteFiles()
//{
//	IDataObject* pDataObject;
//	OleGetClipboard(&pDataObject);
//	PasteOrDropFiles(pDataObject);
//}
//
//VOID DropFiles(IDataObject* pDataObject)
//{
//	PasteOrDropFiles(FALSE);
//}

/*

HGLOBAL GetSelectedInfo()
{

        //get all the selected items
	UINT uFileCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT,  0, 0);

	UINT uBufferSize = sizeof(FILEGROUPDESCRIPTOR)+(uFileCount-1)*sizeof(FILEDESCRIPTOR);

	//GHND: GMEM_MOVEABLE&&GMEM_ZEROINIT, GMEM_SHARE:
	HGLOBAL hFileDescriptor = GlobalAlloc(GPTR, uBufferSize);
	if (NULL == hFileDescriptor)
	return NULL;

	FILEGROUPDESCRIPTOR* pGroupDescriptor = (FILEGROUPDESCRIPTOR*)GlobalLock(hFileDescriptor);  
	if (NULL == pGroupDescriptor)
	{
		GlobalFree(hFileDescriptor);
		return NULL;
	}
	
	FILEDESCRIPTOR*  pFileDescriptorArray = (FILEDESCRIPTOR*) ( (LPBYTE)pGroupDescriptor +sizeof (UINT) ); //因为FILEGROUPDESCRIPTOR的第一个元素是UINT cItem,所以跳过			
	pGroupDescriptor->cItems = uFileCount;
	
	
	int iSelected = -1;
	for (int i=0; i<uFileCount; i++)
	{
		iSelected = SendMessage(g_hListView, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);

		 if (iSelected >=0)
		 { 
			//get the nodeid
			WCHAR szNodeId[LEN_NODEID]={0};
			LVITEM lvItem;
			ZeroMemory(&lvItem, sizeof(LVITEM));
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem =COLUMNINDEX_NODEID;
			lvItem.iItem = iSelected;
			lvItem.pszText = szNodeId;
			lvItem.cchTextMax = LEN_NODEID;
			SendMessage(g_hListView, LVM_GETITEMTEXT, iSelected, (LPARAM)&lvItem);
	
			//Do different process according type : file or folder?
			WCHAR szType[LEN_TYPE]={0};
			ZeroMemory(&lvItem, sizeof(LVITEM));
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem =COLUMNINDEX_TYPE;
			lvItem.iItem = iSelected;
			lvItem.pszText = szType;
			lvItem.cchTextMax = LEN_TYPE;
			SendMessage(g_hListView, LVM_GETITEMTEXT, iSelected, (LPARAM)&lvItem);
			
			//get file/folder name
			//WCHAR szFileName[MAX_PATH]={0};
			//ZeroMemory(&lvItem, sizeof(LVITEM));
			//lvItem.mask = LVIF_TEXT;
			//lvItem.iSubItem =COLUMNINDEX_NAME;
			//lvItem.iItem = iSelected;
			//lvItem.pszText = szFileName;
			//lvItem.cchTextMax = MAX_PATH;
			//SendMessage(hListView, LVM_GETITEMTEXT, iSelected, (LPARAM)&lvItem);


			ZeroMemory(&pFileDescriptorArray[i], sizeof(FILEDESCRIPTOR));
			
			lstrcpy(pFileDescriptorArray[i].cFileName ,szNodeId);  
			pFileDescriptorArray[i].dwFlags = FD_ATTRIBUTES;
			
			if (lstrcmp(szType, L"Folder")==0)
				pFileDescriptorArray[i].dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			else
				pFileDescriptorArray[i].dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		}

	}
	GlobalUnlock(hFileDescriptor);
		
	
	return hFileDescriptor;		
 //       //get all the selected items
	//LONG lSelectedCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT,  0, 0);
 //       
	//UINT uBufferSize =0;
	//int iSelected = -1;
	//for (int i=0; i<lSelectedCount; i++)
	//{
	//     iSelected = SendMessage(g_hListView, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);

	//     if (iSelected >=0)
	//     { 
	//	 //get the NodeId
	//	 WCHAR szNodeId[LEN_NODEID]={0};
	//	LVITEM lvItem;
	//	ZeroMemory(&lvItem, sizeof(LVITEM));
	//	lvItem.mask = LVIF_TEXT;
	//	lvItem.iSubItem =COLUMNINDEX_NODEID;
	//	lvItem.iItem = iSelected;
	//	lvItem.pszText = szNodeId;
	//	lvItem.cchTextMax = LEN_NODEID;
	//	SendMessage(g_hListView, LVM_GETITEMTEXT, iSelected, (LPARAM)&lvItem);
 //   	    
	//	uBufferSize += lstrlen(szNodeId);
	//     }
	//}

	////add 1 extra for the final null char, and the size of the DROPFILES struct.
	//uBufferSize = sizeof(DROPFILES) +sizeof(WCHAR)*(uBufferSize+1);

	////alloocate memory from the heap for the DROPFILES struct.
	////GHND: GMEM_MOVEABLE&&GMEM_ZEROINIT, GMEM_SHARE:
	//HGLOBAL hgDrop = GlobalAlloc(GPTR, uBufferSize);
 //       
	//if (NULL == hgDrop)
	//    return NULL;

	//DROPFILES* pDrop = (DROPFILES*)GlobalLock(hgDrop);

	//if (NULL == pDrop)
	//{
	//    GlobalFree(hgDrop);
	//    return NULL;
	//}

	//pDrop->pFiles = sizeof(DROPFILES);
	//pDrop->fWide = TRUE;

	////copy all the filenames into memory after the end of the DROPFILES
 //       
	//WCHAR* pszBuff = (WCHAR*)(LPBYTE(pDrop)+sizeof(DROPFILES));  //移动指针

	//int iSel = -1;
	//for (int i=0; i<lSelectedCount; i++)
	//{
	//     iSel = SendMessage(g_hListView, LVM_GETNEXTITEM, iSel, LVNI_SELECTED);

	//     if (iSel >=0)
	//     { 
	//	 //get the fileName
	//	 WCHAR szNodeId[LEN_NODEID]={0};
	//	LVITEM lvItem;
	//	ZeroMemory(&lvItem, sizeof(LVITEM));
	//	lvItem.mask = LVIF_TEXT;
	//	lvItem.iSubItem =COLUMNINDEX_NODEID;
	//	lvItem.iItem = iSel;
	//	lvItem.pszText = szNodeId;
	//	lvItem.cchTextMax = LEN_NODEID;
	//	SendMessage(g_hListView, LVM_GETITEMTEXT, iSel, (LPARAM)&lvItem);
 //   	    
	//	lstrcpy(pszBuff, szNodeId);

	//	pszBuff+= lstrlen(szNodeId); //move the pointer to the end.
	//     }
	//}

	//GlobalUnlock(hgDrop);

	//return hgDrop;
}


//
//VOID CopyFiles()
//{
//	HGLOBAL hgDrop = GetSelectedInfo();
//	
//	FORMATETC fmtetc = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
//	STGMEDIUM stgmed = {TYMED_HGLOBAL, {0}, 0};
//	
//	IDataObject* pDataObject = new CDataObject(&fmtetc, &stgmed,1);   
//	
//	HRESULT hr = OleSetClipboard(pDataObject);
//}

*/