#include "stdafx.h"
#include "ToolRoadDelete.h"
#include "WorldFrame.h"

ToolRoadDelete::ToolRoadDelete(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolRoadDelete::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true)
		ToolView::OnMouseMove(e);
}

void ToolRoadDelete::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		ToolView::OnLeftPressed(e);
		return;
	}
	WorldRoad *wr;
	if(_worldFrame->pickRoad(e, wr))
	{
		_worldFrame->deleteRoad(wr);
		_worldFrame->Refresh();
	}
}

bool ToolRoadDelete::alternate(wxMouseEvent &e)
{
	if(e.ControlDown()) return true;
	else return false;
}
