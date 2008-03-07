#ifndef CELLPROPERTYPAGE_H
#define CELLPROPERTYPAGE_H

#include "stdafx.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/propgrid/manager.h>


class WorldFrame;

class CellPropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(CellPropertyPage)

private:
	WorldFrame*		_worldFrame;

	// properties
	wxPGProperty*	_presetProp;
	wxPGProperty*	_typeProp;
	wxPGProperty*	_seedProp;
	wxPGProperty*	_segmentSizeProp;
	wxPGProperty*	_segmentDevianceProp;
	wxPGProperty*	_degreeProp;
	wxPGProperty*	_degreeDevianceProp;
	wxPGProperty*	_aspectProp;
	wxPGProperty*	_snapSizeProp;
	wxPGProperty*	_snapDevianceProp;
	wxPGProperty*	_buildingHeightProp;
	wxPGProperty*	_buildingDevianceProp;
	wxPGProperty*	_roadWidthProp;
	wxPGProperty*	_roadLimitProp;
	wxPGProperty*	_connectivityProp;
	wxPGProperty*	_footpathWidthProp;
	wxPGProperty*	_footpathHeightProp;
	wxPGProperty*	_lotWidthProp;
	wxPGProperty*	_lotDepthProp;
	wxPGProperty*	_lotDevianceProp;
	wxPGProperty*	_debugProp;
	wxPGProperty*	_mcbDebugProp;

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

	virtual void OnPropertyGridChange(wxPropertyGridEvent& event);

};


//class PresetProp : public wxEditEnumProperty 
//{
//
//};


#endif
