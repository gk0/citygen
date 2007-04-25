#include "stdafx.h"
#include "RoadPropertyPage.h"
#include "WorldFrame.h"
#include "WorldRoad.h"

// Required for WX
IMPLEMENT_CLASS(RoadPropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(RoadPropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, RoadPropertyPage::OnPropertyGridChange )
END_EVENT_TABLE()

void RoadPropertyPage::Init()
{
    Append( wxPropertyCategory(wxT("Adaptive Road Parameters")) );

	// Add float property (value type is actually double)
	segmentSizeProp = Append( wxFloatProperty(wxT("Segment Size"), wxPG_LABEL, 0) );
    roadWidthProp = Append( wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0) );
	roadDevianceProp = Append( wxFloatProperty(wxT("Road Deviance Angle"), wxPG_LABEL,0) );
}


void RoadPropertyPage::OnPropertyGridChange( wxPropertyGridEvent& event )
{
    // Get name of changed property
    const wxString& name = event.GetPropertyName();
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if( (eventProp == segmentSizeProp) || (eventProp == roadWidthProp) || (eventProp == roadDevianceProp))
	{
		if(mWorldFrame)
		{
			//mWorldCanvas->setRoadProperties(GetPropertyValueAsInt(seedProp),
			//	GetPropertyValueAsDouble(segmentSizeProp), GetPropertyValueAsDouble(segmentDevianceProp),
			//	GetPropertyValueAsInt(degreeProp), GetPropertyValueAsDouble(degreeDevianceProp),
			//		GetPropertyValueAsDouble(snapSizeProp), GetPropertyValueAsDouble(snapSizeDevianceProp));
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}


void RoadPropertyPage::update()
{
	RoadGenParams rp;
	WorldRoad *wr = mWorldFrame->getSelectedRoad();
	if(wr)
	{
		rp = wr->getGenParams();
		/*
		typedef struct {
			Ogre::Real segmentSize;
			Ogre::Real segmentDeviance;
			Ogre::Real roadWidth;
		} RoadGenParams;
		*/
		
		SetPropertyValue(segmentSizeProp, rp.segmentSize);
		SetPropertyValue(roadDevianceProp, rp.segmentDeviance);
		SetPropertyValue(roadWidthProp, rp.roadWidth);
	}

	RefreshProperty(segmentSizeProp);
	RefreshProperty(roadDevianceProp);
	RefreshProperty(roadWidthProp);
}

void RoadPropertyPage::setWorldFrame(WorldFrame* wf)
{
	mWorldFrame = wf;
}
