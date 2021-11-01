#pragma once


// CDiskTree view

class CDiskTree : public CTreeView
{
	DECLARE_DYNCREATE(CDiskTree)

protected:
	CDiskTree();           // protected constructor used by dynamic creation
	virtual ~CDiskTree();

public:
	virtual void OnFinalRelease();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};


