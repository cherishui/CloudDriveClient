#pragma once
#include <windows.h>
#import <msxml6.dll>

//the index of the listview columns
#define	COLUMNINDEX_NAME		0
#define	COLUMNINDEX_SIZE		1
#define	COLUMNINDEX_TYPE		2
#define	COLUMNINDEX_CREATEDATE		3
#define	COLUMNINDEX_MODIFYDATE		4
//#define	COLUMNINDEX_STATUS		5
#define	COLUMNINDEX_NODEID		5 


void CreateListView(void);
void InitListViewColumns();
void InitListViewImageLists();
void AddNodeToListView(MSXML2::IXMLDOMNodePtr pNode);
void RefreshListView(LPCWSTR lpszFolderNodeId);
void 	HandleListViewNotification(LPNMHDR pnmh);
