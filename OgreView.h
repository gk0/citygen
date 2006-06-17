#ifndef OgreView_H
#define OgreView_H

// Includes //
#include "stdafx.h"

using namespace Ogre;

enum OgreViewMode {
	normal,
	node,
	edge
};

// The OGRE test application class. //
class OgreView : public wxWindow
{
	DECLARE_EVENT_TABLE()

private:
	RenderSystem *mRSys;
	RenderWindow *mOgreRenderWindow;
	SceneManager *mScene;

	long mMouseX, mMouseY;

	Camera *mCamera;
	Viewport *mViewport;

	bool mReady;

	RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	SceneNode *mCurrentObject;         // The newly created object
	int mCount;                        // The number of robots on the screen

	OgreViewMode mViewMode;

protected: 
    void chooseSceneManager(void);
    void createCamera(void);
    void createFrameListener(void);
    void createScene(void);
    void destroyScene(void);
    void createViewports(void);

public:
	OgreView(wxWindow* parent, const wxPoint &pos, const wxSize &size);
	~OgreView();

	void Resize(long w, long h);
	void Update();
	void Init(HWND handle);

	void Tick(Real t);

	void RotateView(Real yaw, Real pitch);
	void MoveView(Real x, Real y, Real z);

	void OnLeftPressed(wxMouseEvent &e);
	void OnLeftDragged(wxMouseEvent &e);

	void OnSize(wxSizeEvent &e);
	void OnPaint(wxPaintEvent &WXUNUSED(e));
	void OnMouse(wxMouseEvent &e);

	void OnSetFocus(wxFocusEvent& e);
	void OnLostFocus(wxFocusEvent& e);
	void OnTimer(wxTimerEvent &e);

	void OnEraseBackground(wxEraseEvent &e);

	void OgreView::AddNode(float x, float y);
	void DeleteSelectedNode();

	void SetMode(OgreViewMode mode);
	OgreViewMode GetMode() { return mViewMode; }

};

#endif
