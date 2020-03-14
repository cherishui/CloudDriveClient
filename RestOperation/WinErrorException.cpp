#include "StdAfx.h"
#include "WinErrorException.h"
//#include <log4cxx\logger.h>

//extern log4cxx::LoggerPtr g_ShareAPILogger;

WinErrorException::WinErrorException(DWORD dwErrCode, LPCWSTR lpszErrMsg)
				:m_dwErrorCode(dwErrCode),
				m_pszErrorCodeDesc(NULL),
				m_pszErrorMsg(NULL),
				m_pszWhat(NULL)
{
	//set errorCodeDesc first,  using FormatMessage() 
	
	//DWORD dwCodeDescSize =0;
	
	int nSize =128;
	m_pszErrorCodeDesc = new WCHAR[nSize]();
	
	if (m_dwErrorCode >= 12000 && m_dwErrorCode <= 12174)
	{
		  FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE,
										GetModuleHandleW(L"wininet.dll"),
										m_dwErrorCode,
										0,
										m_pszErrorCodeDesc,
										nSize,
										NULL);
	}
	else
	{
		 FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 
										NULL,
										m_dwErrorCode,
										0,
										m_pszErrorCodeDesc,
										nSize,
										NULL);
	}
	//set errorMsg
	int nDimension = wcslen(lpszErrMsg) + 1; 
	m_pszErrorMsg =  new WCHAR[nDimension]();
	wcsncpy(m_pszErrorMsg, lpszErrMsg, nDimension); 

	//set what()
	int nMsgDimension = wcslen(m_pszErrorMsg) + 1; 
	char* pszMbErrorMsg =  new char[nMsgDimension]();
	WideCharToMultiByte(CP_UTF8, 0, m_pszErrorMsg, -1, pszMbErrorMsg, nMsgDimension, NULL, NULL);

	char  szErrorCode[32];
	_itoa_s(m_dwErrorCode, szErrorCode, 32, 10);

	int nDescDimension = wcslen(m_pszErrorCodeDesc) +1;
	char* pszMbErrCodeDesc = new char[nDescDimension]();
	WideCharToMultiByte(CP_UTF8,0,m_pszErrorCodeDesc, -1, pszMbErrCodeDesc, nDescDimension, NULL,NULL);

	size_t  uSize = nMsgDimension + nDescDimension +128;

	m_pszWhat = new char[uSize]();
	strcpy_s(m_pszWhat, uSize, pszMbErrorMsg);
	strcat_s(m_pszWhat, uSize, "\r\n  ErrorCode: ");
	strcat_s(m_pszWhat, uSize, szErrorCode);
	strcat_s(m_pszWhat, uSize, "  :  ");
	strcat_s(m_pszWhat, uSize, pszMbErrCodeDesc);

	delete [] pszMbErrorMsg;
	delete [] pszMbErrCodeDesc; 
	
	//log the error to log file
	//LOG4CXX_ERROR(g_ShareAPILogger,  m_pszWhat);
}

WinErrorException::~WinErrorException(void)
{
	//release all allocated memory
	//LocalFree(m_pszErrorCodeDesc);
	
	if (m_pszErrorCodeDesc)
	{
		delete [] m_pszErrorCodeDesc;
	}
	
	if (m_pszErrorMsg)
	{
		delete [] m_pszErrorMsg;
	}

	if (m_pszWhat)
	{
		delete [] m_pszWhat;
	}
}

DWORD WinErrorException::GetErrorCode()
{
	return m_dwErrorCode;
}

LPCWSTR WinErrorException::GetErrorCodeDesc()
{
	return m_pszErrorCodeDesc;
}

LPCWSTR WinErrorException::GetErrorMsg()
{
	return m_pszErrorMsg;
}

const char* WinErrorException::what() const       // with throw():  this function does not throw any exception
{
	return m_pszWhat;

}