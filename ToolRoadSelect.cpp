#include "stdafx.h"
#include "ToolRoadSelect.h"
#include "WorldRoad.h"

ToolRoadSelect::ToolRoadSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolRoadSelect::OnMouseMove(wxMouseEvent &e)
{
	ToolView::OnMouseMove(e);

	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;
/*
	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldRoad *wn;

	// if left drag
	if(mWorldFrame->pickRoad(e, HIGHLIGHTNODESNAPSQ, c))
		mWorldFrame->highlightRoad(c);
*/
	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolRoadSelect::OnLeftPressed(wxMouseEvent &e)
{
	WorldRoad *c;
	if(_worldFrame->pickRoad(e, c))
		_worldFrame->selectRoad(c);
	else
		_worldFrame->selectRoad(0);

	_worldFrame->update();
}
