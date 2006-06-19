#ifndef _OGREVIEW_H_
#define _OGREVIEW_H_

#include "stdafx.h"


class OgreView : public wxControl
{
	DECLARE_CLASS(OgreView)
protected:
	/* WX members */
	wxTimer	mTimer;

	/* Ogre members */
	Ogre::Camera* mCamera;
	Ogre::RenderWindow* mRenderWindow;
	Ogre::SceneManager* mSceneMgr;
	Ogre::Viewport* mViewport;

	long mMouseX, mMouseY;
	
	void OnEraseBackground(wxEraseEvent &e);
	void OnLeftDragged(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnFocusLost(wxFocusEvent& e);	
	void OnFocusSet(wxFocusEvent& e);
	void OnMouse(wxMouseEvent &e);
	void OnPaint(wxPaintEvent &WXUNUSED(e));
	void OnSize(wxSizeEvent &e);
	void OnTimer(wxTimerEvent &e);

protected:
	DECLARE_EVENT_TABLE()
	 
    void chooseSceneManager(void);
    void createCamera(void);
    void createScene(void);
    void createViewports(void);
    void destroyScene(void);

public:
	OgreView(wxFrame* parent);
	virtual ~OgreView();

	void cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void cameraRotate(Ogre::Real yaw, Ogre::Real pitch);
	void update();
};

#endif
