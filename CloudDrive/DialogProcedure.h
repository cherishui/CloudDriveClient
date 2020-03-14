#pragma once
#include <windows.h>

//Login dialog procedure  (modal)
INT CALLBACK LoginDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//wait message dialog (modaless)
INT CALLBACK WaitingMessageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//输入目录名的 modal dialog
INT CALLBACK FolderNameDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

////输入email名字的用来Share的modal dialog
//INT CALLBACK ShareDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//progress dialog procedure  (modaless)
INT CALLBACK ProgressDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//About dialog procedure (modal)
INT CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);