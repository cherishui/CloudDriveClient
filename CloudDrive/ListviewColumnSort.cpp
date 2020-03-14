#include <windows.h>
#include <atlcomtime.h>
#include <commctrl.h>
#include "GlobalVar.h"
#include "ListviewColumnSort.h"
#include "UIListview.h"

//按列排序的比较过程 compare procedure
int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2,  LPARAM   lParamSort)
{
	////注意: 在FillListview的时候,必须		lvItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM ;   		lvItem.lParam = (LPARAM)&files[i];
	int idx1 = (int)lParam1;
	int idx2 =  (int)lParam2;
	int iResult=0;

	switch (lParamSort)
	{
		case COLUMNINDEX_NAME:
		{			
			WCHAR szText1[MAX_PATH] ={0};  
			WCHAR szText2[MAX_PATH] ={0};

			GetListViewSubItemText(idx1, COLUMNINDEX_NAME, szText1, MAX_PATH);
			GetListViewSubItemText(idx2, COLUMNINDEX_NAME, szText2, MAX_PATH);
			
			iResult = lstrcmp(szText1, szText2);
			break;
		}
		case COLUMNINDEX_SIZE:
		{
			WCHAR szText1[32] ={0};  
			WCHAR szText2[32] ={0};

			GetListViewSubItemText(idx1, COLUMNINDEX_SIZE, szText1, 32);
			GetListViewSubItemText(idx2, COLUMNINDEX_SIZE, szText2, 32);
			
			//将数字右对齐！
			WCHAR szText1Aligned[32] ={0};
			WCHAR szText2Aligned[32] ={0};
			
			swprintf(szText1Aligned, L"%030s", szText1);
			swprintf(szText2Aligned, L"%030s", szText2);
			
			iResult = lstrcmp(szText1Aligned, szText2Aligned);
			break;
		}
		
		case COLUMNINDEX_TYPE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			GetListViewSubItemText(idx1, COLUMNINDEX_TYPE, szText1, 64);
			GetListViewSubItemText(idx2, COLUMNINDEX_TYPE, szText2, 64);
			
			iResult = lstrcmp(szText1, szText2);
			break;
		}
		
		case COLUMNINDEX_CREATEDATE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			GetListViewSubItemText(idx1, COLUMNINDEX_CREATEDATE, szText1, 64);
			GetListViewSubItemText(idx2, COLUMNINDEX_CREATEDATE, szText2, 64);
			
			COleDateTime cDt1;
			cDt1.ParseDateTime(szText1);
			COleDateTime cDt2;
			cDt2.ParseDateTime(szText2);
			
			COleDateTimeSpan  dts = cDt1-cDt2;
			if (dts.GetStatus() == COleDateTimeSpan::invalid)
			{
				//ASSERT(FALSE);
				iResult =0 ;
			}
			
			iResult = dts.GetTotalSeconds();
			
			break;
		}
		
		case COLUMNINDEX_MODIFYDATE:
		{
			WCHAR szText1[64] ={0};  
			WCHAR szText2[64] ={0};

			GetListViewSubItemText(idx1, COLUMNINDEX_MODIFYDATE, szText1, 64);
			GetListViewSubItemText(idx2, COLUMNINDEX_MODIFYDATE, szText2, 64);
			
			COleDateTime cDt1;
			cDt1.ParseDateTime(szText1);
			COleDateTime cDt2;
			cDt2.ParseDateTime(szText2);
			
			COleDateTimeSpan  dts = cDt1-cDt2;
			if (dts.GetStatus() == COleDateTimeSpan::invalid)
			{
				//ASSERT(FALSE);
				iResult =0 ;
			}
			
			iResult = dts.GetTotalSeconds();
			
			break;
		}
		
		//case COLUMNINDEX_STATUS:
		//{
		//}
		
		
		
		
		
		
		//...................
		
		default:
		{
			iResult=0;
		}
	}
	
	if (!g_bSortAscending)
		iResult = -iResult;
	
	return iResult;
}





//根据序号和列号得到listview的text
VOID GetListViewSubItemText(int itemIndex,  int subItemIndex,  LPWSTR szText, int iSize)
{
	LVFINDINFO  lvFi;
	ZeroMemory(&lvFi, sizeof(LVFINDINFO));
	lvFi.flags = LVFI_PARAM;
	lvFi.lParam = itemIndex;
	int idx = SendMessage(g_hListView, LVM_FINDITEM, -1,  (LPARAM)&lvFi);

	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = idx;
	lvItem.iSubItem = subItemIndex;
	lvItem.pszText = szText;
	lvItem.cchTextMax = iSize;
	SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);
	
	
	//if the item is a folder, always to be the smallest.  "Folder"
	WCHAR szItemType[128]={0};
//	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = idx;
	lvItem.iSubItem = COLUMNINDEX_TYPE;
	lvItem.pszText = szItemType;
	lvItem.cchTextMax = 128;
	SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);
	
	if ( (lstrcmp(szItemType, L"Folder")==0 ) &&  (subItemIndex == COLUMNINDEX_NAME ||subItemIndex == COLUMNINDEX_SIZE ||subItemIndex== COLUMNINDEX_TYPE ||subItemIndex==COLUMNINDEX_STATUS)  )  //is folder!
	{
	//如果是folder,无论对那一列排序，都根据folder 名字 （前面加上两个空格）来排序
		WCHAR szItemName[MAX_PATH-4]={0};
		LVITEM lvItem;
		ZeroMemory(&lvItem, sizeof(LVITEM));
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = idx;
		lvItem.iSubItem = COLUMNINDEX_NAME;
		lvItem.pszText = szItemName;
		lvItem.cchTextMax = MAX_PATH;
		SendMessage(g_hListView, LVM_GETITEMTEXT, idx, (LPARAM)&lvItem);

		WCHAR szFolderName[MAX_PATH]={L' ', L' '};  
		wcscat(szFolderName, szItemName);
		wcscpy_s(szText, iSize, szFolderName);
	}
}
