/*************************************************************************
	File: ShareAPINodeParent.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/22
	Description:
			ShareAPINodeParent is a class  to manipulate the ShareAPI file/folder structure XML.
			Notice: We do not offer Insert() Delete() or Rename() in this class.  Each time a  file/folder
			is created, deleted, or renamed. We simply call Initialize()  to refresh when XML is changed.
			
	Memo:

*****************************************************************************/

#include "StdAfx.h"
#include "ShareAPINodeParent.h"

ShareAPINodeParent::ShareAPINodeParent(LPCSTR lpszXmlStr)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);   //in this project, usually the result is S_FALSE, because the main window already call OleInitialize() before
	
	//create the domDocument object.
	MSXML2::IXMLDOMDocumentPtr xmlDomDocPtr;
	if (S_OK !=CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (void**)&xmlDomDocPtr))
	{
		throw runtime_error("DOM  Document object create failed!");
	} 
	
	xmlDomDocPtr->loadXML(_bstr_t(lpszXmlStr));
	//typecast  from IxmldomNode  to IxmldomElement
	MSXML2::IXMLDOMNodePtr  parentNodePtr = xmlDomDocPtr->selectSingleNode(_bstr_t("response/node"));
	parentNodePtr->QueryInterface(__uuidof(MSXML2::IXMLDOMElement), (void**)&m_parentElementPtr);
}

ShareAPINodeParent::ShareAPINodeParent(MSXML2::IXMLDOMNodePtr parentNodePtr)
{	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);   //in this project, usually the result is S_FALSE, because the main window already call OleInitialize() before

	parentNodePtr->QueryInterface(__uuidof(MSXML2::IXMLDOMElement), (void**)&m_parentElementPtr);
}

ShareAPINodeParent::~ShareAPINodeParent(void)
{
	CoUninitialize();
}


MSXML2::IXMLDOMNodeListPtr ShareAPINodeParent::GetNodeList() const
{
	MSXML2::IXMLDOMNodeListPtr childrenNodeListPtr = m_xmlDomDocPtr->selectNodes(_bstr_t("response/children/node"));
	return childrenNodeListPtr;
}




///***********************************************************************
//	Get the pointer of a ShareAPINode object from the list by  nodeId
//	Parameter: 
//		lpszNodeName: The nodeid of the file/folder .
//	Return:
//		if succeed, the pointer of the file/folder node.
//		if failed, NULL.
//***********************************************************************/
//MSXML2::IXMLDOMElementPtr  ShareAPINodeParent::GetNodeById(LPCWSTR lpszNodeId) 
//{
//	m_parentElementPtr->selectSingleNode(
//}


LPCWSTR ShareAPINodeParent::GetParentId() const
{
	return m_parentElementPtr->getAttribute("nodeid").bstrVal;
}

LPCWSTR ShareAPINodeParent::GetParentName() const
{
	return  m_parentElementPtr->getAttribute("name").bstrVal;
}

LPCWSTR ShareAPINodeParent::GetParentDescription() const
{
	return m_parentElementPtr->getAttribute("description").bstrVal;
}

INT64 ShareAPINodeParent::GetParentCreateDate() const
{
	return  _wtoi64(m_parentElementPtr->getAttribute("createddate").bstrVal);
}

INT64 ShareAPINodeParent::GetParentModifiedDate() const
{
	return _wtoi64(m_parentElementPtr->getAttribute("modifieddate").bstrVal);
}


LPCWSTR ShareAPINodeParent::GetParentOwner() const
{
	return m_parentElementPtr->getAttribute("owner").bstrVal;
}


LPCWSTR ShareAPINodeParent::GetParentOwnerName() const
{
	return m_parentElementPtr->getAttribute("ownername").bstrVal;
}
