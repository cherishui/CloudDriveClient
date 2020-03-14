#pragma once
#include <windows.h>
/****************************************************************
This file includes functions can be used in several files.
**************************************************************/

void AddErrMsgToDlgListbox(HWND hProgressDlg, LPCWSTR lpszErrMsg);
void	GetSelectedNodeIds(LPWSTR lpszNodeIds);