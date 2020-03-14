#include "Droptarget.h"
#include "resource.h"
#include "DragAndDrop.h"

CDropTarget::CDropTarget(HWND hwnd)//, LPWSTR pCurrentFolderId )
{
	m_lRefCount  = 1;
	m_hWnd       = hwnd;
	m_fAllowDrop = false;
}

CDropTarget::~CDropTarget()
{
}

//IUnknown 
HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}	

ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//position the listview controls' caret under the mouse
void PositionCursor(HWND hListView, POINTL pt)
{
	DWORD curpos; 
		
	// get the character position of mouse
	ScreenToClient(hListView, (POINT *)&pt);
	curpos = SendMessage(hListView, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

	// set cursor position
	SendMessage(hListView, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}



//IDropTarget
HRESULT __stdcall CDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{

	FORMATETC fmtetc = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	HRESULT hr = pDataObject->QueryGetData(&fmtetc);

	// does the dataobject contain data we want?
	m_fAllowDrop = (hr== S_OK);

	if(m_fAllowDrop)
	{
		*pdwEffect = DROPEFFECT_COPY;
		SetFocus(m_hWnd);
		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;  //Drop target cannot accept the data.
	}

	return S_OK;
}


HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop)
	{
		*pdwEffect =  DROPEFFECT_COPY;
		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragLeave
//
HRESULT __stdcall CDropTarget::DragLeave(void)
{
	return S_OK;
}

//
//	IDropTarget::Drop
//
//
HRESULT __stdcall CDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	PositionCursor(m_hWnd, pt);

	if(m_fAllowDrop)
	{
		m_pDataObject = pDataObject;
		DropData();

		*pdwEffect = DROPEFFECT_COPY;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

void  CDropTarget::DropData()
{
	PasteOrDropFiles(m_pDataObject);
}
