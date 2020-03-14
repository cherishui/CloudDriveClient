/*************************************************************
	File: ShareAPINode.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/20
	Description:
			ShareAPINode object is used to represent each item in the xml response file describing the files/folders item.
			a ShareAPINode objext can be either a file node or a folder node. 

	Memo:

*************************************************************/

#pragma once
#include "stdafx.h"


class ShareAPINode 
{
private:
	MSXML2::IXMLDOMElementPtr m_nodePtr;

	bool m_bIsFile;  // file:true  folder:false;
  
public:
	ShareAPINode( MSXML2::IXMLDOMElementPtr nodePtr);
	ShareAPINode( MSXML2::IXMLDOMNodePtr nodePtr);
	~ShareAPINode(void);

public:
		LPCWSTR GetNodeId() ;
		LPWSTR GetName() ;
		
		LPCWSTR GetDiscription() ;
		INT64	GetCreatedDate() ;
		INT64	GetModifiedDate() ;
		LPCWSTR	GetOwner() ;
		LPCWSTR	GetOwnerName();
		bool		IsDirectory();
		bool		HasContent();
		bool		HasLink();

		INT64	GetFileSize();
		LPCWSTR	GetMimeType();
		LPCWSTR	GetAuthor();
		int	GetThumbnailState();
		int	GetFlashPreviewPageCount();
		int	GetFlashPreviewState();
		int	GetPdfState();
		LPCWSTR	GetRecipientUrl();
		int	GetShareLevel();

public:
		HRESULT  SetName(LPCWSTR lpszNewName);  
};
