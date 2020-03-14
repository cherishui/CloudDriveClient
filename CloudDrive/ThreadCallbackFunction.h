#pragma once
#include <windows.h>

class ThreadParamBase;  //forward declaration          

//void ExitProcessingThreadMSG(ThreadParamBase * pParam);
//void ExitProcessingThreadPROGRESS(ThreadParamBase * pParam);
void CreateFolderThread(PVOID pvoid);
void ShareFileThread(PVOID pvoid);
void DeleteFilesThread(PVOID pvoid);