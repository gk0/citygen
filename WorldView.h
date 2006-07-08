#ifndef _WORLDVIEW_H_
#define _WORLDVIEW_H_

#include "stdafx.h"
#include "OgreView.h"
#include "EditModeListener.h"
#include "SelectModeListener.h"


class WorldView : public OgreView, EditModeListener, SelectModeListener
{

protected:
	Ogre::RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	Ogre::SceneNode *mCurrentNode;         // The newly created object

	Ogre::SceneNode *mRoadNode;         // The newly created object

	int mNodeCount, mRoadCount;                        // The number of robots on the screen

	EditModeListener::EditMode mEditMode;
	SelectModeListener::SelectMode mSelectMode;
	
	void onLeftDragged(wxMouseEvent &e);
	void onSelectNode(wxMouseEvent &e);
	void onSelectRoad(wxMouseEvent &e);
	void onLostFocus(wxFocusEvent& e);	
	void onMouse(wxMouseEvent &e);
	void onSetFocus(wxFocusEvent& e);

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
	
	void moveNode(float x, float y);
	//void update();

	void setEditMode(EditModeListener::EditMode mode);
	void setSelectMode(SelectModeListener::SelectMode mode);

};

#endif
