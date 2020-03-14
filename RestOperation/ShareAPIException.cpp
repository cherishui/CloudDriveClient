/*************************************************************************
	File: ShareAPIException.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/20
	Description:
			ShareAPIException is a deried class from std::exception.
			This class is used to handle the ShareAPI response error messages for a Share request, 
			These error messages are described in ShareAPI document.
			All the error messages are stored in a staic map of this class.

			Memo:

*****************************************************************************/

#include "StdAfx.h"
#include <string>
#include <algorithm>          
#include "ShareAPIException.h"

typedef pair<wstring, wstring> ErrorPair;
map<wstring, wstring> s_errorMap;              //static map to store error messages

void ShareAPIException::initializeErrorMap()
{
	//general error messages
	s_errorMap.insert( ErrorPair(L"BADAPIKEY", L"The given API key is invalid.") );
	s_errorMap.insert( ErrorPair(L"BADAUTHHEADER", L"The authorization header has the wrong format or syntax.") );
	s_errorMap.insert( ErrorPair(L"BADAUTHQUERY", L" The query does not contain the required parameters. ") );
	s_errorMap.insert( ErrorPair(L"BADCALLTIME", L"The given clltime has problems:  Either it does not come after the calltime of the most recent successful request, or the request was issued too soon after the prior request. The suggested time between requests per session is half a second") );
	s_errorMap.insert( ErrorPair(L"BADFORMAT", L"The request body is malformed.") );
	s_errorMap.insert( ErrorPair(L"BADMETHOD", L"Not a valid HTTP method on this endpoint.") );
	s_errorMap.insert( ErrorPair(L"BADSESSIONID", L"The given sessionid does not exist or is expired.") );
	s_errorMap.insert( ErrorPair(L"BADSIG", L"The authoriztion signature is incorrect.") );
	s_errorMap.insert( ErrorPair(L"ERROR", L"An unkown error has occurred. The error is returned when the ShareAPI cannot understand the request, for example, when the sent node ID doesn't exist.") );
	s_errorMap.insert( ErrorPair(L"LIMITREACHED", L"The number of allowable requests was exceeded in the last hour.") );
	s_errorMap.insert( ErrorPair(L"MISSINGELEMENTS", L"The authoriztion signature is incorrect.") );
	s_errorMap.insert( ErrorPair(L"UNAUTHORIZED", L"The user does not have authorization to perform the requested action.") );

	//login error messages
	s_errorMap.insert( ErrorPair(L"BADLOGIN", L"Invalid username or password.") );
	s_errorMap.insert( ErrorPair(L"LOCKEDOUT", L"Too many failed login attemptes; try again later.") );
	s_errorMap.insert( ErrorPair(L"NOTVERIFIED", L"User has not verified email address yet.") );
	s_errorMap.insert( ErrorPair(L"NOTOU", L"User has not accepted terms of use aggrement.") );
	s_errorMap.insert( ErrorPair(L"BADAUTHTOKEN", L"Invalid authorization token.") );

	//logout error messages
	s_errorMap.insert( ErrorPair(L"BADSESSIONID", L"The session ID is invalid.") );

	//upload error messages   +Add folder error messages(Ç°3Ìõ) . 
	s_errorMap.insert( ErrorPair(L"BADNODEID", L"Attempt to upload a file into a nonexistent folder, or attempt to create foler in a nonexistent parent folder, or try to delete a nonexistent node.") );
	s_errorMap.insert( ErrorPair(L"BADUPLOAD", L"Upload failed for some other reason.") );
	s_errorMap.insert( ErrorPair(L"DUPLICATENAME", L"A file or folder with the same name already exists.") );
	s_errorMap.insert( ErrorPair(L"ILLEGALFILETYPE", L"Attempt to upload a file which is in an unsupported file format.") );
	s_errorMap.insert( ErrorPair(L"QUOTAREACHED", L"Adding the file would exceed the user's storage quota.") );

	//accessing file rendition  , get file content
	s_errorMap.insert( ErrorPair(L"ACCESSDENIED", L"User does not have the proper privileges to download this file.") );
	s_errorMap.insert( ErrorPair(L"BADRENDTION", L"Not a valid rendition request.") );
	s_errorMap.insert( ErrorPair(L"PROCESSING", L"File is still being processed. Please wait a few minutes.") );
}


ShareAPIException::ShareAPIException(LPCWSTR lpszErrorStatus)
	:m_pszErrorStatus(NULL),
	m_pszErrorDesc(NULL),
	m_pszWhat(NULL)
{
	init(lpszErrorStatus);
}

ShareAPIException::ShareAPIException(LPCSTR lpszErrorStatus)
	:m_pszErrorStatus(NULL),
	m_pszErrorDesc(NULL),
	m_pszWhat(NULL)
{
	//convert lpszErrorStatus 's type from Char* to Wchar*
	int nDimension = strlen(lpszErrorStatus) + 1;
	WCHAR* lpszWideErrStat = new WCHAR[nDimension]();
	MultiByteToWideChar(CP_UTF8,0, lpszErrorStatus, -1, lpszWideErrStat, nDimension);

	init(lpszWideErrStat);
	delete [] lpszWideErrStat;
}

void ShareAPIException::init(LPCWSTR lpszErrorStatus)
{

	if (s_errorMap.size() == 0)  //only initialize errormap  in the first construction of ShareAPI instance. 
	{
		initializeErrorMap();
	}

	wstring strErrStatus;

	if ((lpszErrorStatus == NULL) || (wcslen(lpszErrorStatus) ==0) )
	{
		strErrStatus = L"Unknow Error!";
	}
	else 
	{
		strErrStatus =lpszErrorStatus;
	}
	transform(strErrStatus.begin(), strErrStatus.end(), strErrStatus.begin(), toupper);

	map<wstring, wstring>::const_iterator mapIter = s_errorMap.find(strErrStatus);

	if (mapIter == s_errorMap.end() ) 		//unknow error, or error not defined in Adobe share doc
	{
		int nDimension = wcslen(lpszErrorStatus) + 1; 
		m_pszErrorStatus =  new WCHAR[nDimension]();
		wcsncpy(m_pszErrorStatus, lpszErrorStatus, nDimension); 

		m_pszErrorDesc = new WCHAR[nDimension]();
		wcsncpy(m_pszErrorDesc, lpszErrorStatus, nDimension);
	}
	else
	{
		int nStatusDimension = wcslen(lpszErrorStatus) + 1; 
		m_pszErrorStatus =  new WCHAR[nStatusDimension]();
		wcsncpy(m_pszErrorStatus, lpszErrorStatus, nStatusDimension); 

		int nDescDimension = wcslen(mapIter->second.c_str()) +1;
		m_pszErrorDesc = new WCHAR[nDescDimension]();
		wcsncpy(m_pszErrorDesc, mapIter->second.c_str(), nDescDimension);
	}


	//set what()
	const int nErrStatusSize = wcslen(m_pszErrorStatus)+1;
	const int nErrDescSize = wcslen(m_pszErrorDesc)+1;
	const int nWhatSize = nErrStatusSize + nErrDescSize +80;
	
	m_pszWhat = new char[nWhatSize]();


	char* pszErrStatus = new char[nErrStatusSize]();
	WideCharToMultiByte(CP_UTF8, 0, m_pszErrorStatus, -1,pszErrStatus, nErrStatusSize, NULL,NULL);
	char* pszErrDesc = new char[nErrDescSize]();
	WideCharToMultiByte(CP_UTF8, 0, m_pszErrorDesc, -1,pszErrDesc, nErrDescSize, NULL,NULL);

	strcpy_s(m_pszWhat, nWhatSize, "ErrorStatus: ");
	strcat_s(m_pszWhat, nWhatSize, pszErrStatus);
	strcat_s(m_pszWhat, nWhatSize, "  : ");
	strcat_s(m_pszWhat, nWhatSize, pszErrDesc);

	delete [] pszErrStatus;
	delete [] pszErrDesc;
	
	//log the error to the log file
	//LOG4CXX_ERROR(g_ShareAPILogger, m_pszWhat);
}

ShareAPIException::~ShareAPIException(void)
{
	if (m_pszErrorStatus)
	{
		delete [] m_pszErrorStatus;
	}
	
	if (m_pszErrorDesc)
	{
		delete [] m_pszErrorDesc;
	}

	if (m_pszWhat)
	{
		delete [] m_pszWhat;
	}
}


LPCWSTR ShareAPIException::GetErrorStatus()
{
	return m_pszErrorStatus;
}

LPCWSTR ShareAPIException::GetErrorDesc()
{
	return m_pszErrorDesc;
}

//what()  returns:  "ErrorStatus: " + m_pszErrorDesc + " ErrorDescription: "  + m_pszErrorDescription
const char* ShareAPIException::what() const
{

	return m_pszWhat;
}