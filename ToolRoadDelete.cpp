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

	WorldRoad *wr;
	if(_worldFrame->pickRoad(e, wr))
	{
		if(wr != _worldFrame->getHighlightedRoad())
		{
			_worldFrame->highlightRoad(wr);
			_worldFrame->update();
		}
	}
	else if(_worldFrame->getHighlightedRoad())
	{
		_worldFrame->highlightRoad(0);
		_worldFrame->update();
	}
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
