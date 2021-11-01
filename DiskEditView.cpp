// DiskEditView.cpp : implementation file
//

#include "stdafx.h"
#include "DiskData.h"
#include "DiskEditView.h"


// CDiskEditView

IMPLEMENT_DYNCREATE(CDiskEditView, CRichEditView)

CDiskEditView::CDiskEditView()
{
}

CDiskEditView::~CDiskEditView()
{
}

BEGIN_MESSAGE_MAP(CDiskEditView, CRichEditView)
END_MESSAGE_MAP()


// CDiskEditView diagnostics

#ifdef _DEBUG
void CDiskEditView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CDiskEditView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}
#endif //_DEBUG


// CDiskEditView message handlers
