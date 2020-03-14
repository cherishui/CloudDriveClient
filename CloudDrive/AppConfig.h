#pragma once
#include <windows.h>
#import <msxml6.dll>

class AppConfig
{
public:
	AppConfig(void);
	~AppConfig(void);

public:
	LPCWSTR GetEmail();
	LPCWSTR GetPassword();
	HRESULT SaveEmailPassword(LPCWSTR lpszEmail, LPCWSTR lpszPassword);
	HRESULT ClearEmailPassword();
	LPCWSTR GetForbiddenExt();
	
	
private:
	MSXML2::IXMLDOMDocumentPtr m_xmlDocPtr;
	LPWSTR m_eMail;
	LPWSTR m_password;
	LPWSTR m_forbiddenExt;
};
