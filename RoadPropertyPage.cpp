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
    plotAlgorProp = Append( wxEnumProperty(wxT("Algorithm"), wxPG_LABEL, arrPlot) );

	// Add float property (value type is actually double)
	sampleSizeProp = Append(wxFloatProperty(wxT("Sample Size"), wxPG_LABEL, 0));
    roadWidthProp = Append(wxFloatProperty(wxT("Road Width"), wxPG_LABEL, 0));
	sampleDevianceProp = Append(wxFloatProperty(wxT("Road Deviance Angle"), wxPG_LABEL,0));
	samplesProp = Append(wxIntProperty(wxT("Number of Samples"), wxPG_LABEL,0));

	Append(wxPropertyCategory(wxT("Display Options")));
	segmentDrawSizeProp = Append(wxFloatProperty(wxT("Segment Draw Size"), wxPG_LABEL,0));
	plotDebugProp = Append(wxBoolProperty(wxT("View Plot Debug Info"), wxPG_LABEL,0));
}


void RoadPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	//const wxId& id = event.GetId();
	const wxPGProperty* eventProp = event.GetPropertyPtr();

	if( (eventProp == sampleSizeProp) || (eventProp == roadWidthProp) || 
		(eventProp == sampleDevianceProp) || (eventProp == samplesProp) || 
		(eventProp == plotDebugProp) || (eventProp == plotAlgorProp) || (eventProp == segmentDrawSizeProp))
	{
		WorldRoad* wr = mWorldFrame->getSelectedRoad();
		if(wr)
		{
			RoadGenParams g;
			g.algorithm = static_cast<RoadPlotAlgorithm>(GetPropertyValueAsInt(plotAlgorProp));
			g.sampleSize = GetPropertyValueAsDouble(sampleSizeProp);
			g.sampleDeviance = GetPropertyValueAsDouble(sampleDevianceProp);
			g.roadWidth = GetPropertyValueAsDouble(roadWidthProp);
			g.debug = GetPropertyValueAsBool(plotDebugProp);
			int samples = GetPropertyValueAsInt(samplesProp);
			double drawSz = GetPropertyValueAsDouble(segmentDrawSizeProp);

			//TODO: check params within limits
			if(g.sampleDeviance < Ogre::Degree(0) || g.sampleDeviance > Ogre::Degree(44))
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
				g.numOfSamples = static_cast<Ogre::uint16>(samples);


			if(drawSz < 0.5)
			{
				wxMessageBox(_T("Invalid 'Segment Draw Size' value entered, a greater than 0.5 is required."),
						_("Validation Failed"), wxICON_EXCLAMATION);

				update();
				return;
			}
			else
				g.segmentDrawSize = static_cast<Ogre::Real>(drawSz);

			wr->setGenParams(g);
			mWorldFrame->update();
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
			RoadPlotAlgorithm algorithm;
			Ogre::Real sampleSize;
			Ogre::Degree sampleDeviance;
			Ogre::Real roadWidth;
			Ogre::uint16 numOfSamples;
			bool debug;
			Ogre::Real segmentDrawSize;
		} RoadGenParams;
		*/
		SetPropertyValue(plotAlgorProp, rp.algorithm);
		SetPropertyValue(sampleSizeProp, rp.sampleSize);
		SetPropertyValue(sampleDevianceProp, rp.sampleDeviance.valueDegrees());
		SetPropertyValue(roadWidthProp, rp.roadWidth);
		SetPropertyValue(samplesProp, static_cast<int>(rp.numOfSamples));
		SetPropertyValue(plotDebugProp, rp.debug);
		SetPropertyValue(segmentDrawSizeProp, static_cast<float>(rp.segmentDrawSize));
	}

	RefreshProperty(plotAlgorProp);
	RefreshProperty(sampleSizeProp);
	RefreshProperty(sampleDevianceProp);
	RefreshProperty(roadWidthProp);
	RefreshProperty(samplesProp);
	RefreshProperty(plotDebugProp);
	RefreshProperty(segmentDrawSizeProp);
}


void RoadPropertyPage::setWorldFrame(WorldFrame* wf)
{
	mWorldFrame = wf;
}
