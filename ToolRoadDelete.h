#ifndef TOOLROADDELETE_H
#define TOOLROADDELETE_H

#include "stdafx.h"
#include "ToolView.h"

class ToolRoadDelete : public ToolView
{

public:
	ToolRoadDelete(WorldFrame* wf);

	//void activate();
	//void deactivate();
	//void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);
};

#endif
