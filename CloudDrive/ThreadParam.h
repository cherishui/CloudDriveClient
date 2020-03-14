#pragma once

#include <windows.h>
#include "CloudDrive.h"

//this is an abstract parent class for other threadParam,  
//the m_hMsg is the handle of the message window ( there are 2 types:  message dialog  or progressbar dialog)
class ThreadParamBase
{
public: 
	virtual ~ThreadParamBase()  =0 {}
	
	HWND m_hMsg;
};

/******************************************************************
这里是一个通用型的ThreadParam
很多线程可以使用该参数来传递信息, 可以包含: 一个nodeid, 一个name信息, 一个hwnd
如果不能满足要求, 可以自行定义
*********************************************************************/
class ThreadParam: public ThreadParamBase              
{
public:
	ThreadParam(LPCWSTR lpszNodeId, LPCWSTR lpszName = NULL, HWND hMsg=NULL );
	
	WCHAR m_szNodeId[LEN_NODEID];		//id 参数 (可能是filenodeid, foldernodeid,或者是parentnodeid)
	WCHAR m_szName[LEN_NAME];			//name 参数 (可能是filename, newfoldrename....)
//	HWND   m_hwndMsg;					//对应的 message dialog 的HWND ,没有的话为NULL.
};


/***************************************************
Save files from share online to local disk   ThreadParam

m_szDestPath: the local folder for saving the downloaded file
m_hwndMsg:  the handle of the message window
*****************************************************/
class ThreadParamSave: public ThreadParamBase
{
public:
	ThreadParamSave(LONG lCount, LPWSTR szNodeIds,LPCWSTR lpszDestPath,  HWND hMsg);
	~ThreadParamSave();
	LONG m_lCount;
	WCHAR* m_szNodeIds;
	WCHAR m_szDestPath[MAX_PATH];            //要存放下载　文件的本地目录
//	HWND m_hwndMsg;
};



/**********************************************************
Upload file 打开文件选择窗口,选择一个或多个文件后upload  用到的 thread parameter

m_szFileNames: the OPENFILENAME filename string.  using '\0' to seperate each file name
m_nOffset:  OPENFILENAME 的 file offset.
m_hProgress:  the handle of the Progressbar window.
*********************************************************/
class ThreadParamUpload: public ThreadParamBase
{
public:
	ThreadParamUpload(LPWSTR lpszFileNames, WORD nFileOffset, HWND hMsg);
	~ThreadParamUpload();
	
	WCHAR* m_szFileNames;   //注意！这里不能用数组，因为lpszFileNames这个串很特殊，　中间有很多\0, 所以必须内存传递
	WORD  m_nOffset;
//	HWND  m_hProgress;
};


/**********************************************************************
用来计算选中的文件/文件夹 的数目的线程 用到的ThreadParam

m_hDrop:  HDROP
m_hMsg: the handle of the message window
**********************************************************************/
class ThreadParamCalculate: public ThreadParamBase
{
public:
	ThreadParamCalculate(HDROP hDrop, HWND hMsg);
	
	HDROP  m_hDrop;
//	HWND   m_hMsg;
};

/*******************************************************************
Drop file /Paste file  用的 Thread Param

********************************************************************/
class ThreadParamDrop: public ThreadParamBase
{
public:
	ThreadParamDrop(LPWSTR lpszCurrentFolderId, IDataObject* pDataObject, HWND hMsg);
	
	WCHAR m_szFolderId[LEN_NODEID];        
	LPSTREAM m_pStream;  //用来marsheal  IDataObject对象的stream
//	HWND m_hProgress;
	
};


/*****************************************************************
The thread parameter for the login thread.
This parameter has 3 parts:
	m_szUserEmail:  the login user's email.
	m_szPassword: the input password.
	m_hLoginDlg:  the login Dialg box
*************************************************************/
class ThreadParamLogin: public ThreadParamBase
{
public:
	ThreadParamLogin(LPCWSTR lpszUserEmail, LPCWSTR lpszPassword, HWND hLogin);
	
	WCHAR m_szUserEmail[LEN_USEREMAIL];
	WCHAR m_szPassword[LEN_USERPASSWORD];
//	HWND  m_hLoginDlg;
};


/****************************************************************
The thread param for deleting file/folder
******************************************************************/
class ThreadParamDelete: public ThreadParamBase
{
public:
	ThreadParamDelete(LPWSTR szNodeIds, HWND hMsg);
	~ThreadParamDelete();
	
	WCHAR* m_szNodeIds;
	
}; 