#include "stdafx.h"
#include "ToolNodeDelete.h"
#include "WorldFrame.h"

#define DELETENODESNAPSQ 2500

ToolNodeDelete::ToolNodeDelete(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolNodeDelete::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true) 
	{
		ToolView::OnLeftPressed(e);
		return;
	}
	//TODO: 
	WorldNode *wn;
	if(_worldFrame->pickNode(e, DELETENODESNAPSQ, wn))
	{
		_worldFrame->deleteNode(wn);
		_worldFrame->update();
	}

	//_worldFrame->SetCursor(wxCURSOR_MAGNIFIER);
}

void ToolNodeDelete::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true) 
	{
		ToolView::OnMouseMove(e);
		return;
	}
	WorldNode *wn;
	if(_worldFrame->pickNode(e, DELETENODESNAPSQ, wn))
	{
		if(wn != _worldFrame->getHighlightedNode())
		{
			_worldFrame->highlightNode(wn);
			_worldFrame->update();
		}
	}
	else if(_worldFrame->getHighlightedNode())
	{
		_worldFrame->highlightNode(0);
		_worldFrame->update();
	}
}

bool ToolNodeDelete::alternate(wxMouseEvent &e)
{
	if(e.ControlDown())
	{
		_worldFrame->highlightNode(0);
		return true;
	}
	else return false;
}

void ToolNodeDelete::OnMiddlePressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnMiddlePressed(e);
}

void ToolNodeDelete::OnRightPressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnRightPressed(e);
}
