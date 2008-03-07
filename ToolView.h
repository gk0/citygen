#ifndef TOOLVIEW_H
#define TOOLVIEW_H

#include "stdafx.h"
#include "Tool.h"
#include <OgreRay.h>
#include <OgreVector3.h>

class WorldFrame;

class ToolView : public Tool
{
protected:
	enum Mode {
		world,
		fps,
		author
	} _mode;

	enum Operation {
		none,
		rotate,
		translate,
		zoom
	};
	Operation _leftButtonOp;
	Operation _middleButtonOp;
	Operation _rightButtonOp;
	Operation _activeOp;

	WorldFrame* _worldFrame;
	Ogre::Real _moveSpeed;

	long _mouseX, _mouseY;
	long _mouseDeltaX, _mouseDeltaY;
	Ogre::Vector3 _cameraTarget;
	Ogre::Vector3 _lastTranslateVec;

	wxCursor	_rotateCursor, _translateCursor, _zoomCursor;
	
	Ogre::Vector3 toVec(long mx, long my);
	virtual bool alternate(wxMouseEvent &e);
	
public:
	ToolView(WorldFrame* wf);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnFocusLost(wxFocusEvent& e);
	void OnFocusSet(wxFocusEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnLeftReleased(wxMouseEvent &e);
	void OnMiddlePressed(wxMouseEvent &e);
	void OnMiddleReleased(wxMouseEvent &e);
	void OnRightPressed(wxMouseEvent &e);
	void OnRightReleased(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);

	void setOperation(Operation op);
};

#endif
