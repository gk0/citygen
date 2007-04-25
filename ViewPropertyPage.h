#ifndef VIEWPROPERTYPAGE_H
#define VIEWPROPERTYPAGE_H

#include "stdafx.h"

class WorldFrame;

class ViewPropertyPage : public wxPropertyGridPage
{
private:
	WorldFrame* mWorldFrame;

	wxPGProperty* xProp;
	wxPGProperty* yProp;
	wxPGProperty* zProp;
	wxPGProperty* xDirProp;
	wxPGProperty* yDirProp;
	wxPGProperty* zDirProp;

public:
	ViewPropertyPage(WorldFrame* wf);
	void update();
	void Init();
};

#endif
