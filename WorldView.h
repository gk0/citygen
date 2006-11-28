#ifndef _WORLDVIEW_H_
#define _WORLDVIEW_H_

#include "stdafx.h"
#include "wxOgre.h"
#include "WorldCanvas.h"
#include "WorldDocument.h"


class WorldView : public wxView
{
      DECLARE_DYNAMIC_CLASS(WorldView);
private:
	//std::map<>

public:
    wxFrame *mFrame;
    WorldCanvas *canvas;
    
    WorldView(void);
    virtual ~WorldView();
    
    virtual bool OnCreate(wxDocument *doc, long flags);

	//virtual void OnActivateView(bool activate, wxView *activeView, wxView *deactiveView);
	virtual void OnDraw(wxDC *dc);
    //virtual void OnPrint(wxDC *dc, wxObject *info);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    //virtual void OnClosingDocument();
	bool OnClose(bool);
	/*virtual void OnChangeFilename() {
		int i=0;
	}*/
    
    //void OnCut(wxCommandEvent& event);
    
    DECLARE_EVENT_TABLE()
};


#endif
