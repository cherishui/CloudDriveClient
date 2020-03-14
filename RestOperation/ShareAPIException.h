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
#pragma once
#include <utility>
#include <map>
using namespace std;

class ShareAPIException : public std::exception
{
private:
	WCHAR *m_pszErrorStatus;
	WCHAR *m_pszErrorDesc; 
	char *m_pszWhat;						// "ErrorStatus: " + m_pszErrorStatus + "ErrorDescription: " + m_pszErrorDesc.
	
private:
	void initializeErrorMap();
	void init(LPCWSTR lpszErrorStatus);
	
	//ShareAPIException();
public:
	ShareAPIException(LPCWSTR lpszErrorCode);
	ShareAPIException(LPCSTR lpszErrorCode);
	~ShareAPIException(void);

	LPCWSTR GetErrorStatus(void);
	LPCWSTR GetErrorDesc(void);
	const char* what() const throw();    
};
