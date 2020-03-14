/******************************************************************************************
FileName: ShareAPISession.h
Author:   Jerry Wang
Version: 1.0
Date:	  2009/09/17
Description:
	  This class is responsible for the Adobe Share API user & session related functionalities, 
	  For example, logining in and out, creating sessions, and so on.
	  Usually, one instance of this class is corresponding to one ShareAPI user. 
	  In most scenarios,  one process will only have one ShareAPISession instance, 
	  but can run several threads using  different ShareAPIRequest instances.
Memo:

*******************************************************************************************/
#pragma once
#include "stdafx.h"
#include "ShareAPIUser.h"


class ShareAPISession
{
public:
	ShareAPISession(void);
	~ShareAPISession(void);
private:
	//prevent : copy constructor and assign operator 
	ShareAPISession(const ShareAPISession  &orig);
	ShareAPISession& operator = (const ShareAPISession  &orig);

public:	
	void Login(ShareAPIUser* pUser);  
	void Logout(ShareAPIUser* pUser);

private:
	void generateAuthorizationHeader(ShareAPIUser* pUser, LPCWSTR lpszMethod, LPCWSTR lpszUrl);
	void getAuthToken(ShareAPIUser* pUser);
	void createSession(ShareAPIUser* pUser);
	bool isUserLoggedIn(ShareAPIUser* pUser);

private:
	HINTERNET m_hOpen;								
	HINTERNET m_hConnect;

	WCHAR* m_lpszAuthToken;
	WCHAR* m_lpszAuthHeader;
};
