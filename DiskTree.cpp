// DiskTree.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "DiskTree.h"


// CDiskTree

IMPLEMENT_DYNCREATE(CDiskTree, CTreeView)

CDiskTree::CDiskTree()
{
	EnableAutomation();
}

CDiskTree::~CDiskTree()
{
}

void CDiskTree::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CTreeView::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CDiskTree, CTreeView)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDiskTree, CTreeView)
END_DISPATCH_MAP()

// Note: we add support for IID_IDiskTree to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {32DD7847-9A24-4F19-B160-94B688B4BD75}
static const IID IID_IDiskTree =
{ 0x32DD7847, 0x9A24, 0x4F19, { 0xB1, 0x60, 0x94, 0xB6, 0x88, 0xB4, 0xBD, 0x75 } };

BEGIN_INTERFACE_MAP(CDiskTree, CTreeView)
	INTERFACE_PART(CDiskTree, IID_IDiskTree, Dispatch)
END_INTERFACE_MAP()


// CDiskTree diagnostics

#ifdef _DEBUG
void CDiskTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void CDiskTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


// CDiskTree message handlers
