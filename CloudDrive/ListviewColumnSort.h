#pragma once
#include <windows.h>


int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2,  LPARAM   lParamSort);
VOID GetListViewSubItemText(int itemIndex,  int subItemIndex,  LPWSTR szText, int iSize);

