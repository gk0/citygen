#include "stdafx.h"
#include "TerrainPropertyPage.h"
#include "WorldFrame.h"
#include "WorldTerrain.h"

#include <wx/propgrid/advprops.h>
#include <wx/msgdlg.h>

// Required for WX
IMPLEMENT_CLASS(TerrainPropertyPage, wxPropertyGridPage)

// Portion of an imaginary event table
BEGIN_EVENT_TABLE(TerrainPropertyPage, wxPropertyGridPage)

// This occurs when a property value changes
EVT_PG_CHANGED(wxID_ANY, TerrainPropertyPage::OnPropertyGridChange)
END_EVENT_TABLE()

void TerrainPropertyPage::Init()
{
	if(_worldFrame == 0) return;
	
	BOOST_FOREACH(Property* p, _worldFrame->getWorldTerrain()->getProperties().getList())
		addProperty(p);
}

void TerrainPropertyPage::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	const wxPGProperty* eventProp = event.GetPropertyPtr();
	if(eventProp != 0)
	{
		using namespace std;
		string id = string(_C(eventProp->GetName()));
		Property* p;
		
		if(_worldFrame->getWorldTerrain()->getProperties().findAll(id, p))
		{
			string val = string(_C(eventProp->GetValueAsString()));
			if(p->validate(val))
			{
				p->fromString(val);
				_worldFrame->updateTerrain();
			}
			else
			{
				wxMessageBox(_U(("Invalid value entered for property \'"+p->_name+"\'. "+p->_validator._errorMsg).c_str()),
					_("Validation Failed"), wxICON_EXCLAMATION);
				setPropertyValue(p);
			}
		}
	}
}

void TerrainPropertyPage::update()
{
	BOOST_FOREACH(Property* p, _worldFrame->getWorldTerrain()->getProperties().getList())
		setPropertyValue(p);
}
