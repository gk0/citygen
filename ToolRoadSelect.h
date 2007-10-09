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
	void OnMiddlePressed(wxMouseEvent &e) {}
	void OnRightPressed(wxMouseEvent &e) {}
	//void OnMouseWheel(wxMouseEvent &e);
	bool alternate(wxMouseEvent &e);
};

#endif
