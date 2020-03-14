/*************************************************************************
	File: ShareAPIUser.h
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/18
	Description:
			ShareAPIUser is the wrapper class around user object returned from Share API requests.
			The client provide the username and password, Share returns the SessionId, secrect, 
			and the user's registered Adobe Id full name.
	Memo:

*****************************************************************************/
#pragma once
using namespace std;

class ShareAPIUser
{
private:
	WCHAR* m_pszUserName;       // do not use Array type, use pointer is more flexible and can avoid buffer overflow.
	WCHAR* m_pszPassword;
	WCHAR* m_pszSessionId;
	WCHAR* m_pszSecret;
	WCHAR* m_pszName;

public:
	ShareAPIUser(LPCWSTR lpszUsrName, LPCWSTR lpszPwd);
	~ShareAPIUser(void);

public:
	LPCWSTR GetPassword();
	void SetPassword(LPCWSTR lpszPwd);
	LPCWSTR GetUserName();
	void SetUserName(LPCWSTR lpszUsrName);
	LPCWSTR GetSessionId();
	void SetSessionId(LPCWSTR lpszSessId);
	LPCWSTR GetSecret();
	void SetSecret(LPCWSTR lpszSecret);
	LPCWSTR GetName();
	void SetName(LPCWSTR lpszName);
};
