/****************************************************************
This is the main file for Share API functions
Author: Jerry wang (王建华)
All right reserved.
*************************************************************/
#include "StdAfx.h"
#include "ShareAPIConsts.h"
#include "WinErrorException.h"
#include "ShareAPIException.h"
#include "HelperFunctions.h"   
#include "ShareAPI.h"
#include "ShareAPINode.h"
#include "ShareAPIResponse.h"
#include "Commctrl.h"
#include "..\CloudDrive\resource.h"


extern BOOL g_bUserCanceled;  //if the current thread is cancelled by the user


/********************************************************************* 
ShareAPI constructor:
	Create the ShareApiUser object.
	Internet connection to Adobe Share Service is opened and initialized.
Exception: 
	WinErrorException
********************************************************************/
ShareAPI::ShareAPI(LPCWSTR lpszUsrName, LPCWSTR lpszPwd)
			:m_pUser(new ShareAPIUser(lpszUsrName, lpszPwd) )  
{
	initConnection();
}

//Destructor
ShareAPI::~ShareAPI(void)
{ 
	delete m_pUser;  

	if (m_hOpen)
	{
		InternetCloseHandle(m_hOpen);   
	}
}

/********************************************
Setup the internet connection
***************************************************/
void ShareAPI::initConnection()
{	
	//initialize the Http connection
	m_hOpen = InternetOpenW(L"AdobeShare",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL, NULL);
	if (!m_hOpen)
	{	
		DWORD dwErrCode = ::GetLastError();   
		throw WinErrorException(dwErrCode, L"InternetOpen Failed!");   
	}

	m_hConnect = InternetConnectW(m_hOpen,SHARE_HOSTNAME,INTERNET_DEFAULT_HTTPS_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);  
	if (!m_hConnect)
	{
		DWORD dwErrCode = ::GetLastError();   
		InternetCloseHandle(m_hOpen);
		throw WinErrorException(dwErrCode,L"InternetConnect Failed!");
	}
}

/******************************************************
in some circumestance, connection must be reset 
*******************************************************/
void ShareAPI::ResetConnection()
{
	InternetCloseHandle(m_hOpen);
	initConnection();
}

/*******************************************************
 Login(ShareAPIUser*) constructs a log in request so the user can login to Share. 
	  If successful, user object will have its session ID and secret attributes set.
	 Login has two steps:	1. Requesting an authorization token;
						2. Starting a new session
Exception:
	WinErrorException
	ShareAPIException
	std::runtime_error
*****************************************************/
void ShareAPI::Login()
{
	if (!m_pUser)
	{	
		throw runtime_error("ShareAPI user is NULL! Cannot login");
	}

	if (m_pUser->GetSessionId() != NULL && m_pUser->GetSecret() != NULL)
	{
		return;
	}

	WCHAR szAuthToken[LEN_AUTHTOKEN] = {0};
	getAuthToken(szAuthToken);
	createSession(szAuthToken);
}

/********************************************************************
Logout():   logs a user out by ending the current session.  
the  session is ended by constructing an HTTP request using the DELETE method and the current session ID. 
	 
*********************************************************************/
void ShareAPI::Logout()
{
	assertUserLoggedIn(); 

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLSESSION);
	wcscat_s(szRequestPath, m_pUser->GetSessionId());
	wcscat_s(szRequestPath, L"/");

	//open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"DELETE", szRequestPath,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	//add authentication header
	addAuthHeader(hRequest, L"DELETE", szRequestPath);
	
	//send request
	HttpSendRequest(hRequest,NULL,0,NULL, 0);

	//set the m_pUser
	m_pUser->SetSessionId(NULL);
	m_pUser->SetSecret(NULL);
	m_pUser->SetName(NULL);
}

/*******************************************************************************
//add header to the http request
**************************************************************************/
void ShareAPI::addAuthHeader(HINTERNET hRequest, LPCWSTR lpszVerb, LPCWSTR lpszUrl)
{
	WCHAR szHeader[] = L"Content-Type:application/xml;charset=UTF-8\r\n"; 
	HttpAddRequestHeaders(hRequest, szHeader, wcslen(szHeader), HTTP_ADDREQ_FLAG_REPLACE|HTTP_ADDREQ_FLAG_ADD);
	WCHAR szAuthHeader[LEN_AUTHHEADER]=L"";
	generateAuthorizationHeader(lpszVerb, lpszUrl, szAuthHeader);  
	
	HttpAddRequestHeaders(hRequest,szAuthHeader, wcslen(szAuthHeader), HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
}


/**************************************************************
get authentication token  for the user 
Parameter:
	[out] lpszAuthToken: the buffer to hold  the returned authentication token for this user
Exception:
	   WinErrorExceptionoo9o.
	   ShareAPIException
	   runtime_error)
****************************************************************/
void ShareAPI::getAuthToken(LPWSTR lpszAuthToken) 
{
	//open Request 
	HINTERNET hRequest= HttpOpenRequest(m_hConnect,L"POST", SHARE_URLAUTH, NULL, NULL, NULL,INTERNET_FLAG_SECURE, 0);

	//add authentication header*************************************** 
	addAuthHeader(hRequest, L"POST", SHARE_URLAUTH);

	//Send Request with Request Body.   NOTICE:request body cannot be WideChar!! 
	WCHAR szRequestBodyWideChar[LEN_AUTHREQUESTBODY]=L"";
	wcscat_s(szRequestBodyWideChar, L"<request>");
	wcscat_s(szRequestBodyWideChar, L"<username>");
	wcscat_s(szRequestBodyWideChar,  m_pUser->GetUserName());
	wcscat_s(szRequestBodyWideChar, L"</username>");
	wcscat_s(szRequestBodyWideChar, L"<password>");
	wcscat_s(szRequestBodyWideChar, m_pUser->GetPassword());
	wcscat_s(szRequestBodyWideChar, L"</password>");
	wcscat_s(szRequestBodyWideChar, L"</request>");
	
	//convert widechar Request body to mutibute version
	UINT nSize =  WideCharToMultiByte(CP_UTF8,0,szRequestBodyWideChar,-1,NULL,0,NULL,NULL);   //get the size of the multibyte verion of body. it can be different from the widechar verison
	
	char* lpszRequestBody = new char[nSize]();
	WideCharToMultiByte(CP_UTF8, 0, szRequestBodyWideChar, -1, lpszRequestBody, nSize, NULL, NULL);
	
	//send request
	if (!HttpSendRequestW(hRequest,NULL,0,lpszRequestBody, strlen(lpszRequestBody)))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get authentication token failed!");   
	}
	delete [] lpszRequestBody;	

	//Get Response Information  
	CHAR szResponse[LEN_AUTHRESPONSE+1];
	DWORD dwNumOfByte=0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LEN_AUTHRESPONSE, &dwNumOfByte))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get authentication token failed!");   
	}
	szResponse[dwNumOfByte]='\0';	//add null terminator to the string.

	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); //thread specific, so need to call it here to make sure COM is initialzed!
	
	MSXML2::IXMLDOMDocumentPtr  xmlDomDocPtr;
	if (S_OK !=CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (void**)&xmlDomDocPtr))
	{
		CoUninitialize();  //uninitialize COM
		throw runtime_error("DOM object create failed");
	} 
	
	xmlDomDocPtr->loadXML(_bstr_t(szResponse));
	
		
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);
	if (pResponse->IsStatusOK())
	{
		wcscpy(lpszAuthToken, pResponse->GetResponseNodeText(L"authtoken"));
		delete pResponse;
	}
	else  //error happened. throw exception
	{	
		delete pResponse;
		CoUninitialize();  //uninitialize COM
		throw ShareAPIException(szResponse);  
	}
	
	CoUninitialize();  //uninitialize COM

}


/******************************************************************
create session, and use the session to login the user.
If successful, user object will have its session ID and secret attributes set (logged in)
Parameter:
	[in] lpszAuthToken:  authentication token
Exception:
	   WinErrorException
	   ShareAPIException
	   runtime_error
*******************************************************************/

void  ShareAPI::createSession( LPCWSTR lpszAuthToken)
{
	HINTERNET hRequest= HttpOpenRequest(m_hConnect,L"POST", SHARE_URLSESSION, NULL, NULL, NULL,INTERNET_FLAG_SECURE, 0);

	//add authentication header
	addAuthHeader(hRequest, L"POST", SHARE_URLSESSION);
	
	//add body content and send the httprequest
	WCHAR szRequestBodyWideChar[LEN_SESSIONREQUESTBODY]=L"";
	wcscat_s(szRequestBodyWideChar, L"<request>\n");
	wcscat_s(szRequestBodyWideChar, L"<authtoken>");
	wcscat_s(szRequestBodyWideChar,  lpszAuthToken);
	wcscat_s(szRequestBodyWideChar, L"</authtoken>\n");
	wcscat_s(szRequestBodyWideChar, L"</request>\n");
	
	//convert widechar Request body to mutibute version
	UINT nSize =  WideCharToMultiByte(CP_UTF8,0,szRequestBodyWideChar,-1,NULL,0,NULL,NULL);   //get the size of the multibyte verion of body. it can be different from the widechar verison
	
	char* lpszRequestBody = new char[nSize]();
	WideCharToMultiByte(CP_UTF8, 0, szRequestBodyWideChar, -1, lpszRequestBody, nSize, NULL, NULL);
	
	//send request
	if (!HttpSendRequestW(hRequest,NULL,0,lpszRequestBody, strlen(lpszRequestBody)))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Create session failed!");   
	}
	delete [] lpszRequestBody;	

	//get response data
	CHAR szResponse[LEN_SESSIONRESPONSE+1];
	DWORD dwNumOfByte=0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LEN_SESSIONRESPONSE, &dwNumOfByte))
	{
		DWORD errCode = ::GetLastError();
		throw WinErrorException(errCode, L"Create session failed!");   
	}
	szResponse[dwNumOfByte] = '\0';

	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);
	if (!pResponse->IsStatusOK())
	{
		delete pResponse;
		throw  ShareAPIException(szResponse);
	}
	//setup the user (logged in)
	m_pUser->SetSessionId(pResponse->GetResponseNodeText(L"sessionid") );
	m_pUser->SetSecret(pResponse->GetResponseNodeText( L"secret"));
	m_pUser->SetName(pResponse->GetResponseNodeText( L"name") );
	delete pResponse;
}



/**********************************************************************
Generates an authorization string for a request to start a session with the Share Service.
For details, refer to the API specification 
*[in]method: POST,GET,DELETE,PUT
*[in]url:   such as  https://api.share.acrobat.com/webservices/api/v1/dc 
*[out]authStr: the buffer to hold the result authorization header string,  it's size should be 512.
*****************************************************************************/
void ShareAPI::generateAuthorizationHeader( LPCWSTR lpszMethod, LPCWSTR lpszUrl, LPWSTR lpszAuthStr)
{
	wcscpy_s(lpszAuthStr, LEN_AUTHHEADER, L"Authorization:AdobeAuth ");

	if (m_pUser->GetSessionId() != 0 && m_pUser->GetSessionId()[0]!=0)
	{
		wcscat_s(lpszAuthStr,LEN_AUTHHEADER,L"sessionid=\"");
		wcscat_s(lpszAuthStr,LEN_AUTHHEADER,m_pUser->GetSessionId());
		wcscat_s(lpszAuthStr,LEN_AUTHHEADER,L"\",");
	}
	wcscat_s(lpszAuthStr,LEN_AUTHHEADER, L"apikey=\"");
	wcscat_s(lpszAuthStr,LEN_AUTHHEADER,SHARE_APIKEY);
	wcscat_s(lpszAuthStr,LEN_AUTHHEADER,L"\",");

	//   data="POST https://api.share.acrobat.com/webservices/api/v1/auth/ 1196363602050",
	WCHAR szMillSecond[16];
	_i64tow_s(HelperFunctions::GetCurrentTimeStamp(), szMillSecond, 16, 10);
	WCHAR szDataStr[512]=L"";
	wcscat_s(szDataStr, lpszMethod);
	wcscat_s(szDataStr,L" https://api.share.acrobat.com"); 
	wcscat_s(szDataStr, lpszUrl);
	wcscat_s(szDataStr, L" ");
	wcscat_s(szDataStr, szMillSecond);
	wcscat_s(lpszAuthStr, LEN_AUTHHEADER, L"data=\"");
	wcscat_s(lpszAuthStr, LEN_AUTHHEADER, szDataStr);
	wcscat_s(lpszAuthStr, LEN_AUTHHEADER, L"\",");

	//sig: The MD5 digest of the string formed by concatenating data with the shared secret returned by an authorization token request. 
	WCHAR szSig[512]=L"";  
	wcscpy_s(szSig, szDataStr);
	if (m_pUser->GetSessionId() != NULL)
	{
		wcscat_s(szSig,m_pUser->GetSecret()); 
	}
	else
	{
		wcscat_s(szSig,SHARE_SHAREDSECRET);  
	}
	WCHAR szSigMD5[33]={0};   
	HelperFunctions::MD5Hash(szSig, szSigMD5);

	wcscat_s(lpszAuthStr,LEN_AUTHHEADER, L"sig=\"");
	wcscat_s(lpszAuthStr,LEN_AUTHHEADER,szSigMD5);
	wcscat_s(lpszAuthStr,LEN_AUTHHEADER, L"\"");
}



/*************************************************************************************
get all current files and subfolders of a specific folder   
parameters:
	[in] lpszFolderId:  the folder id of the parent folder which is operated on.
	for the root folder, the id can be NULL , or it can be the rootfolderid which can get from the xml file.
Return:
	a pointer to the files/subfolders list.

**************************************************************************************/
MSXML2::IXMLDOMDocumentPtr ShareAPI::GetNodeList(LPCWSTR lpszFolderId)
{	
	assertUserLoggedIn(); 
	//concat the requestPath url
	WCHAR szRequestPath[LEN_URLFULLPATH]={0};
	wcscpy_s(szRequestPath, SHARE_URLDC);

	if (lpszFolderId && wcslen(lpszFolderId) >0)
	{
		wcscat_s(szRequestPath, lpszFolderId);
		if ( lpszFolderId[wcslen(lpszFolderId)-2] != L'/')              //add the "/" to the end of the path
				wcscat_s(szRequestPath, L"/"); 
	}

	HINTERNET hRequest= HttpOpenRequestW(m_hConnect, L"GET", szRequestPath, NULL, NULL, NULL,
		INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_SECURE, 0);   //no cache
	
	INT nTimeout = 180000;
	InternetSetOption(hRequest, INTERNET_OPTION_SEND_TIMEOUT , &nTimeout, sizeof(nTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT,  &nTimeout, sizeof(nTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,  &nTimeout, sizeof(nTimeout));

	//add authentication header
	addAuthHeader(hRequest, L"GET", szRequestPath);
	
	//send request
	if (!HttpSendRequestW(hRequest,NULL,0,NULL, 0))
	{	
		DWORD dwErrCode = GetLastError();
		InternetCloseHandle(hRequest);
		throw WinErrorException(dwErrCode, L"GetNodeList  failed!");   
	}

	//Get Response Information 
	DWORD dwLengthOfPath = MAX_PATH -14;
	WCHAR szTempPath[MAX_PATH-14] =  {0};
	DWORD dwRetVal = GetTempPath(dwLengthOfPath, szTempPath);

	if ((dwRetVal > dwLengthOfPath) || (dwRetVal==0))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get System Temp Path failed in GetNodelist().");   
	}

	WCHAR szTempFileName[MAX_PATH];
	if (!GetTempFileName(szTempPath, L"sky", 0, szTempFileName) )
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get temp file name failed in GetNodeList().");   
	}

	HANDLE hTempFile = CreateFile(szTempFileName, GENERIC_READ|GENERIC_WRITE, 
												FILE_SHARE_READ,	NULL, CREATE_ALWAYS,
												FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL);  


	CHAR szReadBuf[BUFFERSIZE]={0};
	DWORD dwBytesRead =0;
	DWORD dwBytesWritten =0;
	do 
	{
		if (!InternetReadFile(hRequest, (LPVOID)szReadBuf, BUFFERSIZE, &dwBytesRead))
		{
			DWORD dwErrCode = GetLastError();
			InternetCloseHandle(hRequest);
			throw WinErrorException(dwErrCode, L"InternetReadFile Failed in GetNodeList()");   
		}
		else if ( dwBytesRead )
		{
			WriteFile(hTempFile, szReadBuf, dwBytesRead, &dwBytesWritten, NULL);
		}
		memset(szReadBuf, 0, BUFFERSIZE);
	}while (dwBytesRead);

	SetFilePointer(hTempFile, 0, NULL, FILE_BEGIN);   //SET the file pointer position to the start of the file

	DWORD dwFileSize = GetFileSize(hTempFile, NULL);
	char * szFileContent = new char[dwFileSize+1]();
	
	if (!ReadFile(hTempFile, szFileContent, dwFileSize, &dwBytesRead, NULL))
	{
			DWORD dwErrCode = GetLastError();
			throw WinErrorException(dwErrCode, L"Read file Failed in GetNodeList().");   
	}
	szFileContent[dwFileSize]='\0';

	CloseHandle(hTempFile);
	
	
	//convert to widechar
	WCHAR* szXmlStrWide = new WCHAR[dwFileSize+1]();
	MultiByteToWideChar(CP_UTF8, 0, szFileContent, -1, szXmlStrWide, dwFileSize+1);
	
	MSXML2::IXMLDOMDocumentPtr  xmlDocPtr;
	HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (void**)&xmlDocPtr);
	if (S_OK != hr)
	{
		throw runtime_error("DOM  Document object create failed!");
	} 
	
	xmlDocPtr->loadXML(_bstr_t(szXmlStrWide));
	
	delete[] szXmlStrWide;
	delete[] szFileContent;
	
	return xmlDocPtr;
}


/**********************************************************************************
doUpload : upload the local file to the online storage

Parameters:
	[in]lpszFileFullPathName: the name of the file, including the path. for example: L "e:\test\doc\aaa.txt"
	[in]lpszFolderNodeId:  the node id of the the online folder which is the parent folder for the uploading file, IF NULL,  upload to the root folder.
	[in]lpszDescription : the description of the file , optional.   NULL as default.
	[in] hMsg:  the progressbar  message window
	[in] isRetry:  the first time:FALSE, the second time : TRUE.
	[out] szResponse: the response text;
	
This is long funtion, it contains serveral steps:
	1. check the validation the the arguments
	2.Open  http request
	3. add header
	4. add the body description
	5. add the file content to the http body (using iteration to read/write 4k every time)
	6. add the body tail
	7. end the http request
	8. Get response and handle errors
	
*****************************************************************/
void ShareAPI::doUpload(  LPCWSTR lpszFileFullPathName, LPCWSTR lpszFolderNodeId, LPCWSTR lpszDescription, HWND hMsg, BOOL isRetry,  CHAR* szResponse) 
{
	
	HANDLE hFile = CreateFile(lpszFileFullPathName, GENERIC_READ,
											FILE_SHARE_READ, NULL, OPEN_EXISTING, 
											NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("open file failed!");
	}

	//get the filename from file full path
	WCHAR lpszFileName[MAX_PATH];
	HelperFunctions::GetFileNameFromFilePath(lpszFileFullPathName, lpszFileName, MAX_PATH); 

	if (isRetry)  //original file name is illegal , add ".wjh"
	{
		wcscat_s(lpszFileName, MAX_PATH, L".wjh");
	}

	//boundary string
	LPCWSTR lpszBoundary = L"a4bxyef8934324sdfsdfs066";

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLDC);
	
	//if the folerNodeId  is NULL ,   user SHAR_URLDC
	if (lpszFolderNodeId && wcslen(lpszFolderNodeId) >0)           
	{
		wcscat_s(szRequestPath, lpszFolderNodeId);
		if (lpszFolderNodeId[wcslen(lpszFolderNodeId)-2] != L'/')
				wcscat_s(szRequestPath, L"/");
	}
	//open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"POST", szRequestPath, NULL,NULL,NULL, INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_SECURE,0);  
	if(!hRequest)
	{
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"UploadFile failed in HttpOpenRequest()");
	}
	//add  header
	WCHAR szHeader[256];
	wcscpy_s(szHeader,L"Content-Type:multipart/form-data; boundary=\"");
	wcscat_s(szHeader, lpszBoundary);
	wcscat_s(szHeader,L"\"\r\n");
	HttpAddRequestHeadersW(hRequest, szHeader, wcslen(szHeader), HTTP_ADDREQ_FLAG_REPLACE|HTTP_ADDREQ_FLAG_ADD); 
	
	WCHAR szAuthHeader[LEN_AUTHHEADER]=L"";
	generateAuthorizationHeader( L"POST", szRequestPath, szAuthHeader);   
	if (!HttpAddRequestHeadersW(hRequest,szAuthHeader, -1L, HTTP_ADDREQ_FLAG_ADD))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L" add authentication header failed !");     
	}
	//add body
	WCHAR szDescBodyWide[1024] =L"";
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"--");
	wcscat_s(szDescBodyWide, lpszBoundary);
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"Content-Disposition: form-data; name=\"request\"");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"Content-Type:application/xml;charset=UTF-8\r\n");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"<request>");
	wcscat_s(szDescBodyWide,		L"<file>");
	wcscat_s(szDescBodyWide,			L"<createpdf>false</createpdf>");
	wcscat_s(szDescBodyWide,			L"<name>");
	wcscat_s(szDescBodyWide,					lpszFileName);
	wcscat_s(szDescBodyWide,			L"</name>");
	wcscat_s(szDescBodyWide,			L"<description>");	
	if (lpszDescription && wcslen(lpszDescription)>0)
			wcscat_s(szDescBodyWide,					lpszDescription);
	wcscat_s(szDescBodyWide,			L"</description>");
	wcscat_s(szDescBodyWide,			L"<renditions>false</renditions>");
	wcscat_s(szDescBodyWide,		L"</file>");
	wcscat_s(szDescBodyWide, L"</request>");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"--");
	wcscat_s(szDescBodyWide, lpszBoundary);
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"Content-Disposition: form-data; name=\"file\"; filename=\"");
	wcscat_s(szDescBodyWide, lpszFileName);
	wcscat_s(szDescBodyWide, L"\"\r\n");

	wcscat_s(szDescBodyWide, L"Content-Type: application/octet-stream");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"Content-Transfer-Encoding: binary");
	wcscat_s(szDescBodyWide, L"\r\n");
	wcscat_s(szDescBodyWide, L"\r\n");
	
	CHAR szDescBody[1024];
	size_t sizeOfDescBody = WideCharToMultiByte(CP_UTF8,0,szDescBodyWide,-1,szDescBody,1024,NULL,NULL);

	WCHAR szEndBoundaryWide[128] =L"";
	wcscat_s(szEndBoundaryWide, L"\r\n");
	wcscat_s(szEndBoundaryWide, L"\r\n");
	wcscat_s(szEndBoundaryWide, L"--");
	wcscat_s(szEndBoundaryWide, lpszBoundary);
	wcscat_s(szEndBoundaryWide, L"--");
	wcscat_s(szEndBoundaryWide, L"\r\n");

	CHAR szEndBoundary[128];
	size_t sizeOfEndBound = WideCharToMultiByte(CP_UTF8,0,szEndBoundaryWide,-1,szEndBoundary,128,NULL,NULL);

	 //read binary code from the file.
	INTERNET_BUFFERS bufferIn;
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	bufferIn.Next = NULL;
	bufferIn.lpcszHeader = NULL;
	bufferIn.dwHeadersLength = 0;
	bufferIn.dwHeadersTotal =0;
	bufferIn.lpvBuffer = NULL;
	bufferIn.dwBufferLength =0;
	LARGE_INTEGER fileSize;
	GetFileSizeEx(hFile, &fileSize);
	bufferIn.dwBufferTotal  = sizeOfDescBody + fileSize .QuadPart+ sizeOfEndBound-2;
	bufferIn.dwOffsetLow = 0;
	bufferIn.dwOffsetHigh = 0;

	if (!HttpSendRequestEx(hRequest, &bufferIn, NULL, 0, 0))
	{	
		//HttpEndRequest(hRequest, NULL, 0, 0);
		InternetCloseHandle(hRequest);
		
		CloseHandle(hFile);
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"HttpSendRequestEx failed");
	}

	//first: write the description part.
	DWORD dwBytesRead =0;
	DWORD dwBytesWritten=0;
	InternetWriteFile(hRequest,szDescBody, sizeOfDescBody-1 ,  &dwBytesWritten);

	//write the content of the file
	CHAR szReadBuf[BUFFERSIZE];
	ZeroMemory(szReadBuf, sizeof(szReadBuf));
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);   //SET the file pointer position to the start of the file
	
	//variables used to setup the progress dialog
	INT64 lStartTimeStamp = HelperFunctions::GetCurrentTimeStamp();
	int nBlockNum =0;
	int nSpeed =0;
	
	do 
	{
		if (g_bUserCanceled)
		{
			InternetCloseHandle(hRequest);
			CloseHandle(hFile);
			return ;
		}
		
		if (!ReadFile(hFile, szReadBuf, BUFFERSIZE, &dwBytesRead, NULL))
		{
			//HttpEndRequest(hRequest, NULL, 0,0);
			InternetCloseHandle(hRequest);
			CloseHandle(hFile);
			DWORD dwErrCode = GetLastError();
			throw WinErrorException(dwErrCode, L"ReadFile Failed");   
		}
		else if (dwBytesRead)
		{
			if (InternetWriteFile(hRequest, szReadBuf, dwBytesRead, &dwBytesWritten))
			{
				if( !isRetry)  
				{
					//calculate and set the message dialog
					nBlockNum++;
					INT64 lCurrentTimeStamp = HelperFunctions::GetCurrentTimeStamp();
					
					if (lCurrentTimeStamp > lStartTimeStamp)
						nSpeed = nBlockNum * BUFFERSIZE / (lCurrentTimeStamp - lStartTimeStamp);  //KB数
					
					WCHAR szSpeed[32]={0};
					wsprintf(szSpeed, L"%d KB/second", nSpeed);

					SetDlgItemText(hMsg, IDC_STATICSPEED, szSpeed);
					
					//advance the progress bar
					SendMessage(GetDlgItem(hMsg, IDC_PROGRESSBAR), PBM_STEPIT, 0, 0);
				}
			}
			else
			{
				InternetCloseHandle(hRequest);
				//HttpEndRequest(hRequest, NULL, 0,0);
				CloseHandle(hFile);
				DWORD dwErrCode =::GetLastError();
				throw WinErrorException(dwErrCode, L"InternetWriteFile failed");
			}
		}
		ZeroMemory(szReadBuf, sizeof(szReadBuf));
	}while (dwBytesRead==BUFFERSIZE);
	
	InternetWriteFile(hRequest, szEndBoundary, sizeOfEndBound-1, &dwBytesWritten);  
	
	//close the file .
	CloseHandle(hFile);
	//end request
	HttpEndRequest(hRequest, NULL, 0,0);  
	
	//handle response xml, if OK ,return the uploaded node, if not, throw exception
	DWORD dwByteRead =0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LEN_UPLOADRESPONSE, &dwByteRead))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get Response XML failed");
	}
	szResponse[dwByteRead] ='\0';

}



/*******************************************************************************
Upload file to the Share Server, 
Parameters:
	[in]lpszFileFullPathName: the name of the file, including the path. for example: L "e:\test\doc\aaa.txt"
	[in]lpszFolderNodeId:  the node id of the the online folder which is the parent folder for the uploading file, IF NULL,  upload to the root folder.
	[in]lpszDescription : the description of the file , optional.   NULL as default.
	[in] hMsg:  the progressbar  message window
	

Resoponse XML: 
<response status="ok">
	<node nodeid="Z*R4NAgBciWfhDnr*o7xNw" name="ReadMe - Copy.txt" description=""
	 createddate="1262607845000" modifieddate="1262607845000" sharelistversion="1" directory="false" hascontent="true" link="false" 
	 adobedoc="false" filesize="4090" mimetype="text/plain; charset=UTF-8" author="" behavior=""
	 thumbnailstate="0" flashpreviewstate="1" flashpreviewpagecount="0" pdfstate="0" flashpreviewembed="" 
	 recipienturl="https://acrobat.com/#d=Z*R4NAgBciWfhDnr*o7xNw" sharelevel="0"><recipients />
	 </node>
 </response> 
 
 
if the first time faied and error is "IllegalFileType", which means the file extension name is forbidden, we add the ".wjh" to the filename and upload again.
after sucessful uploading, rename the file back to the original name.

**********************************************************************************************/
MSXML2::IXMLDOMNodePtr ShareAPI::UploadFile( LPCWSTR lpszFileFullPathName, LPCWSTR lpszFolderNodeId, LPCWSTR lpszDescription, HWND hMsg)
{
	SendMessage(GetDlgItem(hMsg, IDC_PROGRESSBAR), PBM_SETPOS, 0, 0);  

	assertUserLoggedIn(); 
	//arguments checking
	if (!lpszFileFullPathName || wcslen(lpszFileFullPathName)==0 )
	{
		throw runtime_error("the file name is invalid!");
	}
	
	CHAR szResponse[LEN_UPLOADRESPONSE+1] ={0};
	doUpload(lpszFileFullPathName, lpszFolderNodeId, lpszDescription, hMsg, FALSE, szResponse);
		
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);  
	
	//succeed in the first time	
	if (pResponse->IsStatusOK())
	{		
		MSXML2::IXMLDOMNodePtr  pNode = pResponse->GetNode();  
		delete pResponse;     //this is ok, it will not release pNode.
		return pNode; 
	}
	else	//FAILED IN THE FIRST TIME
	{
		if (g_bUserCanceled)
		{
			//throw std::runtime_error("User canceled");
			return NULL;   
		}
		
		//IllegalFileType:  add ".wjh" to the file name and try again	
		if ( (strcmp(szResponse, "IllegalFiletype") == 0) ||  (strcmp(szResponse, "ILLEGALFILETYPE") == 0) )
		{
			doUpload(lpszFileFullPathName, lpszFolderNodeId, lpszDescription, hMsg, TRUE, szResponse);  //try again with different extension name
			
			ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);  
			if (!pResponse->IsStatusOK())  //faied for the second time
			{		
				delete pResponse;
				throw ShareAPIException(szResponse);
			}
			
			//succeed in the second time,  rename the file in share sevice, 
			MSXML2::IXMLDOMNodePtr nodePtr= pResponse->GetNode();
			
			ShareAPINode * pNode = new ShareAPINode(nodePtr);
			
			WCHAR szName[MAX_PATH]={0};
			wcscpy_s(szName, MAX_PATH, pNode->GetName());
			szName[lstrlen(szName)-	4]='\0';  
			//renam the file back to original 
			if (RenameNode(pNode->GetNodeId(), szName))
			{
				pNode->SetName(szName);
			}
			delete pNode;
			delete pResponse;
			return nodePtr;
		}
		else // error other than illegalfiletype, throw exception
		{
			delete pResponse;
			throw ShareAPIException(szResponse);
		}
	}
}


/*********************************************************************************************
//private method,    the common part for  GetFile() and Savefileto()
Param:
	lpszFileNodeId:  the file node id
	hFile:  the  handle of the local file
	hMsg: Progressbar message dialog handle 
	
//the PDF and Thumbnail is not well supported by adobe, (NO API for CreatePDF??) so till now, only "src" is supported by this app.
*********************************************************************************************/
void ShareAPI::doDownload(LPCWSTR lpszFileNodeId, HANDLE hFile, HWND hMsg)
{
	SendMessage(GetDlgItem(hMsg, IDC_PROGRESSBAR), PBM_SETPOS, 0, 0);  

	WCHAR  szRequestPath[128]=L"";
	wcscat_s(szRequestPath, L"/webservices/api/v1/dc/");
	wcscat_s(szRequestPath, lpszFileNodeId);
	wcscat_s(szRequestPath, L"/src/");

	HINTERNET hRequest = HttpOpenRequest(m_hConnect, L"GET", szRequestPath,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	if(!hRequest)
	{
		CloseHandle(hFile); 
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"UploadFile failed in HttpOpenRequest()");
	}

	WCHAR szAuthHeader[LEN_AUTHHEADER]= L"";
	generateAuthorizationHeader(L"GET", szRequestPath, szAuthHeader);  
	
	if (!HttpAddRequestHeaders(hRequest,szAuthHeader, wcslen(szAuthHeader), HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE))
	{
		CloseHandle(hFile); 
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L" add authentication header failed in HttpAddRequestHeader() !");     
	}

	if (!HttpSendRequest(hRequest,NULL,0,NULL, 0))
	{
		DWORD dwErrCode = GetLastError();
		InternetCloseHandle(hRequest);
		CloseHandle(hFile); 
		throw WinErrorException(dwErrCode, L"Send Http Request failed.");   
	}

	CHAR szDataBuf[BUFFERSIZE] ={0};
	DWORD dwBytesRead =0;
	DWORD dwBytesWritten =0;

	INT64 lStartTimeStamp = HelperFunctions::GetCurrentTimeStamp();
	int nBlockNum =0;
	int nSpeed =0;
	
	do 
	{
		if (g_bUserCanceled)
		{
			InternetCloseHandle(hRequest);
			CloseHandle(hFile); 
			return;
		}
	
		if (!InternetReadFile(hRequest, (LPVOID)szDataBuf, BUFFERSIZE, &dwBytesRead))
		{
			DWORD dwErrCode = GetLastError();
			InternetCloseHandle(hRequest);
			CloseHandle(hFile); 
			throw WinErrorException(dwErrCode, L"InternetReadFile Failed");   
		}
		else if (dwBytesRead)
		{
			//calculate and set the speed;
			nBlockNum++;
			INT64 lCurrentTimeStamp = HelperFunctions::GetCurrentTimeStamp();
			
			if (lCurrentTimeStamp > lStartTimeStamp)
				nSpeed = nBlockNum * BUFFERSIZE / (lCurrentTimeStamp - lStartTimeStamp);  //KB数
			
			WCHAR szSpeed[32]={0};
			wsprintf(szSpeed, L"%d KB/second", nSpeed);

			SetDlgItemText(hMsg, IDC_STATICSPEED, szSpeed);
			
			//advance the progress bar
			SendMessage(GetDlgItem(hMsg, IDC_PROGRESSBAR), PBM_STEPIT, 0, 0);
		
			//write to file
			WriteFile(hFile, szDataBuf, dwBytesRead, &dwBytesWritten, NULL);
		}
		memset(szDataBuf, 0, BUFFERSIZE);
	}
	while (dwBytesRead);
	

	//Exception handling
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	CHAR szHeadMsg[21]={0};
	ReadFile(hFile, szHeadMsg, 20, &dwBytesRead, NULL);
	szHeadMsg[dwBytesRead]='\0';

	if (strcmp(szHeadMsg, "AccessDenied") ==0 
		|| strcmp(szHeadMsg, "BadRendition") ==0
		|| strcmp(szHeadMsg, "Processing") ==0 
		|| strcmp(szHeadMsg, "Error")==0)
	{
		CloseHandle(hFile);
		throw ShareAPIException(szHeadMsg);
	}

	CloseHandle(hFile); 
}


/*************************************************************************************
	Download the online file into local temporay file,  return the Handle of the local temp file.
	NOTE:  the user need to use CloseHandle() to release the handle resource
Parameters
	[in] lpszFileNodeId:  the node id  of the file in the XML
	[in] lpszFileName: the name of the file
	[out]lpszFileFullPath: the full path of the file ,including the filename
	[in] hMsg:  progress bar window
return:  the handle of the downloaded temp file. 
********************************************************************************/
void ShareAPI::GetFile(LPCWSTR lpszFileNodeId, LPCWSTR lpszFileName, LPWSTR lpszFileFullPathName, HWND hMsg)
{
	assertUserLoggedIn(); 
	//check argument validation
	if (!lpszFileNodeId || wcslen(lpszFileNodeId)==0)
	{
		throw runtime_error("Invalid argument, FileNodeId is NULL!");
	}
	if (!lpszFileName || wcslen(lpszFileName)==0)
	{
		throw runtime_error("Invalid argument, File name is NULL!");
	}
	
	//download the file into the temp folder with the same name.
	DWORD dwLengthOfPath = MAX_PATH -14;
	WCHAR szTempPath[MAX_PATH-14] =  {0};
	DWORD dwRetVal = GetTempPath(dwLengthOfPath, szTempPath);
	if ((dwRetVal > dwLengthOfPath) || (dwRetVal==0))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get System Temp Path failed.");   
	}
	
	wcscat(szTempPath, lpszFileName);
	wcscpy(lpszFileFullPathName,szTempPath);  
	HANDLE hFile = CreateFile(szTempPath, GENERIC_READ|GENERIC_WRITE, 
												FILE_SHARE_READ,	NULL, CREATE_ALWAYS,
												FILE_ATTRIBUTE_TEMPORARY, NULL);  
	
	doDownload(lpszFileNodeId, hFile, hMsg);
}


/********************************************************************
Download the online file into local  file,  return the Handle of the local  file.
	NOTE:  the user need to use CloseHandle() to release the handle resource
Parameters
	[in] lpszFileNodeId:  the node id  of the file in the XML
	[in]lpszFileFullPath: the full path of the file ,including the filename
	[in] hMsg: the progressbar message dialog handle

********************************************************************/
void ShareAPI::SaveFileTo(LPCWSTR lpszFileNodeId, LPCWSTR lpszFileFullPathName, HWND hMsg)
{
	assertUserLoggedIn(); 
	//check argument validation
	if (!lpszFileNodeId || wcslen(lpszFileNodeId)==0)
	{
		throw runtime_error("Invalid argument, FileNodeId is NULL!");
	}
	if (!lpszFileFullPathName || wcslen(lpszFileFullPathName)==0)
	{
		throw runtime_error("Invalid argument, File name is NULL!");
	}
	
	
	HANDLE hFile = CreateFile(lpszFileFullPathName, GENERIC_READ|GENERIC_WRITE, 
												FILE_SHARE_READ,	NULL, CREATE_ALWAYS,
												FILE_ATTRIBUTE_TEMPORARY, NULL);  
												
	if (hFile == NULL)
	{
		throw std::runtime_error("create file failed");
	}
	
	doDownload(lpszFileNodeId, hFile, hMsg);
}


/*******************************************************
add a folder to the current path
Parameters:
 [in]lpszParentNodeId:  the parent node id, NULL for the root.
 [in] lpszFolderName:  the name of the folder to be added
 [in]lpszDescription: the description of the folder, optional,  Default is NULL.
 
 return: the newly added folder's node
Exception:
	WinExcetpion
	ShareAPIException
	runtime_error
	
Response XML	:
<response status="ok">
<node nodeid="OO5L3KHtTNgZEH19hH9jdw" name="uiop" description="" 
createddate="1262608867000" modifieddate="1262608867000" sharelistversion="0" directory="true" 
hascontent="false" link="false" />
</response>
 ***************************************************************/
MSXML2::IXMLDOMNodePtr ShareAPI::AddFolder(LPCWSTR lpszParentNodeId, LPCWSTR lpszFolderName, LPCWSTR lpszDescription)  
{
	assertUserLoggedIn(); 

	if (!lpszFolderName || wcslen(lpszFolderName)==0)
	{
		throw runtime_error("Invalid argument, Folder name is NULL!");
	}

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLDC);
	if (lpszParentNodeId)
	{
		wcscat_s(szRequestPath, lpszParentNodeId);
		if (wcslen(lpszParentNodeId) >0 &&  lpszParentNodeId[wcslen(lpszParentNodeId)-2] != L'/')
				wcscat_s(szRequestPath, L"/");
	}
	//open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"POST", szRequestPath,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	if(!hRequest)
	{
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"CreateFolder failed in HttpOpenRequest()");
	}
	//add authentication header
	addAuthHeader(hRequest, L"POST", szRequestPath);

	//add body
	WCHAR szRequestBodyWide[512]=L"";
	wcscat_s(szRequestBodyWide, L"<request>");
	wcscat_s(szRequestBodyWide, L"<folder>");
	wcscat_s(szRequestBodyWide, L"<name>");
	wcscat_s(szRequestBodyWide,  lpszFolderName);
	wcscat_s(szRequestBodyWide, L"</name>");
	wcscat_s(szRequestBodyWide, L"<description>");
	if (lpszDescription && wcslen(lpszDescription)>0)
		wcscat_s(szRequestBodyWide,  lpszDescription);
	wcscat_s(szRequestBodyWide, L"</description>");
	wcscat_s(szRequestBodyWide, L"</folder>");
	wcscat_s(szRequestBodyWide, L"</request>\n");

	//convert widechar Request body to mutibute version
	UINT nSize =  WideCharToMultiByte(CP_UTF8,0,szRequestBodyWide,-1,NULL,0,NULL,NULL);   //get the size of the multibyte verion of body. it can be different from the widechar verison
	
	char* lpszRequestBody = new char[nSize]();
	WideCharToMultiByte(CP_UTF8, 0, szRequestBodyWide, -1, lpszRequestBody, nSize, NULL, NULL);
	
	//send request
	if (!HttpSendRequestW(hRequest,NULL,0,lpszRequestBody, strlen(lpszRequestBody)))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L" Create folder failed!");   
	}
	delete [] lpszRequestBody;	

	//handle response xml, if OK ,return the uploaded node, if not, throw exception
	const int LENRESPONSE = 2048;
	CHAR szResponse[LENRESPONSE+1] ={0};
	DWORD dwByteRead =0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LENRESPONSE, &dwByteRead))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get Response XML failed");
	}
	szResponse[dwByteRead] ='\0';
	
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);  
	if (pResponse->IsStatusOK())
	{
		MSXML2::IXMLDOMNodePtr  pNode = pResponse->GetNode();
		delete pResponse;
		return pNode; 
	}
	else
	{
		delete pResponse;
		throw ShareAPIException(szResponse);
	}
}

/*================================================
//Delete the node
// [in]nodeId: the id of the file /folder node
//  if the node is a file, delete the file;  if the node is a folder, delete all the files/subfolders inside of it.
//  if nodeId not exist,  do nothing
//  nodeid，　root folder can not be deleted.

Response XML  : "<response status="ok" />"
================================================*/
BOOL ShareAPI::DeleteNode(LPCWSTR lpszNodeId)
{
	assertUserLoggedIn(); 

	if (!lpszNodeId || wcslen(lpszNodeId)==0)   //cannot delete root
	{
		return FALSE;
	}

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLDC);
	wcscat_s(szRequestPath, lpszNodeId);
	if (lpszNodeId[wcslen(lpszNodeId)-2] != L'/')
			wcscat_s(szRequestPath, L"/");

	//open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"DELETE", szRequestPath,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	if(!hRequest)
	{
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"DeleteNode Failed.");
	}
	//add authentication header
	addAuthHeader(hRequest, L"DELETE", szRequestPath);
	
	//send request
	if (!HttpSendRequest(hRequest,NULL,0,NULL, 0))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"DeleteNode Failed.");   
	}

	//handle response xml info
	const int LENRESPONSE = 128;
	CHAR szResponse[LENRESPONSE+1]={0};
	DWORD dwByteRead =0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LENRESPONSE,&dwByteRead))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get Response XML failed");
	}
	szResponse[dwByteRead] ='\0';
	
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);
	if (!pResponse->IsStatusOK())
	{
		delete pResponse;
		return FALSE;
	}
	else
	{
		delete pResponse;
		return TRUE;
	}
}   



/***********************************************************
Response XML :  Rename and Move "<response status="ok" />"
*************************************************************/
BOOL ShareAPI::sendMoveRequest( LPCWSTR lpszUrl)
{
	////open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"MOVE", lpszUrl,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	if(!hRequest)
	{
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"Move/Rename Failed!");
	}
	//add authentication header
	addAuthHeader(hRequest, L"MOVE", lpszUrl);

	if (!HttpSendRequest(hRequest,NULL,0,NULL, 0))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Move/Rename Failed!");   
	}

	//handle response xml info
	const int LENRESPONSE = 128;
	CHAR szResponse[LENRESPONSE+1]={0};
	DWORD dwByteRead =0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LENRESPONSE,&dwByteRead))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get Response XML failed");
	}
	szResponse[dwByteRead] ='\0';
	
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);
	if (!pResponse->IsStatusOK())
	{
		delete pResponse;
		return FALSE;
	}
	else
	{
		delete pResponse;
		return TRUE;
	}
} 


/*********************************************************************************
//Renam the specific node
//Parameter:
	[in]lpszNodeId: the  node id of the file/folder to be renamed
	[in]lpszNewName: the new name of the file/folder

Response XML :  Rename and Move "<response status="ok" />"
***********************************************************************************/
BOOL ShareAPI::RenameNode(LPCWSTR lpszNodeId, LPCWSTR lpszNewName)
{
	assertUserLoggedIn(); 

	if (!lpszNodeId || wcslen(lpszNodeId)==0)
	{
		return FALSE;
	}

	if (!lpszNewName || wcslen(lpszNewName)==0)
	{
		return FALSE;
	}

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLDC);
	wcscat_s(szRequestPath, lpszNodeId);
	if (wcslen(lpszNodeId) >0 &&  lpszNodeId[wcslen(lpszNodeId)-2] != L'/')
			wcscat_s(szRequestPath, L"/");
	wcscat_s(szRequestPath, L"?newname=");
	wcscat_s(szRequestPath, lpszNewName);

	return sendMoveRequest(szRequestPath);
}



/****************************************************************************
Move a file /folder to another folder
Parameters:
	[in] lpszNodeId: the node id of the file/folder to be moved , required.
	[in] lpszName: the file/folder name of the node to be moved, requried, cannot be null
	[in] lpszDestNodeId:  the node id of the destination folder, required.
*************************************************************************/
BOOL ShareAPI::MoveNode(LPCWSTR lpszNodeId, LPCWSTR lpszName, LPCWSTR lpszDestNodeId)
{
	assertUserLoggedIn(); 

	if (!lpszNodeId || wcslen(lpszNodeId)==0)
	{
		return FALSE;
	}

	if (!lpszName || wcslen(lpszName)==0)
	{
		return FALSE;
	}

	if (!lpszDestNodeId ||wcslen(lpszDestNodeId) ==0)
	{
		return FALSE;
	}

	WCHAR szRequestPath[LEN_URLFULLPATH]=L"";
	wcscpy_s(szRequestPath, SHARE_URLDC);
	wcscat_s(szRequestPath, lpszNodeId);
	if (wcslen(lpszNodeId) >0 &&  lpszNodeId[wcslen(lpszNodeId)-2] != L'/')
			wcscat_s(szRequestPath, L"/");
	wcscat_s(szRequestPath, L"?destnodeid=");
	wcscat_s(szRequestPath, lpszDestNodeId);
	wcscat_s(szRequestPath, L"&newname=");
	wcscat_s(szRequestPath, lpszName);

	return sendMoveRequest(szRequestPath);
}

//
///*******************************************************************************************************
//Share a file to one or more email users.
//Parameters:
//	lpszFileNodeId: the node id of the shareing file
//	lpszUsers: email addresses, seperated by ";"
//	lpszSubject: the subject of the mail.
//	lpszMessage: the  body of email, optional, NULL as default.
//	nLevel: share level, 1 as default (restrict only to the email users)
//
//Level
//
//xml Response："<response status="ok" />"	
//
//Till now,  the Adobe share only allow to share the files in the root directory,  so we do not implement this funciton in our app.
//**************************************************************************************************/
BOOL ShareAPI::ShareFile(LPCWSTR lpszFileNodeId, LPWSTR lpszUsers, LPCWSTR lpszSubject, LPCWSTR lpszMessage, int nLevel )  
{
	assertUserLoggedIn(); 

	if (!lpszFileNodeId || wcslen(lpszFileNodeId) ==0)
	{
		return FALSE;
	}

	WCHAR szRequestPath[LEN_URLFULLPATH]={0};
	wcscpy_s(szRequestPath, SHARE_URLDC);
	wcscat_s(szRequestPath, lpszFileNodeId);
	if ( lpszFileNodeId[wcslen(lpszFileNodeId)-2] != L'/')              
		wcscat_s(szRequestPath, L"/"); 
	wcscat_s(szRequestPath, L"share/");

	//open Request
	HINTERNET hRequest = HttpOpenRequestW(m_hConnect, L"PUT", szRequestPath,  NULL,NULL,NULL, INTERNET_FLAG_SECURE,0);
	if(!hRequest)
	{
		DWORD dwErrCode =::GetLastError();
		throw WinErrorException(dwErrCode, L"ShareFile Failed!");
	}
	//add authentication header
	addAuthHeader(hRequest, L"PUT", szRequestPath);

	//add body
	WCHAR* lpszUserMail = NULL;

	int nDimension = lstrlen(lpszUsers);
	WCHAR*  szUserMails = new WCHAR[nDimension+1]();
	wcscpy_s(szUserMails, nDimension+1, lpszUsers);
	
	WCHAR szRequestBodyWide[512]=L"";
	wcscat_s(szRequestBodyWide, L"<request>");
	wcscat_s(szRequestBodyWide, L"<share>");
	
	lpszUserMail = wcstok(szUserMails, L",;");

	while (lpszUserMail !=NULL)
	{
		wcscat_s(szRequestBodyWide, L"<user>");
		wcscat_s(szRequestBodyWide,  lpszUserMail);
		wcscat_s(szRequestBodyWide, L"</user>");
		
		lpszUserMail = wcstok(NULL, L",;");
	}
	wcscat_s(szRequestBodyWide, L"</share>");
	
	delete [] szUserMails;
	
	if (lpszSubject)
	{
		wcscat_s(szRequestBodyWide, L"<subject>");
		wcscat_s(szRequestBodyWide,  lpszSubject);
		wcscat_s(szRequestBodyWide, L"</subject>");
	}
	if (lpszMessage)
	{
		wcscat_s(szRequestBodyWide, L"<message>");
		wcscat_s(szRequestBodyWide,  lpszMessage);
		wcscat_s(szRequestBodyWide, L"</message>");
	}
	wcscat_s(szRequestBodyWide, L"<level>");
	WCHAR szLevel[2];
	_itow_s(nLevel, szLevel, 2,10);
	wcscat_s(szRequestBodyWide,  szLevel);
	wcscat_s(szRequestBodyWide, L"</level>");
	wcscat_s(szRequestBodyWide, L"</request>\n");

	//convert widechar Request body to mutibute version
	UINT nSize =  WideCharToMultiByte(CP_UTF8,0,szRequestBodyWide,-1,NULL,0,NULL,NULL);   //get the size of the multibyte verion of body. it can be different from the widechar verison
	
	char* lpszRequestBody = new char[nSize];
	WideCharToMultiByte(CP_UTF8, 0, szRequestBodyWide, -1, lpszRequestBody, nSize, NULL, NULL);
	
	//send request
	if (!HttpSendRequestW(hRequest,NULL,0,lpszRequestBody, strlen(lpszRequestBody)))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L" Share file failed.");   
	}
	delete [] lpszRequestBody;	

	//handle response xml info
	const int LENRESPONSE = 128;
	CHAR szResponse[LENRESPONSE+1]={0};
	DWORD dwByteRead =0;
	if (!InternetReadFile(hRequest, (LPVOID)szResponse, LENRESPONSE,&dwByteRead))
	{
		DWORD dwErrCode = GetLastError();
		throw WinErrorException(dwErrCode, L"Get Response XML failed");
	}
	szResponse[dwByteRead] ='\0';
	
	ShareAPIResponse* pResponse = new ShareAPIResponse(szResponse);
	if (!pResponse->IsStatusOK())
	{
		delete pResponse;
		return FALSE;
	}
	else
	{
		delete pResponse;
		return TRUE;
	}
}


/****************************************************
Check if the user has logged in, 
if not, exception throwed.
***************************************************/
void ShareAPI::assertUserLoggedIn()
{
	if (m_pUser->GetSessionId() == NULL)
	{
		throw runtime_error("The user must be logged in.");
	}
}
