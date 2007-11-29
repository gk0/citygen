#ifndef ROADPROPERTYPAGE_H
#define ROADPROPERTYPAGE_H

#include "stdafx.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/propgrid/manager.h>

class WorldFrame;

class RoadPropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(RoadPropertyPage)

private:
	WorldFrame*		_worldFrame;

	// properties
	wxPGProperty*	_plotAlgorProp;
	wxPGProperty*	_sampleSizeProp;
	wxPGProperty*	_roadWidthProp;
	wxPGProperty*	_sampleDevianceProp;
	wxPGProperty*	_samplesProp;
	wxPGProperty*	_plotDebugProp;
	wxPGProperty*	_segmentDrawSizeProp;

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
