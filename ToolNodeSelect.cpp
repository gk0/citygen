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
	mMouseDeltaX = e.m_x - mMouseX;
	mMouseDeltaY = e.m_y - mMouseY;

	//TODO: 
	#define HIGHLIGHTNODESNAPSQ 16
	WorldNode *wn;

	// if left drag
	if(e.m_leftDown)
	{
		Ogre::Vector3 pos;
		if(mWorldFrame->pickTerrainIntersection(e, pos))
			mWorldFrame->moveSelectedNode(pos);
	}
	else if(mWorldFrame->pickNode(e, HIGHLIGHTNODESNAPSQ, wn))
		mWorldFrame->highlightNode(wn);

	// save for calc of next deltas
	mMouseX = e.m_x;
	mMouseY = e.m_y;
}

void ToolNodeSelect::OnLeftPressed(wxMouseEvent &e)
{
	//TODO: 
	#define SELECTNODESNAPSQ 16

	WorldNode *wn;
	if(mWorldFrame->pickNode(e, SELECTNODESNAPSQ, wn))
		mWorldFrame->selectNode(wn);
	else
		mWorldFrame->selectNode(0);

	mWorldFrame->update();
}

