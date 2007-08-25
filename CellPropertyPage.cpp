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
	arrPlot.Add(wxT("Select Preset ..."), 0);
	arrPlot.Add(wxT("Manhattan"), 1);
	arrPlot.Add(wxT("Industrial"), 2);
	arrPlot.Add(wxT("Suburbia"), 3);

    //presetProp = Append( wxEditEnumProperty(wxT("Load Preset"), wxPG_LABEL, arrPlot) );
	_presetProp = Append( wxEnumProperty(wxT("Load Preset"), wxPG_LABEL, arrPlot) );


	wxPGChoices arrPlot2;
	arrPlot2.Add(wxT("Downtown"), 0);
	arrPlot2.Add(wxT("Industrial"), 1);
	arrPlot2.Add(wxT("Suburbia"), 2);
	_typeProp = Append( wxEnumProperty(wxT("Building Hint"), wxPG_LABEL, arrPlot2) );

	/*const wxPGEditor* pdedit = GetGrid()->GetPropertyEditor(presetProp);
    wxButton* but = (wxButton*) GetGrid()->GenerateEditorButton(wxDefaultPosition, wxDefaultSize);
	pdedit->InsertItem((wxWindow*)but, "Test", 0);*/



    SetPropertyEditor(_presetProp,wxPG_EDITOR(ChoiceAndButton));
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

	_seedProp = Append(wxIntProperty(wxT("Seed"), wxPG_LABEL, 0));

	// Add float property (value type is actually double)
    _segmentSizeProp = Append(wxFloatProperty(wxT("Segment Size"), wxPG_LABEL, 4.5));
	_segmentDevianceProp = Append(wxFloatProperty(wxT("Segment Deviance"), wxPG_LABEL, 0.1));

	// Add int property
    _degreeProp = Append(wxIntProperty(wxT("Degree"), wxPG_LABEL, 4));
	_degreeDevianceProp = Append(wxFloatProperty(wxT("Degree Deviance"), wxPG_LABEL, 0.1));

	// Add float property (value type is actually double)
    _snapSizeProp = Append(wxFloatProperty(wxT("Snap Size"), wxPG_LABEL, 4.5));
	_snapDevianceProp = Append(wxFloatProperty(wxT("Snap Size Deviance"), wxPG_LABEL, 0.1));

	_roadWidthProp = Append(wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0.));

	_buildingHeightProp = Append(wxFloatProperty(wxT("Building Height"), wxPG_LABEL, 0.));
	_buildingDevianceProp = Append(wxFloatProperty(wxT("Building Deviance"), wxPG_LABEL, 0.));

	_roadLimitProp = Append(wxIntProperty(wxT("Road Limit"), wxPG_LABEL, 4));

	_connectivityProp = Append(wxFloatProperty(wxT("Connectivity"), wxPG_LABEL, 0));

	_lotSizeProp = Append(wxFloatProperty(wxT("Lot Size"), wxPG_LABEL, 2));
	_lotDevianceProp = Append(wxFloatProperty(wxT("Lot Deviance"), wxPG_LABEL, 0.2));

	_debugProp =  Append(wxBoolProperty(wxT("View Debug Info"), wxPG_LABEL,0));

}


void CellPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if(eventProp == _presetProp)
	{
		WorldCell *wc = _worldFrame->getSelectedCell();
		if(wc)
		{
			CellGenParams g;
			switch(GetPropertyValueAsInt(_presetProp))
			{
			// Manhattan
			case 1:
				g._seed = 1;
				g._type = 0;
				g._segmentSize = 5;
				g._segmentDeviance = 0.2;
				g._degree = 4;
				g._degreeDeviance = 0.01;
				g._snapSize = 2.4;
				g._snapDeviance = 0.1;
				g._buildingHeight = 1.4;
				g._buildingDeviance = 0.7;
				g._connectivity = 1.0;
				g._roadWidth = 0.45;
				g._lotSize = 0.7;
				g._lotDeviance = 0.4;
				wc->setGenParams(g);
				_worldFrame->update();
				update();
				break;
			case 2:
				g._seed = 1;
				g._type = 1;
				g._segmentSize = 4;
				g._segmentDeviance = 0.2;
				g._degree = 4;
				g._degreeDeviance = 0.01;
				g._snapSize = 2.4;
				g._snapDeviance = 0.1;
				g._buildingHeight = 0.7;
				g._buildingDeviance = 0.3;
				g._connectivity = 0.3;
				g._roadWidth = 0.40;
				g._lotSize = 2.0;
				g._lotDeviance = 0.7;
				wc->setGenParams(g);
				_worldFrame->update();
				update();
				break;
			case 3:
				g._seed = 1;
				g._type = 2;
				g._segmentSize = 3.8;
				g._segmentDeviance = 0.6;
				g._degree = 9;
				g._degreeDeviance = 0.6;
				g._snapSize = 2.8;
				g._snapDeviance = 0.1;
				g._buildingHeight = 0.4;
				g._buildingDeviance = 0.1;
				g._connectivity = 0.0;
				g._roadWidth = 0.3;
				g._lotSize = 0.8;
				g._lotDeviance = 0.2;
				wc->setGenParams(g);
				_worldFrame->update();
				update();
				break;
			}
		}
	}
	else /*if((eventProp == seedProp) || (eventProp == segmentSizeProp) || (eventProp == degreeProp) 
		|| (eventProp == snapSizeProp) || (eventProp == _segmentDevianceProp) 
		|| (eventProp == degreeDevianceProp) || (eventProp == snapDevianceProp)
		|| (eventProp == buildingHeightProp) || (eventProp == buildingDevianceProp)
		|| (eventProp == roadWidthProp) || (eventProp == roadLimitProp))*/
	{
		// prevent seed 0
		if(GetPropertyValueAsLong(_seedProp)==0)
			update();

		WorldCell *wc = _worldFrame->getSelectedCell();
		if(wc)
		{
			CellGenParams g;
			g._seed = GetPropertyValueAsLong(_seedProp);
			g._type = GetPropertyValueAsInt(_typeProp);
			g._segmentSize = GetPropertyValueAsDouble(_segmentSizeProp);
			g._segmentDeviance = GetPropertyValueAsDouble(_segmentDevianceProp);
			g._degree = GetPropertyValueAsInt(_degreeProp);
			g._degreeDeviance = GetPropertyValueAsDouble(_degreeDevianceProp);
			g._snapSize = GetPropertyValueAsDouble(_snapSizeProp);
			g._snapDeviance = GetPropertyValueAsDouble(_snapDevianceProp);
			g._roadWidth = GetPropertyValueAsDouble(_roadWidthProp);
			g._buildingHeight = GetPropertyValueAsDouble(_buildingHeightProp);
			g._buildingDeviance = GetPropertyValueAsDouble(_buildingDevianceProp);
			g._roadLimit = GetPropertyValueAsInt(_roadLimitProp);
			g._connectivity = GetPropertyValueAsDouble(_connectivityProp);
			g._lotSize = GetPropertyValueAsDouble(_lotSizeProp);
			g._lotDeviance = GetPropertyValueAsDouble(_lotDevianceProp);
			g._debug = GetPropertyValueAsBool(_debugProp);
			
			wc->setGenParams(g);
			_worldFrame->update();
			update();
		}
	}

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}

void CellPropertyPage::update()
{
	CellGenParams g;
	WorldCell *wc = _worldFrame->getSelectedCell();
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
		} CellGenParams;
		*/
		SetPropertyValue(_presetProp, 0);
		SetPropertyValue(_seedProp, g._seed);
		SetPropertyValue(_typeProp, (int) g._type);
		SetPropertyValue(_segmentSizeProp, g._segmentSize);
		SetPropertyValue(_segmentDevianceProp, g._segmentDeviance);
		SetPropertyValue(_degreeProp, (int) g._degree);
		SetPropertyValue(_degreeDevianceProp, g._degreeDeviance);
		SetPropertyValue(_snapSizeProp, g._snapSize);
		SetPropertyValue(_snapDevianceProp, g._snapDeviance);
		SetPropertyValue(_roadWidthProp, g._roadWidth);
		SetPropertyValue(_buildingHeightProp, g._buildingHeight);
		SetPropertyValue(_buildingDevianceProp, g._buildingDeviance);
		SetPropertyValue(_roadLimitProp, (int) g._roadLimit);
		SetPropertyValue(_connectivityProp, g._connectivity);
		SetPropertyValue(_lotSizeProp, g._lotSize);
		SetPropertyValue(_lotDevianceProp, g._lotDeviance);
		SetPropertyValue(_debugProp, g._debug);

	}
	RefreshProperty(_presetProp);
	RefreshProperty(_seedProp);
	RefreshProperty(_segmentSizeProp);
	RefreshProperty(_degreeProp);
	RefreshProperty(_snapSizeProp);
	RefreshProperty(_segmentDevianceProp);
	RefreshProperty(_degreeDevianceProp);
	RefreshProperty(_snapDevianceProp);
	RefreshProperty(_roadWidthProp);
	RefreshProperty(_buildingHeightProp);
	RefreshProperty(_buildingDevianceProp);
	RefreshProperty(_roadLimitProp);
	RefreshProperty(_connectivityProp);
	RefreshProperty(_lotSizeProp);
	RefreshProperty(_lotDevianceProp);
	RefreshProperty(_debugProp);
}

void CellPropertyPage::setWorldFrame(WorldFrame* wf)
{
	_worldFrame = wf;
}
