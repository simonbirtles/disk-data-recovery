// CntrItem.h : interface of the CDiskDataCntrItem class
//

#pragma once

class CDiskDataDoc;
class CDiskDataView;

class CDiskDataCntrItem : public COleDocObjectItem
{
	DECLARE_SERIAL(CDiskDataCntrItem)

// Constructors
public:
	CDiskDataCntrItem(CDiskDataDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer

// Attributes
public:
	CDiskDataDoc* GetDocument()
		{ return reinterpret_cast<CDiskDataDoc*>(COleDocObjectItem::GetDocument()); }
	CDiskDataView* GetActiveView()
		{ return reinterpret_cast<CDiskDataView*>(COleDocObjectItem::GetActiveView()); }

	public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	virtual void OnActivate();
	protected:
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);

// Implementation
public:
	~CDiskDataCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
};

