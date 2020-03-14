#include "StdAfx.h"
#include "ShareAPINode.h"

ShareAPINode::ShareAPINode(const MSXML2::IXMLDOMElementPtr nodePtr)
		:m_nodePtr(nodePtr)
{
	m_bIsFile = HasContent();
}

ShareAPINode::ShareAPINode(MSXML2::IXMLDOMNodePtr nodePtr)
{
	nodePtr->QueryInterface(__uuidof(MSXML2::IXMLDOMElement), (void**)&m_nodePtr);
	m_bIsFile = HasContent();
}

ShareAPINode::~ShareAPINode(void)
{
}

LPCWSTR ShareAPINode::GetNodeId()
{
	return m_nodePtr->getAttribute("nodeid").bstrVal;
}

LPWSTR ShareAPINode::GetName()
{
	return  m_nodePtr->getAttribute("name").bstrVal;
}

HRESULT ShareAPINode::SetName(LPCWSTR lpszNewName)
{
	return m_nodePtr->setAttribute("name", _bstr_t(lpszNewName));
}

LPCWSTR ShareAPINode::GetDiscription()
{
	return  m_nodePtr->getAttribute("description").bstrVal;
}

INT64 ShareAPINode::GetCreatedDate()
{
	return _wtoi64(m_nodePtr->getAttribute("createddate").bstrVal);
}

INT64 ShareAPINode::GetModifiedDate()
{
	return   _wtoi64(m_nodePtr->getAttribute("modifieddate").bstrVal);
}

LPCWSTR ShareAPINode::GetOwner()
{
	return  m_nodePtr->getAttribute("owner").bstrVal;
}

LPCWSTR ShareAPINode::GetOwnerName()
{
	return  m_nodePtr->getAttribute("ownername").bstrVal;
}

bool ShareAPINode::IsDirectory()
{
	return ( _wcsicmp(m_nodePtr->getAttribute("directory").bstrVal , L"true") == 0 );
}

bool ShareAPINode::HasContent()
{
	return   ( _wcsicmp(m_nodePtr->getAttribute("hascontent").bstrVal, L"true") ==0 );
}

bool ShareAPINode::HasLink()
{
	return  ( _wcsicmp(m_nodePtr->getAttribute("link").bstrVal, L"true") ==0 );
}

/*************************************************
Get the size information of the file/folder 

Return value:
	File node: the bytes of the file.
	Folder node:  0
**************************************************/
INT64 ShareAPINode::GetFileSize()
{
	if (m_bIsFile)
	{
		return _wtoi64(m_nodePtr->getAttribute("filesize").bstrVal);
	}
	else
	{
		return 0;
	}
}

/**************************************************************
Get Mime type of the file.
Return value:
	File node: the mime type of the file.
	Folder node: NULL.
***************************************************************/
LPCWSTR ShareAPINode::GetMimeType()
{
	if (m_bIsFile)
	{
		return m_nodePtr->getAttribute("mimetype").bstrVal;
	}
	else
	{
		return NULL;
	}
}

/******************************************************************
Get the Author of the file
Return value:
	File node:  the author of the file, usually empty string.
	Folder node: NULL
*****************************************************************/
LPCWSTR ShareAPINode::GetAuthor()
{
	if(m_bIsFile)
	{
		return m_nodePtr->getAttribute("author").bstrVal;
	}
	else
	{
		return NULL;
	}
}

/*****************************************************************
Get the ThumbnailState of the file
Return value:
	File Node: the thumbnailState of the file.
				-1: Failed
				 0: Queued
				 1: Complete
	Folder node: -1;
****************************************************************/
int ShareAPINode::GetThumbnailState()
{
	if (m_bIsFile)
	{
		return _wtoi(m_nodePtr->getAttribute("thumbnailstate").bstrVal);
	}
	else
	{
		return -1;
	}
}

/*****************************************************************
* Gets the flash preview state for the requested node. Share can return one the following values: 
FileNode:
		-1: Failed
		 0: Queued
		1: Complete
FolderNode: -1
*****************************************************************/

int ShareAPINode::GetFlashPreviewState()
{
	if (m_bIsFile)
	{
		return  _wtoi(m_nodePtr->getAttribute("flashpreviewstate").bstrVal);
	}
	else
	{
		return -1;
	}
}
/*******************************************************************
Get the flash preview page count for the file. only has meaning when the flashpreviewstate is 1.
Return value:
	File node: the page number of the flash preview. if it cannot preview by flash,  return 0;
	Folder node: 0
********************************************************************/
int ShareAPINode::GetFlashPreviewPageCount()
{
	if (m_bIsFile)
	{
		return _wtoi(m_nodePtr->getAttribute("flashpreviewpagecount").bstrVal);
	}
	else
	{
		return 0;
	}
}

/***********************************************************************
	 * Gets the PDF state for the requested node. 
	 Return value:
		File node: 
		   -1: Failed
			0: Unknown
			1: Not available for download
			2: Available for download
			3: Converting
		Folder node;
			-1: always

*************************************************************************/

int ShareAPINode::GetPdfState()
{
	if (m_bIsFile)
	{
		return _wtoi(m_nodePtr->getAttribute("pdfstate").bstrVal);
	}
	else
	{
		return -1;
	}
}

/**************************************************************************
* Gets the URL to the file node that can be shared with uses that have privileges that  will allow them to access the document. 
	 * For example, https://share.acrobat.com/adc/document.do?dod9313. 

	 for the Folder node, return NULL.
*********************************************************************/
LPCWSTR ShareAPINode::GetRecipientUrl()
{
	if (m_bIsFile)
	{
		 return m_nodePtr->getAttribute("recipienturl").bstrVal;
	}
	else
	{
		return NULL;
	}
}

/**********************************************************************************
 * Gets the requested node's share level. This method can be used to get a node's share  level from Share's XML response. 
 The share level is set in the original share  request by the owner when the {@link ShareAPI#shareFile shareFile} method is called.
 Share returns one the following values: 
	 0: Private
	 1: Specified users only
	 2: Public

 for folder node, alwasy return 0;
*********************************************************************************/
int ShareAPINode::GetShareLevel()
{
	if (m_bIsFile)
	{
		return  _wtoi(m_nodePtr->getAttribute("sharelevel").bstrVal);
	}
	else
	{
		return 0;
	}
}

/******************************************************************************** 
Get the list of owner specified recipients that have access to the file. This is  most often when the share level is set to 1, 
since 0 prohibits sharing and 2 makes  the share public. For details, see {@link ShareAPI#shareFile shareFile} and {@link
 ShareAPINode#SHARE_USERS SHARE_USERS} 
	 

//*********************************************************************************/ 
//vector<LPCWSTR> ShareAPINode::GetRecipients()
//{
//	vector<LPCWSTR> recipients;
//
//	if (m_bIsFile)
//	{
//		MSXML2::IXMLDOMNodeListPtr recipientsListPtr = m_nodePtr->selectSingleNode(_bstr_t("recipients"))->GetchildNodes();
//		for (int i=0; i<recipientsListPtr->Getlength(); i++)
//		{
//			recipients.push_back(	recipientsListPtr->Getitem(0)->Gettext() );
//		}
//	}
//
//	return recipients;
//}
//
//
