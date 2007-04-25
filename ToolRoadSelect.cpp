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
	mMouseDeltaX = e.m_x - mMouseX;
	mMouseDeltaY = e.m_y - mMouseY;
/*
	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldRoad *wn;

	// if left drag
	if(mWorldFrame->pickRoad(e, HIGHLIGHTNODESNAPSQ, c))
		mWorldFrame->highlightRoad(c);
*/
	// save for calc of next deltas
	mMouseX = e.m_x;
	mMouseY = e.m_y;
}

void ToolRoadSelect::OnLeftPressed(wxMouseEvent &e)
{
	WorldRoad *c;
	if(mWorldFrame->pickRoad(e, c))
		mWorldFrame->selectRoad(c);
	else
		mWorldFrame->selectRoad(0);

	mWorldFrame->update();
}
