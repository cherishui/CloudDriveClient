/*****************************************************************
This file contains  procedures for different dialog box
Author: Jerry wang --王建华
All right reserved.
********************************************************************/
#include <windows.h>
#include <commctrl.h>
#include <process.h>  
#include "AppConfig.h"
#include "CloudDrive.h"
#include "WinErrorException.h"
#include "ShareAPIException.h"
#include "ThreadParam.h"
#include "DialogProcedure.h"
#include "ShareAPI.h"
#include "resource.h"
/********************************************************************
Share login thread callback procedure
This is a private function, only used in this file.
**********************************************************************/
AppConfig* pAppCfg = NULL;  //global variable for login procedure

void loginThread(PVOID pvoid)
{
	ThreadParamLogin* pParam = (ThreadParamLogin*)pvoid;
	LPCWSTR szUserEmail = pParam->m_szUserEmail;
	LPCWSTR szPassword = pParam->m_szPassword;
	HWND hDlg = pParam->m_hMsg;
	
	BOOL isLoggedin = TRUE;
	try
	{
		g_pShareAPI->Login();
	}
	catch( WinErrorException& exp)  
	{
		isLoggedin = FALSE;
		SetWindowText(GetDlgItem(hDlg, IDC_STATICERROR), L"Connection failed! ");
	}
	catch (  ShareAPIException& exp)  
	{
		isLoggedin = FALSE;
		SetWindowText(GetDlgItem(hDlg, IDC_STATICERROR), exp.GetErrorDesc());
	}
	catch (...)  //other exception, unknown reason
	{
		isLoggedin = FALSE;
		SetWindowText(GetDlgItem(hDlg, IDC_STATICERROR), L" Login failed! ");
	}		
	
	if (isLoggedin)  //login succeed,  save useremail and password , and 
	{
		//save the email and password to config file if the remeber me checkbox is checked
		LRESULT checkStatus = SendMessage(GetDlgItem(hDlg, IDC_CHECKREMEBER), BM_GETCHECK, 0, 0);
		if ( checkStatus == BST_CHECKED)   //checked
		{
			pAppCfg->SaveEmailPassword(szUserEmail, szPassword);
		}
		else
		{
			pAppCfg->ClearEmailPassword();
		}

		//quit
		delete pAppCfg;
		pAppCfg = NULL;
		EndDialog(hDlg, IDOK);
	}
	else  //login failed.
	{
		//enable 	buttons
		EnableWindow(GetDlgItem(hDlg,IDOK), TRUE);
		EnableWindow(GetDlgItem(hDlg,IDCANCEL), TRUE);
	}		
	
	delete pParam;
	_endthread();
}


/***********************************************************************
Login dialog procedure, it handles login dialog's message
This procedure use another thread to excute Share Login.  (LoginThread)
*************************************************************************/
HBRUSH g_hbrBackground = NULL;  

INT  CALLBACK LoginDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{	
			//center the modal dialog
			HWND hDesktop = GetDesktopWindow();
			
			RECT rcDlg, rcDesktop;
			
			GetWindowRect(hDesktop, &rcDesktop);
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, HWND_TOP, 
						(rcDesktop.right - (rcDlg.right - rcDlg.left))/2,
						(rcDesktop.bottom - (rcDlg.bottom - rcDlg.top))/2,
						0, 0,
						SWP_NOSIZE);
			
			//set the checkbox state to Checked
			SendMessage(GetDlgItem(hDlg, IDC_CHECKREMEBER), BM_SETCHECK, BST_CHECKED, 0);
	
			//get the user email and pasword info from config.xml
			pAppCfg = new AppConfig();
			LPCWSTR lpszEmail =  pAppCfg->GetEmail();
			
			if (lpszEmail != NULL)
			{
				HWND hEmailEdit = GetDlgItem(hDlg, IDC_EDITEMAIL);
				SetWindowText(hEmailEdit, lpszEmail);
			
				HWND hPwdEdit = GetDlgItem(hDlg, IDC_EDITPASSWORD);
				SetWindowText(hPwdEdit, pAppCfg->GetPassword());				
			}
		}
		break;
		
		case WM_CTLCOLORSTATIC:   // this message is used to setup the color of the login message text
		{
			if (GetDlgItem(hDlg, IDC_STATICERROR) == (HWND)lParam)
			{
				HDC hdcErrMsg = (HDC)wParam;
				SetTextColor(hdcErrMsg, RGB(0,0,255));
				SetBkMode(hdcErrMsg, TRANSPARENT);
				
				g_hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
				return (INT_PTR)g_hbrBackground;   //If an application processes this message, the return value is a handle to a brush that the system uses to paint the background of the static control.
			}
			else
			{
				return FALSE;
			}
		}
		break;
				
		case WM_NOTIFY:  //open  the sign in hyperlink
		{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;

			switch(pnmhdr->code)
			{
				case NM_CLICK:
				case NM_RETURN:
				{
					PNMLINK pnmlink= (PNMLINK)lParam;
					ShellExecuteW(NULL, L"open",pnmlink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
				}
			}
		}
		break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					//show waitting message , and disable the OK and Cancle button, 
					SetWindowText(GetDlgItem(hDlg, IDC_STATICERROR), L"       Logging in ...... ");

					//disable OK and Cancel Buuton				
					EnableWindow(GetDlgItem(hDlg,IDOK), FALSE);        
					EnableWindow(GetDlgItem(hDlg,IDCANCEL), FALSE);
					
					//initiate g_pShareAPI.
					WCHAR szUserEmail[LEN_USEREMAIL] ={0};
					GetDlgItemText(hDlg, IDC_EDITEMAIL, szUserEmail, LEN_USEREMAIL);
					
					WCHAR szPassword[LEN_USERPASSWORD] ={0};
					GetDlgItemText(hDlg, IDC_EDITPASSWORD, szPassword, LEN_USERPASSWORD);
					
					g_pShareAPI = new ShareAPI( szUserEmail, szPassword);
					
					//create another thread to process the login request,  pass dialog handle as the parame
					ThreadParamLogin *pParam = new ThreadParamLogin(szUserEmail, szPassword, hDlg);
					_beginthread(loginThread, 0, (PVOID)pParam);				
				}
				break;
				case IDCANCEL:
				{					
					delete pAppCfg;
					pAppCfg = NULL;
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
		
		case WM_DESTROY:
		{
			DeleteObject(g_hbrBackground);
			delete pAppCfg;
		}
		break;
		default:
			return FALSE;
	}
}

/***********************************************************************
The dialog procedure for the About Dialogbox
it is a Modal Dialog
***********************************************************************/
INT  CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//center the message dialog
			HWND hOwner = GetParent(hDlg);
			
			RECT rcDlg, rcOwner;
			GetWindowRect(hOwner, &rcOwner);
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, HWND_TOP, 
					rcOwner.left + ((rcOwner.right- rcOwner.left) -(rcDlg.right - rcDlg.left))/2,
					rcOwner.top + ((rcOwner.bottom - rcOwner.top ) - (rcDlg.bottom - rcDlg.top)) /2,
					0, 0,
					SWP_NOSIZE);
		}
		break;
		
		case WM_NOTIFY:
		{
			//Hyperlink (syslink) 
			LPNMHDR pnmhdr = (LPNMHDR)lParam;

			switch(pnmhdr->code)
			{
				case NM_CLICK:
				case NM_RETURN:
				{
					PNMLINK pnmlink= (PNMLINK)lParam;
					ShellExecuteW(NULL, L"open",pnmlink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
				}
			}
		}
		break;
		
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					EndDialog(hDlg, IDOK);
				}
				break;
				case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
		
		default:
			return FALSE;
	}
}

/************************************************************************
// this is the procedure for the waiting message dialog 
This dialog does not have a progress bar, only waiting message is given.
When deleting , creating folder, refreshing listview , waiting message dialog is shown.
************************************************************************/
INT  CALLBACK WaitingMessageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			//center the message dialog
			HWND hOwner = GetParent(hDlg);
			
			RECT rcDlg, rcOwner;
			GetWindowRect(hOwner, &rcOwner);
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, HWND_TOP, 
					rcOwner.left + ((rcOwner.right- rcOwner.left) -(rcDlg.right - rcDlg.left))/2,
					rcOwner.top + ((rcOwner.bottom - rcOwner.top ) - (rcDlg.bottom - rcDlg.top)) /2,
					0, 0,
					SWP_NOSIZE);
			// use the default font, now.
		}
		break;
		
		default:
			return FALSE; 
	}
	return TRUE;
}



/*****************************************************************************
//create folder  dialog
******************************************************************************/
INT CALLBACK FolderNameDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{	
			//center the message dialog
			HWND hOwner = GetParent(hDlg);
			
			RECT rcDlg, rcOwner;
			GetWindowRect(hOwner, &rcOwner);
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, HWND_TOP, 
					rcOwner.left + ((rcOwner.right- rcOwner.left) -(rcDlg.right - rcDlg.left))/2,
					rcOwner.top + ((rcOwner.bottom - rcOwner.top ) - (rcDlg.bottom - rcDlg.top)) /2,
					0, 0,
					SWP_NOSIZE);
		}
		break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					GetDlgItemText(hDlg, IDC_EDITFOLDERNAME, g_szNewFolderName, LEN_NAME);
					EndDialog(hDlg, IDOK);
				}
				break;
				case IDCANCEL:
				{					
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;

		default:
			return FALSE;
	}
}


////Sharer
//INT CALLBACK ShareDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch(uMsg)
//	{
//		case WM_INITDIALOG:
//		{	
//			//center the message dialog
//			HWND hOwner = GetParent(hDlg);
//			
//			RECT rcDlg, rcOwner;
//			GetWindowRect(hOwner, &rcOwner);
//			GetWindowRect(hDlg, &rcDlg);
//			
//			SetWindowPos(hDlg, HWND_TOP, 
//					rcOwner.left + ((rcOwner.right- rcOwner.left) -(rcDlg.right - rcDlg.left))/2,
//					rcOwner.top + ((rcOwner.bottom - rcOwner.top ) - (rcDlg.bottom - rcDlg.top)) /2,
//					0, 0,
//					SWP_NOSIZE);
//		}
//		break;
//
//		case WM_COMMAND:
//		{
//			switch(LOWORD(wParam))
//			{
//				case IDOK:
//				{
//					//WCHAR szShareMails[2048] ={0};
//					GetDlgItemText(hDlg, IDC_EDITMAILS, g_szShareEmails, 2048);
//					//EndDialog() destroys a modal dialog box, causing the system to end any processing for the dialog box.
//					EndDialog(hDlg, IDOK);
//				}	
//				break;
//				case IDCANCEL:
//				{					
//					EndDialog(hDlg, IDCANCEL);
//				}
//				break;
//			}
//		}
//		break;
//		
//
//		default:
//			return FALSE;
//	}
//}



/***********************************************************************
The procedure of the progress Dialog
This dialog is showed when uploading or downloading starts
************************************************************************/
INT CALLBACK ProgressDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			//Horiozen scroll bar setting
			SendDlgItemMessage(hDlg, IDC_LISTMSG, LB_SETHORIZONTALEXTENT, 1400, 0);
			//隐藏listbox控件
			SetDlgItemText(hDlg, IDC_BUTTONHIDE, L"Show details");  // change the text of the button
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTONHIDE), SW_HIDE);  //hide  the  showdetail button.
			
			HWND hList = GetDlgItem(hDlg, IDC_LISTMSG);
			ShowWindow(hList, SW_HIDE);  //hide the listbox
			
			//change the size of the dialog
			RECT rcList;
			GetWindowRect(hList, &rcList);
			
			RECT rcDlg;
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, NULL, 0, 0, rcDlg.right-rcDlg.left, (rcDlg.bottom-rcDlg.top)-(rcList.bottom-rcList.top), 
					SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

			//center the message dialog
			HWND hOwner = GetParent(hDlg);
			
			RECT  rcOwner;
			GetWindowRect(hOwner, &rcOwner);
			GetWindowRect(hDlg, &rcDlg);
			
			SetWindowPos(hDlg, HWND_TOP, 
					rcOwner.left + ((rcOwner.right- rcOwner.left) -(rcDlg.right - rcDlg.left))/2,
					rcOwner.top + ((rcOwner.bottom - rcOwner.top ) - (rcDlg.bottom - rcDlg.top)) /2,
					0, 0,
					SWP_NOSIZE);
			
		}
		break;
		
		case WM_CTLCOLORSTATIC:   // this message is used to setup the color of the Error message text
		{
			if (GetDlgItem(hDlg, IDC_STATICERROR) == (HWND)lParam)
			{
				HDC hdcErrMsg = (HDC)wParam;
				SetTextColor(hdcErrMsg, RGB(255,0,0));
				SetBkColor(hdcErrMsg, GetSysColor(COLOR_BTNFACE));
				SetBkMode(hdcErrMsg, TRANSPARENT);
				
				return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));  
			}
			else
			{
				return FALSE;
			}
		}
		break;
		
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_BUTTONHIDE:  //show or hide the listbox control
				{
					WCHAR szBtnText[32] = {0};
					GetDlgItemText(hDlg, IDC_BUTTONHIDE, szBtnText, 32);
					
					if (wcscmp(szBtnText, L"Show details") == 0)   //show detail
					{
						SetDlgItemText(hDlg, IDC_BUTTONHIDE, L"Hide details");  // change the text of the button
						
						HWND hList = GetDlgItem(hDlg, IDC_LISTMSG);
						ShowWindow(hList, SW_SHOW);  //hide the listbox
						
						//change the size of the dialog
						RECT rcList;
						GetWindowRect(hList, &rcList);
						
						RECT rcDlg;
						GetWindowRect(hDlg, &rcDlg);
						
						SetWindowPos(hDlg, NULL, 0, 0, rcDlg.right-rcDlg.left, (rcDlg.bottom-rcDlg.top)+(rcList.bottom-rcList.top), 
								SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
					
					}
					else   //hide details
					{
						SetDlgItemText(hDlg, IDC_BUTTONHIDE, L"Show details");  // change the text of the button
						
						HWND hList = GetDlgItem(hDlg, IDC_LISTMSG);
						ShowWindow(hList, SW_HIDE);  //hide the listbox
						
						//change the size of the dialog
						RECT rcList;
						GetWindowRect(hList, &rcList);
						
						RECT rcDlg;
						GetWindowRect(hDlg, &rcDlg);
						
						SetWindowPos(hDlg, NULL, 0, 0, rcDlg.right-rcDlg.left, (rcDlg.bottom-rcDlg.top)-(rcList.bottom-rcList.top), 
								SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
					
					}
					
				}					
				break;

				case IDCANCEL:
				{
					g_bUserCanceled = TRUE;		  
					ShowWindow(hDlg, SW_HIDE);
					DestroyWindow(hDlg);
				}
				break;
			}
		}
		break;		
		default:
			return FALSE; 
		
	}
	return TRUE;
}