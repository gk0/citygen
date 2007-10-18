#ifndef TOOLCELLSELECT_H
#define TOOLCELLSELECT_H

#include "stdafx.h"
#include "ToolView.h"

class ToolCellSelect : public ToolView
{

public:
	ToolCellSelect(WorldFrame* wf);

	//void activate();
	//void deactivate();
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);
};

#endif
