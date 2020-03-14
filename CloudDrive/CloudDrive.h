#pragma once
#include <windows.h>
#import <msxml6.dll>

class ShareAPI;

//Length 
#define	LEN_NAME			128                  //node (foder or file) Name length
#define	LEN_NODEID			512
#define	LEN_TYPE				64
#define	LEN_SIZE				32
#define	LEN_CREATEDDATE		64
#define	LEN_MODIFIEDDATE		64
#define	BITMAPSIZE			32


#define	LEN_USEREMAIL		64
#define	LEN_USERPASSWORD	32
#define	LEN_FORBIDDENEXT	512

///////globar varialbe declaration
extern HWND g_hListView;			//ListView Control	
extern HWND g_hTool;				//Toolbar Control
extern HWND g_hStatusBar;			//Statusbar control
extern HWND g_hwnd;				// the main windows
extern HINSTANCE g_hInstance;		// the global modual instance.

extern ShareAPI *g_pShareAPI;				//ShareAPI object pointer

extern WCHAR g_szCurrentFolderId[LEN_NODEID];	// global  current folder Id;
extern MSXML2::IXMLDOMDocumentPtr g_CurrentFolderDomDocPtr; // global current folder's dom doc object.

extern BOOL g_bSortAscending;					// the first sort is ascending when clicking the column

extern WCHAR g_szNewFolderName[LEN_NAME];  
extern WCHAR g_szShareEmails[2048];

extern  BOOL g_bIsBusy;  
extern  BOOL g_bUserCanceled;  

void GetUplevelId(LPWSTR szFolderId);