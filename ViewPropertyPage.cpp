#include "stdafx.h"
#include "ViewPropertyPage.h"
#include "WorldFrame.h"

ViewPropertyPage::ViewPropertyPage(WorldFrame* wf) : wxPropertyGridPage() 
{
	mWorldFrame = wf;
}

void ViewPropertyPage::update()
{
	Ogre::Camera* cam = mWorldFrame->getCamera();
	if(cam)
	{
		Ogre::Vector3 camPos = cam->getPosition();
		SetPropertyValue(xProp, camPos.x);
		SetPropertyValue(yProp, camPos.y);
		SetPropertyValue(zProp, camPos.z);

		Ogre::Vector3 camDir = cam->getDirection();
		SetPropertyValue(xDirProp, camDir.x);
		SetPropertyValue(yDirProp, camDir.y);
		SetPropertyValue(zDirProp, camDir.z);

		RefreshProperty(xProp);
		RefreshProperty(yProp);
		RefreshProperty(zProp);

		RefreshProperty(xDirProp);
		RefreshProperty(yDirProp);
		RefreshProperty(zDirProp);
	}
}

void ViewPropertyPage::Init()
{
	Append( wxPropertyCategory(wxT("Position")) );

	// Add float property (value type is actually double)
	xProp = Append( wxFloatProperty(wxT("x"), wxPG_LABEL, 0.0) );
	yProp = Append( wxFloatProperty(wxT("y"), wxPG_LABEL, 0.0) );
	zProp = Append( wxFloatProperty(wxT("z"), wxPG_LABEL, 0.0) );

	Append( wxPropertyCategory(wxT("Direction")) );

	// Add float property (value type is actually double)
	xDirProp = Append( wxFloatProperty(wxT("x_dir"), wxPG_LABEL, 0.0) );
	yDirProp = Append( wxFloatProperty(wxT("y_dir"), wxPG_LABEL, 0.0) );
	zDirProp = Append( wxFloatProperty(wxT("z_dir"), wxPG_LABEL, 0.0) );

	// Another way  
	Append( wxPropertyCategory(wxT("Advanced")) );

	// Cursor property
	Append( wxCursorProperty (wxT("My Cursor"),
								  wxPG_LABEL,
								  wxCURSOR_ARROW));
}

