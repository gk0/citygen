#ifndef _EDITMODETOOLBAR_H_
#define _EDITMODETOOLBAR_H_

#include "stdafx.h"
#include "EditModeListener.h"

class EditModeToolBar : public wxToolBar {

private:
	EditModeListener* m_EditModeListener;
	DECLARE_EVENT_TABLE()
	void onSelectViewMode(wxCommandEvent &e);
	void onSelectNodeMode(wxCommandEvent &e);
	void onSelectEdgeMode(wxCommandEvent &e);

public:
	// wxToolBar(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, 
	//	 const wxSize& size = wxDefaultSize, long style = wxTB_HORIZonTAL | wxNO_BORDER, 
	//	 const wxString& name = wxPanelNameStr)
	EditModeToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar = false);
	void setListener(EditModeListener* listener);

};

#endif
