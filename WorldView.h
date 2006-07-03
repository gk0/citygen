#ifndef _WORLDVIEW_H_
#define _WORLDVIEW_H_

#include "stdafx.h"
#include "OgreView.h"

enum WorldMode {
	normal,
	node,
	edge
};

enum WorldSelectMode {
	sel,
	add,
	del
};

class WorldView : public OgreView
{

protected:
	Ogre::RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	Ogre::SceneNode *mCurrentObject;         // The newly created object
	int mCount;                        // The number of robots on the screen

	WorldMode mMode;
	WorldSelectMode mSelectMode;
	
	void OnLeftDragged(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnLostFocus(wxFocusEvent& e);	
	void OnMouse(wxMouseEvent &e);
	void OnSetFocus(wxFocusEvent& e);

protected:	
	DECLARE_EVENT_TABLE() 
    //void chooseSceneManager(void);
    //void createCamera(void);
    //void createScene(void);
    //void createViewports(void);
    //void destroyScene(void);

public:
	WorldView(wxFrame* parent);
	virtual ~WorldView();
	
	void addNode(float x, float y);
	void deleteSelectedNode();
	WorldMode getMode() { return mMode; }
	//void update();
	void setMode(WorldMode mode);
	void setSelectMode(WorldSelectMode mode);

};

#endif
