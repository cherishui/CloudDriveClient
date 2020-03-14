/******************************************************************************************
FileName: ShareAPI.h
Author:   Jerry Wang
Version: 1.0
Date:	  2009/09/17
Description:
	This is the main interface of the Share Rest API. 
	This is the helper class for accessing the Adobe Share API, it provides much of the client-side functionality for applications. 
	for example , loging in and out, uploading files, listing files and folders, shareing files,and so on.
	These functions build the requests with the appropriate XML payload and parse the responses.
Memo:

*******************************************************************************************/

#pragma once
#include "stdafx.h"

#include  "ShareAPIUser.h"
#include  "ShareAPINodeParent.h"

const int LEN_AUTHTOKEN =				64;								//The length of the AuthToken Array.
const int LEN_AUTHHEADER =				768;
const int LEN_AUTHREQUESTBODY =	256;
const int LEN_AUTHRESPONSE	 =			256;
const int LEN_SESSIONREQUESTBODY	 = 256;
const int LEN_SESSIONRESPONSE =	256;
const int LEN_URLFULLPATH =				512;
const int LEN_UPLOADRESPONSE = 2048;
const int BUFFERSIZE	=	8192;	//	65536;//32768;//	16384;//					//the size of the buffer used in downloaing/uploading file


class ShareAPI
{
private:
	HINTERNET m_hOpen;     
	HINTERNET m_hConnect;
	ShareAPIUser*  m_pUser;
public:
	ShareAPI(LPCWSTR lpszUsrName, LPCWSTR lpszPwd); 
	~ShareAPI(void);
private:
	//prevent for copy and assignment of this object.
	ShareAPI(const ShareAPI &orig);
	ShareAPI& operator = (const ShareAPI &orig);

public:

	void Login(); 
	void Logout();
	
	void ResetConnection();

	MSXML2::IXMLDOMNodePtr UploadFile(LPCWSTR lpszFileFullPathName, LPCWSTR lpszFolderNodeId,  LPCWSTR lpszDescription = NULL, HWND hMsg = NULL);
	void GetFile(LPCWSTR lpszFileNodeId, LPCWSTR lpszFileName, LPWSTR lpszFileFullPathName, HWND hMsg = NULL);
	void SaveFileTo(LPCWSTR lpszFileNodeId, LPCWSTR lpszFileFullPathName, HWND hMsg = NULL);

	MSXML2::IXMLDOMNodePtr AddFolder(LPCWSTR lpszParentNodeId, LPCWSTR lpszFolderName, LPCWSTR lpszDescription = NULL);
	MSXML2::IXMLDOMDocumentPtr GetNodeList(LPCWSTR lpszFolderNodeId);	
	
	BOOL DeleteNode(LPCWSTR lpszNodeId);
	BOOL RenameNode(LPCWSTR lpszNodeId, LPCWSTR lpszNewName);
	BOOL MoveNode(LPCWSTR lpszNodeId, LPCWSTR lpszName, LPCWSTR lpszDestFolderId);  
	BOOL ShareFile(LPCWSTR lpszFileNodeId, LPWSTR lpszUsers, LPCWSTR lpszSubject, LPCWSTR lpszMessage=NULL, int nLevel =2);  

private:
	void generateAuthorizationHeader(LPCWSTR lpszMethod, LPCWSTR lpszUrl, LPWSTR lpszAuthStr);
	void getAuthToken( LPWSTR lpszAuthToken);
	void createSession(LPCWSTR lpszAuthToken);
	void addAuthHeader(HINTERNET hRequest, LPCWSTR lpszVerb, LPCWSTR lpszUrl);

	void assertUserLoggedIn();

	BOOL sendMoveRequest( LPCWSTR lpszUrl);
	void	doDownload(LPCWSTR lpszFileNodeId, HANDLE hFile, HWND hMsg ); //　getfile()和savefileto()公用的方法
	void	doUpload(  LPCWSTR lpszFileFullPathName, LPCWSTR lpszFolderNodeId, LPCWSTR lpszDescription=NULL, HWND hMsg=NULL, BOOL isRetry=FALSE,  CHAR* szResponse=NULL) ;

	void  initConnection();
};


