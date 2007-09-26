#ifndef TOOLVIEW_H
#define TOOLVIEW_H

#include "stdafx.h"
#include "WorldFrame.h"

class ToolView : public Tool
{
protected:
	enum Mode {
		none,
		rotate,
		translate,
		zoom
	} _mode;
	WorldFrame* _worldFrame;
	Ogre::Real _moveSpeed;

	long _mouseX, _mouseY;
	long _mouseDeltaX, _mouseDeltaY;
	Ogre::Vector3 _cameraTarget;
	Ogre::Vector3 _lastTranslateVec;

	wxCursor	_rotateCursor, _translateCursor, _zoomCursor;

public:
	ToolView(WorldFrame* wf);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnLeftReleased(wxMouseEvent &e);
	void OnMiddlePressed(wxMouseEvent &e);
	void OnMiddleReleased(wxMouseEvent &e);
	void OnRightPressed(wxMouseEvent &e);
	void OnRightReleased(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);

	void setNone();
	void setRotate();
	void setTranslate();
	void setZoom();

	Ogre::Vector3 toVec(long mx, long my)
	{
		// create camera ray
		float mouseX = float(1.0f/ _worldFrame->getViewport()->getActualWidth()) * mx;
		float mouseY = float(1.0f/_worldFrame->getViewport()->getActualHeight()) * my;
		Ogre::Ray mouseRay =  _worldFrame->getCamera()->getCameraToViewportRay(mouseX, mouseY);

		Ogre::Vector3 camPlanePoint(_worldFrame->getCameraNode()->getPosition());
		Ogre::Vector3 camPlaneNormal(_worldFrame->getCamera()->getRealDirection());

		Ogre::Plane camPlane(camPlaneNormal, camPlanePoint);
		Ogre::Real rayDist;
		bool b;
		boost::tie(b, rayDist) = mouseRay.intersects(camPlane);
		return mouseRay.getPoint(rayDist);
	}

};

#endif
