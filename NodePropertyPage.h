#ifndef NODEPROPERTYPAGE_H
#define NODEPROPERTYPAGE_H

#include "stdafx.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/propgrid/manager.h>

class WorldFrame;

class NodePropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(NodePropertyPage)

private:
	WorldFrame*		_worldFrame;

	// properties
	wxPGProperty*	_labelProp;
	wxPGProperty*	_xProp;
	wxPGProperty*	_yProp;
	wxPGProperty*	_zProp;

protected:
	DECLARE_EVENT_TABLE()

public:
	NodePropertyPage(WorldFrame* wf);
	void Init();
	void update();
	void OnPropertyGridChange(wxPropertyGridEvent& event);
};

#endif

