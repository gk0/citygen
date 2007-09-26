#ifndef TOOLNODEDELETE_H
#define TOOLNODEDELETE_H

#include "stdafx.h"
#include "ToolView.h"

class ToolNodeDelete : public ToolView
{

public:
	ToolNodeDelete(WorldFrame* wf);

	//void activate();
	//void deactivate();
	//void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);
};

#endif
