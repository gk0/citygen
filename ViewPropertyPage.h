#ifndef VIEWPROPERTYPAGE_H
#define VIEWPROPERTYPAGE_H

#include "stdafx.h"

class WorldFrame;

class ViewPropertyPage : public wxPropertyGridPage
{
private:
	WorldFrame* mWorldFrame;

public:
	ViewPropertyPage(WorldFrame* wf);
	void update();
	void Init();
};

#endif
