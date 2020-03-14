 /***********************************************************************************
 This is the main process file .
 Mainly foucus on Message handling.
 
 Author:  Jerry Wang (王建华)
 All copyright reserved.
 **********************************************************************************/
 #define OEMRESOURCE
#include <windows.h>
#include <uxtheme.h>
#include <process.h>
#include <commctrl.h>
#include <shlobj.h>
#include "resource.h"
#include "targetver.h"
#include "ShareAPI.h"
#include "CloudDrive.h"
#include "DropTarget.h"
#include "DropSource.h"
#include "DataObject.h"
#include "ThreadParam.h"
#include "UIListview.h"
#include "UIMenuToolbarStatusbar.h"
#include "UploadFile.h"
#include "DownloadFile.h"
#include "DialogProcedure.h"
#include "DragAndDrop.h"
#include "ThreadCallbackFunction.h"
#include "CommonFunctions.h"
#include "UISystemTray.h"

//=====================Global varialbles================================
HWND g_hListView = NULL;			//ListView Control	
HWND g_hTool = NULL;				//Toolbar Control
HWND g_hStatusBar = NULL;			//Statusbar control
HWND g_hwnd = NULL;				// the main windows
HINSTANCE g_hInstance = NULL;		// the global modual instance.

ShareAPI *g_pShareAPI = NULL;				//ShareAPI object pointer

WCHAR g_szCurrentFolderId[LEN_NODEID] ={0};		// global  current folder Id;
MSXML2::IXMLDOMDocumentPtr g_CurrentFolderDomDocPtr = NULL; // global current folder's dom doc object.
WCHAR g_szNewFolderName[LEN_NAME] ={0};		// the newly added foldername 

BOOL g_bSortAscending = TRUE;				// the first sort is ascending when clicking the column
BOOL g_bIsBusy = FALSE;					// if the uploading/download/other  function is in processing
BOOL g_bUserCanceled = FALSE;				// if the user click the Canceled Button.



/******************************************************************
Get Parent folder's node id.
Param:
	szFolderId: [in][out]  the node id of the folder
	after excuting this funciton, the szFolderId will be the parent's node Id;	
note: the szFolderId must be a WCHAR array.
in this app. we use this function on  g_szCurrentFolderId
*********************************************************************/
void GetUplevelId(LPWSTR szFolderId)
{
	if (szFolderId == NULL)
		return;
	
	int iSize = lstrlen(szFolderId);
	if (iSize ==0)
		return;
	
	WCHAR* p = szFolderId+iSize;
	for (int i=0; i<iSize; i++)
	{
		--p;
		if (*p == L':')
			break;
	}
	*p=L'\0';
}


/***********************************************************************
Main Window procedure
*******************************************************************/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static IDropTarget* pDropTarget;  //till now, only drop is supported, drag is not yet.
	
	switch(msg)
	{
	case WM_CREATE:
		{
			g_hwnd = hwnd;  
			
			//set the program locale to the system default
			_wsetlocale( LC_ALL, L"" );  

			//initialize some global variables
			g_hInstance = GetModuleHandle(NULL);
			
			//create controls: Toolbar, Statusbar, ListView...
			CreateToolbar();
			CreateStatusbar();
			CreateListView();
			SetWindowTheme(g_hListView, L"Explorer",  NULL);   //note: must need a manifest file to specify that CommCtrol Version 6  is used.   a manifest file /or a linker directive  is a must!!
			InitListViewColumns();
			InitListViewImageLists();
			
			//Drop target support
			pDropTarget = new CDropTarget(g_hListView); 
			CoLockObjectExternal(pDropTarget, TRUE, FALSE);
			RegisterDragDrop(g_hListView, pDropTarget);			
			
			//initialize listview control
			RefreshListView(g_szCurrentFolderId);
		}
		break;
		
	case WM_SIZE:
		{
			//set the size of Toobar, Status, and ListView control
			RECT rcWindow;
			GetClientRect(hwnd, &rcWindow);

			RECT rcTool;
			SendMessage(g_hTool, TB_AUTOSIZE, 0,0);
			GetClientRect(g_hTool, &rcTool);

			int iTotalWidth = LOWORD(lParam);

			int statusWidths[3] ={0};   //   4parts in the toolbar
			statusWidths[2] = iTotalWidth;
			statusWidths[1] = iTotalWidth -100;
			statusWidths[0] = iTotalWidth -400;       
				
			SendMessage(g_hStatusBar, SB_SETPARTS, 3, (LPARAM)statusWidths);
			SendMessage(g_hStatusBar,WM_SIZE,0, 0);
		
			RECT rcStatus;
			GetClientRect(g_hStatusBar, &rcStatus);
			//the listview 的height = rcWindow.bottom - rcTool.bottom - rcStatus.bottom
			SetWindowPos(g_hListView, NULL, 0, rcTool.bottom, rcWindow.right, rcWindow.bottom-rcTool.bottom-rcStatus.bottom, SWP_NOZORDER);   //  repostion the listview if the size of the parent window changed.
		}
		break;

	case WM_NOTIFY:
		{
			switch(LOWORD(wParam))
			{
				case ID_LISTVIEW:   //handle listview notification
				{
					HandleListViewNotification((LPNMHDR)lParam);
				}
				break;
			}
			
			//handle toolbar notification.  ( because wParam is not IDC_TOOLBAR, so can not inlcueded in CASE IDC_TOOLBAR!! 		
			return  HandleToolbarNotification((LPNMHDR)lParam);
		}
		break;
		
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_FILE_OPENFILEORFOLDER:
				{
					OpenFileOrFolder();
				}
				break;
				
			case ID_FILE_SAVETO:
				{
					SaveFilesTo();
				}
				break;
			case ID_FILE_UPLOAD:
				{
					UploadFiles();
				}
				break;
				
			case ID_VIEW_REFRESH:  
				{
					RefreshListView(g_szCurrentFolderId);
				}
				break;
			case  ID_FILE_CREATEFOLDER:  // create subfolder
				{
					ZeroMemory( g_szNewFolderName, sizeof (g_szNewFolderName));  
					
					if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGFOLDERNAME), hwnd, (DLGPROC)FolderNameDlgProc) == IDOK)
					{
						if ( lstrlen(g_szNewFolderName) >0) 
						{
				
							//disable menu item, toobar button and context menu
							g_bIsBusy = TRUE;
							SetMenuToolbarStatusbar();
							//create and  show the message dialog  (modalesss)
							HWND hMsgDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGMSG),  hwnd, WaitingMessageDlgProc);
							
							if (hMsgDlg != NULL)
							{
								//set up the message on the dialog 
								SetDlgItemText(hMsgDlg, IDC_MESSAGE, L"   Creating a new folder ......"); 
								ShowWindow(hMsgDlg, SW_SHOW);
							}
							
							ThreadParam*  pParam = new ThreadParam(g_szCurrentFolderId, g_szNewFolderName, hMsgDlg);
							_beginthread(CreateFolderThread, 0, (PVOID) pParam);
						}
					}
				}
				break;
			case ID_FILE_RENAME:
				{
					int iSelect = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
					if (iSelect != -1)
					{
						SendMessage(g_hListView, LVM_EDITLABEL, (WPARAM)iSelect, (LPARAM)0);
					}
				}
				break;
			case ID_FILE_DELETE:
				{
					LONG lSelectedCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT,  0, 0);
					if (lSelectedCount >0)
					{
						//disable menu item, toobar button and context menu
						g_bIsBusy = TRUE;
						SetMenuToolbarStatusbar();

						WCHAR* szNodeIds = new WCHAR[lSelectedCount*LEN_NODEID]();
						
						GetSelectedNodeIds(szNodeIds);						
						
						HWND hMsg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGMSG), g_hwnd, WaitingMessageDlgProc);
						if (hMsg != NULL)
						{
							SetDlgItemText(hMsg, IDC_MESSAGE, L"   Deleting items ......"); 
							ShowWindow(hMsg, SW_SHOW);
						}					
						
						ThreadParamDelete* pParam = new ThreadParamDelete(szNodeIds, hMsg);
						
						_beginthread(DeleteFilesThread, 0 , (PVOID)pParam);  
						
						delete [] szNodeIds;
					}
				}
				break;
			
			//case ID_SHARE_SHARETOOTHERS:
			//	{							
			//		int iSelect = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			//		if (iSelect != -1)
			//		{
			//			WCHAR szNodeId[LEN_NODEID] ={0};

			//			LVITEM lvItem;
			//			memset(&lvItem, 0, sizeof(LVITEM));
			//			lvItem.mask = LVIF_TEXT;
			//			lvItem.iSubItem =COLUMNINDEX_NODEID;
			//			lvItem.pszText = szNodeId;
			//			lvItem.cchTextMax = LEN_NODEID;
			//			lvItem.iItem = iSelect;
			//			SendMessage(g_hListView,LVM_GETITEMTEXT,  iSelect, (LPARAM)&lvItem); 
			//			
			//			//input the mails in the dialogbox
			//			if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGSHARE), hwnd, (DLGPROC)ShareDlgProc) == IDOK)
			//			{
			//				if ( (lstrlen(g_szShareEmails)>0) && (wcschr(g_szShareEmails, L'@' )!=NULL))
			//				{
			//					//create and  show the message dialog  (modalesss)
			//					HWND hMsgDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGMSG),  hwnd, WaitingMessageDlgProc);
			//					
			//					if (hMsgDlg != NULL)
			//					{
			//						//set up the message on the dialog 
			//						SetDlgItemText(hMsgDlg, IDC_MESSAGE, L"   Sharing the file ......"); 
			//						ShowWindow(hMsgDlg, SW_SHOW);
			//					}
			//					
			//					ThreadParam *pParam = new ThreadParam(szNodeId,g_szShareEmails, hMsgDlg);						
			//							
			//					_beginthread(ShareFileThread, 0, (PVOID)pParam);
			//					
			//				}
			//				else
			//				{
			//					MessageBox(g_hwnd, L"Invalid email address", L"error", MB_OK|MB_ICONERROR);
			//				}
			//			
			//			}	

			//		//let user input emails to share with
			//			
			//		}
			//	}
			//	break;
			////case ID_EDIT_COPYFILE:
			//	{
			//		CopyFiles();
			//	}
			//	break;
			case ID_EDIT_PASTEFILE:
				{
					IDataObject* pDataObject;
					OleGetClipboard(&pDataObject);	//从clipboard中取得dataobject
									
					PasteOrDropFiles(pDataObject);
				}
				break;
				
			case ID_VIEW_UPONELEVEL:
				{
					if ( (g_szCurrentFolderId == NULL) || wcslen(g_szCurrentFolderId) <3)
					{
						break;
					} 
					
					//change g_szCurrentFolderId  to the parent folder. you should not change g_szCurrentFolderId until you are sure that getlistnode has succeed.
					WCHAR szFolderId[LEN_NODEID]={0};
					wcscpy_s(szFolderId, LEN_NODEID, g_szCurrentFolderId);
					GetUplevelId(szFolderId);  
					
					//refresh the parent folder's  listview 
					RefreshListView(szFolderId);
				}
				break;
				
			case ID_VIEW_LARGEICONS:
				{
					SendMessage(g_hListView, LVM_SETVIEW, (WPARAM)LV_VIEW_ICON, 0);
				}
				break;
			case ID_VIEW_SMALLICONS:
				{
					SendMessage(g_hListView, LVM_SETVIEW, (WPARAM)LV_VIEW_SMALLICON, 0);
				}
				break;
			case ID_VIEW_LISTLIST:
				{
					SendMessage(g_hListView, LVM_SETVIEW, (WPARAM)LV_VIEW_LIST, 0);
				}
				break;
			case ID_VIEW_TILE:
				{
					SendMessage(g_hListView, LVM_SETVIEW, (WPARAM)LV_VIEW_TILE, 0);
				}
				break;
			case ID_VIEW_DETAIL:
				{
					SendMessage(g_hListView, LVM_SETVIEW, (WPARAM)LV_VIEW_DETAILS, 0);
				}
				break;
			case ID_HELP_HELPCONTENT:
				{
					ShellExecute(NULL, L"open", L"help.html", NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			case ID_HELP_ABOUT:
				{
					//show modal dialog box	
					DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOGABOUT), hwnd, (DLGPROC)AboutDlgProc);
				}
				break;	
				
			case IDM_TRAYOPEN:   //restore the window from system tray
				{
					RestoreFromSysTray();				
				}
				break;	
			
			case IDM_TRAYEXIT:   //quit the system.
				{
					//delete the system tray icon
					NOTIFYICONDATA nid = {0};
					nid.cbSize = sizeof(NOTIFYICONDATA);
					nid.hWnd = g_hwnd;
					nid.uID = IDI_APPICON;
					Shell_NotifyIcon(NIM_DELETE,&nid);
					
					SendMessage(hwnd, WM_DESTROY, 0, 0);
				}
				break;
			}
		}
		break;
	case UWM_EXITTHREADMSG:
		{
			ThreadParamBase* pParam = (ThreadParamBase*)lParam;
			HWND hMsg = pParam->m_hMsg;
			//hide and destroy message dialog
			if (hMsg != NULL)
			{
				ShowWindow(hMsg, SW_HIDE);
				DestroyWindow(hMsg);
			}
			//set the gloabal var  to indicate FREE
			g_bIsBusy = FALSE;
			
			if (g_bUserCanceled)
				g_bUserCanceled = FALSE;
			//set menu, toolbar, statusbar's state
			SetMenuToolbarStatusbar();
			
			delete pParam;  //release thread param
			pParam = NULL;
		}
		break;	
		
	case UWM_EXITTHREADPROGRESS:
		{
			ThreadParamBase* pParam = (ThreadParamBase*)lParam;
			HWND hProgressDlg = pParam->m_hMsg;
			
			if ( hProgressDlg != NULL) 
			{
				if (!IsWindowVisible(GetDlgItem( hProgressDlg, IDC_BUTTONHIDE)))   
				{
					ShowWindow(hProgressDlg, SW_HIDE);
					DestroyWindow(hProgressDlg);
				}
			}
			
			g_bIsBusy= FALSE;
			
			if (g_bUserCanceled)
			{
				g_bUserCanceled = FALSE;
			}
			
			SetMenuToolbarStatusbar();

			delete pParam;
			pParam = NULL;
		}
		break;
		
	case UWM_TRAYICON:
	{
		if (wParam != IDI_APPICON)   //if not the app icon, ignore
			break;
		if  (lParam == WM_LBUTTONUP)
		{
			RestoreFromSysTray();
		}
		else if (lParam == WM_RBUTTONUP)
		{
			GenerateSysTrayMenu();
		}
	}
	break;
		
	case WM_CLOSE:  
		{
			MinimizeToSysTray();
		}
		break;
		
	case WM_DESTROY:
		{
			RevokeDragDrop(g_hListView);     //RegisterDragDrop
			CoLockObjectExternal(pDropTarget, FALSE, TRUE);  //remove the strong lock
			if (pDropTarget != NULL)
				pDropTarget->Release(); //release our own reference.

			//quit
			PostQuitMessage(0);
		}			
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}




/*********************************************************************************
WinMain() Function
This is the entry function
**********************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	OleInitialize(0);	

	//login dialogbox
	INT loginResult = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGLOGIN), NULL, (DLGPROC)LoginDlgProc);
	
	if (loginResult == IDCANCEL)
	{
		return 0;
	}

	ZeroMemory(g_szCurrentFolderId, sizeof(g_szCurrentFolderId));
	
	//window GUI and Message Handling
	static const WCHAR  szClassName[] = L"CloudDriveWindowClass";

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;                                               
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra  =0;
	wc.hInstance = hInstance;
	wc.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 32, 32, LR_SHARED);
	wc.hCursor= (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, SM_CXCURSOR, SM_CYCURSOR, LR_SHARED);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE +1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENUMAIN);
	wc.lpszClassName = szClassName;
	wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, TEXT("Class Registration Failed"), TEXT("ERROR"), MB_OK|MB_ICONERROR);
		return 0;
	}

	CreateWindowEx(0, szClassName, TEXT("CloudDrive"),  WS_OVERLAPPEDWINDOW  ,
								CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								NULL, NULL, hInstance, NULL);

	ShowWindow(g_hwnd, nCmdShow);
	UpdateWindow(g_hwnd);
	
	// ctrl+C, CTRL+V 
	HACCEL hAccel = LoadAccelerators(hInstance,  MAKEINTRESOURCE(IDR_ACCELERATORCOPYPASTE));

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)>0)
	{
		if (!TranslateAccelerator(g_hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (g_pShareAPI)
		delete g_pShareAPI;

	OleUninitialize();  

	return msg.wParam;
}
 