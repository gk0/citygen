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
	_mode = world;

#ifdef __WXMSW__
	_rotateCursor = wxCursor(_("rotato"));
	_translateCursor = wxCursor(_("translato"));
	_zoomCursor = wxCursor(_("zoomo"));
#else
	//wxBITMAP_TYPE_XBM
#endif

	_leftButtonOp = translate;
	_middleButtonOp = zoom;
	_rightButtonOp = rotate;
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

void ToolView::OnFocusLost(wxFocusEvent& e)
{
}

void ToolView::OnFocusSet(wxFocusEvent& e)
{
}

Vector3 shit;

void ToolView::OnMouseMove(wxMouseEvent &e)
{
	// Compute deltas
	_mouseDeltaX = e.m_x - _mouseX;
	_mouseDeltaY = e.m_y - _mouseY;

	if(e.LeftIsDown() && e.RightIsDown() || e.MiddleIsDown()) setOperation(_middleButtonOp);
	else if(e.LeftIsDown()) setOperation(_leftButtonOp);
	else if(e.RightIsDown())  setOperation(_rightButtonOp);
	else setOperation(none);

	switch(_mode)
	{
	case world:
		{
			switch(_activeOp)
			{
			case none:
				break;
			case rotate:
				{
					int w,h;
					_worldFrame->GetSize(&w,&h);
					Real wReal(w), hReal(h);
					_worldFrame->cameraNodeRotate(-_mouseDeltaX/wReal * 180, -_mouseDeltaY/hReal * 90);
					break;
				}
			case translate:
				{
					if(_lastTranslateVec == Vector3::ZERO)
					{
						shit = Vector3::ZERO;
						if(!_worldFrame->pickTerrainIntersection(e, _lastTranslateVec)) break;
					}
					Vector3 currentTranslateVec;
					if(!_worldFrame->pickTerrainIntersection(e, currentTranslateVec)) break;
					Vector3 translateDelta =  _lastTranslateVec - currentTranslateVec;
					_lastTranslateVec = currentTranslateVec;
					translateDelta = (translateDelta + shit);
					shit = translateDelta;

					_worldFrame->cameraNodeMove(translateDelta.x, translateDelta.z);
					break;
				}
			case zoom:
				{
					Real camDist = _worldFrame->getCamera()->getPosition().z;
					_worldFrame->cameraZoom(-(_mouseDeltaY + _mouseDeltaX) * camDist * _moveSpeed / 400);
					break;
				}
			}
			break;
		}
	case fps:
		break;
	case author:
		break;
	}

	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolView::OnLeftPressed(wxMouseEvent &e)
{
	//Vector3 tmp;
	//bool b = _worldFrame->pickAnyIntersection(e, tmp);
	//LogManager::getSingleton().logMessage(StringConverter::toString(b)+":"+StringConverter::toString(tmp));
	if(e.RightIsDown()) setOperation(_middleButtonOp);
	else setOperation(_leftButtonOp);
}

void ToolView::OnLeftReleased(wxMouseEvent &e)
{
	if(e.RightIsDown()) setOperation(_rightButtonOp);
	else if(e.MiddleIsDown()) setOperation(_middleButtonOp);
	else setOperation(none);
}

void ToolView::OnMiddlePressed(wxMouseEvent &e)
{
	setOperation(_middleButtonOp);
}

void ToolView::OnMiddleReleased(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setOperation(_leftButtonOp);
	else if(e.RightIsDown()) setOperation(_rightButtonOp);
	else setOperation(none);
}

void ToolView::OnRightPressed(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setOperation(_leftButtonOp);
	else setOperation(_rightButtonOp);
}

void ToolView::OnRightReleased(wxMouseEvent &e)
{
	if(e.LeftIsDown()) setOperation(_leftButtonOp);
	else if(e.MiddleIsDown()) setOperation(_middleButtonOp);
	else setOperation(none);
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

void ToolView::setOperation(Operation op)
{
	if(op == _activeOp) return;
	switch(op)
	{
	case none:
		_worldFrame->SetCursor(wxCURSOR_ARROW);
		break;
	case translate:
		_lastTranslateVec = Vector3::ZERO;
		_worldFrame->SetCursor(_translateCursor);
		break;
	case rotate:
		_worldFrame->SetCursor(_rotateCursor);
		break;
	case zoom:
		_worldFrame->SetCursor(_zoomCursor);
		break;
	}
	_activeOp = op;
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

