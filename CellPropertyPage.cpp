#include "stdafx.h"
#include "CellPropertyPage.h"
#include "WorldFrame.h"
#include "WorldCell.h"

#include <wx/propgrid/advprops.h>

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

    Append(wxPropertyCategory(wxT("Road Parameters")));

	_seedProp = Append(wxIntProperty(wxT("Seed"), wxPG_LABEL, 0));

	// Add float property (value type is actually double)
    _segmentSizeProp = Append(wxFloatProperty(wxT("Segment Size (m)"), wxPG_LABEL, 0));
	_segmentDevianceProp = Append(wxFloatProperty(wxT("Segment Deviance"), wxPG_LABEL, 0));

	// Add int property
    _degreeProp = Append(wxIntProperty(wxT("Degree"), wxPG_LABEL, 0));
	_degreeDevianceProp = Append(wxFloatProperty(wxT("Degree Deviance"), wxPG_LABEL, 0));

	// Add float property (value type is actually double)
    _snapSizeProp = Append(wxFloatProperty(wxT("Snap Size (m)"), wxPG_LABEL, 0));
	_snapDevianceProp = Append(wxFloatProperty(wxT("Snap Size Deviance"), wxPG_LABEL, 0));

	_roadWidthProp = Append(wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0));
	_connectivityProp = Append(wxFloatProperty(wxT("Connectivity"), wxPG_LABEL, 0));

	_roadLimitProp = Append(wxIntProperty(wxT("Road Limit"), wxPG_LABEL, 0));
	SetPropertyEditor(wxT("Road Limit"), wxPG_EDITOR(SpinCtrl));

	Append(wxPropertyCategory(wxT("Block Parameters")));
	_footpathWidthProp = Append(wxFloatProperty(wxT("Footpath width (m)"), wxPG_LABEL, 0));
	_footpathHeightProp = Append(wxFloatProperty(wxT("Footpath height (m)"), wxPG_LABEL, 0));

	Append(wxPropertyCategory(wxT("Lot Parameters")));
	_lotWidthProp = Append(wxFloatProperty(wxT("Lot width (m)"), wxPG_LABEL, 0));
	_lotDepthProp = Append(wxFloatProperty(wxT("Lot depth (m)"), wxPG_LABEL, 0));
	_lotDevianceProp = Append(wxFloatProperty(wxT("Lot Deviance"), wxPG_LABEL, 0));

	Append(wxPropertyCategory(wxT("Building Parameters")));
	_buildingHeightProp = Append(wxFloatProperty(wxT("Building Height (m)"), wxPG_LABEL, 0));
	_buildingDevianceProp = Append(wxFloatProperty(wxT("Building Deviance"), wxPG_LABEL, 0));

	wxPGChoices arrPlot2;
	arrPlot2.Add(wxT("Downtown"), 0);
	arrPlot2.Add(wxT("Industrial"), 1);
	arrPlot2.Add(wxT("Suburbia"), 2);
	_typeProp = Append( wxEnumProperty(wxT("Building Hint"), wxPG_LABEL, arrPlot2) );

	Append(wxPropertyCategory(wxT("Display Options")));

	_debugProp =  Append(wxBoolProperty(wxT("View Debug Info"), wxPG_LABEL,0));
}


void CellPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();


	CellParams g;
	WorldCell *wc = _worldFrame->getSelectedCell();
	if(wc) g = wc->getGenParams();
	else g = WorldCell::getDefaultGenParams();

	if(eventProp == _presetProp)
	{
		switch(GetPropertyValueAsInt(_presetProp))
		{
		// Manhattan
		case 1:
			g = CellParams::MANHATTAN;
			break;
		case 2:
			g = CellParams::INDUSTRIAL;
			break;
		case 3:
			g = CellParams::SUBURBIA;
			break;
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
		g._footpathWidth = GetPropertyValueAsDouble(_footpathWidthProp);
		g._footpathHeight = GetPropertyValueAsDouble(_footpathHeightProp);
		g._lotWidth = GetPropertyValueAsDouble(_lotWidthProp);
		g._lotDepth = GetPropertyValueAsDouble(_lotDepthProp);
		g._lotDeviance = GetPropertyValueAsDouble(_lotDevianceProp);
		g._debug = GetPropertyValueAsBool(_debugProp);
	}

	if(wc)
	{
		wc->setGenParams(g);
		_worldFrame->Refresh();
	}
	else WorldCell::setDefaultGenParams(g);
	update();

    // Get resulting value - wxVariant is convenient here.
    wxVariant value = event.GetPropertyValue();
}

void CellPropertyPage::update()
{
	CellParams g;
	WorldCell *wc = _worldFrame->getSelectedCell();
	if(wc) g = wc->getGenParams();
	else g = WorldCell::getDefaultGenParams();
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
	SetPropertyValue(_footpathHeightProp, g._footpathHeight);
	SetPropertyValue(_footpathWidthProp, g._footpathWidth);
	SetPropertyValue(_lotWidthProp, g._lotWidth);
	SetPropertyValue(_lotDepthProp, g._lotDepth);
	SetPropertyValue(_lotDevianceProp, g._lotDeviance);
	SetPropertyValue(_debugProp, g._debug);

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
	RefreshProperty(_footpathWidthProp);
	RefreshProperty(_footpathHeightProp);
	RefreshProperty(_lotWidthProp);
	RefreshProperty(_lotDepthProp);
	RefreshProperty(_lotDevianceProp);
	RefreshProperty(_debugProp);
}

void CellPropertyPage::setWorldFrame(WorldFrame* wf)
{
	_worldFrame = wf;
}
