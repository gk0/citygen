#ifndef wxOgre_H
#define wxOgre_H

#include "stdafx.h"


class wxOgre : public wxControl
{
	DECLARE_CLASS(wxOgre)
protected:
	/* WX members */
	wxTimer	mTimer;

	/* Ogre members */
	Ogre::Camera* mCamera;
	Ogre::RenderWindow* mRenderWindow;
	Ogre::SceneManager* mSceneMgr;
	Ogre::Viewport* mViewport;

	long mMouseX, mMouseY;
	
	virtual void OnEraseBackground(wxEraseEvent &e);
	virtual void OnLeftDragged(wxMouseEvent &e);
	virtual void OnLeftPressed(wxMouseEvent &e);
	virtual void OnFocusLost(wxFocusEvent& e);	
	virtual void OnFocusSet(wxFocusEvent& e);
	virtual void OnMouse(wxMouseEvent &e);
	virtual void OnPaint(wxPaintEvent &WXUNUSED(e));
	virtual void OnSize(wxSizeEvent &e);
	virtual void OnTimer(wxTimerEvent &e);

protected:
	DECLARE_EVENT_TABLE()
	 
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createScene(void);
    virtual void createViewports(void);
    void destroyScene(void);
	void destroyViewport();
	void destroySceneManager();
	void destroyCamera();

public:
	wxOgre(wxFrame* parent);
	virtual ~wxOgre();

	void Init();
	void UnInit();

	void cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void cameraRotate(Ogre::Real yaw, Ogre::Real pitch);
	void toggleTimerRendering();
	void Update();
};

#endif
