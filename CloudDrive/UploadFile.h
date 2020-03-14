#pragma once
#include <windows.h>
#import <msxml6.dll>
void UploadFiles();

MSXML2::IXMLDOMNodePtr UploadOneFile(LPWSTR lpszFileFullPathName,LPCWSTR lpszFolderNodeId, int iCurrent, HWND hProgressDlg);