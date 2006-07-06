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
	
	virtual void onEraseBackground(wxEraseEvent &e);
	virtual void onLeftDragged(wxMouseEvent &e);
	virtual void onLeftPressed(wxMouseEvent &e);
	virtual void onFocusLost(wxFocusEvent& e);	
	virtual void onFocusSet(wxFocusEvent& e);
	virtual void onMouse(wxMouseEvent &e);
	virtual void onPaint(wxPaintEvent &WXUNUSED(e));
	virtual void onSize(wxSizeEvent &e);
	virtual void onTimer(wxTimerEvent &e);

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
