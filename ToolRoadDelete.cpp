#include "stdafx.h"
#include "ToolRoadDelete.h"

ToolRoadDelete::ToolRoadDelete(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolRoadDelete::OnMouseMove(wxMouseEvent &e)
{
	ToolView::OnMouseMove(e);
}

void ToolRoadDelete::OnLeftPressed(wxMouseEvent &e)
{
	WorldRoad *wr;
	if(_worldFrame->pickRoad(e, wr))
	{
		_worldFrame->deleteRoad(wr);
		_worldFrame->update();
	}
}
