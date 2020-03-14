#include "PasswordEncryption.h"


/***************************************************
 szPassword:  original password:
 szPasswordEncrypted:  encrypted password
*************************************************/
void CPasswordEncryption::PasswordEncrypt(  LPCWSTR szPassword,  LPWSTR szPasswordEncrypted)
{
	int key = 129;
	
	ZeroMemory(szPasswordEncrypted, sizeof (szPasswordEncrypted));
	
	for (int i=0; i<lstrlen(szPassword); i++)
	{
		szPasswordEncrypted[i] =   (szPassword[i]^key);
	}
};


void CPasswordEncryption::PasswordDecrypt(LPCWSTR szPasswordEncrypted, LPWSTR originalPassword)
{
	PasswordEncrypt(szPasswordEncrypted, originalPassword);
}
