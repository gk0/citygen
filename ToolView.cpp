#include "stdafx.h"
#include "ToolView.h"

using namespace Ogre;

ToolView::ToolView(WorldFrame* wf)
{
	_worldFrame = wf;
	_moveSpeed = 4.0;
}

void ToolView::activate()
{
	_mouseX = 0;
	_mouseY = 0;
	_mouseDeltaX = 0;
	_mouseDeltaY = 0;
}

void ToolView::deactivate()
{
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
		_worldFrame->cameraMove(0.0f, _moveSpeed, 0.0f);
		_worldFrame->update();
		break;
	case WXK_PAGEDOWN:
	case WXK_NUMPAD_PAGEDOWN:
		_worldFrame->cameraMove(0.0f, -_moveSpeed, 0.0f);
		_worldFrame->update();
		break;
	case 'w':
	case 'W':
	case WXK_UP:
	case WXK_NUMPAD_UP:
		_worldFrame->cameraMove(0.0f, 0.0f, -_moveSpeed);
		_worldFrame->update();
		break;
	case 's':
	case 'S':
	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		_worldFrame->cameraMove(0.0f, 0.0f, _moveSpeed);
		_worldFrame->update();
		break;
	case 'a':
	case 'A':
	case WXK_LEFT:
	case WXK_NUMPAD_LEFT:
		_worldFrame->cameraMove(-_moveSpeed, 0.0f, 0.0f);
		_worldFrame->update();
		break;
	case 'd':
	case 'D':
	case WXK_RIGHT:
	case WXK_NUMPAD_RIGHT:
		_worldFrame->cameraMove(_moveSpeed, 0.0f, 0.0f);
		_worldFrame->update();
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

	// 
	if(e.m_leftDown)
	{
		if(e.m_rightDown)
			_worldFrame->cameraMove(0.0f, 0.0f, _mouseDeltaY * (_moveSpeed / 4));
		else
			_worldFrame->cameraRotate(_mouseDeltaX*2, _mouseDeltaY);
		_worldFrame->update();
	}
	else if(e.m_rightDown)
	{
		_worldFrame->cameraMove((Ogre::Real)(-_mouseDeltaX) * (_moveSpeed / 4), (Ogre::Real)_mouseDeltaY * (_moveSpeed / 4), 0.0f);
		_worldFrame->update();
	}

	// save for calc of next deltas
	_mouseX = e.m_x;
	_mouseY = e.m_y;
}

void ToolView::OnLeftPressed(wxMouseEvent &e)
{
}

void ToolView::OnMouseWheel(wxMouseEvent &e)
{
	if(e.GetWheelRotation()!=0)
	{
		_worldFrame->cameraMove(0, 0, (e.GetWheelRotation() / 40) * _moveSpeed);
		_worldFrame->update();
	}
}
