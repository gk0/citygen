#ifndef _FileToolBar_H_
#define _FileToolBar_H_

#include "stdafx.h"
#include "FileToolBar.h"

class FileToolBar : public wxToolBar {

private:
	DECLARE_EVENT_TABLE()

public:
	// wxToolBar(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, 
	//	 const wxSize& size = wxDefaultSize, long style = wxTB_HORIZonTAL | wxNO_BORDER, 
	//	 const wxString& name = wxPanelNameStr)
	FileToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar = false);

};

#endif
