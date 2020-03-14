#pragma once

/*
This class holds static helper functions for this  Share app.
*/
class HelperFunctions
{
public:
	static INT64		GetFileSize(LPCWSTR lpszFilePath) ; 
	static void		MD5Hash(LPCWSTR lpszOriginalStr,LPWSTR lpszHashedStr);
	static INT64		GetCurrentTimeStamp();
	static void		GetFileExtFromFileName(LPCWSTR lpszFileName, LPWSTR lpszFileExt, size_t uSize);
	static void		GetFileNameFromFilePath(LPCWSTR lpszFilePath, LPWSTR lpszFileName, size_t size); 
	//static void		CheckFileNameAddExt(LPCWSTR lpszFileName, LPWSTR lpszFileNameNewExt, size_t size);

	static LPWSTR		ConvertToUpper(LPWSTR szStr);
	static LPWSTR		ConvertToLower(LPWSTR szStr);

};
