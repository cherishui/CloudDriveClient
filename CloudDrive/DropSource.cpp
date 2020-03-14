#include "DropSource.h"

CDropSource::CDropSource(void)
{
       m_lRefCount =1;
}

CDropSource::~CDropSource(void)
{
}



ULONG __stdcall CDropSource::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CDropSource::Release()
{
    LONG count = InterlockedDecrement(&m_lRefCount);
    if (count ==0)
    {
	delete this;
	return 0;
    }
    else
    {
	return count;
    }
}

HRESULT __stdcall CDropSource::QueryInterface(const IID &iid, void **ppvObject)
{
    if (iid == IID_IDropSource || iid == IID_IUnknown)
    {
	AddRef();
	*ppvObject = this;
	return S_OK;
    }
    else
    {
	*ppvObject =0;
	return E_NOINTERFACE;
    }
}


HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed)
	return DRAGDROP_S_CANCEL;		//CANCEL THE DROP , the drag operaiton should be canceled.

    if((grfKeyState & MK_LBUTTON) == 0)
	return DRAGDROP_S_DROP;	    //the drop opeation should occur completin the drag opeation. ,  when the left mouse button is released.

    return S_OK;   //the drag operation should continue.

}


HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;  //USE the default mouse cursor images
}