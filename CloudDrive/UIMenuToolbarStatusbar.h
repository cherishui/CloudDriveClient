#pragma once
#include <windows.h>

//the status of  Menu , toolbar and statusbar 
enum  CurrentState
{
	NONSELECTED,			//
	SIGLEFILESELECTED,
	SIGLEFOLDERSELECTED,
	MULTISELECTED,
	BUSY
};


//Toolbar
#define	TOOLBARBUTTONCOUNT   12			//Toolbar 上共有8个button + 4个seperator

LPWSTR const szButton0Text = L"Parent Folder";
LPWSTR const szButton0Tip = L"Go to the upper level";

LPWSTR const szButton1Text = L"Refresh";
LPWSTR const szButton1Tip = L"Refresh";

LPWSTR const szButton2Text = L"Open";
LPWSTR const szButton2Tip = L"Open the file or folder";

LPWSTR const szButton3Text = L"Download";
LPWSTR const szButton3Tip = L"Download selected files";

LPWSTR const szButton4Text = L"Upload";
LPWSTR const szButton4Tip = L"Upload files to online storage";

LPWSTR const szButton5Text = L"Delete";
LPWSTR const szButton5Tip = L"Delete selected items";

LPWSTR const szButton6Text = L"Views";
LPWSTR const szButton6Tip = L"Choose how to view the contents of this folder";

LPWSTR const szButton7Text = L"Help";
LPWSTR const szButton7Tip = L"Help";

//the index of the parts in the StatusBar
#define	PARTINDEX_MAIN		0
#define	PARTINDEX_BACKGROUND_UPLOADDOWNLOAD	1
#define	PARTINDEX_STATUS		2
 

void  CreateToolbar(void);
void  CreateStatusbar(void);
HMENU GeneratePopupMenu();

LRESULT  HandleToolbarNotification( LPNMHDR lParam);
void  SetMenuToolbarStatusbar(); 





