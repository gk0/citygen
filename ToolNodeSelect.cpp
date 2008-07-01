#include "stdafx.h"
#include "ToolNodeSelect.h"
#include "WorldNode.h"
#include "WorldFrame.h"

#define NODESELECTSNAPSZSQ 2500


ToolNodeSelect::ToolNodeSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolNodeSelect::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true) 
	{
		ToolView::OnMouseMove(e);
		return;
	}
	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;

	WorldNode *wn;
	Ogre::Vector3 pos;

	// if left drag
	if(e.m_leftDown)
	{
		if(_worldFrame->pickTerrainIntersection(e, pos))
		{
			_worldFrame->moveSelectedNode(pos);
			_worldFrame->update();
		}
	}
	else if(_worldFrame->pickNode(e, NODESELECTSNAPSZSQ, wn))
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

	//TODO: take a good look at the number of update event we're causing 
	// and see if WM_PAINT or some event shit can reduce this

	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolNodeSelect::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true) 
	{
		ToolView::OnLeftPressed(e);
		return;
	}

	WorldNode *wn;
	if(_worldFrame->pickNode(e, NODESELECTSNAPSZSQ, wn))
		_worldFrame->selectNode(wn);
	else
		_worldFrame->selectNode(0);

	//_worldFrame->Refresh();
	_worldFrame->update();
}

bool ToolNodeSelect::alternate(wxMouseEvent &e)
{
	if(e.ControlDown())
	{
		_worldFrame->highlightNode(0);
		return true;
	}
	else return false;
}

void ToolNodeSelect::OnMiddlePressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnMiddlePressed(e);
}

void ToolNodeSelect::OnRightPressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnRightPressed(e);
}
