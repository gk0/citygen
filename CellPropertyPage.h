#ifndef CELLPROPERTYPAGE_H
#define CELLPROPERTYPAGE_H

#include "stdafx.h"

class WorldCanvas;

class CellPropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(CellPropertyPage)

private:
	wxPGProperty* seedProp;
	wxPGProperty* segmentSizeProp;
	wxPGProperty* segmentDevianceProp;
	wxPGProperty* degreeProp;
	wxPGProperty* degreeDevianceProp;
	wxPGProperty* snapSizeProp;
	wxPGProperty* snapSizeDevianceProp;
	WorldCanvas* mWorldCanvas;


protected:
	DECLARE_EVENT_TABLE()

public:
	virtual void Init();

	void updateData(const int& seed, const float& segSz, const float& segDev, const int& degree,
				const float& degreeDev, const float& snapSz, const float& snapDev);
	void setCanvas(WorldCanvas* c);

	virtual void OnPropertyGridChange( wxPropertyGridEvent& event );

};

#endif
