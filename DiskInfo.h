#pragma once



// CDiskInfo command target

class CDiskInfo : public CObject
{
public:
	CDiskInfo();
	virtual ~CDiskInfo();

private:
	void GetPhyInfo();
};


