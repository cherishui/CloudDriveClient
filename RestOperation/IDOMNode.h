#pragma once

/*this is a base class for DOMFolderNode and DOMFileNode,
this class is an Interface
*/
class IDOMNode
{

public:
	virtual ~IDOMNode(void)=0;

public:
	virtual	LPCWSTR GetNodeId() =0 ;
	virtual	LPCWSTR GetName() =0;
	virtual  LPCWSTR GetDiscription()=0 ;
	virtual	INT64	GetCreatedDate()=0 ;
	virtual	INT64	GetModifiedDate()=0 ;
	virtual	LPCWSTR	GetOwner()=0 ;
	virtual	LPCWSTR	GetOwnerName()=0;
	virtual	bool		IsDirectory()=0;
	virtual	bool		HasContent()=0;
	virtual	bool		HasLink()=0;
};
