#include "stdafx.h"
#include "CellPropertyPage.h"
#include "WorldFrame.h"
#include "WorldCell.h"

// Required for WX
IMPLEMENT_CLASS(CellPropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(CellPropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, CellPropertyPage::OnPropertyGridChange)
END_EVENT_TABLE()

void CellPropertyPage::Init()
{
    Append(wxPropertyCategory(wxT("Generation Parameters")));

	seedProp = Append(wxIntProperty(wxT("Seed"), wxPG_LABEL, 0));

	// Add float property (value type is actually double)
    segmentSizeProp = Append(wxFloatProperty(wxT("Segment Size"), wxPG_LABEL, 4.5));
	segmentDevianceProp = Append(wxFloatProperty(wxT("Segment Deviance"), wxPG_LABEL, 0.1));

	// Add int property
    degreeProp = Append(wxIntProperty(wxT("Degree"), wxPG_LABEL, 4));
	degreeDevianceProp = Append(wxFloatProperty(wxT("Degree Deviance"), wxPG_LABEL, 0.1));

	// Add float property (value type is actually double)
    snapSizeProp = Append(wxFloatProperty(wxT("Snap Size"), wxPG_LABEL, 4.5));
	snapDevianceProp = Append(wxFloatProperty(wxT("Snap Size Deviance"), wxPG_LABEL, 0.1));
}


void CellPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
    // Get name of changed property
    const wxString& name = event.GetPropertyName();
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if((eventProp == seedProp) || (eventProp == segmentSizeProp) || (eventProp == degreeProp) || (eventProp == snapSizeProp) 
		|| (eventProp == segmentDevianceProp) || (eventProp == degreeDevianceProp) || (eventProp == snapDevianceProp))
	{
		WorldCell *wc = mWorldFrame->getSelectedCell();
		if(wc)
		{
			GrowthGenParams g;
			g.seed = GetPropertyValueAsLong(seedProp);
			g.segmentSize = GetPropertyValueAsDouble(segmentSizeProp);
			g.segmentDeviance = GetPropertyValueAsDouble(segmentDevianceProp);
			g.degree = GetPropertyValueAsInt(degreeProp);
			g.degreeDeviance = GetPropertyValueAsDouble(degreeDevianceProp);
			g.snapSize = GetPropertyValueAsDouble(snapSizeProp);
			g.snapDeviance = GetPropertyValueAsDouble(snapDevianceProp);
			
			wc->setGrowthGenParams(g);
			mWorldFrame->update();
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}


void CellPropertyPage::update()
{
	GrowthGenParams g;
	WorldCell *wc = mWorldFrame->getSelectedCell();
	if(wc)
	{
		g = wc->getGrowthGenParams();
		/*
		typedef struct {
			int seed;
			Ogre::Real segmentSize;
			Ogre::Real segmentDeviance;
			unsigned int degree;
			Ogre::Real degreeDeviance;
			Ogre::Real snapSize;
			Ogre::Real snapDeviance;
		} GrowthGenParams;
		*/
		SetPropertyValue(seedProp, g.seed);
		SetPropertyValue(segmentSizeProp, g.segmentSize);
		SetPropertyValue(segmentDevianceProp, g.segmentDeviance);
		SetPropertyValue(degreeProp, (int) g.degree);
		SetPropertyValue(degreeDevianceProp, g.degreeDeviance);
		SetPropertyValue(snapSizeProp, g.snapSize);
		SetPropertyValue(snapDevianceProp, g.snapDeviance);
	}
	RefreshProperty(seedProp);
	RefreshProperty(segmentSizeProp);
	RefreshProperty(degreeProp);
	RefreshProperty(snapSizeProp);
	RefreshProperty(segmentDevianceProp);
	RefreshProperty(degreeDevianceProp);
	RefreshProperty(snapDevianceProp);
}

void CellPropertyPage::setWorldFrame(WorldFrame* wf)
{
	mWorldFrame = wf;
}
