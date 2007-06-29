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
	WorldCell* wc = mWorldFrame->getSelectedCell();
	switch(key)
	{
	case 'U':
	case 'u':
		if(wc)
		{
			LogManager::getSingleton().logMessage("Begin Cell Decomposition.");
			wc->beginGraphicalCellDecomposition();
		}
		break;
	case 'I':
	case 'i':
		if(wc)
		{
			LogManager::getSingleton().logMessage("Step Forward.");
			wc->stepGraphicalCellDecomposition();
		}
		break;
	case 'O':
	case 'o':
		LogManager::getSingleton().logMessage("Step Back.");
		// call to road graph
		break;
	default:
		ToolView::OnChar(e);
		break;
	}
	mWorldFrame->update();
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
