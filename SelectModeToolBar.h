#ifndef _NODEEDITTOOLBAR_H_
#define _NODEEDITTOOLBAR_H_

#include "stdafx.h"
#include "EditModeListener.h"
#include "SelectModeListener.h"

class SelectModeToolBar : public wxToolBar, EditModeListener {

private:
	DECLARE_EVENT_TABLE()
	bool mLargeToolbar;
	SelectModeListener*  mSelectModeListener;

public:
	// wxToolBar(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, 
	//	 const wxSize& size = wxDefaultSize, long style = wxTB_HORIZonTAL | wxNO_BORDER, 
	//	 const wxString& name = wxPanelNameStr)
	SelectModeToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar = false);

	void setEditMode(EditModeListener::EditMode mode);
	void init(EditModeListener::EditMode mode);
	void setListener(SelectModeListener* listener);

	void onSelectSelMode(wxCommandEvent &e);
	void onSelectAddMode(wxCommandEvent &e);
	void onSelectDelMode(wxCommandEvent &e);
};

#endif
