/*************************************************************
	File: ShareAPIResponse.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/24
	Description:
		This class is used to get information from the response xml of the Share request.
		like below:
			<response status="ok">
			   <sessionid>1ab1a6190dc1f1f849ca7649190cca83</sessionid>
			   <secret>3965c47eb9c937f78d49f3f33e19ca45</secret>
			   <level>1</level>
			</response>

	another form of response is :
		Badlogin
		NotVerified
	

	Memo:

*************************************************************/

#pragma once
#include "stdafx.h"

class ShareAPINode;

class ShareAPIResponse
{
private:
	MSXML2::IXMLDOMDocumentPtr  m_xmlDomDocPtr;
public:
	ShareAPIResponse(LPCSTR lpszResponse);  
	~ShareAPIResponse(void);
public:
	bool IsStatusOK();
	LPCWSTR GetResponseNodeText(LPCWSTR lpszNodeName);
	MSXML2::IXMLDOMNodePtr  GetNode();

};
