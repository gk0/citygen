#include "stdafx.h"
#include "ToolCellSelect.h"
#include "WorldCell.h"

using namespace Ogre;

ToolCellSelect::ToolCellSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolCellSelect::OnChar(wxKeyEvent &e)
{
	int key = e.GetKeyCode();
	WorldCell* wc = _worldFrame->getSelectedCell();
	switch(key)
	{
	// cell decomposition was here
	default:
		ToolView::OnChar(e);
		break;
	}
	_worldFrame->update();
}

void ToolCellSelect::OnMouseMove(wxMouseEvent &e)
{
	ToolView::OnMouseMove(e);

	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;
/*
	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldCell *wn;

	// if left drag
	if(mWorldFrame->pickCell(e, HIGHLIGHTNODESNAPSQ, c))
		mWorldFrame->highlightCell(c);
*/
	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolCellSelect::OnLeftPressed(wxMouseEvent &e)
{
	WorldCell *c;
	if(_worldFrame->pickCell(e, c))
		_worldFrame->selectCell(c);
	else
		_worldFrame->selectCell(0); 

	_worldFrame->update();
}
