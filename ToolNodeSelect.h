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
	//void OnMouseWheel(wxMouseEvent &e);
};

#endif
