#ifndef TOOLNODESELECT_H
#define TOOLNODESELECT_H

#include "stdafx.h"
#include "ToolView.h"

class ToolNodeSelect : public ToolView
{

public:
	ToolNodeSelect(WorldFrame* wf);

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
