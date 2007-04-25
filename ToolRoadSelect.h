#ifndef TOOLROADSELECT_H
#define TOOLROADSELECT_H

#include "stdafx.h"
#include "ToolView.h"

class ToolRoadSelect : public ToolView
{

public:
	ToolRoadSelect(WorldFrame* wf);

	//void activate();
	//void deactivate();
	//void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);
};

#endif
