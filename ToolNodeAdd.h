#ifndef TOOLNODEADD_H
#define TOOLNODEADD_H

#include "stdafx.h"
#include "ToolView.h"

class WorldNode;
class WorldRoad;

class ToolNodeAdd : public ToolView
{
private:
	Ogre::SceneManager* mSceneManager;
	RoadGraph& _roadGraph;
	RoadGraph& _simpleRoadGraph;

	WorldNode *mProposedNode;
	WorldRoad *mProposedRoad;

	int mSnapState;
	WorldRoad *mIntersectingRoad;
	WorldNode *mSnapNode;


public:
	ToolNodeAdd(WorldFrame* wf, Ogre::SceneManager* sm, RoadGraph &g, RoadGraph &s);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);

	void updateState(wxMouseEvent &e);
};

#endif
