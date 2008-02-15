#include "stdafx.h"
#include "ToolView.h"
#include "WorldFrame.h"

#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreRay.h>


using namespace Ogre;

ToolView::ToolView(WorldFrame* wf)
{
	_worldFrame = wf;
	_moveSpeed = 1.0;
	_cameraTarget = Vector3(1026.25f, 0.1f, 946.25f);

#ifdef __WXMSW__
	_rotateCursor = wxCursor(_("rotato"));
	_translateCursor = wxCursor(_("translato"));
	_zoomCursor = wxCursor(_("zoomo"));
#else
	//wxBITMAP_TYPE_XBM
#endif
}

void ToolView::activate()
{
	_mouseX = 0;
	_mouseY = 0;
	_mouseDeltaX = 0;
	_mouseDeltaY = 0;
	_worldFrame->SetCursor(wxCURSOR_ARROW);
}

void ToolView::deactivate()
{
	_worldFrame->SetCursor(wxCURSOR_ARROW);
}

void ToolView::OnChar(wxKeyEvent& e)
{
	int key = e.GetKeyCode();
	switch(key)
	{
	case '+':
	case '=':
		_moveSpeed += 0.1f;
		LogManager::getSingleton().logMessage("Move Speed: "+StringConverter::toString(_moveSpeed), LML_CRITICAL);
		break;
	case '_':
	case '-':
		_moveSpeed -= 0.1f;
		LogManager::getSingleton().logMessage("Move Speed: "+StringConverter::toString(_moveSpeed), LML_CRITICAL);
		break;
	case WXK_PAGEUP:
	case WXK_NUMPAD_PAGEUP:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(0.0f,_moveSpeed,0.0f));
		break;
	case WXK_PAGEDOWN:
	case WXK_NUMPAD_PAGEDOWN:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(0.0f,-_moveSpeed,0.0f));
		break;
	case 'w':
	case 'W':
	case WXK_UP:
	case WXK_NUMPAD_UP:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(0.0f, 0.0f, -_moveSpeed));
		break;
	case 's':
	case 'S':
	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(0.0f, 0.0f, _moveSpeed));
		break;
	case 'a':
	case 'A':
	case WXK_LEFT:
	case WXK_NUMPAD_LEFT:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(-_moveSpeed, 0.0f, 0.0f));
		break;
	case 'd':
	case 'D':
	case WXK_RIGHT:
	case WXK_NUMPAD_RIGHT:
		_worldFrame->cameraNodeMove(_worldFrame->getCameraNode()->getPosition() +
			_worldFrame->getCameraNode()->getOrientation() * Vector3(_moveSpeed, 0.0f, 0.0f));
		break;

	default:
		e.Skip();
		break;
	}
}


void ToolView::OnMouseMove(wxMouseEvent &e)
{
	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;

	if(e.LeftIsDown() && e.RightIsDown() || e.MiddleIsDown()) setTranslate(); 
	else if(e.LeftIsDown()) setRotate();
	else if(e.RightIsDown()) setZoom();
	else setNone();

	switch(_mode)
	{
	case none:
		break;
	case rotate:
		//FPS: 
		//_worldFrame->cameraRotate(_mouseDeltaX*2, _mouseDeltaY);

		//Editor
		_worldFrame->cameraNodeRotate(-_mouseDeltaX/2.0f, -_mouseDeltaY/2.0f);
		break;
	case translate:
		{
			//FPS: 
			//_worldFrame->cameraMove((Ogre::Real)(-_mouseDeltaX) * (_moveSpeed / 4), (Ogre::Real)_mouseDeltaY * (_moveSpeed / 4), 0.0f);

			//Editor
			// create camera ray
			if(_lastTranslateVec == Vector3::ZERO) _lastTranslateVec = toVec(_mouseX, _mouseY);
			Vector3 currentTranslateVec(toVec(e.m_x, e.m_y));
			Vector3 translateDelta =  _lastTranslateVec - currentTranslateVec;
			_lastTranslateVec = currentTranslateVec;
			_worldFrame->cameraNodeMove(translateDelta.x, translateDelta.y, translateDelta.z);
			break;
		}
	case zoom:
		{
			Real camDist = _worldFrame->getCamera()->getPosition().z;
			_worldFrame->cameraZoom(-(_mouseDeltaY + _mouseDeltaX) * camDist * _moveSpeed / 400);
			break;
		}
	}

	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolView::OnLeftPressed(wxMouseEvent &e)
{
	if(e.RightIsDown()) setTranslate();
	else setRotate();
}

void ToolView::OnLeftReleased(wxMouseEvent &e)
{
	if(e.RightIsDown()) setZoom();
	else if(e.MiddleIsDown()) setTranslate(); 
	else setNone();
}

void ToolView::OnMiddlePressed(wxMouseEvent &e)
{
	setTranslate();
}

void ToolView::OnMiddleReleased(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setRotate();
	else if(e.RightIsDown()) setZoom(); 
	else setNone();
}

void ToolView::OnRightPressed(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setTranslate();
	else setZoom();
}

void ToolView::OnRightReleased(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setRotate();
	else if(e.MiddleIsDown()) setTranslate(); 
	else setNone();
}

void ToolView::OnMouseWheel(wxMouseEvent &e)
{
	if(e.GetWheelRotation()!=0)
	{
		Real camDist = _worldFrame->getCamera()->getPosition().z;
		_worldFrame->cameraMove(0.0f, 0.0f, -e.GetWheelRotation() * camDist * _moveSpeed / 1200);
		_worldFrame->Refresh();
	}
}

void ToolView::setNone()
{
	_mode = none;
	_worldFrame->SetCursor(wxCURSOR_ARROW);
}

void ToolView::setRotate()
{
	_mode = rotate;
	_worldFrame->SetCursor(_rotateCursor);
}

void ToolView::setTranslate()
{
	_mode = translate;
	_lastTranslateVec = Vector3::ZERO;
	_worldFrame->SetCursor(_translateCursor);
}

void ToolView::setZoom()

{
	_mode = zoom;
	_worldFrame->SetCursor(_zoomCursor);
}

Ogre::Vector3 ToolView::toVec(long mx, long my)
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

bool ToolView::alternate(wxMouseEvent &e)
{
	if(e.ControlDown()) return true;
	else return false;
}
