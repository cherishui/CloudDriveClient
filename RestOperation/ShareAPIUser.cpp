/*************************************************************************
	File: ShareAPIUser.cpp
	Author: Jerry Wang
	Version: 1.0 
	Date: 2009/9/18
	Description:
			ShareAPIUser is the wrapper class around user object returned from Share API requests.
			The client provide the username and password, Share returns the SessionId, secrect, 
			and the user's registered Adobe Id full name.
	Memo:

*****************************************************************************/

#include "StdAfx.h"
#include "ShareAPIUser.h"

/******************************************************************************
Function:   ShareAPIUser Constructor
Description:  
		The constructor of the class. After the construction, you get a ShareAPIUser object without SessionId and Secret,
		which means this user is not loggedin.  
		Only after calling the Login function (using ShareAPISession object),  the ShareAPIUser object's SessionId, Secret and Name  
		are initialized,  the user has a loggedin state.

Parameters:
		[in]  lpszUsrName:	the user name.
		[in]  lpszPwd:			the password.

Return:
		No return value.

Other:
		Dynamic array is used to allocate memory to the member variables of this class, which can avoid
		buffer-overflow bugs for fixed array.  It also can save memory!

****************************************************************************/
ShareAPIUser::ShareAPIUser(LPCWSTR lpszUsrName, LPCWSTR lpszPwd):
	m_pszSessionId(NULL), m_pszSecret(NULL),m_pszName(NULL)
{		
	if (lpszUsrName == NULL)
	{
		m_pszUserName = NULL;
	}
	else
	{	//this is the typical (probably the best) method to copy string!
		int nDimension = wcslen(lpszUsrName) + 1; 
		m_pszUserName =  new WCHAR[nDimension]();
		wcsncpy(m_pszUserName, lpszUsrName, nDimension);
	}

	if (lpszPwd == NULL)
	{
		m_pszPassword = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszPwd) + 1;
		m_pszPassword = new WCHAR[nDimension]();
		wcsncpy(m_pszPassword, lpszPwd, nDimension);
	}

	//m_pszSessionId = NULL;
	//m_pszSecret = NULL;
	//m_pszName = NULL;
}

/******************************************************************************
Function:   ShareAPIUser Destructor
Description:  
		The destructor of the class. 
		In this function, we must release all the allocated memory of the member variables. 

Parameters:
		None

Return:
		None

Other:
		delete []  not delete is employed!
****************************************************************************/
ShareAPIUser::~ShareAPIUser(void)
{
	if (m_pszUserName)
	{
		delete []  m_pszUserName;
	}

	if (m_pszPassword)
	{
		delete []  m_pszPassword;
	}

	if (m_pszSessionId)
	{
		delete []  m_pszSessionId;
	}

	if (m_pszSecret)
	{
		delete []  m_pszSecret;
	}

	if (m_pszName)
	{
		delete []  m_pszName;
	}
} 



/****************************************************************************
About SetXXX function:
	If the member variables has been initialized before, delete the old value, then copy the new value into it.
********************************************************************************/
LPCWSTR ShareAPIUser::GetPassword()
{
	return m_pszPassword;
}

void ShareAPIUser::SetPassword(LPCWSTR lpszPwd)
{
	if (m_pszPassword)
	{
		delete []  m_pszPassword;
	}
	if (lpszPwd == NULL)
	{
		m_pszPassword = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszPwd) +1;
		m_pszPassword = new WCHAR[nDimension]();
		wcsncpy(m_pszPassword, lpszPwd, nDimension);
	}
}

LPCWSTR ShareAPIUser::GetUserName()
{
	return m_pszUserName;
}

void ShareAPIUser::SetUserName(LPCWSTR lpszUsrName)
{
	if (m_pszUserName)
	{
		delete []  m_pszUserName;
	}
	if (lpszUsrName == NULL)
	{
		m_pszUserName = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszUsrName) +1;
		m_pszUserName = new WCHAR[nDimension]();
		wcsncpy(m_pszUserName, lpszUsrName, nDimension);
	}
}

/*****************************************************************************
About: Get|Set -- SessionId|Secret|Name 
	We can get the SessionID/Secret/Name from Share's response after a new session request containing data proviede by the ShareAPIUser object.
	The secret is unique to this session and is used in the authorization header of all subsequent request.
	<response status="ok">
		<sessionid>1ab1a6190dc1f1f849ca7649190cca83</sessionid>
		<secret>3965c47eb9c937f78d49f3f33e19ca45</secret>
		<name>John Q. Example</name>
		<level>1</level>
	 </response>	

****************************************************************************/
LPCWSTR ShareAPIUser::GetSessionId()
{
	//if (m_pszSessionId)
		return m_pszSessionId;
	//else
	//	return NULL;
}

void ShareAPIUser::SetSessionId(LPCWSTR lpszSessionId)
{
	if (m_pszSessionId)
	{
		delete [] m_pszSessionId;	
	}
	if (lpszSessionId == NULL)
	{
		m_pszSessionId = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszSessionId) + 1;
		m_pszSessionId = new WCHAR[nDimension]();
		wcsncpy(m_pszSessionId, lpszSessionId, nDimension);
	}
}

LPCWSTR ShareAPIUser::GetSecret()
{
	return m_pszSecret;
}

void ShareAPIUser::SetSecret(LPCWSTR lpszSecret)
{
	if (m_pszSecret)
	{
		delete []  m_pszSecret;
	}
	if (lpszSecret == NULL)
	{
		m_pszSecret = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszSecret) +1;
		m_pszSecret = new WCHAR[nDimension]();
		wcsncpy(m_pszSecret, lpszSecret, nDimension);
	}
}

//Get the user's registered Adobe ID name from Share's response after Share receives a new session request .
LPCWSTR ShareAPIUser::GetName()
{
	return m_pszName;
}

void ShareAPIUser::SetName(LPCWSTR lpszName)
{
	if (m_pszName)
	{
		delete []  m_pszName;
	}
	if (lpszName == NULL)
	{
		m_pszName = NULL;
	}
	else
	{
		int nDimension = wcslen(lpszName)+1;
		m_pszName = new WCHAR[nDimension]();
		wcsncpy(m_pszName, lpszName, nDimension);
	}
}

