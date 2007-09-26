#include "stdafx.h"
#include "ToolNodeDelete.h"

#define DELETENODESNAPSQ 16

ToolNodeDelete::ToolNodeDelete(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolNodeDelete::OnLeftPressed(wxMouseEvent &e)
{
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
	WorldNode *wn;
	if(_worldFrame->pickNode(e, DELETENODESNAPSQ, wn)) _worldFrame->highlightNode(wn);
	else _worldFrame->highlightNode(0);
	_worldFrame->update();
}