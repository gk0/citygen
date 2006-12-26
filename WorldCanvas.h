#ifndef WorldCanvas_H
#define WorldCanvas_H

#include "stdafx.h"
#include "wxOgre.h"
#include "EditModeListener.h"
#include "SelectModeListener.h"
#include "WorldDocument.h"
#include "WorldNode.h"
#include "NodePropertyPage.h"
#include "CellPropertyPage.h"
/*
typedef struct {
	WorldNode* worldNode;
	NodeDescriptor nodeDesc;
} NodePair;
*/

class NodePair{
public:
	NodePair() {}
	NodePair(WorldNode* wn, NodeDescriptor nd) { worldNode = wn; nodeDesc = nd; }
	WorldNode* worldNode;
	NodeDescriptor nodeDesc;
};


typedef std::map< Ogre::SceneNode*, NodePair > SceneNodeMap;
typedef std::map< Ogre::SceneNode*, RoadDescriptor > SceneRoadMap;
typedef std::map< Ogre::SceneNode*, std::pair<CityCell*, RoadDescriptor> > SceneInnerRoadMap;
typedef std::map< CityCell*, SceneNode*> CellSceneMap;

class WorldCanvas : public wxOgre, EditModeListener, SelectModeListener, public Ogre::RenderTargetListener
{
private:
	//These maps tie our doc and canvas data
	SceneNodeMap mSceneNodeMap;
	SceneRoadMap mSceneRoadMap;

	SceneInnerRoadMap mSceneInnerRoadMap;

	NodePropertyPage* mNodePropertyPage;
	CellPropertyPage* mCellPropertyPage;
	CityCell* mCityCell;

	//Ogre::SceneNode* mCityCellNode;
	long mMouseDeltaX, mMouseDeltaY;

	CellSceneMap mCellSceneMap;

	WorldNode* mCurrentWorldNode;
	WorldNode* mSelectedWorldNode;

public:
    wxView *view;
    
    WorldCanvas(wxView *v, wxFrame *frame, NodePropertyPage* nProp, CellPropertyPage* cProp);

	virtual void OnDraw(wxDC& dc);
	void Refresh();
	void clear();
	void prepare();

	void setNodeProperties(const string& l, const double& x, const double& y, const double& z);
	void setCellProperties(const int& seed, const float& segSz, const float& segDev, const int& degree,
							const float& degreeDev, const float& snapSz, const float& snapDev);
    
    DECLARE_EVENT_TABLE()

protected:
	Ogre::RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
	Ogre::RaySceneQueryResult result2;
	//Ogre::SceneNode *mCurrentNode;         // The newly created object

	Ogre::SceneNode *mRoadNode;         // The newly created object
	Ogre::Camera * mRTTCam;

	Ogre::SceneNode *mManualObject;

	Ogre::ColourValue fadeColour;

	int mNodeCount, mRoadCount;                        // The number of robots on the screen
	bool prepared;

	EditModeListener::EditMode mEditMode;
	SelectModeListener::SelectMode mSelectMode;
	
	void OnChar(wxKeyEvent& event);
	void OnLeftDragged(wxMouseEvent &e);
	void OnRightDragged(wxMouseEvent &e);

	void OnLostFocus(wxFocusEvent& e);

	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);

	void OnSetFocus(wxFocusEvent& e);

	void selectCell(wxMouseEvent &e);
	void selectNode(wxMouseEvent &e);
	void selectRoad(wxMouseEvent &e);
	

	//void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
    //void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

	virtual void createScene(void);

	WorldNode* createNode(const Ogre::Vector3& v);
	Ogre::SceneNode* createRoad(const Ogre::Vector3& u, const Ogre::Vector3& v); 
	Ogre::SceneNode* createRoad(const Ogre::Vector3& u, const Ogre::Vector3& v, const std::string& name);

	static Ogre::SceneNode* getMovable(const Ogre::RaySceneQueryResult& result, const std::string& mask);
	Ogre::RaySceneQueryResult doMouseRay(const wxMouseEvent &e);
	static Ogre::Vector3* getTerrainIntersection(const Ogre::RaySceneQueryResult& result);
	void highlightCell(const CityCell& cell, WorldDocument* doc);
	void doSomeSecondary(CityCell& cell, WorldDocument* doc);

	void growRoad(const Ogre::Vector2& direction, const Ogre::Vector2& location, int rotateInterval = 90);

	void moveNode(WorldNode* sn, const Vector3& pos);
	void selectNode(WorldNode* wn);

	void deleteNode(WorldNode* wn);

	void createCell(CityCell* cell);

    //void chooseSceneManager(void);
    //void createCamera(void);
    //void createScene(void);
    //void createViewports(void);
    //void destroyScene(void);
	bool plotPointOnTerrain(const Ogre::Vector2& pos2D, Ogre::Vector3& pos3D);
	Ogre::Vector2 convert3DPointTo2D(const Ogre::Vector3& pos3D);
	void refreshSceneNodeMap();

	bool highlightNodeFromLoc(const Vector2 &loc);
	//void highlightNode(SceneNode* sn);


public:

	
	void moveCurrentNode(wxMouseEvent &e);
	//void update();

	void setEditMode(EditModeListener::EditMode mode);
	void setSelectMode(SelectModeListener::SelectMode mode);

	void loadDoc();


};


#endif
