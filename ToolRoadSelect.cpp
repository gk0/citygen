#include "stdafx.h"
#include "ToolRoadSelect.h"
#include "WorldRoad.h"
#include "WorldFrame.h"

ToolRoadSelect::ToolRoadSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolRoadSelect::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		ToolView::OnMouseMove(e);
		return;
	}
	WorldRoad *wr;
	if(_worldFrame->pickRoad(e, wr))
	{
		_worldFrame->highlightRoad(wr);
		_worldFrame->update();
	}
	else if(_worldFrame->getHighlightedRoad())
	{
		_worldFrame->highlightRoad(0);
		_worldFrame->update();
	}
}

void ToolRoadSelect::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		ToolView::OnLeftPressed(e);
		return;
	}
	WorldRoad *wr;
	if(_worldFrame->pickRoad(e, wr))
	{
		_worldFrame->selectRoad(wr);
		_worldFrame->update();
	}
	else
	{
		_worldFrame->selectRoad(0);
		_worldFrame->update();
	}
}

bool ToolRoadSelect::alternate(wxMouseEvent &e)
{
	if(e.ControlDown()) return true;
	else return false;
}


