#include "stdafx.h"
#include "ToolNodeDelete.h"

ToolNodeDelete::ToolNodeDelete(WorldFrame* wf)
: ToolView(wf)
{
}

void ToolNodeDelete::OnLeftPressed(wxMouseEvent &e)
{
	//TODO: 
	#define DELETENODESNAPSQ 16

	WorldNode *wn;
	if(_worldFrame->pickNode(e, DELETENODESNAPSQ, wn))
	{
		_worldFrame->deleteNode(wn);
		_worldFrame->update();
	}
}
