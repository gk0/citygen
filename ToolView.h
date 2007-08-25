#ifndef TOOLVIEW_H
#define TOOLVIEW_H

#include "stdafx.h"
#include "WorldFrame.h"

class ToolView : public Tool
{
protected:
	WorldFrame* _worldFrame;
	Ogre::Real _moveSpeed;

	long _mouseX, _mouseY;
	long _mouseDeltaX, _mouseDeltaY;

public:
	ToolView(WorldFrame* wf);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);
};

#endif
