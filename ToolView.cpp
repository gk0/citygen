#include "stdafx.h"
#include "ToolView.h"

using namespace Ogre;

ToolView::ToolView(WorldFrame* wf)
{
	mWorldFrame = wf;
	mMoveSpeed = 4.0;
}

void ToolView::activate()
{
	mMouseX = 0;
	mMouseY = 0;
	mMouseDeltaX = 0;
	mMouseDeltaY = 0;
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
		mMoveSpeed += 0.1f;
		LogManager::getSingleton().logMessage("Move Speed: "+StringConverter::toString(mMoveSpeed), LML_CRITICAL);
		break;
	case '_':
	case '-':
		mMoveSpeed -= 0.1f;
		LogManager::getSingleton().logMessage("Move Speed: "+StringConverter::toString(mMoveSpeed), LML_CRITICAL);
		break;
	case WXK_PAGEUP:
	case WXK_NUMPAD_PAGEUP:
		mWorldFrame->cameraMove(0.0f, mMoveSpeed, 0.0f);
		mWorldFrame->update();
		break;
	case WXK_PAGEDOWN:
	case WXK_NUMPAD_PAGEDOWN:
		mWorldFrame->cameraMove(0.0f, -mMoveSpeed, 0.0f);
		mWorldFrame->update();
		break;
	case 'w':
	case 'W':
	case WXK_UP:
	case WXK_NUMPAD_UP:
		mWorldFrame->cameraMove(0.0f, 0.0f, -mMoveSpeed);
		mWorldFrame->update();
		break;
	case 's':
	case 'S':
	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		mWorldFrame->cameraMove(0.0f, 0.0f, mMoveSpeed);
		mWorldFrame->update();
		break;
	case 'a':
	case 'A':
	case WXK_LEFT:
	case WXK_NUMPAD_LEFT:
		mWorldFrame->cameraMove(-mMoveSpeed, 0.0f, 0.0f);
		mWorldFrame->update();
		break;
	case 'd':
	case 'D':
	case WXK_RIGHT:
	case WXK_NUMPAD_RIGHT:
		mWorldFrame->cameraMove(mMoveSpeed, 0.0f, 0.0f);
		mWorldFrame->update();
		break;
	default:
		e.Skip();
		break;
	}
}


void ToolView::OnMouseMove(wxMouseEvent &e)
{
	// Compute deltas
	mMouseDeltaX = e.m_x - mMouseX;
	mMouseDeltaY = e.m_y - mMouseY;

	// 
	if(e.m_leftDown)
	{
		if(e.m_rightDown)
			mWorldFrame->cameraMove(0.0f, 0.0f, mMouseDeltaY * (mMoveSpeed / 4));
		else
			mWorldFrame->cameraRotate(mMouseDeltaX*2, mMouseDeltaY);
		mWorldFrame->update();
	}
	else if(e.m_rightDown)
	{
		mWorldFrame->cameraMove((Ogre::Real)(-mMouseDeltaX) * (mMoveSpeed / 4), (Ogre::Real)mMouseDeltaY * (mMoveSpeed / 4), 0.0f);
		mWorldFrame->update();
	}

	// save for calc of next deltas
	mMouseX = e.m_x;
	mMouseY = e.m_y;
}

void ToolView::OnLeftPressed(wxMouseEvent &e)
{
}

void ToolView::OnMouseWheel(wxMouseEvent &e)
{
	if(e.GetWheelRotation()!=0)
	{
		mWorldFrame->cameraMove(0, 0, (e.GetWheelRotation() / 40) * mMoveSpeed);
		mWorldFrame->update();
	}
}
