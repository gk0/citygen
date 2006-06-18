#ifndef _WORLDVIEW_H_
#define _WORLDVIEW_H_

#include "stdafx.h"
#include "OgreView.h"

enum WorldViewMode {
	normal,
	node,
	edge
};

class WorldView : public OgreView
{

protected:
	Ogre::RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	Ogre::SceneNode *mCurrentObject;         // The newly created object
	int mCount;                        // The number of robots on the screen

	WorldViewMode mViewMode;
	
	void onLeftDragged(wxMouseEvent &e);
	void onLeftPressed(wxMouseEvent &e);
	void onLostFocus(wxFocusEvent& e);	
	void onMouse(wxMouseEvent &e);
	void onSetFocus(wxFocusEvent& e);

protected:	 
    //void chooseSceneManager(void);
    //void createCamera(void);
    //void createScene(void);
    //void createViewports(void);
    //void destroyScene(void);

public:
	WorldView(wxFrame* parent);
	~WorldView();
	
	void addNode(float x, float y);
	void deleteSelectedNode();
	WorldViewMode getMode() { return mViewMode; }
	//void update();
	void setMode(WorldViewMode mode) { mViewMode = mode; }

};

#endif
