#pragma once


// CDiskEditView view

class CDiskEditView : public CRichEditView
{
	DECLARE_DYNCREATE(CDiskEditView)

protected:
	CDiskEditView();           // protected constructor used by dynamic creation
	virtual ~CDiskEditView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
};


