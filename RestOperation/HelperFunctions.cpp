#include "StdAfx.h"
#include "HelperFunctions.h"
#include "WinErrorException.h"
#include <time.h>
#include <sys/timeb.h>
#include <assert.h>
#include "../CloudDrive/AppConfig.h"
/*get the right fileName from the whole file path.
[in] szFilePath:   the path of the file
[out] fileName:    the output result of file name.
[in] uSize:  size of the fileName buffer.
*/
void HelperFunctions::GetFileNameFromFilePath(LPCWSTR lpszFilePath, LPWSTR lpszFileName, size_t size)  
{
	//if the last charactor of the file path is '/' or '\' , get rid of it.
	size_t lengthOfFilePath = wcslen(lpszFilePath);		 
	if (lengthOfFilePath <= 0)
		throw runtime_error("the filePath can not be empty");
	WCHAR *filePathCopy = new WCHAR[lengthOfFilePath+1](); 
	wcscpy_s(filePathCopy, lengthOfFilePath+1, lpszFilePath);	
	WCHAR* pLasWCHAR = filePathCopy+lengthOfFilePath -1;
	if (* pLasWCHAR == L'/' || *pLasWCHAR ==L'\\')
		*pLasWCHAR = L'\0';  

	for (size_t i=0;i<lengthOfFilePath; i++)
	{
		--pLasWCHAR;
		if ( *pLasWCHAR == L'/' || *pLasWCHAR == L'\\' )
			break;
	}
	wcscpy_s(lpszFileName, size, ++pLasWCHAR);
	delete[] filePathCopy;
}

/* get the length of a file, using windows API , 
this function is valid for large file which is more than 2G bytes. 
(howerver, we should forbid uploading files whose size is more than 2GB)
*/
INT64 HelperFunctions::GetFileSize(LPCWSTR szFilePath)
{
	LARGE_INTEGER sizeOfFile;
	HANDLE hFile = CreateFile(szFilePath, 
														GENERIC_READ,
														0,
														NULL,
														OPEN_EXISTING,
														0,
														0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD errCode =::GetLastError();
		throw WinErrorException(errCode, L"create file failed. ");
	}
	if (! GetFileSizeEx( hFile , &sizeOfFile) )
	{
		DWORD errCode =::GetLastError();
		throw WinErrorException(errCode, L"get file size failed. Note: only windows2000 and above are supported.");
	}
	CloseHandle(hFile);
	return sizeOfFile.QuadPart;
}


/*********************************************************************************
get the MD5 hash code for a specific string
	[in]originalStr: the orignianl string 
	[out] lpszHashedStr: the MD5 hashed result. NOTICE:The Length of this buffer should be 33 or above!
***************************************************************************************/
void HelperFunctions::MD5Hash(LPCWSTR lpszOriginalStr, LPWSTR lpszHashedStr)
{ 
	HCRYPTPROV hCryptProv; 
	HCRYPTHASH hHash; 
	BYTE szHashData[17]; 
	DWORD dwHashLen= 16; // The MD5 algorithm always returns 16 bytes. 
   

	int nStrLength = WideCharToMultiByte(CP_UTF8,0, lpszOriginalStr,-1, NULL, 0, NULL,NULL);

	CHAR*  lpszContent = new CHAR[nStrLength+1]();
	WideCharToMultiByte(CP_UTF8,0, lpszOriginalStr,-1, lpszContent, nStrLength+1, NULL,NULL);
	
	if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) )
	{
//		DWORD aa = GetLastError();
		throw runtime_error("CryptAcquireContext failed");
	}
	if(!CryptCreateHash(hCryptProv, CALG_MD5,	0, 0, &hHash)) 
	{
		throw runtime_error("CryptCreateHash failed");
	}
	if(!CryptHashData(hHash, (BYTE*)lpszContent, strlen(lpszContent), 0))
	{
		throw runtime_error("CryptHashData failed");
	}
	if (!CryptGetHashParam(hHash, HP_HASHVAL, szHashData, &dwHashLen, 0)) 
	{
		throw runtime_error("CryptGetHashParam failed");
	}

	delete [] lpszContent;
	CryptDestroyHash(hHash); 
	CryptReleaseContext(hCryptProv, 0); 

	for(int i=0;i<16; i++)
	{
		_swprintf(lpszHashedStr+2*i, L"%02x", szHashData[i]);
	}
	lpszHashedStr[32]= L'\0';
}


/*get the millisecond count since midnight , January 1, 1970, UTC.
*/
INT64 HelperFunctions::GetCurrentTimeStamp()
{
	struct _timeb currentTime;
	_ftime64_s(&currentTime);

	return (currentTime.time*1000 +currentTime.millitm);
}

///*Generator Random String with specific length, 
//  randStr[in, out]:  the  buffer to hold  the randome string, its length should be strLength.
//  strLength[in]:   the length of  randStr, in WCHAR, including the terminal NULL.
//*/
//VOID  HelperFunctions::GenerateRandomStr(LPWSTR randStr, size_t  strLength)
//{
//	const WCHAR CCH[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//	//seed the randome-number generator with current time so that the numbers will be different everytime we run
//	srand((unsigned)time(NULL));
//	
//	for (int i=0; i<strLength-1; ++i)
//	{
//		int x = rand()/(RAND_MAX/51);
//		randStr[i] = CCH[x];
//	}
//	randStr[strLength-1] = '\0';
//}


/*get the filename extension (like exe, dll, txt, doc) from the filename.
[in] szFileName:   the filename of the file
[out] szFileExt:    the extension name of file .
[in] size:  size of the szFileExt buffer.

if the filename do not have an extension, like filename "abc",  the szFileExt will be empty string;
*/
void HelperFunctions::GetFileExtFromFileName(LPCWSTR szFileName, LPWSTR szFileExt, size_t uSize)  
{
	size_t uLengthOfFileName = wcslen(szFileName);		 

	if (uLengthOfFileName <= 0)
		throw runtime_error("the file name can not be empty");


	const WCHAR* pLasWCHAR = szFileName+uLengthOfFileName -1;       
	
	bool hasExt = false;
	for (size_t i=0;i<uLengthOfFileName; i++)
	{
		--pLasWCHAR;
		if ( *pLasWCHAR == L'.' )
		{
			hasExt = true;
			break;
		}
	}
		
	memset(szFileExt, 0, uSize); 
	if (hasExt)
		wcscpy_s(szFileExt, uSize, ++pLasWCHAR);
}


/**************************************
convert the string to Upper case

the return value is the converted uppercase string.
******************************************/
LPWSTR HelperFunctions::ConvertToUpper(LPWSTR szSource)
{
	assert(szSource != NULL);
	
	LPWSTR szString = szSource;
	
	while(*szString)
	{
		*szString = towupper(*szString);
		szString ++;
	}
	return szSource;
} 

/****************************************************************
Convert the string to lower case
 the return value is the converted lowercase string.
******************************************************************/
LPWSTR HelperFunctions::ConvertToLower(LPWSTR szSource)
{
	assert(szSource != NULL);
	
	LPWSTR szString = szSource;
	
	while(*szString)
	{
		*szString = towlower(*szString);
		szString ++;
	}
	return szSource;
}


//void	HelperFunctions::CheckFileNameAddExt(LPCWSTR lpszFileName, LPWSTR lpszFileNameNewExt, size_t size)
//{
//	WCHAR szExt[16] ={0};
//	HelperFunctions::GetFileExtFromFileName(lpszFileName, szExt, 16);
//	
//	AppConfig* pCfg = new AppConfig();
//	
//	LPCWSTR lpszForbiddenExt = pCfg->GetForbiddenExt();
//	
//	if (lpszForbiddenExt == NULL) 
//	{
//		lpszForbiddenExt = L"AAC,MOV,MP3,WMV,OGG,FLV,XVID,DIVX,TTF,DFONT,OTF,TGZ,7Z,SIT,EXE,DLL,JAR,RAR,WAR,CAB,JS,VB,BAT,CHM,CMD,COM,CPL,CRT,HLP,HTA,INF,INS,ISP,JSE,LNK,MDB,MS,PCD,PIF,RETG,SCR,SCT,SHS,WS,SH,CSH,TCSH,KSH,BASH,ZSH";
//	}
//	
//	
//	wcscpy_s(lpszFileNameNewExt, size, lpszFileName);
//	//¼Ó.wjh
//	if (wcsstr(lpszForbiddenExt, HelperFunctions::ConvertToUpper(szExt)) != NULL)
//	{
//		wcscat_s(lpszFileNameNewExt,  size, L".wjh");
//	}
//	delete pCfg;
//	
//}
