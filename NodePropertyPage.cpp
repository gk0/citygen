#include "stdafx.h"
#include "NodePropertyPage.h"
#include "WorldFrame.h"
#include "WorldNode.h"

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
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if((eventProp == _xProp) || (eventProp == _yProp) || (eventProp == _zProp)
		|| (eventProp == _labelProp))
	{
		WorldNode* wn = _worldFrame->getSelected();
		if(wn)
		{
			//wn->setLabel(GetPropertyValueAsString(labelProp));
			wn->setPosition2D(GetPropertyValueAsDouble(_xProp), GetPropertyValueAsDouble(_zProp));
			std::string s(_C(GetPropertyValueAsString(_labelProp)));
			wn->setLabel(s);
			update();
			_worldFrame->update();
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}

void NodePropertyPage::Init()
{
	Append(wxPropertyCategory(wxT("Main")));

	//Add some properties just to test this out
	_labelProp = Append(wxStringProperty(wxT("Label"),wxT("Name"),wxT("Node x")));

	// Add a bool property
	Append(wxBoolProperty(wxT("Selected"), wxPG_LABEL, true));

	// Colour property with arbitrary colour.
	Append(wxColourProperty(wxT("Node Colour"),
                             wxPG_LABEL,
                             wxColour(200,0,0)));

	Append(wxPropertyCategory(wxT("Position")));

	// Add float property (value type is actually double)
	_xProp = Append(wxFloatProperty(wxT("x"), wxPG_LABEL, 0.0));
	_yProp = Append(wxFloatProperty(wxT("y"), wxPG_LABEL, 0.0));
	_zProp = Append(wxFloatProperty(wxT("z"), wxPG_LABEL, 0.0));

	Append(wxPropertyCategory(wxT("Extra")));
}

void NodePropertyPage::update()
{
	//get data from worldframe
	Ogre::Vector3 nodePos;
	Ogre::String label;
	WorldNode* wn = _worldFrame->getSelected();
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
