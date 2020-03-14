/*************************************************************************
	File: ShareAPIException.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/20
	Description:
		This class is designed for Windows API function call exceptions, including Wininet API call exceptions and other API call exceptions.
		When API call failed,  a WinErrorException is throwed. 
	Memo:

************************************************************************/
#pragma once
#include <exception>

class WinErrorException : public std::exception   
{
private:
	DWORD	m_dwErrorCode;
	WCHAR	  *m_pszErrorCodeDesc;			//the corresponding description of a errorCode
	WCHAR	  *m_pszErrorMsg;					//the input error message to describe what makes the error happen.
	char		  *m_pszWhat;
public:

	WinErrorException(DWORD dwErrCode, LPCWSTR lpszErrMsg);
	~WinErrorException(void);
	
	DWORD GetErrorCode(void); 
	LPCWSTR GetErrorCodeDesc(void);
	LPCWSTR GetErrorMsg(void);
	const char* what() const throw();   //errorMsg+ errorCode + errorCodeDesc  . NOTICTE: this is not wide char!!!!
};
