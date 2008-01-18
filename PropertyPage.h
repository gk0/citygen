#ifndef PROPERTYPAGE_H
#define PROPERTYPAGE_H

#include "stdafx.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/propgrid/manager.h>

class Property;
class WorldFrame;

class PropertyPage : public wxPropertyGridPage
{
//	DECLARE_CLASS(PropertyPage)

protected:
	WorldFrame*		_worldFrame;

public:
	PropertyPage(WorldFrame* wf) : wxPropertyGridPage() 
	{
		setWorldFrame(wf);
	}

	virtual void Init() = 0;

	void update();

	void setWorldFrame(WorldFrame* wf)
	{
		_worldFrame = wf;
	}

	void addProperty(Property* p);
	void setPropertyValue(Property* p);

};


#endif
