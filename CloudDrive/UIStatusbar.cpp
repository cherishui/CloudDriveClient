#include <windows.h>
#include  <commctrl.h>
#include "GlobalVar.h"
#include "UIStatusbar.h"
#include "resource.h"

//!!!!!!!!!!!!!这个文件要和UIMenuToolbar合并到一起！！！


/**************************************************************************
Create StatusBar
the Statusbar has 4 parts, for example:
	80 items/5 items is selected,    uploading: 4 ,  downloading: 9,   statusIcon: online
	
The size of the statusbar parts is set in the WM_SIZE message procedure.
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
	//SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_UPLOADING, (LPARAM)L"uploading: 0");
	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_BACKGROUND_UPLOADDOWNLOAD, (LPARAM)L"  ");
	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_STATUS, (LPARAM)L"online");
	//set icon
	HANDLE hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STATUSONLINE), IMAGE_ICON,16 ,16,0);
	SendMessage(g_hStatusBar, SB_SETICON, PARTINDEX_STATUS, (LPARAM)hIcon);
}


/****************************************************************************
当有新的下载时更新StatusBar上的信息
Param:
	sbc:

	
fileInfo: only used wen the sbc== SB_UPDOWNPROGRESS,  other situation, ignored
	


**********************************************************************/
void RefreshStatusBar ( )
{

	//if (sbc == SB_SELECTEDCHANGE)
	//{	
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
	//} 
	//
	////用来处理文件传输的
	//if (sbc == SB_UPDOWNPROGRESS)
	//{
	////基本的格式如下:  Downloading/Uploading  aaa.jpg   354KB of 1233KB (32%)   23.4kb/s   32 files  are pending.
	//
	//	WCHAR szPart[256]={0};
	//	
	//	if (pFileInfo->iDirection == UPLOADING)
	//		wcscpy_s(szPart, 256, L"Uploading ");
	//	else if (pFileInfo->iDirection == DOWNLOADING)
	//		wcscpy_s(szPart, 256, L"Downloading ");
	//	else
	//		return;    //如果不是 downloading or uploading, 不刷新
	//		
	//	wcscpy_s(szPart, 256, pFileInfo->szFileName);
	//	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_BACKGROUND_UPLOADDOWNLOAD, (LPARAM)szPart);
	//}
	//if (sbc == SB_UPDOWNFINISHED)
	//{
	//	SendMessage(g_hStatusBar, SB_SETTEXT, PARTINDEX_BACKGROUND_UPLOADDOWNLOAD, (LPARAM)L"");
	//}
	//
}

