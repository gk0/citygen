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
	Append(wxPropertyCategory(wxT("Presets")));
	

	wxPGChoices arrPlot;
	arrPlot.Add(wxT("Manhattan"), 0);
	arrPlot.Add(wxT("Industrial"), 1);
	arrPlot.Add(wxT("Suburbia"), 2);

    //presetProp = Append( wxEditEnumProperty(wxT("Load Preset"), wxPG_LABEL, arrPlot) );
	presetProp = Append( wxEnumProperty(wxT("Load Preset"), wxPG_LABEL, arrPlot) );

	/*const wxPGEditor* pdedit = GetGrid()->GetPropertyEditor(presetProp);
    wxButton* but = (wxButton*) GetGrid()->GenerateEditorButton(wxDefaultPosition, wxDefaultSize);
	pdedit->InsertItem((wxWindow*)but, "Test", 0);*/



    SetPropertyEditor(presetProp,wxPG_EDITOR(ChoiceAndButton));
	//presetProp

/*
	const wxChar* flags_prop_labels[] = { wxT("wxICONIZE"),
            wxT("wxCAPTION"), wxT("wxMINIMIZE_BOX"), wxT("wxMAXIMIZE_BOX"), NULL };

        // this value array would be optional if values matched string indexes
        long flags_prop_values[] = { wxICONIZE, wxCAPTION, wxMINIMIZE_BOX,
            wxMAXIMIZE_BOX };

        Append( wxFlagsProperty(wxT("Window Style"),
                                    wxPG_LABEL,
                                    flags_prop_labels,
                                    flags_prop_values,
                                    wxDEFAULT_FRAME_STYLE) );
*/

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

	roadWidthProp = Append(wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0.));

	buildingHeightProp = Append(wxFloatProperty(wxT("Building Height"), wxPG_LABEL, 0.));
	buildingDevianceProp = Append(wxFloatProperty(wxT("Building Deviance"), wxPG_LABEL, 0.));

	roadLimitProp = Append(wxIntProperty(wxT("Road Limit"), wxPG_LABEL, 4));

	lotSizeProp = Append(wxFloatProperty(wxT("Lot Size"), wxPG_LABEL, 2));
	lotDevianceProp = Append(wxFloatProperty(wxT("Lot Deviance"), wxPG_LABEL, 0.2));

}


void CellPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if(eventProp == presetProp)
	{
		WorldCell *wc = mWorldFrame->getSelectedCell();
		if(wc)
		{
			GrowthGenParams g;
			switch(GetPropertyValueAsInt(presetProp))
			{
			// Manhattan
			case 0:
				g.seed = 0;
				g.segmentSize = 6;
				g.segmentDeviance = 0.4;
				g.degree = 4;
				g.degreeDeviance = 0.01;
				g.snapSize = 2.4;
				g.snapDeviance = 0.1;
				g.buildingHeight = 2.4;
				g.buildingDeviance = 0.1;
				g.roadWidth = 0.4;
				wc->setGenParams(g);
				mWorldFrame->update();
				break;
			case 1:
				g.seed = 0;
				g.segmentSize = 6;
				g.segmentDeviance = 2;
				g.degree = 4;
				g.degreeDeviance = 0.01;
				g.snapSize = 2.4;
				g.snapDeviance = 0.1;
				g.buildingHeight = 1.8;
				g.buildingDeviance = 0.3;
				g.roadWidth = 0.4;
				wc->setGenParams(g);
				mWorldFrame->update();
				break;
			case 2:
				break;
			}
		}
	}
	else /*if((eventProp == seedProp) || (eventProp == segmentSizeProp) || (eventProp == degreeProp) 
		|| (eventProp == snapSizeProp) || (eventProp == segmentDevianceProp) 
		|| (eventProp == degreeDevianceProp) || (eventProp == snapDevianceProp)
		|| (eventProp == buildingHeightProp) || (eventProp == buildingDevianceProp)
		|| (eventProp == roadWidthProp) || (eventProp == roadLimitProp))*/
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
			g.roadWidth = GetPropertyValueAsDouble(roadWidthProp);
			g.buildingHeight = GetPropertyValueAsDouble(buildingHeightProp);
			g.buildingDeviance = GetPropertyValueAsDouble(buildingDevianceProp);
			g.roadLimit = GetPropertyValueAsInt(roadLimitProp);
			g.lotSize = GetPropertyValueAsDouble(lotSizeProp);
			g.lotDeviance = GetPropertyValueAsDouble(lotDevianceProp);
			
			wc->setGenParams(g);
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
		g = wc->getGenParams();
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
		SetPropertyValue(roadWidthProp, g.roadWidth);
		SetPropertyValue(buildingHeightProp, g.buildingHeight);
		SetPropertyValue(buildingDevianceProp, g.buildingDeviance);
		SetPropertyValue(roadLimitProp, (int) g.roadLimit);
		SetPropertyValue(lotSizeProp, g.lotSize);
		SetPropertyValue(lotDevianceProp, g.lotDeviance);
	}
	RefreshProperty(seedProp);
	RefreshProperty(segmentSizeProp);
	RefreshProperty(degreeProp);
	RefreshProperty(snapSizeProp);
	RefreshProperty(segmentDevianceProp);
	RefreshProperty(degreeDevianceProp);
	RefreshProperty(snapDevianceProp);
	RefreshProperty(roadWidthProp);
	RefreshProperty(buildingHeightProp);
	RefreshProperty(buildingDevianceProp);
	RefreshProperty(roadLimitProp);
	RefreshProperty(lotSizeProp);
	RefreshProperty(lotDevianceProp);
}

void CellPropertyPage::setWorldFrame(WorldFrame* wf)
{
	mWorldFrame = wf;
}
