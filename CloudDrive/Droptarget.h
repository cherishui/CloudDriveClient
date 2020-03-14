#pragma once
#include "oleidl.h"

/******************************************
Implement the IDropTarget interface if youare developing an app that can act as a target for a drag and drop operation

you do not call the methods of IDropTarget directly, The DoDragDrop funciton calls the IDropTarget methods during the drag and drop operation. 
实际上是个循环检测的程序, 随时检测鼠标键盘的状态,并调用对应的方法
**************************************/
class CDropTarget :   public IDropTarget
{
public:
	// IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef (void);
	ULONG	__stdcall Release (void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragLeave (void);
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

	// Constructor/destructor
	CDropTarget(HWND hwnd);
	~CDropTarget();

private:
	LONG	m_lRefCount;
	HWND	m_hWnd;
	bool		m_fAllowDrop;
	IDataObject *m_pDataObject;

	void DropData();
};
