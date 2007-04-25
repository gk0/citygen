#ifndef CELLPROPERTYPAGE_H
#define CELLPROPERTYPAGE_H

#include "stdafx.h"

class WorldFrame;

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
	WorldFrame* mWorldFrame;


protected:
	DECLARE_EVENT_TABLE()

public:
	CellPropertyPage(WorldFrame* wf) : wxPropertyGridPage() 
	{
		setWorldFrame(wf);
	}

	virtual void Init();

	void update();

	void setWorldFrame(WorldFrame* wf);

	virtual void OnPropertyGridChange( wxPropertyGridEvent& event );

};

#endif
