#ifndef WorldWindow_H
#define WorldWindow_H

#include "stdafx.h"
#include "wxOgre.h"
#include "EditModeListener.h"
#include "SelectModeListener.h"


class WorldWindow : public wxOgre, EditModeListener, SelectModeListener
{
public:
    wxView *view;
    
    WorldWindow(wxView *v, wxFrame *frame);

	virtual void OnDraw(wxDC& dc);
	void Refresh();
	void clear();
	void prepare();
    
    DECLARE_EVENT_TABLE()

protected:
	Ogre::RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	Ogre::SceneNode *mCurrentNode;         // The newly created object

	Ogre::SceneNode *mRoadNode;         // The newly created object

	Ogre::SceneNode *mManualObject;

	std::vector<Ogre::SceneNode*> mNodes;
	std::vector<Ogre::SceneNode*> mRoads;

	int mNodeCount, mRoadCount;                        // The number of robots on the screen

	EditModeListener::EditMode mEditMode;
	SelectModeListener::SelectMode mSelectMode;
	
	void OnLeftDragged(wxMouseEvent &e);
	void OnSelectNode(wxMouseEvent &e);
	void OnSelectRoad(wxMouseEvent &e);
	void OnLostFocus(wxFocusEvent& e);	
	void OnMouse(wxMouseEvent &e);
	void OnSetFocus(wxFocusEvent& e);

	void createNode(const Ogre::Vector3& v);
	void createRoad(const Ogre::Vector3& u, const Ogre::Vector3& v, const std::string& name); 

protected:	
    //void chooseSceneManager(void);
    //void createCamera(void);
    //void createScene(void);
    //void createViewports(void);
    //void destroyScene(void);


public:

	
	void moveNode(float x, float y);
	//void update();

	void setEditMode(EditModeListener::EditMode mode);
	void setSelectMode(SelectModeListener::SelectMode mode);

};


#endif

