#include "stdafx.h"
#include "ViewPropertyPage.h"
#include "WorldFrame.h"

#include <wx/propgrid/advprops.h>

ViewPropertyPage::ViewPropertyPage(WorldFrame* wf) : wxPropertyGridPage()
{
	_worldFrame = wf;
}

void ViewPropertyPage::update()
{
	Ogre::Camera* cam = _worldFrame->getCamera();
	Ogre::SceneNode* camNode = _worldFrame->getCameraNode();
	if(cam && camNode)
	{
		Ogre::Vector3 camNodePos = camNode->getPosition();
		SetPropertyValue(_xProp, camNodePos.x);
		SetPropertyValue(_yProp, camNodePos.y);
		SetPropertyValue(_zProp, camNodePos.z);

		Ogre::Vector3 camDir = cam->getDerivedDirection();
		SetPropertyValue(_xDirProp, camDir.x);
		SetPropertyValue(_yDirProp, camDir.y);
		SetPropertyValue(_zDirProp, camDir.z);

		SetPropertyValue(_zoomProp, cam->getPosition().z);

		RefreshProperty(_xProp);
		RefreshProperty(_yProp);
		RefreshProperty(_zProp);

		RefreshProperty(_xDirProp);
		RefreshProperty(_yDirProp);
		RefreshProperty(_zDirProp);

		RefreshProperty(_zoomProp);
	}
}

void ViewPropertyPage::Init()
{
	wxPGChoices arrPlot;
	arrPlot.Add(wxT("World"), 0);
	arrPlot.Add(wxT("FPS"), 1);
	arrPlot.Add(wxT("3D Author"), 2);

    //presetProp = Append( wxEditEnumProperty(wxT("Load Preset"), wxPG_LABEL, arrPlot) );
	_cameraProp = Append(wxEnumProperty(wxT("Camera Model"), wxPG_LABEL, arrPlot) );

	Append(wxPropertyCategory(wxT("Camera Target")));

	// Add float property (value type is actually double)
	_xProp = Append(wxFloatProperty(wxT("x"), wxPG_LABEL, 0.0));
	_yProp = Append(wxFloatProperty(wxT("y"), wxPG_LABEL, 0.0));
	_zProp = Append(wxFloatProperty(wxT("z"), wxPG_LABEL, 0.0));

	Append(wxPropertyCategory(wxT("Camera Direction")));

	// Add float property (value type is actually double)
	_xDirProp = Append(wxFloatProperty(wxT("x_dir"), wxPG_LABEL, 0.0));
	_yDirProp = Append(wxFloatProperty(wxT("y_dir"), wxPG_LABEL, 0.0));
	_zDirProp = Append(wxFloatProperty(wxT("z_dir"), wxPG_LABEL, 0.0));

	Append(wxPropertyCategory(wxT("Zoom Distance")));
	_zoomProp = Append(wxFloatProperty(wxT("zoom"), wxPG_LABEL, 0.0));

	// Another way
	Append(wxPropertyCategory(wxT("Advanced")));

	// Cursor property
	Append(wxCursorProperty (wxT("My Cursor"),
								  wxPG_LABEL,
								  wxCURSOR_ARROW));
}

