#include "stdafx.h"
#include "PropertyPage.h"
#include "WorldFrame.h"

#include <wx/propgrid/advprops.h>
#include <wx/msgdlg.h>



void PropertyPage::setPropertyValue(Property* p)
{
	wxString id = _U(p->_name.c_str());
	if(typeid(*p) == typeid(PropertyBool))
	{
		SetPropertyValue(GetPropertyByLabel(id), static_cast<PropertyBool*>(p)->_data);
	}
	else if(typeid(*p) == typeid(PropertyEnum))
	{
		SetPropertyValue(GetPropertyByLabel(id), _U(static_cast<PropertyBool*>(p)->toString().c_str()));
	}
	else if(typeid(*p) == typeid(PropertyImage))
	{
		SetPropertyValue(GetPropertyByLabel(id), _U(static_cast<PropertyImage*>(p)->toString().c_str()));
	}
	else if(typeid(*p) == typeid(PropertyList))
	{
		BOOST_FOREACH(Property* p2, static_cast<PropertyList*>(p)->getList()) setPropertyValue(p2);
	}
	else if(typeid(*p) == typeid(PropertyReal))
	{
		SetPropertyValue(GetPropertyByLabel(id), static_cast<PropertyReal*>(p)->_data);
	}
	else if(typeid(*p) == typeid(PropertyString))
	{
		SetPropertyValue(GetPropertyByLabel(id), _U(static_cast<PropertyString*>(p)->toString().c_str()));
	}
	else if(typeid(*p) == typeid(PropertyUInt))
	{
		SetPropertyValue(GetPropertyByLabel(id), (int) static_cast<PropertyUInt*>(p)->_data);
	}
	else if(typeid(*p) == typeid(PropertyUShort))
	{
		SetPropertyValue(GetPropertyByLabel(id), static_cast<PropertyUShort*>(p)->_data);
	}

	// Refresh it if its not a category
	if(typeid(*p) != typeid(PropertyEnum)) RefreshProperty(GetPropertyByLabel(id));
}

void PropertyPage::addProperty(Property* p)
{
	wxString id = _U(p->_name.c_str());
	if(typeid(*p) == typeid(PropertyBool))
	{
		Append(wxBoolProperty(id, wxPG_LABEL, static_cast<PropertyBool*>(p)->_data));
	}
	else if(typeid(*p) == typeid(PropertyEnum))
	{
		PropertyEnum* pe = static_cast<PropertyEnum*>(p);
		wxArrayString enumOptions;
		wxArrayInt enumValues;

		for(size_t i=0; i < pe->_enumNames.size(); i++)
		{
			enumOptions.Add(_U(pe->_enumNames[i].c_str()));
			enumValues.Add((int)i);
		}
		Append(wxEnumProperty(id, wxPG_LABEL, enumOptions, enumValues, static_cast<int>(pe->_data)));
	}
	else if(typeid(*p) == typeid(PropertyImage))
	{
		Append(wxImageFileProperty(id,wxPG_LABEL, _U(static_cast<PropertyImage*>(p)->toString().c_str())));
	}
	else if(typeid(*p) == typeid(PropertyList))
	{
		Append(wxPropertyCategory(id));
		BOOST_FOREACH(Property* p2, static_cast<PropertyList*>(p)->getList()) addProperty(p2);
	}
	else if(typeid(*p) == typeid(PropertyReal))
	{
		Append(wxFloatProperty(id, wxPG_LABEL, static_cast<PropertyReal*>(p)->_data));
	}
	else if(typeid(*p) == typeid(PropertyString))
	{
		Append(wxStringProperty(id, wxPG_LABEL, _U(static_cast<PropertyString*>(p)->_data.c_str())));
	}
	else if(typeid(*p) == typeid(PropertyUInt))
	{
		Append(wxIntProperty(id, wxPG_LABEL, static_cast<PropertyUInt*>(p)->_data));
	}
	else if(typeid(*p) == typeid(PropertyUShort))
	{
		Append(wxIntProperty(id, wxPG_LABEL, static_cast<PropertyUShort*>(p)->_data));
	}
}

void PropertyPage::update()
{
}

