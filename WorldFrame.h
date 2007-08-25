#ifndef WORLDFRAME_H
#define WORLDFRAME_H

#include "stdafx.h"
#include "RoadGraph.h"
#include "MainWindow.h"
#include "WorldTerrain.h"
#include "Tool.h"

class WorldNode;
class WorldRoad;
class WorldCell;



class WorldFrame : public wxControl, public Ogre::Singleton<WorldFrame>
{
	DECLARE_CLASS(WorldFrame);

private:
	/* WX members */
	wxTimer				_timer;

	/* Ogre members */
	Ogre::Camera*		_camera;
	Ogre::RenderWindow* _renderWindow;
	Ogre::SceneManager* _sceneManager;
	Ogre::Viewport*		_viewport;

	bool				_isDocOpen;

	Ogre::RaySceneQuery* _raySceneQuery;

	MainWindow::ViewMode		_viewMode;
	MainWindow::ToolsetMode		_toolsetMode;
	MainWindow::ActiveTool		_activeTool;

	RoadGraph			_roadGraph;
	RoadGraph			_simpleRoadGraph;

	WorldTerrain		_worldTerrain;
	std::map< Ogre::SceneNode*, WorldNode* > _sceneNodeMap;
	std::map< Ogre::SceneNode*, WorldRoad* > _sceneRoadMap;
	std::map< Ogre::SceneNode*, WorldCell* > _sceneCellMap;

	WorldNode*			_highlightedNode;
	WorldNode*			_selectedNode;
	WorldRoad*			_selectedRoad;
	WorldCell*			_selectedCell;

	std::set<WorldCell*> _cellSet;

	bool				_intersectionPresent;
	RoadId				_roadIntersection;

	std::vector<Tool*>	_tools;

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

	Ogre::SceneManager* getSceneManager() { return _sceneManager; }

	WorldNode* createNode();
	WorldRoad* createRoad(WorldNode* wn1, WorldNode* wn2);
	void insertNodeOnRoad(WorldNode* wn, WorldRoad* wr);
	void deleteNode(WorldNode* wn);

	void deleteRoad(WorldRoad* wr);

	void cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void cameraRotate(Ogre::Real yaw, Ogre::Real pitch);
	void init();
	void toggleTimerRendering();
	void update();
	bool loadXML(const TiXmlHandle& worldRoot);
	TiXmlElement* saveXML();

	void setViewMode(MainWindow::ViewMode mode);
	void setToolsetMode(MainWindow::ToolsetMode mode);
	void setActiveTool(MainWindow::ActiveTool tool);
	void onNewDoc();
	void onCloseDoc();

	bool plotPointOnTerrain(Ogre::Real x, Ogre::Real &y, Ogre::Real z);
	bool plotPointOnTerrain(const Ogre::Vector2& pos2D, Ogre::Vector3& pos3D);
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
		return _camera;
	}

	/**
	 * Get the value for the currently highlighted node
	 * @return 0 if no node currently highlighted 
	 */
	WorldNode* getHighlighted()
	{
		return _highlightedNode;
	}

	/**
	 * Get the value for the currently selected node
	 * @return 0 if no node currently selected 
	 */
	WorldNode* getSelected()
	{
		return _selectedNode;
	}

	/**
	 * Get the value for the currently selected cell
	 * @return 0 if no cell currently selected 
	 */
	WorldRoad* getSelectedRoad()
	{
		return _selectedRoad;
	}

	/**
	 * Get the value for the currently selected cell
	 * @return 0 if no cell currently selected 
	 */
	WorldCell* getSelectedCell()
	{
		return _selectedCell;
	}

	/**
	 * Pick a node from the scene using a mouse event with coordinates
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successful
	 */
	bool pickNode(wxMouseEvent &e, Ogre::Real snapSq, WorldNode *&wn);

   /**
	 * Pick a point of intersection from the terrain in the scene 
	 * using a mouse event with coordinates
	 * @param e a wxMouseEvent.
	 * @param pos a Vector3 reference that can be used to store the intersection point.
	 * @return true if successful
	 */
	bool pickTerrainIntersection(wxMouseEvent& e, Ogre::Vector3& pos);

	/**
	 * Pick a node from the scene using a mouse event with coordinates
	 * @param e an wxMouseEvent.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successful
	 */
	bool pickNode(wxMouseEvent &e, WorldNode *&wn);


	void modify(bool b);

	static WorldFrame& getSingleton(void);
    static WorldFrame* getSingletonPtr(void);
};

#endif
