#pragma once
#include "skyclient.h"

//the index of the parts in the StatusBar
#define	PARTINDEX_MAIN		0
#define	PARTINDEX_BACKGROUND_UPLOADDOWNLOAD	1
#define	PARTINDEX_STATUS		2


/*几种需要Refresh statusbar的情况
ITEMCOUNT:   15 items
SELECTED:		2  items selected
NEWUPLOADING:	uploading....         uploading++  
UPLOADINGFINISHED:  15 items,     uploading--
NEWDOWNLOADING:  downloading....     downloading++
DOWNLOADINGFINISHED:  15 items,  downloading--
*/
//enum  StatusBarChange
//{
//	SB_SELECTEDCHANGE, 
//	SB_UPDOWNPROGRESS,
//	SB_UPDOWNFINISHED
//};
//
//
//#define UPLOADING  1
//#define DOWNLOADING 2
//
//struct FILETRANSFERINFO  //
//{
//	INT	iDirection;   //UPLOADING or DOWNLOADING
//	WCHAR szFileName[MAX_PATH];
//	//LONG lFileSize;			//the whole size of the file
//	//LONG lFinishedSize;
//	//INT	iLastedTime;   //how many seconds spend
//};


void  CreateStatusbar(void);
void RefreshStatusBar (/*StatusBarChange  sbc, FILETRANSFERINFO* pFileInfo*/);


