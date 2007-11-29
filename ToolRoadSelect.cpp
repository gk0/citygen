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
}

void ToolRoadSelect::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		ToolView::OnLeftPressed(e);
		return;
	}
	WorldRoad *c;
	if(_worldFrame->pickRoad(e, c))
		_worldFrame->selectRoad(c);
	else
		_worldFrame->selectRoad(0);

	_worldFrame->update();
}

bool ToolRoadSelect::alternate(wxMouseEvent &e)
{
	if(e.ControlDown()) return true;
	else return false;
}


