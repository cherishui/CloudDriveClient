#include "stdafx.h"
#include "ShareAPIResponse.h"
#include "ShareAPINode.h"


ShareAPIResponse::ShareAPIResponse(LPCSTR lpszResponse)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (S_OK !=CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (void**)&m_xmlDomDocPtr))
	{
		throw runtime_error("DOM object create failed");
	} 
	
	//must convert the multibyte string to widechar, otherwise, some Chinese/japanese... characters cannot be displayed correctly
	int nDimension = strlen(lpszResponse)+1;
	WCHAR* szResponseWide = new WCHAR[nDimension]();
	MultiByteToWideChar(CP_UTF8, 0, lpszResponse, -1, szResponseWide, nDimension); 
		
	m_xmlDomDocPtr->loadXML(_bstr_t(szResponseWide));  
	
	delete [] szResponseWide;
}

ShareAPIResponse::~ShareAPIResponse(void)
{
	if (m_xmlDomDocPtr != NULL)
		m_xmlDomDocPtr.Release();
	CoUninitialize();
}



/**************************************************************************
Get if the response status is OK ( if the http request  has been well processed)

			<response status="ok">
			   <sessionid>1ab1a6190dc1f1f849ca7649190cca83</sessionid>
			   <secret>3965c47eb9c937f78d49f3f33e19ca45</secret>
			   <level>1</level>
			</response>
			
another form of response is :
		Badlogin
		NotVerified
		.................
*************************************************************************/
bool ShareAPIResponse::IsStatusOK()
{
	MSXML2::IXMLDOMNodePtr responseNodePtr = m_xmlDomDocPtr->selectSingleNode(_bstr_t("response"));
	if (!responseNodePtr)   //if not "<response status...> node,  then it will be a  error message
	{
		return false;
	}
	MSXML2::IXMLDOMNodePtr statusAttributePtr = responseNodePtr->attributes->getNamedItem(_bstr_t("status"));
	LPCWSTR lpszStatus = statusAttributePtr->GetnodeValue().bstrVal ;

	if (wcscmp(lpszStatus, L"ok") == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**************************************************************************
Get the value of the response node, like sessionId, secret......

a response example :
			<response status="ok">
			   <sessionid>1ab1a6190dc1f1f849ca7649190cca83</sessionid>
			   <secret>3965c47eb9c937f78d49f3f33e19ca45</secret>
			   <level>1</level>
			</response>
**************************************************************************/
LPCWSTR ShareAPIResponse::GetResponseNodeText(LPCWSTR lpszNodeName)
{
	if (!IsStatusOK())
	{
		throw runtime_error("Response Status is not right");
	}
	MSXML2::IXMLDOMNodePtr responseNodePtr = m_xmlDomDocPtr->selectSingleNode(_bstr_t("response"));
	return responseNodePtr->selectSingleNode(_bstr_t(lpszNodeName))->text;
}


/*****************************************************************
*******************************************************************/
MSXML2::IXMLDOMNodePtr ShareAPIResponse::GetNode()
{
	if (IsStatusOK())
	{
		return m_xmlDomDocPtr->selectSingleNode(_bstr_t("response/node"));
	}
	else
	{
		return NULL;
	}
}
