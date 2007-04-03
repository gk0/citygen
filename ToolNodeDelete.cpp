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
	if(mWorldFrame->pickNode(e, DELETENODESNAPSQ, wn))
	{
		mWorldFrame->deleteNode(wn);
		mWorldFrame->update();
	}
}
