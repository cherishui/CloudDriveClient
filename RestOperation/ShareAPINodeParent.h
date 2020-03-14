/*************************************************************************
	File: ShareAPINodeParent.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/22
	Description:
			ShareAPINodeParent is a class  to manipulate the ShareAPI file/folder structure XML.
			Notice: We do not offer Insert() Delete() or Rename() in this class.  Each time a  file/folder
			is created, deleted, or renamed. We simply call Initialize()  to refresh it when XML is changed.
			
	Memo:

*****************************************************************************/

#pragma once

class ShareAPINodeParent
{
private:
	MSXML2::IXMLDOMElementPtr  m_parentElementPtr;

public:
	ShareAPINodeParent(LPCSTR lpszXmlStr); 
	ShareAPINodeParent(MSXML2::IXMLDOMNodePtr parentNodePtr);
	~ShareAPINodeParent(void);

	//MSXML2::IXMLDOMElementPtr GetNodeById(LPCWSTR lpszNodeId) ;
	MSXML2::IXMLDOMNodeListPtr  GetNodeList() const;

	LPCWSTR GetParentId() const;
	LPCWSTR GetParentName() const;
	LPCWSTR GetParentDescription() const;
	INT64	GetParentCreateDate() const;
	INT64	GetParentModifiedDate() const;
	LPCWSTR GetParentOwner() const;
	LPCWSTR GetParentOwnerName() const;
};
