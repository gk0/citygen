#include "stdafx.h"
#include "ViewPropertyPage.h"
#include "WorldFrame.h"

ViewPropertyPage::ViewPropertyPage(WorldFrame* wf) : wxPropertyGridPage() 
{
	_worldFrame = wf;
}

void ViewPropertyPage::update()
{
	Ogre::Camera* cam = _worldFrame->getCamera();
	if(cam)
	{
		Ogre::Vector3 camPos = cam->getPosition();
		SetPropertyValue(_xProp, camPos.x);
		SetPropertyValue(_yProp, camPos.y);
		SetPropertyValue(_zProp, camPos.z);

		Ogre::Vector3 camDir = cam->getDirection();
		SetPropertyValue(_xDirProp, camDir.x);
		SetPropertyValue(_yDirProp, camDir.y);
		SetPropertyValue(_zDirProp, camDir.z);

		RefreshProperty(_xProp);
		RefreshProperty(_yProp);
		RefreshProperty(_zProp);

		RefreshProperty(_xDirProp);
		RefreshProperty(_yDirProp);
		RefreshProperty(_zDirProp);
	}
}

void ViewPropertyPage::Init()
{
	Append(wxPropertyCategory(wxT("Position")));

	// Add float property (value type is actually double)
	_xProp = Append(wxFloatProperty(wxT("x"), wxPG_LABEL, 0.0));
	_yProp = Append(wxFloatProperty(wxT("y"), wxPG_LABEL, 0.0));
	_zProp = Append(wxFloatProperty(wxT("z"), wxPG_LABEL, 0.0));

	Append(wxPropertyCategory(wxT("Direction")));

	// Add float property (value type is actually double)
	_xDirProp = Append(wxFloatProperty(wxT("x_dir"), wxPG_LABEL, 0.0));
	_yDirProp = Append(wxFloatProperty(wxT("y_dir"), wxPG_LABEL, 0.0));
	_zDirProp = Append(wxFloatProperty(wxT("z_dir"), wxPG_LABEL, 0.0));

	// Another way  
	Append(wxPropertyCategory(wxT("Advanced")));

	// Cursor property
	Append(wxCursorProperty (wxT("My Cursor"),
								  wxPG_LABEL,
								  wxCURSOR_ARROW));
}

