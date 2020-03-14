#include "StdAfx.h"
#include "ShareAPIConsts.h"
#include "ShareAPISession.h"
#include "WinErrorException.h"
#include "ShareAPIException.h"

/************************************************************
ShareAPISession Constructor
Initialize member variables and Setup the Internet connection
Exception:
	WinErrorException
*************************************************************/
ShareAPISession::ShareAPISession(void)
	:m_lpszAuthToken(NULL), m_lpszAuthHeader(NULL)
{
	m_hOpen = InternetOpenW(L"AdobeShare",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL, NULL);
	if (!m_hOpen)
	{	
		DWORD dwErrCode = ::GetLastError();   
		throw WinErrorException(dwErrCode, L"InternetOpen");   
	}

	m_hConnect = InternetConnectW(m_hOpen,SHARE_HOSTNAME,INTERNET_DEFAULT_HTTPS_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);  
	if (!hConnect)
	{
		DWORD dwErrCode = ::GetLastError();   
		InternetCloseHandle(m_hOpen);
		throw WinErrorException(dwErrCode,L"InternetConnect");
	}
}

/*******************************************************
Destructor: 
		Close Internet connection   and  release all the memory allocations.
*********************************************************/
ShareAPISession::~ShareAPISession(void)
{
	if (m_hOpen)
	{
		InternetCloseHandle(hOpen);   
	}

	if (m_lpszAuthToken)
	{
		delete [] m_lpszAuthToken;
	}
	
	if (m_lpszAuthHeader)
	{
		delete [] m_lpszAuthHeader;
	}
}

/*******************************************************
 Login(ShareAPIUser*) constructs a log in request so the user can login to Share. 
	  If successful, user object will have its session ID and secret attributes set.
Parameter:
	pUser: the pointer of the user to login
Exception:
	WinErrorException
	ShareAPIException
	std::runtime_error

*****************************************************/
void ShareAPISession::Login(ShareAPIUser *pUser)
{
	if (!pUser)
	{
		throw runtime_error("ShareAPI user is NULL! Cannot login");
	}
	getAuthToken(pUser);
	createSession(pUser);
}

/******************************************************************
get authentication token  for the user,  set the vaule of m_lpszAuthToken
Parameter:
	[in] puser : the ShareAPI user;
exception:   
	WinErrorException
	runtime_error
	ShareAPIException

******************************************************************/
void ShareAPISession::getAuthToken(ShareAPIUser *pUser)
{

	HINTERNET hRequest= HttpOpenRequest(m_hConnect,L"POST", SHARE_URLAUTH, NULL, NULL, NULL,INTERNET_FLAG_SECURE, 0);
	if (!hRequest)
	{
		DWORD dwErrCode = ::GetLastError();   
		throw WinErrorException(dwErrCode, L"HttpOpenRequest() faild in GetAuthToken()! ");   
	}
	//add authentication header
	WCHAR szHeader[] = L"Content-Type:application/xml;charset=UTF-8\r\n"; 
	HttpAddRequestHeaders(hRequest, szHeader, wcslen(szHeader), HTTP_ADDREQ_FLAG_REPLACE|HTTP_ADDREQ_FLAG_ADD);
	
	WCHAR szAuthHeaderStr[LEN_AUTHHEADER]=L"";
	generateAuthorizationHeader(L"POST", SHARE_URLAUTH, authHeaderStr);  
	if (!HttpAddRequestHeaders(hRequest,authHeaderStr, wcslen(authHeaderStr), HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE))
	{
		DWORD errCode = GetLastError();
		throw WinErrorException(errCode, L" add authentication header failed in HttpAddRequestHeader() !");     
	}
	//Send Request with Request Body.   NOTICE:request body cannot be WideChar!! 
	WCHAR requestBodyWideChar[LEN_AUTHREQUESTBODY]=L"";
	wcscat_s(requestBodyWideChar, L"<request>");
	wcscat_s(requestBodyWideChar, L"<username>");
	wcscat_s(requestBodyWideChar,  pUser->GetUserName());
	wcscat_s(requestBodyWideChar, L"</username>");
	wcscat_s(requestBodyWideChar, L"<password>");
	wcscat_s(requestBodyWideChar, pUser->GetPassword());
	wcscat_s(requestBodyWideChar, L"</password>");
	wcscat_s(requestBodyWideChar, L"</request>");
	CHAR requestBody[LEN_AUTHREQUESTBODY];
	WideCharToMultiByte(CP_UTF8,0,requestBodyWideChar,-1,requestBody,LEN_AUTHREQUESTBODY,NULL,NULL);

	if (!HttpSendRequestW(hRequest,NULL,0,requestBody, strlen(requestBody)))
	{
		DWORD errCode = GetLastError();
		throw WinErrorException(errCode, L"Send Http Request failed.");   
	}

	//Get Response Information
	CHAR responseData[LEN_AUTHRESPONSE];
	DWORD numOfByte=0;
	if (!InternetReadFile(hRequest, (LPVOID)responseData, LEN_AUTHRESPONSE, &numOfByte))
	{
		DWORD errCode = GetLastError();
		throw WinErrorException(errCode, L"InternetReadFile Failed");   
	}
	//add null terminator to the string.
	responseData[numOfByte]='\0';

	WCHAR response[LEN_AUTHRESPONSE];
	MultiByteToWideChar(CP_UTF8,0, responseData, -1, response, LEN_AUTHRESPONSE);

	DOMResponse* xmlDom = new DOMResponse();
	xmlDom->LoadXmlStr(response);
	WCHAR responseStatus[16];
	xmlDom->GetResponseStatus(responseStatus);

	if (wcscmp(responseStatus, L"ok") !=0 )
	{
		throw runtime_error("Response Status is not right");
	}
	xmlDom->GetResponseNodeText(L"authtoken", authToken);
	delete xmlDom;




}




/*
Generates an authorization string for a request to start a session with the Share Service.
For details, refer to the API specification 
*[in]method: POST,GET,DELETE,PUT
*[in]url:   such as  https://api.share.acrobat.com/webservices/api/v1/dc 
*[out]authStr: the buffer to hold the result authorization header string,  it's size should be 512.

return the size of the ...
*/
size_t ShareAPI::generateAuthorizationHeader( LPCWSTR method, LPCWSTR url, LPWSTR authStr)
{
	wcscpy_s(authStr, LEN_AUTHHEADER, L"Authorization:AdobeAuth ");

	if (pUser->GetSessionId() != 0 && pUser->GetSessionId()[0]!=0)
	{
		wcscat_s(authStr,LEN_AUTHHEADER,L"sessionid=\"");
		wcscat_s(authStr,LEN_AUTHHEADER,pUser->GetSessionId());
		wcscat_s(authStr,LEN_AUTHHEADER,L"\",");
	}
	wcscat_s(authStr,LEN_AUTHHEADER, L"apikey=\"");
	wcscat_s(authStr,LEN_AUTHHEADER,ShareAPI::APIKEY);
	wcscat_s(authStr,LEN_AUTHHEADER,L"\",");

	//   data="POST https://api.share.acrobat.com/webservices/api/v1/auth/ 1196363602050",
	WCHAR millSecond[16];
	_i64tow_s(HelperFunctions::GetCurrentTimeStamp(), millSecond, 16, 10);
	WCHAR dataStr[256]=L"";
	wcscat_s(dataStr, method);
	wcscat_s(dataStr, L" ");
	wcscat_s(dataStr,L"https://api.share.acrobat.com"); 
	wcscat_s(dataStr, url);
	wcscat_s(dataStr, L" ");
	wcscat_s(dataStr, millSecond);
	wcscat_s(authStr, LEN_AUTHHEADER, L"data=\"");
	wcscat_s(authStr, LEN_AUTHHEADER,dataStr);
	wcscat_s(authStr, LEN_AUTHHEADER, L"\",");

	//sig: The MD5 digest of the string formed by concatenating data with the shared secret returned by an authorization token request. 
	WCHAR sigStr[256]=L"";  
	wcscpy_s(sigStr, dataStr);
	if (pUser->GetSessionId()[0] != 0)
	{
		wcscat_s(sigStr,pUser->GetSecret()); 
	}
	else
	{
		wcscat_s(sigStr,this->SHAREDSECRET);  
	}
	WCHAR sigStrMD5[33];   
	HelperFunctions::MD5Hash(sigStr, sigStrMD5);

	wcscat_s(authStr,LEN_AUTHHEADER, L"sig=\"");
	wcscat_s(authStr,LEN_AUTHHEADER,sigStrMD5);
	wcscat_s(authStr,LEN_AUTHHEADER, L"\"");
}









ShareAPISession::Logout(ShareAPIUser *pUser)
{
	if (isUserLoggedIn(pUser))
	{


		//send request to logout
		...............


		pUser->SetSessionId(NULL);
		pUser->SetSecret(NULL);
	}


}
