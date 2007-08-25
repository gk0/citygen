#include "stdafx.h"
#include "ToolNodeSelect.h"
#include "WorldNode.h"

ToolNodeSelect::ToolNodeSelect(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolNodeSelect::OnMouseMove(wxMouseEvent &e)
{
	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;

	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldNode *wn;

	// if left drag
	if(e.m_leftDown)
	{
		Ogre::Vector3 pos;
		if(_worldFrame->pickTerrainIntersection(e, pos))
			_worldFrame->moveSelectedNode(pos);
	}
	else if(e.m_rightDown)
	{
		_worldFrame->cameraMove((Ogre::Real)(-_mouseDeltaX) * (_moveSpeed / 4), (Ogre::Real)_mouseDeltaY * (_moveSpeed / 4), 0.0f);
		_worldFrame->update();
	}
	else if(_worldFrame->pickNode(e, HIGHLIGHTNODESNAPSQ, wn))
		_worldFrame->highlightNode(wn);

	if(!_worldFrame->getSelected()) ToolView::OnMouseMove(e);

	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolNodeSelect::OnLeftPressed(wxMouseEvent &e)
{
	//TODO: 
	#define SELECTNODESNAPSQ 16

	WorldNode *wn;
	if(_worldFrame->pickNode(e, SELECTNODESNAPSQ, wn))
		_worldFrame->selectNode(wn);
	else
		_worldFrame->selectNode(0);

	_worldFrame->update();
}

