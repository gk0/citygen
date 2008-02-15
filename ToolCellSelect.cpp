#include "stdafx.h"
#include "ToolCellSelect.h"
#include "WorldCell.h"
#include "WorldFrame.h"

using namespace Ogre;

ToolCellSelect::ToolCellSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolCellSelect::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true)
		ToolView::OnMouseMove(e);

	WorldCell *wc;
	if(_worldFrame->pickCell(e, wc))
	{
		_worldFrame->highlightCell(wc);
		_worldFrame->update();
	}
	else if(_worldFrame->getHighlightedCell())
	{
		_worldFrame->highlightCell(0);
		_worldFrame->update();
	}
}

void ToolCellSelect::OnLeftPressed(wxMouseEvent &e)
{
	WorldCell *c;
	if(_worldFrame->pickCell(e, c))
		_worldFrame->selectCell(c);
	else
		_worldFrame->selectCell(0); 

	_worldFrame->Refresh();
}
