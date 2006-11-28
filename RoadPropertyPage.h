#ifndef ROADPROPERTYPAGE_H
#define ROADPROPERTYPAGE_H

#include "stdafx.h"

class RoadPropertyPage : public wxPropertyGridPage
{
private:

protected:

public:
	virtual void Init()
	{
		// Another way
		Append( wxPropertyCategory(wxT("Main")) );

		//Add some properties just to test this out
		Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );

		// Add int property
		Append( wxIntProperty(wxT("IntProperty"), wxPG_LABEL, 12345678) );

		// Add float property (value type is actually double)
		Append( wxFloatProperty(wxT("FloatProperty"), wxPG_LABEL, 12345.678) );

		// Add a bool property
		Append( wxBoolProperty(wxT("BoolProperty"), wxPG_LABEL, false) );

		// A string property that can be edited in a separate editor dialog.
		Append( wxLongStringProperty(wxT("LongStringProperty"),
										 wxPG_LABEL,
										 wxT("This is much longer string than the ")
										 wxT("first one. Edit it by clicking the button.")));

		// Another way
		Append( wxPropertyCategory(wxT("Extended")) );


		// String editor with dir selector button.
		Append( wxDirProperty(wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) );

		// A file selector property.
		Append( wxFileProperty(wxT("FileProperty"), wxPG_LABEL, wxEmptyString) );

		// Extra: set wildcard for file property (format same as in wxFileDialog).
		//SetPropertyAttribute(wxT("TextFile"),
		//                         wxPG_FILE_WILDCARD,
		//                         wxT("All files (*.*)|*.*"));

		// Another way  
		Append( wxPropertyCategory(wxT("Advanced")) );


		// wxArrayStringProperty embeds a wxArrayString.
		Append( wxArrayStringProperty(wxT("Label of ArrayStringProperty"),
										  wxT("NameOfArrayStringProp")));

		// Date property.
		// NB: This will use wxDatePickerCtrl only if wxPG_ALLOW_WXADV is defined
		//     in propgrid.h or in the library project settings.
		Append( wxDateProperty(wxT("MyDateProperty"),
								   wxPG_LABEL,
								   wxDateTime::Now()) );

		// Image file property. Wildcard is auto-generated from available
		// image handlers, so it is not set this time.
		Append( wxImageFileProperty(wxT("Label of ImageFileProperty"),
										wxT("NameOfImageFileProp")));

		// Font property has sub-properties. Note that we give window's font as
		// initial value.
		//Append( wxFontProperty(wxT("Font"),
		//            wxPG_LABEL,
		//            GetFont()) );

		// Colour property with arbitrary colour.
		Append( wxColourProperty(wxT("My Colour 1"),
									 wxPG_LABEL,
									 wxColour(242,109,0) ) );

		// System colour property.
		Append( wxSystemColourProperty (wxT("My SysColour 1"),
											wxPG_LABEL,
											wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

		// System colour property with custom colour.
		Append( wxSystemColourProperty (wxT("My SysColour 2"),
											wxPG_LABEL,
											wxColour(0,200,160) ) );

		// Cursor property
		Append( wxCursorProperty (wxT("My Cursor"),
									  wxPG_LABEL,
									  wxCURSOR_ARROW));
	}

};

#endif
