#ifndef VIEWPROPERTYPAGE_H
#define VIEWPROPERTYPAGE_H

#include "stdafx.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/propgrid/manager.h>

class WorldFrame;

class ViewPropertyPage : public wxPropertyGridPage
{
private:
	WorldFrame*		_worldFrame;

	// properties
	wxPGProperty*	_xProp;
	wxPGProperty*	_yProp;
	wxPGProperty*	_zProp;
	wxPGProperty*	_xDirProp;
	wxPGProperty*	_yDirProp;
	wxPGProperty*	_zDirProp;

public:
	ViewPropertyPage(WorldFrame* wf);
	void update();
	void Init();
};

#endif
