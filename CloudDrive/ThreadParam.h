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
������һ��ͨ���͵�ThreadParam
�ܶ��߳̿���ʹ�øò�����������Ϣ, ���԰���: һ��nodeid, һ��name��Ϣ, һ��hwnd
�����������Ҫ��, �������ж���
*********************************************************************/
class ThreadParam: public ThreadParamBase              
{
public:
	ThreadParam(LPCWSTR lpszNodeId, LPCWSTR lpszName = NULL, HWND hMsg=NULL );
	
	WCHAR m_szNodeId[LEN_NODEID];		//id ���� (������filenodeid, foldernodeid,������parentnodeid)
	WCHAR m_szName[LEN_NAME];			//name ���� (������filename, newfoldrename....)
//	HWND   m_hwndMsg;					//��Ӧ�� message dialog ��HWND ,û�еĻ�ΪNULL.
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
	WCHAR m_szDestPath[MAX_PATH];            //Ҫ������ء��ļ��ı���Ŀ¼
//	HWND m_hwndMsg;
};



/**********************************************************
Upload file ���ļ�ѡ�񴰿�,ѡ��һ�������ļ���upload  �õ��� thread parameter

m_szFileNames: the OPENFILENAME filename string.  using '\0' to seperate each file name
m_nOffset:  OPENFILENAME �� file offset.
m_hProgress:  the handle of the Progressbar window.
*********************************************************/
class ThreadParamUpload: public ThreadParamBase
{
public:
	ThreadParamUpload(LPWSTR lpszFileNames, WORD nFileOffset, HWND hMsg);
	~ThreadParamUpload();
	
	WCHAR* m_szFileNames;   //ע�⣡���ﲻ�������飬��ΪlpszFileNames����������⣬���м��кܶ�\0, ���Ա����ڴ洫��
	WORD  m_nOffset;
//	HWND  m_hProgress;
};


/**********************************************************************
��������ѡ�е��ļ�/�ļ��� ����Ŀ���߳� �õ���ThreadParam

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
Drop file /Paste file  �õ� Thread Param

********************************************************************/
class ThreadParamDrop: public ThreadParamBase
{
public:
	ThreadParamDrop(LPWSTR lpszCurrentFolderId, IDataObject* pDataObject, HWND hMsg);
	
	WCHAR m_szFolderId[LEN_NODEID];        
	LPSTREAM m_pStream;  //����marsheal  IDataObject�����stream
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