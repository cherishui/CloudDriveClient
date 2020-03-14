#include <windows.h>
#include "CloudDrive.h"
#include "ThreadParam.h"

ThreadParam::ThreadParam(LPCWSTR lpszNodeId, LPCWSTR lpszName, HWND hMsg)
{
	ZeroMemory(m_szNodeId, sizeof(m_szNodeId));
	ZeroMemory(m_szName, sizeof(m_szName));
	m_hMsg = NULL;
	
	if (lpszNodeId != NULL)
	{
		wcscpy_s(m_szNodeId, LEN_NODEID, lpszNodeId);
	}
	
	if (lpszName != NULL)
	{
		wcscpy_s(m_szName, LEN_NAME, lpszName);
	}
	
	m_hMsg = hMsg;
}


ThreadParamSave::ThreadParamSave(LONG lCount, LPWSTR szNodeIds, LPCWSTR lpszDestPath, HWND hMsg)
{
	m_lCount  = lCount;
	
	size_t  nSize = wcslen(szNodeIds)+1;
	
	m_szNodeIds = new WCHAR[nSize]();
	wcscpy_s(m_szNodeIds, nSize, szNodeIds);
	
	ZeroMemory(m_szDestPath, sizeof(m_szDestPath));
	
	if (lpszDestPath != NULL)
	{
		wcscpy_s(m_szDestPath, MAX_PATH, lpszDestPath);
	}
	
	m_hMsg = hMsg;
}

ThreadParamSave::~ThreadParamSave()
{
	delete [] m_szNodeIds;
	m_szNodeIds = NULL;
}


ThreadParamUpload::ThreadParamUpload(LPWSTR lpszFileNames, WORD nFileOffset, HWND hMsg)
{
	m_szFileNames = lpszFileNames;
	m_nOffset = nFileOffset;
	m_hMsg = hMsg;
}

ThreadParamUpload::~ThreadParamUpload()
{
	if (m_szFileNames)
		delete [] m_szFileNames;   
}

ThreadParamCalculate::ThreadParamCalculate(HDROP hDrop, HWND hMsg)
{
	m_hDrop = hDrop;
	m_hMsg = hMsg;
}


ThreadParamDrop::ThreadParamDrop(LPWSTR lpszCurrentFolderId, IDataObject *pDataObject, HWND hMsg)
{
	ZeroMemory(m_szFolderId, sizeof(m_szFolderId));

	if (lpszCurrentFolderId != NULL)
	{
		wcscpy_s(m_szFolderId, LEN_NODEID, lpszCurrentFolderId);
	}
	
	m_hMsg= hMsg;
	
	CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pDataObject, &m_pStream);
}


ThreadParamLogin::ThreadParamLogin(LPCWSTR lpszUserEmail, LPCWSTR lpszPassword, HWND hLogin)
{
	ZeroMemory(m_szUserEmail, sizeof(m_szUserEmail));
	ZeroMemory(m_szPassword, sizeof(m_szPassword));
	m_hMsg = NULL;
	
	if (lpszUserEmail != NULL)
	{
		wcscpy_s(m_szUserEmail, LEN_USEREMAIL, lpszUserEmail);
	}
	
	if (lpszPassword != NULL)
	{
		wcscpy_s(m_szPassword, LEN_USERPASSWORD, lpszPassword);
	}
	
	m_hMsg = hLogin;
}

ThreadParamDelete::ThreadParamDelete(LPWSTR szNodeIds, HWND hMsg)
{
	size_t  nSize = wcslen(szNodeIds)+1;
	m_szNodeIds = new WCHAR[nSize]();
	wcscpy_s(m_szNodeIds, nSize, szNodeIds);
	
	
	m_hMsg = hMsg;
}

ThreadParamDelete::~ThreadParamDelete()
{
	delete [] m_szNodeIds;
	m_szNodeIds = NULL;
}