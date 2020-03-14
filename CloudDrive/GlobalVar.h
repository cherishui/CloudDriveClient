#pragma once      //���������ifndef������
#include <windows.h>
#include <ShareAPI.h>

#include "skyclient.h"
//declare the extern global variables

extern HWND g_hListView;			//ListView Control	
extern HWND g_hTool;				//Toolbar Control
extern HWND g_hStatusBar;			//Statusbar control
extern HWND g_hwnd;				// the main windows
extern HINSTANCE g_hInstance;		// the global modual instance.

//extern HWND g_hRefreshMsgDlg;

extern ShareAPI *g_pShareAPI;				//ShareAPI object pointer

extern WCHAR g_szCurrentFolderId[LEN_NODEID];	// global  current folder Id;

extern BOOL g_bSortAscending;					// the first sort is ascending when clicking the column

extern WCHAR g_szNewFolderName[LEN_NAME];   //���ڱ����½�folder�����ֵ�ȫ�ֱ���
extern WCHAR g_szShareEmails[2048];
//extern PTP_POOL	g_pThreadPool ;           //thread pool pointer
//extern TP_CALLBACK_ENVIRON g_CBEnviron;		 //callback environment
//extern PTP_CLEANUP_GROUP  g_pCleanupGroup;		//cleanup group

extern stack<WCHAR*> g_stackPathTrace;	//����Ӹ�Ŀ¼����ǰĿ¼�ĸ�Ŀ¼����Ϣ,���� stack����

extern CRITICAL_SECTION  g_csCurrentFolderChange;

//extern int g_nPendingFileNum;  //the number of current pending files

extern  BOOL g_bIsBusy;  //��ǰ�Ƿ����߳����ڴ���(ǰ̨�߳�, һ����˵���жԻ�����ʾ��ǰ��)
extern  BOOL g_bUserCanceled;  //�û��Ƿ�cancel�˵�ǰ���߳�