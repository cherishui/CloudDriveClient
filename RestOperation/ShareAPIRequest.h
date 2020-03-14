/******************************************************************************************
FileName: ShareAPIRequest.h
Author:   Jerry Wang
Version: 1.0
Date:	  2009/09/17
Description:
	 This class is responsible for the Adobe Share API operation functionalities,  For example, uploading/downloading files, 
	 creating subfolders, renaming, and so on.
	  Usually, one instance of  ShareAPIRequest is corresponding to one thread.
Memo:

*******************************************************************************************/

#pragma once

const int BUFFERSIZE	=						4096;							//the size of the buffer used in downloaing/uploading file


class ShareAPIRequest
{
public:
	ShareAPIRequest( ShareAPISession / ShareAPIUser??);
	~ShareAPIRequest(void);
public: 
	void UploadFile (HANDLE  hFile, LPCWSTR lpszFileName, LPCWSTR lpszDescription=NULL);
	HANDLE GetFile (LPCWSTR lpszFileNodeId, LPWSTR lpszFilePath, LPCWSTR lpszFileExt);
	HANDLE GetFileThumbnail (LPCWSTR lpszFileNodeId);
	void ShareFile (LPCWSTR lpszFileNodeId, LPCWSTR lpszUserMails, LPCWSTR lpszSubject, LPCWSTR lpszMessage, int nLevel);  //users可以包含多个email,用 ; 分割

	void AddFolder (LPCWSTR lpszFolderName, LPCWSTR lpszDescription=NULL);
	void GetFilesAndFoldersList (LPCWSTR lpszFolderNodeId);
	
	void DeleteNode (LPCWSTR lpszNodeId);
	void RenameNode (LPCWSTR lpszNodeId, LPCWSTR lpszNewName);
	void MoveNode (LPCWSTR lpszNodeId, LPCWSTR lpszNewFolderId);  

	void	SetCurrentFolder (LPCWSTR lpszFolderNodeId);
private:
	static WCHAR szRootNodeId[64];					

public:
	DOMNodeList*  pNodeList;			//	the pointer to current nodes(files/folders) list 

};
