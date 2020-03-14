/***************************************************************************
This file contains functions to setup the Menu item, Toolbar button and statusbar's text
Author: Jerry wang （王建华）
All right reserved.
***************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "CloudDrive.h"
#include "UIMenuToolbarStatusbar.h"
#include "UIListview.h"
#include "resource.h"

/***************************************************************************
Add buttons to toolbar
The toolbar is a IE-style toolbar, 
Some buttons have text, some haven't.   View Button has dropdown menu.
Message: TB_ADDBUTTONS
private method, only used in this file. 
****************************************************************************/
void addButtonsToToolbar()
{      
	//add buttons to the toolbar
	TBBUTTON  buttons[TOOLBARBUTTONCOUNT];
	ZeroMemory(buttons, sizeof(buttons));
	//Up Level button
	buttons[0].iBitmap = 0;
	buttons[0].idCommand = ID_VIEW_UPONELEVEL;
	buttons[0].fsState = TBSTATE_ENABLED;
	buttons[0].iString = (int)szButton0Text;
	//Refresh button
	buttons[1].iBitmap = 1;
	buttons[1].idCommand = ID_VIEW_REFRESH;
	buttons[1].fsState = TBSTATE_ENABLED;
	buttons[1].iString = (int)szButton1Text;
	buttons[1].fsStyle = BTNS_AUTOSIZE;
	//Seperator
	buttons[2].iBitmap = 6;		//widtch in pixels
	buttons[2].fsStyle = BTNS_SEP;
	//Open file button
	buttons[3].iBitmap = 2;
	buttons[3].idCommand = ID_FILE_OPENFILEORFOLDER;
	buttons[3].fsState = TBSTATE_ENABLED;
	buttons[3].iString = (int)szButton2Text;
	buttons[3].fsStyle = BTNS_AUTOSIZE;
	//Download file button
	buttons[4].iBitmap = 3;
	buttons[4].idCommand = ID_FILE_SAVETO;
	buttons[4].fsState = TBSTATE_ENABLED;
	buttons[4].iString= (int)szButton3Text;
	buttons[4].fsStyle = BTNS_AUTOSIZE;	
	//Upload Button
	buttons[5].iBitmap = 4;
	buttons[5].idCommand = ID_FILE_UPLOAD;
	buttons[5].fsState = TBSTATE_ENABLED;
	buttons[5].iString= (int)szButton4Text;
	buttons[5].fsStyle = BTNS_AUTOSIZE;

	//Seperator
	buttons[6].iBitmap = 6;		//widtch in pixels
	buttons[6].fsStyle = BTNS_SEP;

	//delete button
	buttons[7].iBitmap = 5;
	buttons[7].idCommand = ID_FILE_DELETE;
	buttons[7].fsState = TBSTATE_ENABLED;
	buttons[7].fsStyle = BTNS_AUTOSIZE;
	buttons[7].iString = (int)szButton5Text;
	//Seperator
	buttons[8].iBitmap = 6;  //widtch in pixels
	buttons[8].fsStyle = BTNS_SEP;
	//Views button
	buttons[9].iBitmap = 6;
	buttons[9].idCommand = ID_VIEW_DETAIL;
	buttons[9].fsState = TBSTATE_ENABLED;
	buttons[9].iString = (int)szButton6Text;
	buttons[9].fsStyle = BTNS_DROPDOWN;

	//Seperator
	buttons[10].iBitmap = 6;  //widtch in pixels
	buttons[10].fsStyle = BTNS_SEP;
	
	//help
	buttons[11].iBitmap = 7;
	buttons[11].idCommand = ID_HELP_HELPCONTENT;
	buttons[11].fsState = TBSTATE_ENABLED;
	buttons[11].fsStyle = BTNS_AUTOSIZE;

	//add buttons to toolbar
	SendMessage(g_hTool, TB_ADDBUTTONS,TOOLBARBUTTONCOUNT, (LPARAM)&buttons);
}

/****************************************************************************
Create toolbar
this toolbar is an IE-style toolbar, which is located on a Rebar control
***************************************************************************/
void CreateToolbar(void)
{
	//Register specific common control classes, including toolbar&StatusBar ,   Rebar controls
	//Toolbar&StatusBar Registration
	INITCOMMONCONTROLSEX  comCtl;
	comCtl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	comCtl.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&comCtl);
	//Rebar Registration
	comCtl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	comCtl.dwICC = ICC_COOL_CLASSES;
	InitCommonControlsEx(&comCtl);

	//Create Rebar control 
	HWND hRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL, 
			RBS_BANDBORDERS|RBS_DBLCLKTOGGLE|RBS_REGISTERDROP|RBS_VARHEIGHT|
			CCS_NODIVIDER|CCS_NOPARENTALIGN|
			WS_BORDER|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE,
			0,0,2400,0,
			g_hwnd,NULL,GetModuleHandle(NULL), NULL);

	//Create & Set ToolBar;  
	g_hTool = CreateWindowEx(NULL, TOOLBARCLASSNAME, NULL,  
					WS_CHILD|WS_VISIBLE| TBSTYLE_TOOLTIPS|TBSTYLE_FLAT|TBSTYLE_LIST, 
					0,0,0,0, g_hwnd, (HMENU)IDC_TOOLBAR, GetModuleHandle(NULL),NULL);
					
	SendMessage(g_hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(g_hTool, TB_SETEXTENDEDSTYLE,  0, (LPARAM)TBSTYLE_EX_DRAWDDARROWS);    //this style must use TB_SETEXTENDEDSTYLE, cannot use in CreateWindowEx.

	// Set the Toolbar's  image list.
	HIMAGELIST hImageList = ImageList_LoadImage(g_hInstance,
			 MAKEINTRESOURCE(IDB_TOOLBARBITMAP),
			 BITMAPSIZE,
			 0,
			 CLR_DEFAULT ,
			 IMAGE_BITMAP,
			 LR_CREATEDIBSECTION);
	SendMessage(g_hTool, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

	//Add buttons to toolbar
	addButtonsToToolbar();

	//get the button size (the height of the toolbar)
	DWORD dwBtnSize = SendMessage(g_hTool, TB_GETBUTTONSIZE, 0,0);

	//Initialize the REBARINFO structure , no imagelist is combinded
	REBARINFO rbi;
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask =0;
	rbi.himl = (HIMAGELIST)NULL;   
	SendMessage(hRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

	//INSERTBAND: add toolbar to rebar control
	REBARBANDINFO rbBand;
	::ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);
	rbBand.fMask = RBBIM_COLORS|RBBIM_TEXT|RBBIM_STYLE|RBBIM_CHILD|RBBIM_CHILDSIZE|RBBIM_SIZE;
	
	rbBand.clrBack = GetSysColor(COLOR_WINDOW);
	rbBand.lpText = L"";
	rbBand.fStyle = RBBS_CHILDEDGE;
	rbBand.hwndChild = g_hTool;
	rbBand.cxMinChild = 100;
	rbBand.cyMinChild = HIWORD(dwBtnSize);
	rbBand.cx = 800;
	//rbBand.
	SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
}


/**********************************************************
When right click the Listview control,  popup  the context menu.
there are 4 types of popup menus:
1. right click one single file.
2. right click one single folder.
3. right click multiple files/folders
4. right click the ListView with any selection

*********************************************************************/
HMENU GeneratePopupMenu()
{	
	if (g_bIsBusy)  
		return NULL; 
	
	HMENU hPopupMenu =  CreatePopupMenu();

	int nSel = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (nSel == -1)  //no selection
	{
		AppendMenu(hPopupMenu, MF_STRING, ID_FILE_CREATEFOLDER, TEXT("&Create Folder"));
		AppendMenu(hPopupMenu, MF_STRING, ID_FILE_UPLOAD, TEXT("&Upload..."));
	}
	else
	{
		//append menu items to the popup  according to the selection
		if (ListView_GetSelectedCount(g_hListView) ==1)		//single file or single folder
		{
			AppendMenu(hPopupMenu, MF_STRING, ID_FILE_OPENFILEORFOLDER, TEXT("&Open"));
			AppendMenu(hPopupMenu, MF_STRING, ID_FILE_SAVETO, TEXT("Down&load..."));
			//AppendMenu(hPopupMenu, MF_STRING, ID_FILE_RENAME, TEXT("&Rename"));  //share api has problem !
			AppendMenu(hPopupMenu, MF_STRING, ID_FILE_DELETE, TEXT("&Delete"));
		}
		else        //multiple selection
		{
			AppendMenu(hPopupMenu, MF_STRING, ID_FILE_SAVETO, TEXT("Down&load..."));
			AppendMenu(hPopupMenu, MF_STRING, ID_FILE_DELETE, TEXT("&Delete"));
		}
	}
	//create and add view Menu item. (this menu item has a submenu)
	AppendMenu(hPopupMenu, MF_SEPARATOR, NULL, NULL);  //seperator
	HMENU hViewSubMenu = CreatePopupMenu();
	AppendMenu(hViewSubMenu, MF_STRING, ID_VIEW_LARGEICONS, L"Large Icons");
	AppendMenu(hViewSubMenu, MF_STRING, ID_VIEW_SMALLICONS, L"Small Icons");
	AppendMenu(hViewSubMenu, MF_STRING, ID_VIEW_LISTLIST, L"List");
	AppendMenu(hViewSubMenu, MF_STRING, ID_VIEW_TILE, L"Tiles");
	AppendMenu(hViewSubMenu, MF_STRING, ID_VIEW_DETAIL, L"Details");
	
	AppendMenu(hPopupMenu, MF_STRING|MF_POPUP, (UINT)hViewSubMenu, L"&Views");
	
	if ( IsClipboardFormatAvailable(CF_HDROP))
	{
		AppendMenu(hPopupMenu, MF_STRING, ID_EDIT_PASTEFILE, L"Paste");
	}
	
	return hPopupMenu;
}



/**************************************************************************
process the WM_NOTIFY message from Toolbar control
the notification includes:  Tooltip (TTN_GETDISPINFO) and  dropdown menu (TBN_DROPDOWN)
****************************************************************************/
LRESULT HandleToolbarNotification( LPNMHDR lParam)
{
	switch(lParam->code)
	{
		case TTN_GETDISPINFO:  //show tooltip
		{
			LPNMTTDISPINFO  lpttt = (LPNMTTDISPINFO)lParam;
			lpttt->hinst = g_hInstance;
			
			UINT_PTR idButton = lpttt->hdr.idFrom;
			switch(idButton)
			{
				case ID_VIEW_UPONELEVEL:
				{
					lpttt->lpszText = szButton0Tip;
				}
				break;
				case ID_VIEW_REFRESH:
				{
					lpttt->lpszText = szButton1Tip;
				}
				break;
				case ID_FILE_OPENFILEORFOLDER:
				{
					lpttt->lpszText = szButton2Tip;
				}
				break;
				case ID_FILE_SAVETO:
				{	
					lpttt->lpszText = szButton3Tip;
				}
				break;
				case ID_FILE_UPLOAD:
				{
					lpttt->lpszText = szButton4Tip;
				}
				break;
				case ID_FILE_DELETE:
				{
					lpttt->lpszText = szButton5Tip;
				}
				break;
				case ID_VIEW_DETAIL:
				{
					lpttt->lpszText = szButton6Tip;
				}
				break;
				case ID_HELP_HELPCONTENT:
				{
					lpttt->lpszText = szButton7Tip;
				}
				break;
			}
		}
		return FALSE;

		case TBN_DROPDOWN:
		{	
			//目前来说,只有view这个button有下拉菜单 //给这个Button发TB_GETRECT,
			RECT rc;
			SendMessage( ((LPNMTOOLBAR)lParam)->hdr.hwndFrom,  TB_GETRECT,  ((LPNMTOOLBAR)lParam)->iItem, (LPARAM)&rc);
			MapWindowPoints( ((LPNMTOOLBAR)lParam)->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc,  2);
						
			LPNMTOOLBAR  lpnmt = (LPNMTOOLBAR)lParam;
			int idCmd = lpnmt->iItem;
			if (idCmd ==  ID_VIEW_DETAIL)
			{
				HMENU hDropDownMenu =  CreatePopupMenu();
				AppendMenu(hDropDownMenu, MF_STRING, ID_VIEW_LARGEICONS, L"Large Icons");
				AppendMenu(hDropDownMenu, MF_STRING, ID_VIEW_SMALLICONS, L"Small Icons");
				AppendMenu(hDropDownMenu, MF_STRING, ID_VIEW_LISTLIST, L"List");
				AppendMenu(hDropDownMenu, MF_STRING, ID_VIEW_TILE, L"Tiles");
				AppendMenu(hDropDownMenu, MF_STRING, ID_VIEW_DETAIL, L"Details");
				TrackPopupMenu(hDropDownMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,  rc.left, rc.bottom,0, g_hwnd, 0); 
			}
		}
		
		return FALSE;

		default:
			return FALSE;  
	}
}


/**************************************************************************
Set the status of Menu items and Toolbar button
	//NONSELECTED,			//
	//SIGLEFILESELECTED,
	//SIGLEFOLDERSELECTED,
	//MULTISELECTED,
	//BUSY

//private method, only used in this file
***************************************************************************/
VOID setMenuAndToolbar(CurrentState state )
{
	HMENU hMenu = GetMenu(g_hwnd);
	
	HMENU hFileMenu = GetSubMenu(hMenu, 0);
	
	//upload
	if ( state == BUSY )
	{
		EnableMenuItem(hFileMenu, ID_FILE_UPLOAD, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_UPLOAD, FALSE);  //toolbar button
	}
	else
	{
		EnableMenuItem(hFileMenu, ID_FILE_UPLOAD, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_UPLOAD, TRUE);
	}
	
	//open
	if ( (state == SIGLEFILESELECTED) || (state ==   SIGLEFOLDERSELECTED) )
	{
		EnableMenuItem(hFileMenu, ID_FILE_OPENFILEORFOLDER, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_OPENFILEORFOLDER, TRUE);
	}
	else
	{
		EnableMenuItem(hFileMenu, ID_FILE_OPENFILEORFOLDER, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_OPENFILEORFOLDER, FALSE);  //BUTTON
	}
	
	//save to
	if ( (state == BUSY ) || (state == NONSELECTED))
	{
		EnableMenuItem(hFileMenu, ID_FILE_SAVETO, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_SAVETO, FALSE);
	}
	else
	{
		EnableMenuItem(hFileMenu, ID_FILE_SAVETO, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_SAVETO, TRUE);
	}
	
	//create folder
	if ( state == BUSY )
	{
		EnableMenuItem(hFileMenu, ID_FILE_CREATEFOLDER, MF_BYCOMMAND|MF_GRAYED);
	}
	else
	{
		EnableMenuItem(hFileMenu, ID_FILE_CREATEFOLDER, MF_BYCOMMAND|MF_ENABLED);
	}
	
	//if  ((state == SIGLEFILESELECTED ) || (state == SIGLEFOLDERSELECTED))
	//{
	//	EnableMenuItem(hFileMenu, ID_FILE_RENAME, MF_BYCOMMAND|MF_ENABLED);
	//}
	//else
	//{
		EnableMenuItem(hFileMenu, ID_FILE_RENAME, MF_BYCOMMAND|MF_GRAYED);
	//}
	
	//delete
	if  ((state == SIGLEFILESELECTED ) || (state == SIGLEFOLDERSELECTED) || (state == MULTISELECTED))
	{
		EnableMenuItem(hFileMenu, ID_FILE_DELETE, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_DELETE, TRUE);
	}
	else
	{
		EnableMenuItem(hFileMenu, ID_FILE_DELETE, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_FILE_DELETE, FALSE);
	}
		
	//
	HMENU hEditMenu = GetSubMenu(hMenu, 1);
	
	////cut
	//if  ((state == SIGLEFILESELECTED ) || (state == SIGLEFOLDERSELECTED) || (state == MULTISELECTED))
	//{
	//	EnableMenuItem(hEditMenu, ID_EDIT_CUTFILE, MF_BYCOMMAND|MF_ENABLED);
	//}
	//else
	//{
	//	EnableMenuItem(hEditMenu, ID_EDIT_CUTFILE, MF_BYCOMMAND|MF_GRAYED);
	//}
	////, copy
	//if  ((state == SIGLEFILESELECTED ) || (state == SIGLEFOLDERSELECTED) || (state == MULTISELECTED))
	//{
	//	EnableMenuItem(hEditMenu, ID_EDIT_COPYFILE, MF_BYCOMMAND|MF_ENABLED);
	//}
	//else
	//{
	//	EnableMenuItem(hEditMenu, ID_EDIT_COPYFILE, MF_BYCOMMAND|MF_GRAYED);
	//}
	
	if ( IsClipboardFormatAvailable(CF_HDROP))
	{
		EnableMenuItem(hEditMenu, ID_EDIT_PASTEFILE, MF_BYCOMMAND|MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hEditMenu, ID_EDIT_PASTEFILE, MF_BYCOMMAND|MF_GRAYED);
	}
	
	
	HMENU hViewMenu = GetSubMenu(hMenu, 2);

	BOOL isRoot = FALSE;
	if (g_szCurrentFolderId == NULL || wcslen(g_szCurrentFolderId) <3)
	{
		isRoot=TRUE;
	}
	//if (wcsstr(g_szCurrentFolderId, L":") == NULL)
	//{
	//	isRoot = TRUE;
	//}
	
	if ((state != BUSY) && (!isRoot) )
	{
		EnableMenuItem(hViewMenu, ID_VIEW_UPONELEVEL, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool,TB_ENABLEBUTTON, ID_VIEW_UPONELEVEL, TRUE);
	
	}
	else
	{
		EnableMenuItem(hViewMenu, ID_VIEW_UPONELEVEL, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool,TB_ENABLEBUTTON, ID_VIEW_UPONELEVEL, FALSE);
	}
	
	//Refresh
	if (state != BUSY)
	{
		EnableMenuItem(hViewMenu, ID_VIEW_REFRESH, MF_BYCOMMAND|MF_ENABLED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_VIEW_REFRESH, TRUE);	
	}
	else
	{
		EnableMenuItem(hViewMenu, ID_VIEW_REFRESH, MF_BYCOMMAND|MF_GRAYED);
		SendMessage(g_hTool, TB_ENABLEBUTTON, ID_VIEW_REFRESH, FALSE);	
	}
}




/****************************************************************************
Refresh the information text on  StatusBar
Till now, only the 1st part is refreshed.  other 2 parts have not been implemented yet.
**********************************************************************/
void refreshStatusBar ( )
{
	WCHAR szPart[32];
	LRESULT iSelectedCount = SendMessage(g_hListView, LVM_GETSELECTEDCOUNT, 0,0);
	if (iSelectedCount >0)
	{
		_itow(iSelectedCount, szPart, 10);
		wcscat(szPart, L"  items seleted");
		SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_MAIN, (LPARAM)szPart);
	}
	else
	{
		LRESULT iItemCount = SendMessage(g_hListView, LVM_GETITEMCOUNT, 0,0);
		_i64tow(iItemCount, szPart, 10);
		wcscat(szPart, L"  items");
		SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_MAIN, (LPARAM)szPart);
	}
}

/***********************************************************************
SET  the status of  Menu, Toolbar  and Statusbar
this function calls:  setMenuAndToolbar()   + refreshStatusBar()
***********************************************************************/
void  SetMenuToolbarStatusbar()
{
	if ( g_bIsBusy)
	{
		setMenuAndToolbar(BUSY);
	}
	else
	{	
		int nSel = SendMessage(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
		
		if (nSel == -1) //no selection
		{
			setMenuAndToolbar(NONSELECTED);
		}
		else
		{
			if (ListView_GetSelectedCount(g_hListView) ==1)  //one selected
			{
				//get the selected item's type (Folder or File)
				const int iTypeSize =64;
				WCHAR itemType[iTypeSize] ={0};
								
				LVITEM lvItem;
				lvItem.mask = LVIF_TEXT;
				lvItem.iItem = nSel;
				lvItem.iSubItem =COLUMNINDEX_TYPE;
				lvItem.pszText = itemType;
				lvItem.cchTextMax = iTypeSize;   

				SendMessage(g_hListView, LVM_GETITEM, 0, (LPARAM)&lvItem);

				if (lstrcmp(itemType, L"Folder") == 0)  //folder
				{
					setMenuAndToolbar(SIGLEFOLDERSELECTED);
				}
				else //file
				{
					setMenuAndToolbar(SIGLEFILESELECTED);
				}
				
			}
			else  // multiitem selected
			{
				setMenuAndToolbar(MULTISELECTED);
			}
		}
	}
	
	//statusbar
	refreshStatusBar();
}


/**************************************************************************
Create StatusBar
	
The size of the statusbar parts is set in the WM_SIZE message procedure of Mainwindow
The Text of the parts is set in  ............
**************************************************************************/
void  CreateStatusbar(void)
{
	g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, 
					WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
					0,0,0,0, g_hwnd, (HMENU)IDC_STATUSBAR, GetModuleHandle(NULL), NULL);

	int statusWidths[] = {0, 0, 0};   //initialize all width to 0, the real position will be set in WM_SIZE message.
	SendMessage(g_hStatusBar, SB_SETPARTS, 3, (LPARAM)statusWidths);

	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_MAIN, (LPARAM)L" ");
	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_BACKGROUND_UPLOADDOWNLOAD, (LPARAM)L"  ");
	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_STATUS, (LPARAM)L"online");
	//set icon
	HANDLE hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STATUSONLINE), IMAGE_ICON,16 ,16,0);
	SendMessage(g_hStatusBar, SB_SETICON, PARTINDEX_STATUS, (LPARAM)hIcon);
}

