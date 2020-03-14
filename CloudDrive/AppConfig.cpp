#include <stdexcept>
#include "AppConfig.h"
#include "CloudDrive.h"
#include "PasswordEncryption.h"

/*This calss operate "Config.xml" file to get and set informaition*/
AppConfig::AppConfig(void) :
	m_xmlDocPtr(NULL),
	m_eMail(NULL),
	m_password(NULL),
	m_forbiddenExt(NULL)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (S_OK !=CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (void**)&m_xmlDocPtr))
	{
		throw std::runtime_error("DOM  Document object create failed!");
	} 
	
	VARIANT_BOOL bSuccessful = m_xmlDocPtr->load(L"Config.xml");
	if ( !bSuccessful)
	{
		m_xmlDocPtr = NULL;
	}
}

AppConfig::~AppConfig(void)
{
	if (m_eMail)
		delete [] m_eMail;
	if (m_password)
		delete [] m_password;
	if (m_forbiddenExt)
		delete [] m_forbiddenExt;
		
	if (m_xmlDocPtr != NULL)
		m_xmlDocPtr.Release();
		
	CoUninitialize();  
}


/****************************************************
Return the email in the Config.xml file.

Return NULL, If  1. Config.xml does not exist    or  2. can not find the node "configure/user/email"  

*****************************************************/
LPCWSTR AppConfig::GetEmail()
{
	if (m_xmlDocPtr != NULL)
	{
		MSXML2::IXMLDOMNodePtr  emailNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/email"));
		
		if (emailNodePtr != NULL)
		{
			m_eMail = new WCHAR[LEN_USEREMAIL]();
			wcscpy_s( m_eMail, LEN_USEREMAIL, emailNodePtr->Gettext());
		}
	}
	return m_eMail;
}

LPCWSTR AppConfig::GetPassword()
{
	if (m_xmlDocPtr != NULL)
	{
		MSXML2::IXMLDOMNodePtr  pwdNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/password"));
		
		if ( pwdNodePtr != NULL)
		{
			m_password = new WCHAR[LEN_USERPASSWORD]();
			CPasswordEncryption::PasswordDecrypt(pwdNodePtr->Gettext(), m_password);  //decrypt
		}
	}
	return m_password;
}

LPCWSTR AppConfig::GetForbiddenExt()
{
	if (m_xmlDocPtr != NULL)
	{
		MSXML2::IXMLDOMNodePtr  forbiddenExtPtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/forbiddenextension"));
		
		if (forbiddenExtPtr != NULL)
		{
			m_forbiddenExt= new WCHAR[LEN_FORBIDDENEXT]();
			wcscpy_s(m_forbiddenExt, LEN_FORBIDDENEXT,  forbiddenExtPtr->Gettext());
		}
	}
	return m_forbiddenExt;
}

HRESULT AppConfig::SaveEmailPassword(LPCWSTR lpszEmail, LPCWSTR lpszPassword)
{
	if (m_xmlDocPtr != NULL)
	{
		MSXML2::IXMLDOMNodePtr  emailNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/email"));
		emailNodePtr->text = _bstr_t(lpszEmail);

		MSXML2::IXMLDOMNodePtr  pwdNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/password"));
		WCHAR szEncryptedPwd[LEN_USERPASSWORD] ={0};
		CPasswordEncryption::PasswordEncrypt(lpszPassword, szEncryptedPwd);   //encrypt the password!!
		pwdNodePtr->text =szEncryptedPwd;
		
		HRESULT hr;
		try
		{
			hr = m_xmlDocPtr->save(_variant_t("Config.xml"));
		}
		catch(...)   
		{
			hr = E_FAIL;
		}
		
		return  hr;
	}
	else
		return  E_FAIL;   //the file does not exist or other reason
}


HRESULT AppConfig::ClearEmailPassword()
{
	if (m_xmlDocPtr != NULL)
	{
		MSXML2::IXMLDOMNodePtr  emailNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/email"));
		emailNodePtr->text = _bstr_t();

		MSXML2::IXMLDOMNodePtr  pwdNodePtr = m_xmlDocPtr->selectSingleNode(_bstr_t("configure/user/password"));
		pwdNodePtr->text = _bstr_t();
		
		HRESULT hr;
		
		try
		{
			hr =  m_xmlDocPtr->save(L"Config.xml");
		}
		catch(...)
		{
			hr = E_FAIL;
		}
		
		return hr;
	}
	else
		return E_FAIL;
}


