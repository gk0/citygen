#include "stdafx.h"
#include "ToolCellSelect.h"
#include "WorldCell.h"

ToolCellSelect::ToolCellSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolCellSelect::OnMouseMove(wxMouseEvent &e)
{
	ToolView::OnMouseMove(e);

	// Compute deltas
	mMouseDeltaX = e.m_x - mMouseX;
	mMouseDeltaY = e.m_y - mMouseY;
/*
	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldCell *wn;

	// if left drag
	if(mWorldFrame->pickCell(e, HIGHLIGHTNODESNAPSQ, c))
		mWorldFrame->highlightCell(c);
*/
	// save for calc of next deltas
	mMouseX = e.m_x;
	mMouseY = e.m_y;
}

void ToolCellSelect::OnLeftPressed(wxMouseEvent &e)
{
	WorldCell *c;
	if(mWorldFrame->pickCell(e, c))
		mWorldFrame->selectCell(c);
	else
		mWorldFrame->selectCell(0);

	mWorldFrame->update();
}
