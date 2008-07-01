#ifndef TOOLNODEADD_H
#define TOOLNODEADD_H

#include "stdafx.h"
#include "ToolView.h"
#include "RoadGraph.h"

class WorldNode;
class WorldRoad;

class ToolNodeAdd : public ToolView
{
private:
	Ogre::SceneManager* _sceneManager;
	RoadGraph&		_roadGraph;
	RoadGraph&		_simpleRoadGraph;

	WorldNode*		_proposedNode;
	WorldRoad*		_proposedRoad;

	int				_snapState;
	WorldRoad*		_intersectingRoad;
	WorldNode*		_snapNode;


public:
	ToolNodeAdd(WorldFrame* wf, Ogre::SceneManager* sm, RoadGraph &g, RoadGraph &s);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnMiddlePressed(wxMouseEvent &e);
	void OnRightPressed(wxMouseEvent &e);
	bool alternate(wxMouseEvent &e);
	void updateState(wxMouseEvent &e);
};

#endif
