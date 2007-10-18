#include "stdafx.h"
#include "RoadPropertyPage.h"
#include "WorldFrame.h"
#include "WorldRoad.h"

// Required for WX
IMPLEMENT_CLASS(RoadPropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(RoadPropertyPage, wxPropertyGridPage)

    // This occurs when a property value changes
    EVT_PG_CHANGED(wxID_ANY, RoadPropertyPage::OnPropertyGridChange)
END_EVENT_TABLE()

void RoadPropertyPage::Init()
{
    Append(wxPropertyCategory(wxT("Adaptive Road Parameters")));


    wxPGChoices arrPlot;
	arrPlot.Add(wxT("Even Elevation Diff."), EvenElevationDiff);
	arrPlot.Add(wxT("Minimum Elevation Diff."), MinimumElevationDiff);
	arrPlot.Add(wxT("Minimum Elevation"), MinimumElevation);
    _plotAlgorProp = Append( wxEnumProperty(wxT("Algorithm"), wxPG_LABEL, arrPlot) );

	// Add float property (value type is actually double)
	_sampleSizeProp = Append(wxFloatProperty(wxT("Sample Size"), wxPG_LABEL, 0));
    _roadWidthProp = Append(wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0));
	_sampleDevianceProp = Append(wxFloatProperty(wxT("Road Deviance Angle"), wxPG_LABEL,0));
	_samplesProp = Append(wxIntProperty(wxT("Number of Samples"), wxPG_LABEL,0));

	Append(wxPropertyCategory(wxT("Display Options")));
	_segmentDrawSizeProp = Append(wxFloatProperty(wxT("Segment Draw Size"), wxPG_LABEL,0));
	_plotDebugProp = Append(wxBoolProperty(wxT("View Plot Debug Info"), wxPG_LABEL,0));
}


void RoadPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	//const wxPGProperty* eventProp = event.GetPropertyPtr();

	WorldRoad* wr = _worldFrame->getSelectedRoad();
	RoadGenParams g;
	if(wr) g = wr->getGenParams();
	else g = WorldRoad::getDefaultGenParams();

	g._algorithm = static_cast<RoadPlotAlgorithm>(GetPropertyValueAsInt(_plotAlgorProp));
	g._sampleSize = static_cast<Ogre::Real>(GetPropertyValueAsDouble(_sampleSizeProp));
	g._sampleDeviance = GetPropertyValueAsDouble(_sampleDevianceProp);
	g._roadWidth = GetPropertyValueAsDouble(_roadWidthProp);
	g._debug = GetPropertyValueAsBool(_plotDebugProp);
	int samples = GetPropertyValueAsInt(_samplesProp);
	double drawSz = GetPropertyValueAsDouble(_segmentDrawSizeProp);

	//TODO: check params within limits
	if(g._sampleDeviance < Ogre::Degree(0) || g._sampleDeviance > Ogre::Degree(44))
	{
		//alert user
		wxMessageBox(_T("Invalid 'sample deviance' value entered, a value between 0-44 is required."),
				_("Validation Failed"), wxICON_EXCLAMATION);

		//restore value
		update();

		//exit
		return;
	}

	if(samples < 1)
	{
		wxMessageBox(_T("Invalid 'Number of Samples' value entered, a greater than 0 is required."),
				_("Validation Failed"), wxICON_EXCLAMATION);

		update();
		return;
	}
	else
		g._numOfSamples = static_cast<Ogre::uint16>(samples);


	if(drawSz < 0.5)
	{
		wxMessageBox(_T("Invalid 'Segment Draw Size' value entered, a greater than 0.5 is required."),
				_("Validation Failed"), wxICON_EXCLAMATION);

		update();
		return;
	}
	else
		g._segmentDrawSize = static_cast<Ogre::Real>(drawSz);


	if(wr)
	{
		wr->setGenParams(g);
		_worldFrame->update();
	}
	else
	{
		WorldRoad::setDefaultGenParams(g);
		_worldFrame->update();
	}
	update();

    // Get resulting value - wxVariant is convenient here.
    //wxVariant value = event.GetPropertyValue();
}


void RoadPropertyPage::update()
{
	RoadGenParams rp;
	WorldRoad *wr = _worldFrame->getSelectedRoad();
	if(wr) rp = wr->getGenParams();
	else rp = WorldRoad::getDefaultGenParams();

	SetPropertyValue(_plotAlgorProp, rp._algorithm);
	SetPropertyValue(_sampleSizeProp, static_cast<double>(rp._sampleSize));
	SetPropertyValue(_sampleDevianceProp, rp._sampleDeviance.valueDegrees());
	SetPropertyValue(_roadWidthProp, rp._roadWidth);
	SetPropertyValue(_samplesProp, static_cast<int>(rp._numOfSamples));
	SetPropertyValue(_plotDebugProp, rp._debug);
	SetPropertyValue(_segmentDrawSizeProp, static_cast<float>(rp._segmentDrawSize));

	RefreshProperty(_plotAlgorProp);
	RefreshProperty(_sampleSizeProp);
	RefreshProperty(_sampleDevianceProp);
	RefreshProperty(_roadWidthProp);
	RefreshProperty(_samplesProp);
	RefreshProperty(_plotDebugProp);
	RefreshProperty(_segmentDrawSizeProp);
}


void RoadPropertyPage::setWorldFrame(WorldFrame* wf)
{
	_worldFrame = wf;
}
