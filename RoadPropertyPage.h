#ifndef ROADPROPERTYPAGE_H
#define ROADPROPERTYPAGE_H

#include "stdafx.h"

class WorldFrame;

class RoadPropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(RoadPropertyPage)

private:
	wxPGProperty* plotAlgorProp;
	wxPGProperty* sampleSizeProp;
	wxPGProperty* roadWidthProp;
	wxPGProperty* sampleDevianceProp;
	wxPGProperty* samplesProp;
	wxPGProperty* plotDebugProp;
	wxPGProperty* segmentDrawSizeProp;
	WorldFrame* mWorldFrame;


protected:
	DECLARE_EVENT_TABLE()

public:
	RoadPropertyPage(WorldFrame* wf) : wxPropertyGridPage() 
	{
		setWorldFrame(wf);
	}

	virtual void Init();

	void update();

	void setWorldFrame(WorldFrame* wf);

	virtual void OnPropertyGridChange(wxPropertyGridEvent& event);

};

#endif
