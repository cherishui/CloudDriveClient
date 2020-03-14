#include <shlobj.h>
#include "DataObject.h"
#include "DownloadFile.h"

CDataObject::CDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count)
{
    m_lRefCount=1;
    m_nNumFormats = count;

    m_pFormatEtc = new FORMATETC[count];
    m_pStgMedium = new STGMEDIUM[count];

    for (int i=0; i<count; i++)
    {
	m_pFormatEtc[i] = fmtetc[i];
	m_pStgMedium[i] = stgmed[i];
    }
}

CDataObject::~CDataObject(void)
{
    if (m_pFormatEtc) delete [] m_pFormatEtc;
    if (m_pStgMedium) delete [] m_pStgMedium;
}


//IUnknown
ULONG __stdcall CDataObject::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CDataObject::Release()
{
    LONG count =  InterlockedDecrement(&m_lRefCount);

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

HRESULT __stdcall CDataObject::QueryInterface(const IID &iid, void **ppvObject)
{
    if(iid == IID_IDataObject || iid == IID_IUnknown)
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



/*********************************************
//IDataObject::GetData
Called by a data consumer to obtain data fron a source data object.
The GetData method renders the data described in the specified FORMATETC stucture and transfers it through the specified STGMEDIUM structure.
这个功能现在有问题,暂时不实现
*********************************************/
HRESULT __stdcall CDataObject::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
    int idx = -1;

    for (int i=0; i< m_nNumFormats; i++)
    {
	if ( (pFormatEtc->tymed & m_pFormatEtc[i].tymed) 
	    && (pFormatEtc->cfFormat == m_pFormatEtc[i].cfFormat)
	    && (pFormatEtc->dwAspect == m_pFormatEtc[i].dwAspect) )
	{
	   idx =i;
	}
    }
    
    if (idx ==-1)
	return DV_E_FORMATETC;

	
	if (pFormatEtc->cfFormat == RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR))
	{
		DWORD len = GlobalSize(m_pStgMedium[idx].hGlobal);
		
		PVOID  source = GlobalLock(m_pStgMedium[idx].hGlobal);

		//create a fixed global block,
		PVOID dest = GlobalAlloc(GMEM_FIXED, len);
		memcpy(dest, source, len);
		GlobalUnlock(m_pStgMedium[idx].hGlobal);
		
		
		FILEGROUPDESCRIPTOR*  pGroupDescriptor = (FILEGROUPDESCRIPTOR*)dest;
		
		int nCount = pGroupDescriptor->cItems;
		
		FILEDESCRIPTOR*  pFileDescriptorArray = (FILEDESCRIPTOR*) ( (LPBYTE)pGroupDescriptor +sizeof (UINT) ); //因为FILEGROUPDESCRIPTOR的第一个元素是UINT cItem,所以跳过			

		for (int i=0; i<nCount; i++)
		{
			if (pFileDescriptorArray[i].dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				MessageBox(NULL, L"create folder.....", NULL, MB_OK);
			}
			else
			{
				MessageBox(NULL, L"uploading file.....", NULL, MB_OK);
			}
		}
		
		
		//HGLOBAL hGlobal 
		
	}


	    
	    
	    
	    pMedium->hGlobal = NULL;
	
	//break;
 //   default:
	//return DV_E_FORMATETC;
 //   }
    return S_OK;
}

HRESULT __stdcall CDataObject::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
    //GetDataHere is only requiree for IStream and IStoreage mediums,
    // it is an error to call GetDatahere for things like hGLOBAL and other clipboard formats
    return DATA_E_FORMATETC;   //Invalid value for pFormatetc
}

/*
QueryGetData(): Determins whether the data object is capable of rendering the data described in the FORMATETC structure.
objects attempting a paste or drop operation call call this method before calling IDataObject::GetData() to get an indication of whether the operation may be successful
*/
HRESULT __stdcall CDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
    	for(int i = 0; i < m_nNumFormats; i++)
	{
		if((pFormatEtc->tymed    &  m_pFormatEtc[i].tymed)   &&
			pFormatEtc->cfFormat == m_pFormatEtc[i].cfFormat && 
			pFormatEtc->dwAspect == m_pFormatEtc[i].dwAspect)
		{
			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}



HRESULT __stdcall CDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
    //apparently we have to set this field to NULL, even though we don't do anything else
    pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}

/*SetData: called by an object conatining a data source to transfer data to the object that implements this method*/
HRESULT __stdcall CDataObject::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
    if (dwDirection == DATADIR_GET)  //GetData()用的   ：enumerate the formats that can be passed in to a call to GetData();
    {
	//SHCreateStdEnumFmtEtc: Create an IEnumFORMATETC interface given an array of FORMATETC structs
	return SHCreateStdEnumFmtEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
    }
    else   //the direction specified is not support for drag anddrop
    {
	return E_NOTIMPL;   
    }
}


HRESULT __stdcall CDataObject::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}


HRESULT __stdcall CDataObject::DUnadvise(DWORD dwConnetion)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall CDataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;

}

