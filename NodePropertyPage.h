#ifndef NODEPROPERTYPAGE_H
#define NODEPROPERTYPAGE_H

#include "stdafx.h"

class WorldFrame;

class NodePropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(NodePropertyPage)

private:
	wxPGProperty* labelProp;
	wxPGProperty* xProp;
	wxPGProperty* yProp;
	wxPGProperty* zProp;
	WorldFrame* mWorldFrame;

protected:
	DECLARE_EVENT_TABLE()

public:
	NodePropertyPage(WorldFrame* wf);
	void Init();
	void update();
	void OnPropertyGridChange(wxPropertyGridEvent& event);
};

#endif

