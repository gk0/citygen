#ifndef WORLDFRAME_H
#define WORLDFRAME_H

#include "stdafx.h"
#include "RoadGraph.h"
#include "MainWindow.h"
#include "Tool.h"

class WorldNode;
class WorldRoad;
class WorldCell;


class WorldFrame : public wxControl, public Ogre::Singleton<WorldFrame>
{
	DECLARE_CLASS(WorldFrame);

	int intType;

private:
	/* WX members */
	wxTimer	mTimer;

	/* Ogre members */
	Ogre::Camera* mCamera;
	Ogre::RenderWindow* mRenderWindow;
	Ogre::SceneManager* mSceneManager;
	Ogre::Viewport* mViewport;

	bool isDocOpen;

	Ogre::RaySceneQuery* mRaySceneQuery;

	MainWindow::EditMode mEditMode;
	MainWindow::ActiveTool mActiveTool;

	RoadGraph mRoadGraph;
	RoadGraph mSimpleRoadGraph;

	std::map< Ogre::SceneNode*, WorldNode* > mSceneNodeMap;
	std::map< Ogre::SceneNode*, WorldRoad* > mSceneRoadMap;
	std::map< Ogre::SceneNode*, WorldCell* > mSceneCellMap;

	WorldNode* mHighlightedNode;
	WorldNode* mSelectedNode;
	WorldRoad* mSelectedRoad;
	WorldCell* mSelectedCell;

	std::set<WorldCell*> mCells;
	std::vector<RoadInterface*> mFilaments;

	bool mIntersectionPresent;
	RoadId mRoadIntersection;

	std::vector<Tool*> mTools;
	//Ogre::uint32 mActiveTool;

protected:
	void OnChar(wxKeyEvent& e);
	void OnEraseBackground(wxEraseEvent &e);
	void OnFocusLost(wxFocusEvent& e);	
	void OnFocusSet(wxFocusEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);
	void OnPaint(wxPaintEvent &WXUNUSED(e));
	void OnSize(wxSizeEvent &e);
	void OnTimer(wxTimerEvent &e);


	DECLARE_EVENT_TABLE();
	 
    void createCamera(void);
    void createScene(void);
    void createViewport(void);
    void destroyScene(void);
	void destroyViewport();
	void destroyCamera();

	void endNodeMode();
	void beginNodeMode();

	void updateProperties();

public:
	WorldFrame(wxFrame* parent);
	virtual ~WorldFrame();

	Ogre::SceneManager* getSceneManager() { return mSceneManager; }

	WorldNode* createNode();
	WorldRoad* createRoad(WorldNode* wn1, WorldNode* wn2);
	void insertNodeOnRoad(WorldNode* wn, WorldRoad* wr);
	void deleteNode(WorldNode* wn);

	void deleteRoad(WorldRoad* wr);

	void cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void cameraRotate(Ogre::Real yaw, Ogre::Real pitch);
	void toggleTimerRendering();
	void update();
	bool loadXML(const TiXmlHandle& worldRoot);
	TiXmlElement* saveXML();

	void setEditMode(MainWindow::EditMode mode);
	void setActiveTool(MainWindow::ActiveTool tool);
	void onNewDoc();
	void onCloseDoc();

	bool plotPointOnTerrain(Ogre::Real x, Ogre::Real &y, Ogre::Real z);
	void selectNode(WorldNode* wn);
	void highlightNode(WorldNode* wn);
	bool highlightNodeFromLoc(const Ogre::Vector2 &loc);
	void moveSelectedNode(const Ogre::Vector3& pos);

	bool pickCell(wxMouseEvent& e, WorldCell *&wc);
	void selectCell(WorldCell* wn);

	bool pickRoad(wxMouseEvent& e, WorldRoad *&wr);
	void selectRoad(WorldRoad* wn);

	/**
	 * Get a pointer to the camera
	 * @return mCamera
	 */
	Ogre::Camera* getCamera()
	{
		return mCamera;
	}

	/**
	 * Get the value for the currently highlighted node
	 * @return 0 if no node currently highlighted 
	 */
	WorldNode* getHighlighted()
	{
		return mHighlightedNode;
	}

	/**
	 * Get the value for the currently selected node
	 * @return 0 if no node currently selected 
	 */
	WorldNode* getSelected()
	{
		return mSelectedNode;
	}

	/**
	 * Get the value for the currently selected cell
	 * @return 0 if no cell currently selected 
	 */
	WorldRoad* getSelectedRoad()
	{
		return mSelectedRoad;
	}

	/**
	 * Get the value for the currently selected cell
	 * @return 0 if no cell currently selected 
	 */
	WorldCell* getSelectedCell()
	{
		return mSelectedCell;
	}

	/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successfull
	 */
	bool pickNode(wxMouseEvent &e, Ogre::Real snapSq, WorldNode *&wn);

   /**
	 * Pick a point of intersection from the terrain in the scene 
	 * using a mouse event with coords
	 * @param e a wxMouseEvent.
	 * @param pos a Vector3 reference that can be used to store the intersection point.
	 * @return true if successfull
	 */
	bool pickTerrainIntersection(wxMouseEvent& e, Ogre::Vector3& pos);

	/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successfull
	 */
	bool pickNode(wxMouseEvent &e, WorldNode *&wn);


	void modify(bool b);

	static WorldFrame& getSingleton(void);
    static WorldFrame* getSingletonPtr(void);
};

#endif
