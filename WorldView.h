#ifndef _WORLDVIEW_H_
#define _WORLDVIEW_H_

#include "stdafx.h"

#include "WorldWindow.h"


class WorldView : public wxView
{
    DECLARE_DYNAMIC_CLASS(WorldView)
private:
public:
    wxFrame *frame;
    WorldWindow *canvas;
    
    WorldView(void) { canvas = (WorldWindow *) NULL; frame = (wxFrame *) NULL; };
    virtual ~WorldView();
    
    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = true);


	//virtual void OnActivateView(bool activate, wxView *activeView, wxView *deactiveView);
    //virtual void OnPrint(wxDC *dc, wxObject *info);
    //virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    //virtual void OnClosingDocument();
	/*virtual void OnChangeFilename() {
		int i=0;
	}*/
    
    //void OnCut(wxCommandEvent& event);
    
    DECLARE_EVENT_TABLE()
};


#endif
