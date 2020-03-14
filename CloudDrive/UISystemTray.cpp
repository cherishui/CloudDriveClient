/***********************************************************************
Functions related to the System tray operataion, 
including  minimize window to system tray and restore from system tray....

Author: Jerry wang (Íõ½¨»ª)
All right reserved.
**********************************************************************/
#include <windows.h>
#include <shlobj.h>
#include "resource.h"
#include "clouddrive.h"
#include "UISystemTray.h"

/*******************************************************************
Restore the App window from system tray,  and delete the system tray icon
*****************************************************************/
void RestoreFromSysTray()
{
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = g_hwnd;
	nid.uID = IDI_APPICON;
	Shell_NotifyIcon(NIM_DELETE,&nid);
	
	SetForegroundWindow(g_hwnd);
	ShowWindow(g_hwnd, SW_RESTORE);
}

/************************************************************************
Minimize the main window to the system tray icon, hide all the owned(child) window
the system tray icon has Tooltip and Balloon tooltip.
**************************************************************************/
void MinimizeToSysTray()
{
	ShowWindow(g_hwnd, SW_MINIMIZE); //  hide the owned windows!
	ShowWindow(g_hwnd, SW_HIDE);
	
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	 
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = g_hwnd;
	nid.uID = IDI_APPICON;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP |NIF_INFO;
	nid.uCallbackMessage = UWM_TRAYICON;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, 0);
	wcscpy(nid.szTip, L"CloudDrive");
	//balloon tips
	nid.uTimeout =2000;
	nid.dwInfoFlags = NIIF_INFO;
	wcscpy(nid.szInfo,  L"CloudDrive is still running in background...");
	wcscpy(nid.szInfoTitle,  L"CloudDrive");

	Shell_NotifyIcon(NIM_ADD, &nid); 
}

/************************************************************
Generate the System tray's context menu
*************************************************************/
void  GenerateSysTrayMenu()
{
	HMENU hMenu = CreatePopupMenu();
	
	AppendMenu(hMenu, MF_STRING, IDM_TRAYOPEN, L"Open");
	AppendMenu(hMenu, MF_STRING, IDM_TRAYEXIT, L"Exit");
	
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	
	SetForegroundWindow(g_hwnd);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN,ptCursor.x, ptCursor.y, 0, g_hwnd, NULL);
}