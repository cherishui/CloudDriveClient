#pragma once
#include "oleidl.h"

class CDropSource :    public IDropSource
{
public:
    //IUnknown members
    HRESULT  __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);

    //IDropSource Members
    HRESULT __stdcall QueryContinueDrag (BOOL fEscapePressed, DWORD grfKeyState);
    HRESULT __stdcall GiveFeedback(DWORD dwEffect);

    //constructor/destructor
    CDropSource(void);
    ~CDropSource(void);

private:
    LONG m_lRefCount;

};
