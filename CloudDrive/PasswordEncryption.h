#pragma once
#include <windows.h>

class CPasswordEncryption
{
public:
	static  void PasswordEncrypt( LPCWSTR szPassword,  LPWSTR szPasswordEncrypted);
	static  void PasswordDecrypt(LPCWSTR szPasswordEncrypted, LPWSTR originalPassword);

};
