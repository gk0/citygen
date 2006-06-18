#ifndef _OGREVIEW_H_
#define _OGREVIEW_H_

#include "stdafx.h"


class OgreView : public wxControl
{
	DECLARE_CLASS(OgreView)
private:
	/* WX members */
	wxTimer	mTimer;

	/* Ogre members */
	Ogre::Camera* mCamera;
	Ogre::RenderWindow* mRenderWindow;
	Ogre::SceneManager* mSceneMgr;
	Ogre::Viewport* mViewport;

	long mMouseX, mMouseY;
	
	void onEraseBackground(wxEraseEvent &e);
	void onLeftDragged(wxMouseEvent &e);
	void onLeftPressed(wxMouseEvent &e);
	void onFocusLost(wxFocusEvent& e);	
	void onFocusSet(wxFocusEvent& e);
	void onMouse(wxMouseEvent &e);
	void onPaint(wxPaintEvent &WXUNUSED(e));
	void onSize(wxSizeEvent &e);
	void onTimer(wxTimerEvent &e);

protected:
	DECLARE_EVENT_TABLE()
	 
    void chooseSceneManager(void);
    void createCamera(void);
    void createScene(void);
    void createViewports(void);
    void destroyScene(void);

public:
	OgreView(wxFrame* parent);
	~OgreView();

	void cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void cameraRotate(Ogre::Real yaw, Ogre::Real pitch);
	void update();
};

#endif
