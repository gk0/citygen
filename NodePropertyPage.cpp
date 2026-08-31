#include "stdafx.h"
#include "NodePropertyPage.h"
#include "WorldFrame.h"
#include "WorldNode.h"

#include <wx/toolbar.h>
#include <wx/propgrid/advprops.h>

// Required for WX
IMPLEMENT_CLASS(NodePropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(NodePropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, NodePropertyPage::OnPropertyGridChange)
END_EVENT_TABLE()

//BEGIN_EVENT_TABLE(wxMyPropertyGridPage, wxPropertyGridPage)
//    EVT_PG_SELECTED(wxID_ANY, wxMyPropertyGridPage::OnPropertySelect)
//    EVT_PG_CHANGED(wxID_ANY, wxMyPropertyGridPage::OnPropertyChange)
//    EVT_PG_PAGE_CHANGED(wxID_ANY, wxMyPropertyGridPage::OnPageChange)
//END_EVENT_TABLE()


NodePropertyPage::NodePropertyPage(WorldFrame* wf)
 : wxPropertyGridPage()
{
	_worldFrame = wf;
}


void NodePropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetProperty();

	if((eventProp == _xProp) || (eventProp == _yProp) || (eventProp == _zProp)
		|| (eventProp == _labelProp))
	{
		WorldNode* wn = _worldFrame->getSelectedNode();
		if(wn)
		{
			//wn->setLabel(GetPropertyValueAsString(labelProp));
			wn->setPosition2D(GetPropertyValueAsDouble(_xProp), GetPropertyValueAsDouble(_zProp));
			std::string s(_C(GetPropertyValueAsString(_labelProp)));
			wn->setLabel(s);
			update();
			_worldFrame->Refresh();
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}

void NodePropertyPage::Init()
{
	Append(new wxPropertyCategory(wxT("Main")));

	//Add some properties just to test this out
	_labelProp = Append(new wxStringProperty(wxT("Label"),wxT("Name"),wxT("Node x")));

	// Add a bool property
	Append(new wxBoolProperty(wxT("Selected"), wxPG_LABEL, true));

	// Colour property with arbitrary colour.
	Append(new wxColourProperty(wxT("Node Colour"),
                             wxPG_LABEL,
                             wxColour(200,0,0)));

	Append(new wxPropertyCategory(wxT("Position")));

	// Add float property (value type is actually double)
	_xProp = Append(new wxFloatProperty(wxT("x (m)"), wxPG_LABEL, 0.0));
	_yProp = Append(new wxFloatProperty(wxT("y (m)"), wxPG_LABEL, 0.0));
	_zProp = Append(new wxFloatProperty(wxT("z (m)"), wxPG_LABEL, 0.0));

	Append(new wxPropertyCategory(wxT("Extra")));
}

void NodePropertyPage::update()
{
	//get data from worldframe
	Ogre::Vector3 nodePos(Ogre::Vector3::ZERO);
	Ogre::String label;
	WorldNode* wn = _worldFrame->getSelectedNode();
	if(wn != 0)
	{
		nodePos = wn->getPosition3D();
		label = wn->getLabel();
	}

	SetPropertyValue(_labelProp, wxString(label.c_str(), wxConvUTF8));
	SetPropertyValue(_xProp, nodePos.x);
	SetPropertyValue(_yProp, nodePos.y);
	SetPropertyValue(_zProp, nodePos.z);

	RefreshProperty(_labelProp);
	RefreshProperty(_xProp);
	RefreshProperty(_yProp);
	RefreshProperty(_zProp);
}
