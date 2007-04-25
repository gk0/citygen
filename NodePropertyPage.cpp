#include "stdafx.h"
#include "NodePropertyPage.h"
#include "WorldFrame.h"

// Required for WX
IMPLEMENT_CLASS(NodePropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(NodePropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, NodePropertyPage::OnPropertyGridChange )
END_EVENT_TABLE()

//BEGIN_EVENT_TABLE(wxMyPropertyGridPage, wxPropertyGridPage)
//    EVT_PG_SELECTED( wxID_ANY, wxMyPropertyGridPage::OnPropertySelect )
//    EVT_PG_CHANGED( wxID_ANY, wxMyPropertyGridPage::OnPropertyChange )
//    EVT_PG_PAGE_CHANGED( wxID_ANY, wxMyPropertyGridPage::OnPageChange )
//END_EVENT_TABLE()


void NodePropertyPage::OnPropertyGridChange( wxPropertyGridEvent& event )
{
    // Get name of changed property
    const wxString& name = event.GetPropertyName();
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if( (eventProp == xProp) || (eventProp == yProp) || (eventProp == zProp)
		|| (eventProp == labelProp) )
	{
		if(mWorldFrame)
		{
//			mWorldCanvas->setNodeProperties(GetPropertyValueAsString(labelProp), GetPropertyValueAsDouble(xProp),
//					GetPropertyValueAsDouble(yProp), GetPropertyValueAsDouble(zProp));
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}

void NodePropertyPage::Init()
{
	Append( wxPropertyCategory(wxT("Main")) );

	//Add some properties just to test this out
	labelProp = Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Node x")) );

	// Add a bool property
	Append( wxBoolProperty(wxT("Selected"), wxPG_LABEL, true) );

	// Colour property with arbitrary colour.
	Append( wxColourProperty(wxT("Node Colour"),
                             wxPG_LABEL,
                             wxColour(200,0,0) ) );

	Append( wxPropertyCategory(wxT("Position")) );

	// Add float property (value type is actually double)
	xProp = Append( wxFloatProperty(wxT("x"), wxPG_LABEL, 0.0) );
	yProp = Append( wxFloatProperty(wxT("y"), wxPG_LABEL, 0.0) );
	zProp = Append( wxFloatProperty(wxT("z"), wxPG_LABEL, 0.0) );

	Append( wxPropertyCategory(wxT("Extra")) );
}

void NodePropertyPage::update()
{
	//get data from worldframe
	float x,y,z;

//	SetPropertyValue(labelProp, l);
	SetPropertyValue(xProp, x);
	SetPropertyValue(yProp, y);
	SetPropertyValue(zProp, z);

	RefreshProperty(labelProp);
	RefreshProperty(xProp);
	RefreshProperty(yProp);
	RefreshProperty(zProp);
}

void NodePropertyPage::setWorldFrame(WorldFrame* wf)
{
	mWorldFrame = wf;
}

