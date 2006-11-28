#include "stdafx.h"
#include "CellPropertyPage.h"
#include "WorldCanvas.h"

// Required for WX
IMPLEMENT_CLASS(CellPropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(CellPropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, CellPropertyPage::OnPropertyGridChange )
END_EVENT_TABLE()

void CellPropertyPage::Init()
{
    Append( wxPropertyCategory(wxT("Generation Parameters")) );

	seedProp = Append( wxIntProperty(wxT("Seed"), wxPG_LABEL, 0) );

	// Add float property (value type is actually double)
    segmentSizeProp = Append( wxFloatProperty(wxT("Segment Size"), wxPG_LABEL, 4.5) );
	segmentDevianceProp = Append( wxFloatProperty(wxT("Segment Deviance"), wxPG_LABEL, 0.1) );

	// Add int property
    degreeProp = Append( wxIntProperty(wxT("Degree"), wxPG_LABEL, 4) );
	degreeDevianceProp = Append( wxFloatProperty(wxT("Degree Deviance"), wxPG_LABEL, 0.1) );

	// Add float property (value type is actually double)
    snapSizeProp = Append( wxFloatProperty(wxT("Snap Size"), wxPG_LABEL, 4.5) );
	snapSizeDevianceProp = Append( wxFloatProperty(wxT("Snap Size Deviance"), wxPG_LABEL, 0.1) );
}


void CellPropertyPage::OnPropertyGridChange( wxPropertyGridEvent& event )
{
    // Get name of changed property
    const wxString& name = event.GetPropertyName();
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if( (eventProp == seedProp) || (eventProp == segmentSizeProp) || (eventProp == degreeProp) || (eventProp == snapSizeProp) 
		|| (eventProp == segmentDevianceProp) || (eventProp == degreeDevianceProp) || (eventProp == snapSizeDevianceProp))
	{
		if(mWorldCanvas)
		{
			mWorldCanvas->setCellProperties(GetPropertyValueAsInt(seedProp),
				GetPropertyValueAsDouble(segmentSizeProp), GetPropertyValueAsDouble(segmentDevianceProp),
				GetPropertyValueAsInt(degreeProp), GetPropertyValueAsDouble(degreeDevianceProp),
					GetPropertyValueAsDouble(snapSizeProp), GetPropertyValueAsDouble(snapSizeDevianceProp));
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}


void CellPropertyPage::updateData(const int& seed, const float& segSz, const float& segDev, const int& degree,
								const float& degreeDev, const float& snapSz, const float& snapDev)
{
	SetPropertyValue(seedProp, seed);
	SetPropertyValue(segmentSizeProp, segSz);
	SetPropertyValue(segmentDevianceProp, segDev);
	SetPropertyValue(degreeProp, degree);
	SetPropertyValue(degreeDevianceProp, degreeDev);
	SetPropertyValue(snapSizeProp, snapSz);
	SetPropertyValue(snapSizeDevianceProp, snapDev);

	RefreshProperty(seedProp);
	RefreshProperty(segmentSizeProp);
	RefreshProperty(degreeProp);
	RefreshProperty(snapSizeProp);
	RefreshProperty(segmentDevianceProp);
	RefreshProperty(degreeDevianceProp);
	RefreshProperty(snapSizeDevianceProp);
}

void CellPropertyPage::setCanvas(WorldCanvas* c)
{
	mWorldCanvas = c;
}
